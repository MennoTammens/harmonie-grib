"""KNMI Open Data API client."""

import asyncio
import logging
from typing import Any, Dict, Optional

import aiohttp

from app.config import (
    DATASET_NAME,
    DATASET_VERSION,
    HTTP_TIMEOUT,
    KNMI_OPEN_DATA_API_KEY,
    OPEN_DATA_BASE_URL,
)

logger = logging.getLogger(__name__)


class OpenDataAPIError(Exception):
    """Exception raised when an Open Data API request fails."""


class OpenDataAPIClient:
    """Client for KNMI Open Data API."""

    def __init__(
        self,
        api_key: Optional[str] = None,
        base_url: Optional[str] = None,
    ):
        self.api_key = api_key or KNMI_OPEN_DATA_API_KEY
        self.base_url = (base_url or OPEN_DATA_BASE_URL).rstrip("/")
        self._session: Optional[aiohttp.ClientSession] = None

    async def get_session(self) -> aiohttp.ClientSession:
        if self._session is None or self._session.closed:
            headers = {"Authorization": self.api_key}
            timeout = aiohttp.ClientTimeout(total=HTTP_TIMEOUT)
            self._session = aiohttp.ClientSession(headers=headers, timeout=timeout)
        return self._session

    async def close(self):
        if self._session and not self._session.closed:
            await self._session.close()

    async def list_files(
        self,
        dataset_name: str = DATASET_NAME,
        dataset_version: str = DATASET_VERSION,
        params: Optional[Dict[str, Any]] = None,
    ) -> Dict[str, Any]:
        """List files in a dataset."""
        session = await self.get_session()
        url = f"{self.base_url}/datasets/{dataset_name}/versions/{dataset_version}/files"
        async with session.get(url, params=params) as resp:
            if resp.status != 200:
                text = await resp.text()
                raise OpenDataAPIError(
                    f"list_files failed with status {resp.status}: {text}"
                )
            return await resp.json()

    async def get_latest_filename(
        self,
        dataset_name: str = DATASET_NAME,
        dataset_version: str = DATASET_VERSION,
    ) -> Optional[str]:
        """Return filename of the latest file published in dataset."""
        params = {"maxKeys": 1, "sorting": "desc", "orderBy": "created"}
        data = await self.list_files(dataset_name, dataset_version, params=params)
        files = data.get("files", [])
        if not files:
            return None
        return files[0].get("filename")

    async def get_temporary_download_url(
        self,
        dataset_name: str = DATASET_NAME,
        dataset_version: str = DATASET_VERSION,
        filename: str = "",
    ) -> str:
        """Get the temporary pre-signed download URL for a dataset file."""
        if not filename:
            raise ValueError("filename must not be empty")

        session = await self.get_session()
        url = f"{self.base_url}/datasets/{dataset_name}/versions/{dataset_version}/files/{filename}/url"
        async with session.get(url) as resp:
            if resp.status != 200:
                text = await resp.text()
                raise OpenDataAPIError(
                    f"get_temporary_download_url failed for {filename} with status {resp.status}: {text}"
                )
            data = await resp.json()
            download_url = data.get("temporaryDownloadUrl")
            if not download_url:
                raise OpenDataAPIError(
                    f"Response missing temporaryDownloadUrl for {filename}: {data}"
                )
            return download_url
