import argparse
import builtins
import colorama
import pytest

from mock import MagicMock

from src.commandrunners.cmake.cmake import CMake
from src.beastengine.commands import build


class CommonTestData:
    CONFIG_NAMES = [
        build.BuildConfigNames.CONFIG_DEBUG,
        build.BuildConfigNames.CONFIG_RELEASE,
        build.BuildConfigNames.CONFIG_MIN_SIZE_REL,
        build.BuildConfigNames.CONFIG_REL_WITH_DEBUG,
    ]

    def __init__(self):
        self.parser_mock = MagicMock(argparse.ArgumentParser)
        self.parser_mock.parse_args = MagicMock()

        self.print_mock = MagicMock()
        self.build_configs_mock = MagicMock(build.BuildConfigNames)

    def mock_print_function(self):
        builtins.print = self.print_mock

    def mock_config_names_from_string_method(self, side_effect=None):
        build.BuildConfigNames.from_string = MagicMock(return_value=self.build_configs_mock, side_effect=side_effect)


@pytest.mark.parametrize('config', ['', False, None])
def test_constructor_will_print_warning_message_when_passed_arguments_config_parameter_is_empty(config):
    expected_info_message =\
        f'{colorama.Fore.YELLOW}' \
        f'No configuration specified, ' \
        f'building for all configurations {colorama.Fore.RESET}'

    test_data = CommonTestData()
    test_data.mock_print_function()
    test_data.mock_config_names_from_string_method()

    build.Build(MagicMock(CMake), config)
    test_data.print_mock.assert_called_with(expected_info_message)


@pytest.mark.parametrize('expected_config', CommonTestData.CONFIG_NAMES)
def test_constructor_will_convert_config_string_to_enum(expected_config):
    test_data = CommonTestData()
    test_data.mock_config_names_from_string_method()

    build.Build(MagicMock(CMake), expected_config)
    build.BuildConfigNames.from_string.assert_called_once_with(expected_config)


def test_constructor_will_throw_exception_when_passed_configuration_is_not_valid():
    with pytest.raises(ValueError) as expected_exception:
        invalid_config = 'invalid_config'
        expected_info_message =\
            f"{colorama.Fore.LIGHTRED_EX}'{invalid_config}' is not a valid configuration!\n" \
            f"{colorama.Fore.YELLOW}The available configurations are: {build.BuildConfigNames.available_names()}"

        test_data = CommonTestData()
        test_data.mock_config_names_from_string_method(side_effect=ValueError)

        build.Build(MagicMock(CMake), invalid_config)
        assert expected_exception.value == expected_info_message


@pytest.mark.parametrize('expected_config', CommonTestData.CONFIG_NAMES)
def test_constructor_will_run_cmake_build_command_with_proper_config_name(expected_config):
    test_data = CommonTestData()
    test_data.mock_config_names_from_string_method()

    expected_config = test_data.build_configs_mock
    config_argument_in_method_call_index = 0

    cmake_mock = MagicMock(CMake)
    cmake_mock.build = MagicMock()

    build.Build(cmake_mock, expected_config)

    method_call = cmake_mock.build.call_args[0]
    assert method_call[config_argument_in_method_call_index] == expected_config


def test_constructor_will_run_cmake_build_command():
    test_data = CommonTestData()
    test_data.mock_config_names_from_string_method()

    cmake_mock = MagicMock(CMake)
    cmake_mock.build = MagicMock()

    build.Build(cmake_mock, 'Debug')
    cmake_mock.build.assert_called_once()


def test_constructor_will_not_run_cmake_build_command_when_invalid_config_name_passed():
    with pytest.raises(ValueError):
        invalid_config = 'invalid_config'

        test_data = CommonTestData()
        test_data.mock_print_function()

        test_data.mock_config_names_from_string_method(side_effect=ValueError)

        cmake_mock = MagicMock(CMake)
        cmake_mock.build = MagicMock()

        build.Build(cmake_mock, invalid_config)
        cmake_mock.build.assert_not_called()
