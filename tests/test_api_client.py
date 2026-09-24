"""Unit tests for app.api_client module."""

from unittest.mock import AsyncMock, patch
import unittest

from app.api_client import OpenDataAPIClient, OpenDataAPIError


class DummyResponse:
    def __init__(self, status, json_data, text=""):
        self.status = status
        self._json_data = json_data
        self._text = text

    async def json(self):
        return self._json_data

    async def text(self):
        return self._text

    async def __aenter__(self):
        return self

    async def __aexit__(self, exc_type, exc_val, exc_tb):
        pass


class TestApiClient(unittest.IsolatedAsyncioTestCase):
    async def test_get_temporary_download_url_success(self):
        client = OpenDataAPIClient(api_key="dummy_key", base_url="https://mock.api")
        mock_data = {
            "temporaryDownloadUrl": "https://s3.amazonaws.com/bucket/file.tar?token=abc"
        }

        with patch("aiohttp.ClientSession.get", return_value=DummyResponse(200, mock_data)):
            url = await client.get_temporary_download_url(
                dataset_name="test_ds",
                dataset_version="1.0",
                filename="test.tar",
            )
            self.assertEqual(url, "https://s3.amazonaws.com/bucket/file.tar?token=abc")
        await client.close()

    async def test_get_temporary_download_url_error(self):
        client = OpenDataAPIClient(api_key="dummy_key", base_url="https://mock.api")
        with patch("aiohttp.ClientSession.get", return_value=DummyResponse(404, {}, text="Not Found")):
            with self.assertRaises(OpenDataAPIError):
                await client.get_temporary_download_url(
                    dataset_name="test_ds",
                    dataset_version="1.0",
                    filename="missing.tar",
                )
        await client.close()

    async def test_get_latest_filename(self):
        client = OpenDataAPIClient(api_key="dummy_key", base_url="https://mock.api")
        mock_data = {
            "files": [
                {"filename": "HARM46_V1_P1_2026092412.tar", "size": 123456}
            ]
        }
        with patch("aiohttp.ClientSession.get", return_value=DummyResponse(200, mock_data)):
            fname = await client.get_latest_filename("test_ds", "1.0")
            self.assertEqual(fname, "HARM46_V1_P1_2026092412.tar")
        await client.close()


if __name__ == "__main__":
    unittest.main()
