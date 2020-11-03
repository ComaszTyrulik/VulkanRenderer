import os

from pathlib import Path

import pytest

from src.files.file_opener import FileOpener
from src.beastengine.commands.class_commands.class_files_helper import ClassFilesHelper


def test_does_header_file_exist_will_return_true_if_header_is_on_the_target_headers_list_and_file_exists(tmpdir):
    headers_base_directory = tmpdir
    class_name = 'test_class'
    target_config = {'headers': {'files': [f'{class_name}.h']}}

    # Create file
    file_path = f'{headers_base_directory}/{class_name}.h'
    open(file_path, 'x')

    sut = ClassFilesHelper(FileOpener())
    assert sut.does_class_header_file_exist(class_name, headers_base_directory, target_config) is True


def test_does_header_file_exist_will_return_false_if_header_is_on_the_target_headers_list_but_file_does_not_exist(tmpdir):
    headers_base_directory = tmpdir
    class_name = 'test_class'
    target_config = {'headers': {'files': [f'{class_name}.h']}}

    sut = ClassFilesHelper(FileOpener())
    assert sut.does_class_header_file_exist(class_name, headers_base_directory, target_config) is False


def test_does_header_file_exist_will_return_false_if_header_is_not_on_the_target_headers_list_but_file_exists(tmpdir):
    headers_base_directory = tmpdir
    class_name = 'test_class'
    target_config = {'headers': {'files': []}}

    # Create file
    file_path = f'{headers_base_directory}/{class_name}.h'
    open(file_path, 'x')

    sut = ClassFilesHelper(FileOpener())
    assert sut.does_class_header_file_exist(class_name, headers_base_directory, target_config) is False


def test_does_source_file_exist_will_return_true_if_source_is_on_the_target_sources_list_and_file_exists(tmpdir):
    sources_base_directory = tmpdir
    class_name = 'test_class'
    target_config = {'sources': {'files': [f'{class_name}.cpp']}}

    # Create file
    file_path = f'{sources_base_directory}/{class_name}.cpp'
    open(file_path, 'x')

    sut = ClassFilesHelper(FileOpener())
    assert sut.does_class_source_file_exist(class_name, sources_base_directory, target_config) is True


def test_does_source_file_exist_will_return_false_if_source_is_on_the_target_sources_list_but_file_does_not_exist(tmpdir):
    sources_base_directory = tmpdir
    class_name = 'test_class'
    target_config = {'sources': {'files': [f'{class_name}.cpp']}}

    sut = ClassFilesHelper(FileOpener())
    assert sut.does_class_source_file_exist(class_name, sources_base_directory, target_config) is False


def test_does_source_file_exist_will_return_false_if_source_is_not_on_the_target_sources_list_but_file_exists(tmpdir):
    sources_base_directory = tmpdir
    class_name = 'test_class'
    target_config = {'sources': {'files': []}}

    # Create file
    file_path = f'{sources_base_directory}/{class_name}.cpp'
    open(file_path, 'x')

    sut = ClassFilesHelper(FileOpener())
    assert sut.does_class_source_file_exist(class_name, sources_base_directory, target_config) is False


###################################
# ADDING FILES TESTS ##############
###################################
def test_create_class_header_will_create_header_file_in_proper_path_with_base_directory_and_return_valid_filename(tmpdir):
    header_class_name = 'TestClass'

    expected_filename = f'{header_class_name}.h'
    file_base_dir = tmpdir
    full_file_path = f'{file_base_dir}/{expected_filename}'

    # Make sure that the file doesn't exist before
    file_status_before = os.path.isfile(full_file_path)

    sut = ClassFilesHelper(FileOpener())
    actual_filename = sut.create_class_header(header_class_name, file_base_dir)

    file_status_after = os.path.isfile(full_file_path)

    assert file_status_before != file_status_after
    assert file_status_after is True
    assert expected_filename == actual_filename


