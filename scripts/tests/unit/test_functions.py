import argparse
import src.functions as functions

from mock import MagicMock
from tests.tests_utilities.micro_mock import MicroMock


def test_get_project_path():
    expected_path = 'path/to/project'

    traceback_mock = MicroMock(filename='filename')
    functions.getframeinfo = MagicMock(return_value=traceback_mock)

    path_mock = MicroMock(parent=MicroMock(parent=MicroMock(parent=expected_path)))
    functions.Path.resolve = MagicMock(return_value=path_mock)

    assert functions.get_project_path() == expected_path


def test_get_build_dir_name_will_return_valid_build_dir_name():
    expected_dir_name = 'build'
    assert functions.get_build_dir_name() == expected_dir_name


def test_get_build_dir_path_will_return_valid_path():
    build_dir_name = 'build'
    project_path = 'project/path'
    functions.get_project_path = MagicMock(return_value=project_path)

    expected_build_dir_path = f'{project_path}/{build_dir_name}'
    assert functions.get_build_dir_path() == expected_build_dir_path


def test_get_config_path_will_return_valid_path():
    config_path = 'config/config.json'
    project_path = 'project/path'
    functions.get_project_path = MagicMock(return_value=project_path)

    expected_config_path = f'{project_path}/{config_path}'
    assert functions.get_config_path() == expected_config_path


def test_create_arguments_parser_will_create_empty_parser_when_no_data_passed():
    expected_program = None
    expected_usage = None
    expected_description = None

    argument_parser_mock = MagicMock(argparse.ArgumentParser)
    functions.ArgumentParser = argument_parser_mock

    functions.create_arguments_parser()
    argument_parser_mock.assert_called_with(
        prog=expected_program,
        usage=expected_usage,
        description=expected_description,
    )


def test_create_arguments_parser_will_create_parser_with_given_program():
    expected_program = 'Program'

    argument_parser_mock = MagicMock(argparse.ArgumentParser)
    functions.ArgumentParser = argument_parser_mock

    functions.create_arguments_parser(program=expected_program)
    argument_parser_mock.assert_called_with(
        prog=expected_program,
        usage=None,
        description=None,
    )


def test_create_arguments_parser_will_create_parser_with_given_usage():
    expected_usage = 'Program Usage'

    argument_parser_mock = MagicMock(argparse.ArgumentParser)
    functions.ArgumentParser = argument_parser_mock

    functions.create_arguments_parser(usage=expected_usage)
    argument_parser_mock.assert_called_with(
        prog=None,
        usage=expected_usage,
        description=None,
    )


def test_create_arguments_parser_will_create_parser_with_given_description():
    expected_description = 'Program Description'

    argument_parser_mock = MagicMock(argparse.ArgumentParser)
    functions.ArgumentParser = argument_parser_mock

    functions.create_arguments_parser(description=expected_description)
    argument_parser_mock.assert_called_with(
        prog=None,
        usage=None,
        description=expected_description,
    )


def test_create_arguments_parser_will_create_parser_with_given_formatter_class():
    expected_formatter = MagicMock()

    argument_parser_mock = MagicMock(argparse.ArgumentParser)
    functions.ArgumentParser = argument_parser_mock

    functions.create_arguments_parser(formatter_class=expected_formatter)
    argument_parser_mock.assert_called_with(
        prog=None,
        usage=None,
        description=None,
        formatter_class=expected_formatter
    )


def test_get_target_cmake_variables_file_path_will_return_path_created_from_project_dir_cmake_dir_name_and_target_vars_file_path():
    cmake_dir_name = 'dir_name'

    variables_file_path = 'vars_file_path'
    target_variables = {'target_cmake_variables_file_path': variables_file_path}

    project_path = 'project/path'
    functions.get_project_path = MagicMock(return_value=project_path)

    expected_path = f'{project_path}/{cmake_dir_name}/{variables_file_path}'
    assert functions.get_target_cmake_variables_full_file_path(cmake_dir_name, target_variables) == expected_path
