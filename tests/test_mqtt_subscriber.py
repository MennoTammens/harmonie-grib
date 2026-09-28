"""Unit tests for MQTT subscriber and IndexerService event handling."""

import asyncio
import threading
import unittest
from unittest.mock import MagicMock, patch

from app.main import IndexerService
from app.mqtt_subscriber import NotificationSubscriber


class TestMqttSubscriber(unittest.TestCase):
    def test_on_message_valid_json(self):
        callback = MagicMock()
        sub = NotificationSubscriber(event_callback=callback)

        msg = MagicMock()
        msg.topic = "test/topic"
        msg.payload = b'{"hello": "world"}'

        sub._on_message(None, None, msg)
        callback.assert_called_once_with({"hello": "world"})

    def test_on_message_invalid_json(self):
        callback = MagicMock()
        sub = NotificationSubscriber(event_callback=callback)

        msg = MagicMock()
        msg.topic = "test/topic"
        msg.payload = b"not-json"

        sub._on_message(None, None, msg)
        callback.assert_not_called()

    def test_on_message_callback_exception_handled(self):
        callback = MagicMock(side_effect=RuntimeError("callback failure"))
        sub = NotificationSubscriber(event_callback=callback)

        msg = MagicMock()
        msg.topic = "test/topic"
        msg.payload = b'{"status": "ok"}'

        # Should not raise exception
        sub._on_message(None, None, msg)
        callback.assert_called_once_with({"status": "ok"})


class TestIndexerServiceMqttMessage(unittest.IsolatedAsyncioTestCase):
    async def test_on_mqtt_message_from_different_thread(self):
        """Simulate MQTT message callback arriving on a secondary thread."""
        service = IndexerService()
        service._loop = asyncio.get_running_loop()

        sample_event = {
            "specversion": "1.0",
            "type": "nl.knmi.dataplatform.file.created.v1",
            "data": {
                "datasetName": "harmonie_arome_cy46_p1",
                "datasetVersion": "1.0",
                "filename": "HARM46_V1_P1_2026092813.tar",
            },
        }

        def bg_thread_target():
            # In this thread, there is no event loop
            service.on_mqtt_message(sample_event)

        thread = threading.Thread(
            target=bg_thread_target,
            name="paho-mqtt-client-euros-webcie-harmonie-grib",
        )
        thread.start()
        thread.join()

        queued_event = await asyncio.wait_for(service.queue.get(), timeout=1.0)
        self.assertEqual(queued_event, sample_event)