def test_create_class_header_will_create_throw_exception_if_file_already_exist(tmpdir):
    with pytest.raises(FileExistsError):
        header_class_name = 'TestClass'

        expected_filename = f'{header_class_name}.h'
        file_base_dir = tmpdir
        full_file_path = f'{file_base_dir}/{expected_filename}'

        open(full_file_path, 'x')

        sut = ClassFilesHelper(FileOpener())
        sut.create_class_header(header_class_name, file_base_dir)


def test_create_class_header_will_create_subdirectories_with_base_directory_if_given_file_path_contains_any(tmpdir):
    subdir1 = 'subdir1'
    subdir2 = 'subdir2'
    subdir3 = 'subdir3'
    header_class_name = f'{subdir1}/{subdir2}/{subdir3}/TestClass'

    filename = f'{header_class_name}.h'
    file_base_dir = tmpdir
    full_file_path = f'{file_base_dir}/{filename}'

    # Make sure that the file doesn't exist before
    file_status_before = os.path.isfile(full_file_path)

    sut = ClassFilesHelper(FileOpener())
    sut.create_class_header(header_class_name, file_base_dir)

    file_status_after = os.path.isfile(full_file_path)

    assert file_status_before != file_status_after
    assert file_status_after is True


def test_create_class_header_will_not_create_subdirectories_with_base_directory_if_they_already_exist(tmpdir):
    subdir1 = 'subdir1'
    subdir2 = 'subdir2'
    subdir3 = 'subdir3'
    subdirectories = f'{subdir1}/{subdir2}/{subdir3}'

    header_class_name = f'{subdirectories}/TestClass'

    filename = f'{header_class_name}.h'
    file_base_dir = tmpdir
    full_file_path = f'{file_base_dir}/{filename}'

    # Create subdirectories
    subdirectories_path = f'{file_base_dir}/{subdirectories}'
    os.makedirs(subdirectories_path)

    # Make sure that the file doesn't exist before
    file_status_before = os.path.isfile(full_file_path)

    sut = ClassFilesHelper(FileOpener())
    sut.create_class_header(header_class_name, file_base_dir)

    file_status_after = os.path.isfile(full_file_path)

    assert file_status_before != file_status_after
    assert file_status_after is True


def test_create_class_header_will_not_create_subdirectories_with_base_directory_if_they_already_exist_and_contain_file(tmpdir):
    subdir1 = 'subdir1'
    subdir2 = 'subdir2'
    subdir3 = 'subdir3'
    subdirectories = f'{subdir1}/{subdir2}/{subdir3}'

    header_class_name = f'{subdirectories}/TestClass'

    filename = f'{header_class_name}.h'
    file_base_dir = tmpdir
    full_file_path = f'{file_base_dir}/{filename}'

    # Create subdirectories and additional file
    subdirectories_path = f'{file_base_dir}/{subdirectories}'
    os.makedirs(subdirectories_path)

    additional_file = f'{subdirectories_path}/AdditionalFile.h'
    open(additional_file, 'x')

    # Make sure that the file doesn't exist before
    file_status_before = os.path.isfile(full_file_path)

    sut = ClassFilesHelper(FileOpener())
    sut.create_class_header(header_class_name, file_base_dir)

    file_status_after = os.path.isfile(full_file_path)

    assert file_status_before != file_status_after
    assert file_status_after is True


def test_create_class_header_will_create_header_file_with_namespace_if_namespace_parameter_passed(tmpdir):
    expected_namespace = 'be::ns'
    expected_namespace_in_file = f'namespace {expected_namespace}'

    header_class_name = 'TestClass'

    filename = f'{header_class_name}.h'
    file_base_dir = tmpdir
    full_file_path = f'{file_base_dir}/{filename}'

    sut = ClassFilesHelper(FileOpener())
    sut.create_class_header(header_class_name, file_base_dir, expected_namespace)

    file = open(full_file_path, 'r')
    file_content = file.read()

    assert file_content.find(expected_namespace_in_file) != -1


