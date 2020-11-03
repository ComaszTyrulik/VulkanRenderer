import copy
from unittest.mock import MagicMock

import pytest

from src.config.config import Config
from src.json_utils.json_manager import JSONManager


class CommonTestData:
    def __init__(self):
        self.json_manager_mock = MagicMock(JSONManager)
        self.json_manager_mock.load_from_file = MagicMock()
        self.config_json = \
            {
                "default_build_type": "Debug",
                "cmake_config": {
                    "directory_name": "cmake/config",
                    "config_files": {
                        "dist_filename": "config.cmake.dist",
                        "filename": "config.cmake"
                    },
                    "project": {
                        "name": "BeastEngine",
                        "version_major": "0",
                        "version_minor": "0",
                        "version_patch": "1",
                        "name_placeholder": "project_name",
                        "version_major_placeholder": "project_version_major",
                        "version_minor_placeholder": "project_version_minor",
                        "version_patch_placeholder": "project_version_patch"
                    },
                    "targets": {
                        "lib": {
                            "target_name": "beastengine",
                            "target_name_placeholder": "lib_target_name",
                            "config_files": {
                                "dist_filename": "beastengine/config.cmake.dist",
                                "filename": "beastengine/config.cmake"
                            },
                            "variables": {
                                "target_cmake_variables_file_path_placeholder": "beast_cmake_vars_file_path",
                                "target_cmake_variables_file_path": "beastengine/beast_vars.txt",
                            },
                            "directories": {
                                "include_directory_placeholder": "beast_include_dir",
                                "include_directory": "\"${BeastEngine_SOURCE_DIR}/include\"",
                                "source_directory_placeholder": "beast_src_dir",
                                "source_directory": "\"${BeastEngine_SOURCE_DIR}/src\""
                            },
                            "headers": {
                                "base_dir": "${BEAST_INCLUDE_DIR}/BeastEngine",
                                "files_list_placeholder": "beast_headers",
                                "files": [
                                    "beastengine.h"
                                ]
                            },
                            "sources": {
                                "base_dir": "${BEAST_SRC_DIR}/BeastEngine",
                                "files_list_placeholder": "beast_sources",
                                "files": [
                                    "beastengine.cpp"
                                ]
                            }
                        },
                        "exe": {
                            "target_name": "sandbox",
                            "target_name_placeholder": "exe_target_name",
                            "config_files": None,
                            "variables": None,
                            "directories": None,
                            "headers": {
                                "base_dir": "",
                                "files_list_placeholder": "",
                                "files": []
                            },
                            "sources": {
                                "base_dir": "",
                                "files_list_placeholder": "",
                                "files": []
                            }
                        },
                        "tests": {
                            "target_name": "lab",
                            "target_name_placeholder": "tests_target_name",
                            "config_files": None,
                            "variables": None,
                            "directories": None,
                            "headers": {
                                "base_dir": "",
                                "files_list_placeholder": "",
                                "files": []
                            },
                            "sources": {
                                "base_dir": "",
                                "files_list_placeholder": "",
                                "files": []
                            }
                        }
                    }
                }
            }


def test_init_will_load_config_json_from_given_file():
    test_data = CommonTestData()

    expected_file_path = 'path/to/json/file/json'
    test_data.json_manager_mock.load_from_file.return_value = test_data.config_json

    Config(expected_file_path, test_data.json_manager_mock)
    test_data.json_manager_mock.load_from_file.assert_called_with(expected_file_path)


def test_update_config_will_save_updated_config_object_into_selected_config_file():
    test_data = CommonTestData()
    file_path = 'path/to/json/file/json'

    expected_config_after = copy.deepcopy(test_data.config_json)
    expected_config_after['default_build_type'] = 'BuildType'
    expected_config_after['cmake_config'] = None

    test_data.json_manager_mock.load_from_file.return_value = test_data.config_json
    test_data.json_manager_mock.save_to_file = MagicMock()

    sut = Config(file_path, test_data.json_manager_mock)
    config_before = copy.deepcopy(sut.json_config)

    sut.config = expected_config_after
    sut.update()

    test_data.json_manager_mock.save_to_file.assert_called_with(sut.json_config, file_path, sut.JSON_STR_INDENT)
    assert config_before != sut.config
    assert sut.config == expected_config_after


def test_get_target_config_by_name_will_return_target_with_given_name_if_one_exists():
    target_name = 'lib'
    test_data = CommonTestData()
    test_data.json_manager_mock.load_from_file.return_value = test_data.config_json

    expected_target = test_data.config_json['cmake_config']['targets'][target_name]
    sut = Config('', test_data.json_manager_mock)

    actual_config = sut.get_target_config_by_name(target_name)
    assert expected_target == actual_config


def test_get_target_config_by_name_will_throw_exception_if_given_target_does_not_exist():
    with pytest.raises(ValueError):
        target_name = 'invalid'
        test_data = CommonTestData()
        test_data.json_manager_mock.load_from_file.return_value = test_data.config_json

        sut = Config('', test_data.json_manager_mock)
        sut.get_target_config_by_name(target_name)


def test_list_targets_names_will_return_list_of_defined_targets():
    expected_list_of_targets_names = ['lib', 'exe', 'tests']

    test_data = CommonTestData()
    test_data.json_manager_mock.load_from_file.return_value = test_data.config_json

    sut = Config('', test_data.json_manager_mock)

    actual_list_of_targets_names = sut.list_targets_names()
    assert expected_list_of_targets_names == actual_list_of_targets_names
