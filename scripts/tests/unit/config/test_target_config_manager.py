import pytest
from mock import MagicMock

from src.config.target_config_manager import TargetConfigManager
from src.beastengine.commands.class_commands.target_cmake_vars_file_opener import TargetCMakeVarsFileOpener


def test_get_headers_base_directory_will_return_empty_string_when_no_base_directory_defined():
    expected_base_directory_path = ''

    cmake_vars_file_opener_mock = MagicMock(TargetCMakeVarsFileOpener)
    cmake_vars_file_opener_mock.open = MagicMock(return_value={})

    target_config = {'headers': {'base_dir': expected_base_directory_path}}
    cmake_config = {'targets': {'lib': target_config}}

    sut = TargetConfigManager(cmake_vars_file_opener_mock)
    actual_base_directory_path = sut.get_headers_base_directory(target_config, cmake_config)

    assert actual_base_directory_path == expected_base_directory_path


def test_get_headers_base_directory_will_return_full_headers_base_directory_path_based_on_base_directory():
    expected_base_directory_path = 'base/dir/path'

    cmake_vars_file_opener_mock = MagicMock(TargetCMakeVarsFileOpener)
    cmake_vars_file_opener_mock.open = MagicMock(return_value={})

    target_config = {'headers': {'base_dir': expected_base_directory_path}}
    cmake_config = {'targets': {'lib': target_config}}

    sut = TargetConfigManager(cmake_vars_file_opener_mock)
    actual_base_directory_path = sut.get_headers_base_directory(target_config, cmake_config)

    assert actual_base_directory_path == expected_base_directory_path


def test_get_headers_base_directory_will_return_full_path_when_base_dir_contains_cmake_variable():
    base_dir_name = 'base_dir'

    cmake_variable_name = '${BEAST_INCLUDE_DIR}'
    cmake_variable_value = 'variable_value'
    cmake_variables_map = {cmake_variable_name: cmake_variable_value}

    cmake_vars_file_opener_mock = MagicMock(TargetCMakeVarsFileOpener)
    cmake_vars_file_opener_mock.open = MagicMock(return_value=cmake_variables_map)

    expected_base_directory_path = f'{cmake_variable_value}/{base_dir_name}'
    target_config = {'headers': {'base_dir': expected_base_directory_path}}
    cmake_config = {'targets': {'lib': target_config}}

    sut = TargetConfigManager(cmake_vars_file_opener_mock)
    actual_base_directory_path = sut.get_headers_base_directory(target_config, cmake_config)

    assert actual_base_directory_path == expected_base_directory_path


def test_get_headers_base_directory_will_return_full_path_when_base_dir_contains_multiple_cmake_variables():
    base_dir_name = 'base_dir'

    cmake_variable1_name = '${BEAST_INCLUDE_DIR}'
    cmake_variable1_value = 'include_dir'
    cmake_variable2_name = '${BEAST_SOME_VARIABLE}'
    cmake_variable2_value = 'some_variable'

    cmake_variables_map = {cmake_variable1_name: cmake_variable1_value, cmake_variable2_name: cmake_variable2_value}

    cmake_vars_file_opener_mock = MagicMock(TargetCMakeVarsFileOpener)
    cmake_vars_file_opener_mock.open = MagicMock(return_value=cmake_variables_map)

    expected_base_directory_path = f'{cmake_variable1_value}/{cmake_variable2_value}/{base_dir_name}'
    target_config = {'headers': {'base_dir': expected_base_directory_path}}
    cmake_config = {'targets': {'lib': target_config}}

    sut = TargetConfigManager(cmake_vars_file_opener_mock)
    actual_base_directory_path = sut.get_headers_base_directory(target_config, cmake_config)

    assert actual_base_directory_path == expected_base_directory_path


def test_get_headers_base_directory_will_throw_exception_when_cmake_variable_from_base_dir_does_not_exist_in_vars_file():
    with pytest.raises(ValueError):
        base_dir_name = 'base_dir'

        cmake_variable1_name = '${BEAST_INCLUDE_DIR}'
        cmake_variable1_value = 'include_dir'
        cmake_variable2_name = '${BEAST_SOME_VARIABLE}'
        cmake_variable2_value = 'some_variable'
        non_existent_cmake_variable_name = '${THIS_VARIABLE_DOES_NOT_EXIST}'

        cmake_variables_map = {cmake_variable1_name: cmake_variable1_value, cmake_variable2_name: cmake_variable2_value}

        cmake_vars_file_opener_mock = MagicMock(TargetCMakeVarsFileOpener)
        cmake_vars_file_opener_mock.open = MagicMock(return_value=cmake_variables_map)

        base_dir_path = f'{cmake_variable1_name}/{cmake_variable2_name}/{non_existent_cmake_variable_name}{base_dir_name}'
        target_config = {'name': 'beastengine', 'headers': {'base_dir': base_dir_path}}
        cmake_config = {'targets': {'lib': target_config}}

        sut = TargetConfigManager(cmake_vars_file_opener_mock)
        sut.get_headers_base_directory(target_config, cmake_config)