def test_create_class_source_will_create_source_file_in_proper_path_with_base_directory_and_return_valid_filename(tmpdir):
    source_class_name = 'TestClass'

    expected_filename = f'{source_class_name}.cpp'
    file_base_dir = tmpdir
    full_file_path = f'{file_base_dir}/{expected_filename}'

    # Make sure that the file doesn't exist before
    file_status_before = os.path.isfile(full_file_path)

    sut = ClassFilesHelper(FileOpener())
    actual_filename = sut.create_class_source(source_class_name, file_base_dir)

    file_status_after = os.path.isfile(full_file_path)

    assert file_status_before != file_status_after
    assert file_status_after is True
    assert expected_filename == actual_filename


def test_create_class_source_will_throw_exception_if_file_already_exists(tmpdir):
    with pytest.raises(FileExistsError):
        source_class_name = 'TestClass'

        expected_filename = f'{source_class_name}.cpp'
        file_base_dir = tmpdir
        full_file_path = f'{file_base_dir}/{expected_filename}'
        open(full_file_path, 'x')

        sut = ClassFilesHelper(FileOpener())
        sut.create_class_source(source_class_name, file_base_dir)


def test_create_class_source_will_create_subdirectories_with_base_directory_if_given_file_path_contains_any(tmpdir):
    subdir1 = 'subdir1'
    subdir2 = 'subdir2'
    subdir3 = 'subdir3'
    source_class_name = f'{subdir1}/{subdir2}/{subdir3}/TestClass'

    filename = f'{source_class_name}.cpp'
    file_base_dir = tmpdir
    full_file_path = f'{file_base_dir}/{filename}'

    # Make sure that the file doesn't exist before
    file_status_before = os.path.isfile(full_file_path)

    sut = ClassFilesHelper(FileOpener())
    sut.create_class_source(source_class_name, file_base_dir)

    file_status_after = os.path.isfile(full_file_path)

    assert file_status_before != file_status_after
    assert file_status_after is True


def test_create_class_source_will_not_create_subdirectories_with_base_directory_if_they_already_exist(tmpdir):
    subdir1 = 'subdir1'
    subdir2 = 'subdir2'
    subdir3 = 'subdir3'
    subdirectories = f'{subdir1}/{subdir2}/{subdir3}'

    source_class_name = f'{subdirectories}/TestClass'

    filename = f'{source_class_name}.cpp'
    file_base_dir = tmpdir
    full_file_path = f'{file_base_dir}/{filename}'

    # Create subdirectories
    subdirectories_path = f'{file_base_dir}/{subdirectories}'
    os.makedirs(subdirectories_path)

    # Make sure that the file doesn't exist before
    file_status_before = os.path.isfile(full_file_path)

    sut = ClassFilesHelper(FileOpener())
    sut.create_class_source(source_class_name, file_base_dir)

    file_status_after = os.path.isfile(full_file_path)

    assert file_status_before != file_status_after
    assert file_status_after is True


def test_create_class_header_will_not_create_subdirectories_with_base_directory_if_they_already_exist_and_contain_file(tmpdir):
    subdir1 = 'subdir1'
    subdir2 = 'subdir2'
    subdir3 = 'subdir3'
    subdirectories = f'{subdir1}/{subdir2}/{subdir3}'

    source_class_name = f'{subdirectories}/TestClass'

    filename = f'{source_class_name}.cpp'
    file_base_dir = tmpdir
    full_file_path = f'{file_base_dir}/{filename}'

    # Create subdirectories and additional file
    subdirectories_path = f'{file_base_dir}/{subdirectories}'
    os.makedirs(subdirectories_path)

    additional_file = f'{subdirectories_path}/AdditionalFile.cpp'
    open(additional_file, 'x')

    # Make sure that the file doesn't exist before
    file_status_before = os.path.isfile(full_file_path)

    sut = ClassFilesHelper(FileOpener())
    sut.create_class_source(source_class_name, file_base_dir)

    file_status_after = os.path.isfile(full_file_path)

    assert file_status_before != file_status_after
    assert file_status_after is True


