import argparse
from mock import MagicMock

from src.commandrunners.cmake.cmake import CMake
from src.beastengine.commands import configure


class CommonTestData:
    def __init__(self):
        self.parser_mock = MagicMock(argparse.ArgumentParser)
        self.parser_mock.parse_args = MagicMock()
        self.project_dir = 'project/dir'

    def mock_get_build_dir_name_function_to_return(self, return_value):
        configure.get_build_dir_name = MagicMock(return_value=return_value)


def test_constructor_will_generate_cmake_configs():
    cmake_mock = MagicMock(CMake)
    cmake_mock.generate_configs = MagicMock()

    configure.Configure(cmake_mock)
    cmake_mock.generate_configs.assert_called_once()


def test_constructor_will_run_cmake_configure_command_with_valid_verbose_argument():
    build_dir_name = 'build_dir_name'

    test_data = CommonTestData()
    test_data.mock_get_build_dir_name_function_to_return(build_dir_name)

    cmake_mock = MagicMock(CMake)
    cmake_mock.generate_configs = MagicMock()

    configure.Configure(cmake_mock)
    cmake_mock.configure.assert_called_once()
