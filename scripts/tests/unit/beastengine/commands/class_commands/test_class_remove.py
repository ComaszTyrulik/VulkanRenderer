import argparse
import builtins
import sys
from copy import deepcopy

import pytest
from mock import MagicMock, call

from src.config.target_config_manager import TargetConfigManager
from src.config.config import Config
from src.beastengine.commands.class_commands import class_files_helper
from src.beastengine.commands.class_commands.class_remove import ClassRemove
from tests.tests_utilities.micro_mock import MicroMock


class CommonTestData:
    def __init__(self):
        self.target_config_manager_mock = MagicMock(TargetConfigManager)
        self.class_files_helper_mock = MagicMock(class_files_helper.ClassFilesHelper)

        self.parser_mock = MagicMock(argparse.ArgumentParser)
        self.parser_mock.parse_args = MagicMock()
        self.project_dir = 'project/dir'

        self.headers_base_dir = 'headers/base/dir'
        self.sources_base_dir = 'sources/base/dir'

        self.config_mock = MagicMock(Config)
        self.config_mock.cmake = {}

        self.print_mock = MagicMock()
        self.original_print = builtins.print


def test_constructor_will_remove_header_file_if_corresponding_class_file_exist():
    expected_class_name = 'class_name'
    expected_header_file_name = f'{expected_class_name}.h'
    target_config = []

    test_data = CommonTestData()
    test_data.class_files_helper_mock.does_class_header_file_exist = MagicMock(return_value=True)
    test_data.class_files_helper_mock.remove_class_header_file = MagicMock()
    test_data.class_files_helper_mock.get_header_filename = MagicMock(return_value=expected_header_file_name)

    test_data.target_config_manager_mock.remove_file_from_headers_list = MagicMock()
    test_data.target_config_manager_mock.get_headers_base_directory = MagicMock(return_value=test_data.headers_base_dir)

    test_data.config_mock.update = MagicMock()

    ClassRemove(
        test_data.config_mock,
        test_data.target_config_manager_mock,
        test_data.class_files_helper_mock,
        expected_class_name,
        target_config
    )

    test_data.class_files_helper_mock.remove_class_header_file.assert_called_with(expected_class_name, test_data.headers_base_dir)
    test_data.target_config_manager_mock.remove_file_from_headers_list.assert_called_with(expected_header_file_name, target_config)
    test_data.config_mock.update.assert_called()


def test_constructor_will_not_remove_header_file_if_corresponding_class_file_does_not_exist():
    class_name = 'class_name'
    target_config = []

    test_data = CommonTestData()
    test_data.class_files_helper_mock.does_class_header_file_exist = MagicMock(return_value=False)
    test_data.class_files_helper_mock.remove_class_header_file = MagicMock()

    test_data.target_config_manager_mock.remove_file_from_headers_list = MagicMock()
    test_data.target_config_manager_mock.get_headers_base_directory = MagicMock()

    test_data.config_mock.update = MagicMock()

    ClassRemove(
        test_data.config_mock,
        test_data.target_config_manager_mock,
        test_data.class_files_helper_mock,
        class_name,
        target_config
    )

    test_data.class_files_helper_mock.remove_class_header_file.assert_not_called()
    test_data.target_config_manager_mock.remove_file_from_headers_list.assert_not_called()
    test_data.config_mock.update.assert_not_called()


def test_constructor_will_remove_source_file_if_corresponding_class_file_exist():
    expected_class_name = 'class_name'
    expected_source_file_name = f'{expected_class_name}.cpp'
    target_config = []

    test_data = CommonTestData()
    test_data.class_files_helper_mock.does_class_source_file_exist = MagicMock(return_value=True)
    test_data.class_files_helper_mock.remove_class_source_file = MagicMock()
    test_data.class_files_helper_mock.get_source_filename = MagicMock(return_value=expected_source_file_name)

    test_data.target_config_manager_mock.remove_file_from_sources_list = MagicMock()
    test_data.target_config_manager_mock.get_sources_base_directory = MagicMock(return_value=test_data.sources_base_dir)

    test_data.config_mock.update = MagicMock()

    ClassRemove(
        test_data.config_mock,
        test_data.target_config_manager_mock,
        test_data.class_files_helper_mock,
        expected_class_name,
        target_config
    )

    test_data.class_files_helper_mock.remove_class_source_file.assert_called_with(expected_class_name, test_data.sources_base_dir)
    test_data.target_config_manager_mock.remove_file_from_sources_list.assert_called_with(expected_source_file_name, target_config)
    test_data.config_mock.update.assert_called()


def test_constructor_will_not_remove_source_file_if_corresponding_class_file_does_not_exist():
    expected_class_name = 'class_name'
    expected_source_file_name = f'{expected_class_name}.cpp'
    target_config = []

    test_data = CommonTestData()
    test_data.class_files_helper_mock.does_class_source_file_exist = MagicMock(return_value=False)
    test_data.class_files_helper_mock.remove_class_source_file = MagicMock()
    test_data.class_files_helper_mock.get_source_filename = MagicMock(return_value=expected_source_file_name)

    test_data.target_config_manager_mock.remove_file_from_sources_list = MagicMock()
    test_data.target_config_manager_mock.get_sources_base_directory = MagicMock(return_value=test_data.sources_base_dir)

    test_data.config_mock.update = MagicMock()

    ClassRemove(
        test_data.config_mock,
        test_data.target_config_manager_mock,
        test_data.class_files_helper_mock,
        expected_class_name,
        target_config
    )

    test_data.class_files_helper_mock.remove_class_source_file.assert_not_called()
    test_data.target_config_manager_mock.remove_file_from_sources_list.assert_not_called()
    test_data.config_mock.update.assert_not_called()