def test_create_class_source_will_create_source_file_with_namespace_if_namespace_parameter_passed(tmpdir):
    expected_namespace = 'be::ns'
    expected_namespace_in_file = f'namespace {expected_namespace}'

    source_class_name = 'TestClass'

    filename = f'{source_class_name}.cpp'
    file_base_dir = tmpdir
    full_file_path = f'{file_base_dir}/{filename}'

    sut = ClassFilesHelper(FileOpener())
    sut.create_class_source(source_class_name, file_base_dir, expected_namespace)

    file = open(full_file_path, 'r')
    file_content = file.read()

    assert file_content.find(expected_namespace_in_file) != -1


###################################
# REMOVING FILES TESTS ############
###################################
def test_delete_class_header_file_will_remove_given_header_file(tmpdir):
    headers_base_directory = tmpdir

    # Create file - it should be deleted by SUT
    header_filename = 'header'
    header_file_path = f'{headers_base_directory}/{header_filename}.h'
    open(header_file_path, 'x')

    sut = ClassFilesHelper(FileOpener)
    sut.remove_class_header_file(header_filename, headers_base_directory)

    assert Path(header_file_path).exists() is False


def test_delete_class_header_file_will_remove_header_file_and_its_subdirectory_when_subdirectory_is_empty(tmpdir):
    headers_base_directory = tmpdir

    # Create file's subdirectory path - it should be deleted by SUT
    header_subdirectory = 'subdirectory'
    header_subdirectory_path = f'{headers_base_directory}/{header_subdirectory}'
    os.mkdir(header_subdirectory_path)

    # Create file - it should be deleted by SUT
    header_filename = 'header'
    header_file_path = f'{header_subdirectory_path}/{header_filename}.h'
    open(header_file_path, 'x')

    header_class_name_to_delete = f'{header_subdirectory}/{header_filename}'

    sut = ClassFilesHelper(FileOpener)
    sut.remove_class_header_file(header_class_name_to_delete, headers_base_directory)

    assert Path(header_file_path).exists() is False


def test_delete_class_header_file_will_remove_header_file_and_its_subdirectories_when_subdirectories_are_empty(tmpdir):
    headers_base_directory = tmpdir

    # Create file's subdirectories
    header_subdirectories = 'subdirectory1/subdirectory2'
    header_subdirectories_path = f'{headers_base_directory}/{header_subdirectories}'
    os.makedirs(header_subdirectories_path)

    # Create header file to delete
    header_filename = 'header'
    header_file_path = f'{header_subdirectories_path}/{header_filename}.h'
    open(header_file_path, 'x')

    header_class_name_to_delete = f'{header_subdirectories}/{header_filename}'

    sut = ClassFilesHelper(FileOpener)
    sut.remove_class_header_file(header_class_name_to_delete, headers_base_directory)

    assert not os.listdir(headers_base_directory)
    assert Path(header_file_path).exists() is False


def test_delete_header_file_will_remove_header_file_and_its_subdirectories_but_only_empty_ones(tmpdir):
    headers_base_directory = tmpdir

    # Create file's subdirectories
    header_empty_subdirectory = 'empty'
    header_not_empty_subdirectory = 'not_empty_subdirectory'
    header_subdirectories = f'{header_not_empty_subdirectory}/{header_empty_subdirectory}'
    header_subdirectories_path = f'{headers_base_directory}/{header_subdirectories}'
    os.makedirs(header_subdirectories_path)

    # Create additional file to reside in not_empty_subdir
    additional_file_path = f'{headers_base_directory}/{header_not_empty_subdirectory}/additional_file.h'
    open(additional_file_path, 'x')

    # Create header file to delete
    header_filename = 'header'
    header_file_path = f'{header_subdirectories_path}/{header_filename}.h'
    open(header_file_path, 'x')

    header_class_name_to_delete = f'{header_subdirectories}/{header_filename}'

    sut = ClassFilesHelper(FileOpener)
    sut.remove_class_header_file(header_class_name_to_delete, headers_base_directory)

    assert Path(additional_file_path).exists() is True
    assert Path(header_file_path).exists() is False