def test_get_sources_base_directory_will_return_empty_string_when_no_base_directory_defined():
    expected_base_directory_path = ''

    cmake_vars_file_opener_mock = MagicMock(TargetCMakeVarsFileOpener)
    cmake_vars_file_opener_mock.open = MagicMock(return_value={})

    target_config = {'sources': {'base_dir': expected_base_directory_path}}
    cmake_config = {'targets': {'lib': target_config}}

    sut = TargetConfigManager(cmake_vars_file_opener_mock)
    actual_base_directory_path = sut.get_sources_base_directory(target_config, cmake_config)

    assert actual_base_directory_path == expected_base_directory_path


def test_get_sources_base_directory_will_return_full_sources_base_directory_path_based_on_base_directory():
    expected_base_directory_path = 'base/dir/path'

    cmake_vars_file_opener_mock = MagicMock(TargetCMakeVarsFileOpener)
    cmake_vars_file_opener_mock.open = MagicMock(return_value={})

    target_config = {'sources': {'base_dir': expected_base_directory_path}}
    cmake_config = {'targets': {'lib': target_config}}

    sut = TargetConfigManager(cmake_vars_file_opener_mock)
    actual_base_directory_path = sut.get_sources_base_directory(target_config, cmake_config)

    assert actual_base_directory_path == expected_base_directory_path


def test_get_sources_base_directory_will_return_full_path_when_base_dir_contains_cmake_variable():
    base_dir_name = 'base_dir'

    cmake_variable_name = '${BEAST_SRC_DIR}'
    cmake_variable_value = 'variable_value'
    cmake_variables_map = {cmake_variable_name: cmake_variable_value}

    cmake_vars_file_opener_mock = MagicMock(TargetCMakeVarsFileOpener)
    cmake_vars_file_opener_mock.open = MagicMock(return_value=cmake_variables_map)

    expected_base_directory_path = f'{cmake_variable_value}/{base_dir_name}'
    target_config = {'sources': {'base_dir': expected_base_directory_path}}
    cmake_config = {'targets': {'lib': target_config}}

    sut = TargetConfigManager(cmake_vars_file_opener_mock)
    actual_base_directory_path = sut.get_sources_base_directory(target_config, cmake_config)

    assert actual_base_directory_path == expected_base_directory_path


def test_get_sources_base_directory_will_return_full_path_when_base_dir_contains_multiple_cmake_variables():
    base_dir_name = 'base_dir'

    cmake_variable1_name = '${BEAST_SRC_DIR}'
    cmake_variable1_value = 'include_dir'
    cmake_variable2_name = '${BEAST_SOME_VARIABLE}'
    cmake_variable2_value = 'some_variable'

    cmake_variables_map = {cmake_variable1_name: cmake_variable1_value, cmake_variable2_name: cmake_variable2_value}

    cmake_vars_file_opener_mock = MagicMock(TargetCMakeVarsFileOpener)
    cmake_vars_file_opener_mock.open = MagicMock(return_value=cmake_variables_map)

    expected_base_directory_path = f'{cmake_variable1_value}/{cmake_variable2_value}/{base_dir_name}'
    target_config = {'sources': {'base_dir': expected_base_directory_path}}
    cmake_config = {'targets': {'lib': target_config}}

    sut = TargetConfigManager(cmake_vars_file_opener_mock)
    actual_base_directory_path = sut.get_sources_base_directory(target_config, cmake_config)

    assert actual_base_directory_path == expected_base_directory_path


def test_get_sources_base_directory_will_throw_exception_when_cmake_variable_from_base_dir_does_not_exist_in_vars_file():
    with pytest.raises(ValueError):
        base_dir_name = 'base_dir'

        cmake_variable1_name = '${BEAST_INCLUDE_DIR}'
        cmake_variable1_value = 'include_dir'
        cmake_variable2_name = '${BEAST_SOME_VARIABLE}'
        cmake_variable2_value = 'some_variable'
        non_existent_cmake_variable_name = '${THIS_VARIABLE_DOES_NOT_EXIST}'

        cmake_variables_map = {cmake_variable1_name: cmake_variable1_value, cmake_variable2_name: cmake_variable2_value}

        cmake_vars_file_opener_mock = MagicMock(TargetCMakeVarsFileOpener)
        cmake_vars_file_opener_mock.open = MagicMock(return_value=cmake_variables_map)

        base_dir_path = f'{cmake_variable1_name}/{cmake_variable2_name}/{non_existent_cmake_variable_name}{base_dir_name}'
        target_config = {'name': 'beastengine', 'sources': {'base_dir': base_dir_path}}
        cmake_config = {'targets': {'lib': target_config}}

        sut = TargetConfigManager(cmake_vars_file_opener_mock)
        sut.get_sources_base_directory(target_config, cmake_config)


