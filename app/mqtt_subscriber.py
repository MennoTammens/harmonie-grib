"""MQTT Notification Service subscriber for KNMI Data Platform."""

import asyncio
import json
import logging
import ssl
from typing import Any, Callable, Dict, Optional

import paho.mqtt.client as mqtt_client
import paho.mqtt.properties as properties

from app.config import (
    KNMI_NOTIFICATION_SERVICE_API_KEY,
    MQTT_BROKER,
    MQTT_CLIENT_ID,
    MQTT_PORT,
    MQTT_TOPIC,
    MQTT_TRANSPORT,
)

logger = logging.getLogger(__name__)


class NotificationSubscriber:
    """Subscribes to KNMI MQTT notification service and queues incoming file events."""

    def __init__(
        self,
        event_callback: Callable[[Dict[str, Any]], None],
        broker: str = MQTT_BROKER,
        port: int = MQTT_PORT,
        topic: str = MQTT_TOPIC,
        token: str = KNMI_NOTIFICATION_SERVICE_API_KEY,
        client_id: str = MQTT_CLIENT_ID,
        clean_start: bool = False,
    ):
        self.event_callback = event_callback
        self.broker = broker
        self.port = port
        self.topic = topic
        self.token = token
        self.client_id = client_id
        self.clean_start = clean_start

        self.client: Optional[mqtt_client.Client] = None
        self._connected = False

    def is_connected(self) -> bool:
        return self._connected

    def start(self):
        """Initialize MQTT client and start background network loop."""
        if not self.token:
            raise ValueError(
                "KNMI_NOTIFICATION_SERVICE_API_KEY is not set. Cannot connect to MQTT broker."
            )

        self.client = mqtt_client.Client(
            mqtt_client.CallbackAPIVersion.VERSION2,
            client_id=self.client_id,
            protocol=mqtt_client.MQTTv5,
            transport=MQTT_TRANSPORT,
        )
        self.client.tls_set(tls_version=ssl.PROTOCOL_TLS)

        connect_properties = properties.Properties(properties.PacketTypes.CONNECT)
        connect_properties.SessionExpiryInterval = 3600 * 24  # 24 hours

        self.client.username_pw_set("token", self.token)
        self.client.on_connect = self._on_connect
        self.client.on_subscribe = self._on_subscribe
        self.client.on_message = self._on_message
        self.client.on_disconnect = self._on_disconnect

        logger.info(
            "Connecting to MQTT broker %s:%d (client_id=%s, clean_start=%s)",
            self.broker,
            self.port,
            self.client_id,
            self.clean_start,
        )
        self.client.connect(
            host=self.broker,
            port=self.port,
            keepalive=60,
            clean_start=self.clean_start,
            properties=connect_properties,
        )
        self.client.loop_start()

    def stop(self):
        """Stop MQTT client and network loop."""
        if self.client:
            logger.info("Disconnecting MQTT client...")
            self.client.loop_stop()
            self.client.disconnect()
            self._connected = False

    def _on_connect(self, client, userdata, flags, reason_code, props=None):
        logger.info(
            "Connected to MQTT broker: reason_code=%s, session_present=%s",
            reason_code,
            flags.session_present,
        )
        self._connected = True
        logger.info("Subscribing to topic: %s (QoS 1)", self.topic)
        client.subscribe(self.topic, qos=1)

    def _on_subscribe(self, client, userdata, mid, reason_codes, properties):
        logger.info("Subscribed successfully (mid=%s, reason_codes=%s)", mid, reason_codes)

    def _on_disconnect(self, client, userdata, flags, reason_code, properties=None):
        self._connected = False
        logger.warning("Disconnected from MQTT broker: reason_code=%s", reason_code)

    def _on_message(self, client, userdata, message):
        """Handle incoming MQTT message.

        MUST be fast to avoid interfering with PUBACK messages.
        Dispatches payload directly to event_callback.
        """
        payload_str = message.payload.decode("utf-8", errors="replace")
        logger.info("Received notification on topic: %s", message.topic)
        try:
            event = json.loads(payload_str)
        except Exception as e:
            logger.error("Failed to parse incoming notification payload: %s; error: %s", payload_str, e)
            return

        try:
            self.event_callback(event)
        except Exception as e:
            logger.exception("Error executing event callback: %s", e)