def test_delete_class_source_file_will_remove_given_source_file(tmpdir):
    sources_base_directory = tmpdir

    # Create file - it should be deleted by SUT
    source_filename = 'source'
    source_file_path = f'{sources_base_directory}/{source_filename}.cpp'
    open(source_file_path, 'x')

    sut = ClassFilesHelper(FileOpener)
    sut.remove_class_source_file(source_filename, sources_base_directory)

    assert Path(source_file_path).exists() is False


def test_delete_class_source_file_will_remove_source_file_and_its_subdirectory_when_subdirectory_is_empty(tmpdir):
    sources_base_directory = tmpdir

    # Create file's subdirectory path - it should be deleted by SUT
    source_subdirectory = 'subdirectory'
    source_subdirectory_path = f'{sources_base_directory}/{source_subdirectory}'
    os.mkdir(source_subdirectory_path)

    # Create file - it should be deleted by SUT
    source_filename = 'source'
    source_file_path = f'{source_subdirectory_path}/{source_filename}.cpp'
    open(source_file_path, 'x')

    source_class_name_to_delete = f'{source_subdirectory}/{source_filename}'

    sut = ClassFilesHelper(FileOpener)
    sut.remove_class_source_file(source_class_name_to_delete, sources_base_directory)

    assert Path(source_file_path).exists() is False


def test_delete_class_source_file_will_remove_source_file_and_its_subdirectories_when_subdirectories_are_empty(tmpdir):
    sources_base_directory = tmpdir

    # Create file's subdirectories
    sources_subdirectories = 'subdirectory1/subdirectory2'
    sources_subdirectories_path = f'{sources_base_directory}/{sources_subdirectories}'
    os.makedirs(sources_subdirectories_path)

    # Create source file to delete
    source_filename = 'source'
    source_file_path = f'{sources_subdirectories_path}/{source_filename}.cpp'
    open(source_file_path, 'x')

    source_class_name_to_delete = f'{sources_subdirectories}/{source_filename}'

    sut = ClassFilesHelper(FileOpener)
    sut.remove_class_source_file(source_class_name_to_delete, sources_base_directory)

    assert not os.listdir(sources_base_directory)
    assert Path(source_file_path).exists() is False


def test_delete_source_file_will_remove_source_file_and_its_subdirectories_but_only_empty_ones(tmpdir):
    sources_base_directory = tmpdir

    # Create file's subdirectories
    source_empty_subdirectory = 'empty'
    source_not_empty_subdirectory = 'not_empty_subdirectory'
    source_subdirectories = f'{source_not_empty_subdirectory}/{source_empty_subdirectory}'
    source_subdirectories_path = f'{sources_base_directory}/{source_subdirectories}'
    os.makedirs(source_subdirectories_path)

    # Create additional file to reside in not_empty_subdir
    additional_file_path = f'{sources_base_directory}/{source_not_empty_subdirectory}/additional_file.cpp'
    open(additional_file_path, 'x')

    # Create source file to delete
    source_filename = 'source'
    source_file_path = f'{source_subdirectories_path}/{source_filename}.cpp'
    open(source_file_path, 'x')

    source_class_name_to_delete = f'{source_subdirectories}/{source_filename}'

    sut = ClassFilesHelper(FileOpener)
    sut.remove_class_source_file(source_class_name_to_delete, sources_base_directory)

    assert Path(additional_file_path).exists() is True
    assert Path(source_file_path).exists() is False