def test_add_file_to_headers_list_file_will_add_given_header_file_to_the_list_of_target_headers():
    expected_header_name = 'Header.h'
    expected_target_headers = [expected_header_name]

    target_config = {'headers': {'files': []}}

    cmake_vars_file_opener_mock = MagicMock(TargetCMakeVarsFileOpener)
    sut = TargetConfigManager(cmake_vars_file_opener_mock)
    sut.add_file_to_headers_list(expected_header_name, target_config)

    assert expected_target_headers == target_config['headers']['files']


def test_add_file_to_headers_list_will_add_given_header_file_to_the_list_of_target_headers_without_affecting_already_defined_files():
    header_before1 = 'header_before1.h'
    header_before2 = 'header_before2.h'

    expected_header_name = 'Header.h'
    expected_target_headers = [header_before1, header_before2, expected_header_name]

    target_config = {'headers': {'files': [header_before1, header_before2]}}

    cmake_vars_file_opener_mock = MagicMock(TargetCMakeVarsFileOpener)
    sut = TargetConfigManager(cmake_vars_file_opener_mock)
    sut.add_file_to_headers_list(expected_header_name, target_config)

    assert expected_target_headers == target_config['headers']['files']


def test_add_file_to_sources_list_will_add_given_source_file_to_the_list_of_target_sources():
    expected_source_name = 'Source.cpp'
    expected_target_sources = [expected_source_name]

    target_config = {'sources': {'files': []}}

    cmake_vars_file_opener_mock = MagicMock(TargetCMakeVarsFileOpener)
    sut = TargetConfigManager(cmake_vars_file_opener_mock)
    sut.add_file_to_sources_list(expected_source_name, target_config)

    assert expected_target_sources == target_config['sources']['files']


def test_add_file_to_sources_list_will_add_given_source_file_to_the_list_of_target_sources_without_affecting_already_defined_files():
    source_before1 = 'source_before1.cpp'
    source_before2 = 'source_before2.cpp'

    expected_source_name = 'Source.cpp'
    expected_target_sources = [source_before1, source_before2, expected_source_name]

    target_config = {'sources': {'files': [source_before1, source_before2]}}

    cmake_vars_file_opener_mock = MagicMock(TargetCMakeVarsFileOpener)
    sut = TargetConfigManager(cmake_vars_file_opener_mock)
    sut.add_file_to_sources_list(expected_source_name, target_config)

    assert expected_target_sources == target_config['sources']['files']


def test_remove_file_from_headers_list_will_remove_given_header_file_from_the_list_of_target_headers():
    expected_header_name = 'Header.h'
    expected_target_headers = []

    target_config = {'headers': {'files': [expected_header_name]}}

    cmake_vars_file_opener_mock = MagicMock(TargetCMakeVarsFileOpener)
    sut = TargetConfigManager(cmake_vars_file_opener_mock)
    sut.remove_file_from_headers_list(expected_header_name, target_config)

    assert expected_target_headers == target_config['headers']['files']


def test_remove_file_from_headers_list_will_remove_given_header_file_from_the_list_of_target_headers_without_affecting_already_defined_files():
    header_before1 = 'header_before1.h'
    header_before2 = 'header_before2.h'

    expected_header_name = 'Header.h'
    expected_target_headers = [header_before1, header_before2]

    target_config = {'headers': {'files': [header_before1, header_before2, expected_header_name]}}

    cmake_vars_file_opener_mock = MagicMock(TargetCMakeVarsFileOpener)
    sut = TargetConfigManager(cmake_vars_file_opener_mock)
    sut.remove_file_from_headers_list(expected_header_name, target_config)

    assert expected_target_headers == target_config['headers']['files']


def test_remove_file_from_sources_list_will_remove_given_source_file_from_the_list_of_target_sources():
    expected_source_name = 'Source.cpp'
    expected_target_sources = []

    target_config = {'sources': {'files': [expected_source_name]}}

    cmake_vars_file_opener_mock = MagicMock(TargetCMakeVarsFileOpener)
    sut = TargetConfigManager(cmake_vars_file_opener_mock)
    sut.remove_file_from_sources_list(expected_source_name, target_config)

    assert expected_target_sources == target_config['sources']['files']


def test_remove_file_from_sources_list_will_remove_given_source_file_from_the_list_of_target_sources_without_affecting_already_defined_files():
    source_before1 = 'source_before1.cpp'
    source_before2 = 'source_before2.cpp'

    expected_source_name = 'Source.cpp'
    expected_target_sources = [source_before1, source_before2]

    target_config = {'sources': {'files': [source_before1, source_before2, expected_source_name]}}

    cmake_vars_file_opener_mock = MagicMock(TargetCMakeVarsFileOpener)
    sut = TargetConfigManager(cmake_vars_file_opener_mock)
    sut.remove_file_from_sources_list(expected_source_name, target_config)

    assert expected_target_sources == target_config['sources']['files']
