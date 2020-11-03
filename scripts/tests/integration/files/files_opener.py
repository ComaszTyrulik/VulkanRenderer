import os
from src.files.file_opener import FileOpener


def test_open_will_return_given_file(tmpdir):
    file_name = 'filename.txt'
    file_path = f'{tmpdir}/{file_name}'
    expected_file_content = 'This is some test content of the file.\n Really cool!'

    file = open(file_path, 'w+')
    file.write(expected_file_content)

    sut = FileOpener()
    file = sut.open(file_path)
    actual_file_content = file.get_content()

    assert expected_file_content == actual_file_content


def test_create_will_create_given_file(tmpdir):
    file_name = 'filename.txt'
    file_path = f'{tmpdir}/{file_name}'

    file_status_before = os.path.exists(file_path)

    sut = FileOpener()
    sut.create(file_path)

    file_status_after = os.path.exists(file_path)

    assert file_status_before != file_status_after
    assert file_status_after is True
