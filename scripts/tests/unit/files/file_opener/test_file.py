import builtins
import io

from mock import MagicMock, mock_open

from src.files.file_opener import FileOpener


def test_constructor_will_open_given_file_in_read_append_mode():
    expected_file_path = 'path/to/file.foo'
    expected_mode = 'r+'

    open_mock = mock_open()
    builtins.open = open_mock

    FileOpener.File(expected_file_path)
    open_mock.assert_called_with(expected_file_path, expected_mode)


def test_get_content_will_return_file_content():
    expected_content = 'this is file content'
    file_path = 'file/path'

    open_mock = mock_open(read_data=expected_content)
    builtins.open = open_mock

    sut = FileOpener.File(file_path)
    assert sut.get_content() == expected_content


def test_get_content_will_return_file_content2():
    file_path = 'file/path'
    expected_truncate_value = 0
    expected_seek_value = 0
    expected_content = 'this is file content'

    file_mock = MagicMock(io.StringIO)
    file_mock.truncate = MagicMock()
    file_mock.seek = MagicMock()
    file_mock.write = MagicMock()

    open_mock = mock_open()
    open_mock.return_value = file_mock
    builtins.open = open_mock

    f = FileOpener.File(file_path)
    f.replace_content(expected_content)

    file_mock.truncate.assert_called_with(expected_truncate_value)
    file_mock.seek.assert_called_with(expected_seek_value)
    file_mock.write.assert_called_with(expected_content)
