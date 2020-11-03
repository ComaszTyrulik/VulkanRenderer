from unittest.mock import MagicMock
import json

from src.files.file_opener import FileOpener
from src.json_utils.json_manager import JSONManager


class CommonTestData:
    def __init__(self):
        self.file_opener_mock = MagicMock(FileOpener)
        self.file_opener_mock.open = MagicMock()


def test_load_from_file_will_load_json_data_from_given_file_path():
    test_data = CommonTestData()

    expected_file_path = 'json_file/path.json'
    expected_json_string = '{"this":{"is":"json"}}'
    expected_json_object = {'this': {'is': 'json'}}

    file_mock = MagicMock(FileOpener.File)
    file_mock.get_content = MagicMock(return_value=expected_json_string)
    test_data.file_opener_mock.open.return_value = file_mock

    json.loads = MagicMock(return_value=expected_json_object)

    sut = JSONManager(test_data.file_opener_mock)
    actual_json_object = sut.load_from_file(expected_file_path)

    json.loads.assert_called_with(expected_json_string)
    assert expected_json_object == actual_json_object


def test_save_to_file_will_save_given_dictionary_as_json_string_to_given_file():
    test_data = CommonTestData()

    expected_json_indent = 2
    expected_json_string = '{"this":{"is":"json"}}'
    expected_json_object = {'this': {'is': 'json'}}
    expected_file_path = 'json_file/path.json'

    file_mock = MagicMock(FileOpener.File)
    file_mock.replace_content = MagicMock()
    test_data.file_opener_mock.open.return_value = file_mock

    json.dumps = MagicMock(return_value=expected_json_string)

    sut = JSONManager(test_data.file_opener_mock)
    sut.save_to_file(expected_json_object, expected_file_path, expected_json_indent)

    json.dumps.assert_called_with(expected_json_object, indent=expected_json_indent)
    test_data.file_opener_mock.open.assert_called_with(expected_file_path)
    file_mock.replace_content.assert_called_with(expected_json_string)
