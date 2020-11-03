from unittest.mock import MagicMock, call

import pytest

from src.commandrunners.cmake.cmake_config_files_creator import CMakeConfigFilesCreator
from src.config.config_names import BuildConfigNames
from src.commandrunners.cmake.cmake import CMake
from src.commandrunners.command_runner import CommandRunner
from src.config.config import Config


class CommonTestData:
    PROJECT_DIR = 'project/dir'
    BUILD_DIR = 'build/dir'
    CMAKE_CONFIG_DIR = 'cmake/config'
    CMAKE_CONFIG_DIR_FULL_PATH = f'{PROJECT_DIR}/cmake/config'

    CONFIG_NAMES = [
        BuildConfigNames.CONFIG_DEBUG,
        BuildConfigNames.CONFIG_RELEASE,
        BuildConfigNames.CONFIG_REL_WITH_DEBUG,
        BuildConfigNames.CONFIG_MIN_SIZE_REL,
    ]

    def __init__(self):
        self.command_runner_mock = MagicMock(CommandRunner)
        self.command_runner_mock.run_command = MagicMock()

        self.config_mock = MagicMock(Config)
        type(self.config_mock).config = {
            'cmake_config': {
                'directory_name': self.CMAKE_CONFIG_DIR,
                'targets': {}
            }
        }
        type(self.config_mock).cmake = type(self.config_mock).config['cmake_config']

        self.cmake_config_creator_mock = MagicMock(CMakeConfigFilesCreator)
        self.cmake_config_creator_mock.generate_main_config = MagicMock()
        self.cmake_config_creator_mock.generate_target_config = MagicMock()

        self.sut = CMake(
            self.command_runner_mock,
            self.cmake_config_creator_mock,
            self.config_mock,
            self.PROJECT_DIR,
            self.BUILD_DIR,
        )


@pytest.mark.parametrize('expected_config_name', CommonTestData.CONFIG_NAMES)
def test_build_will_run_proper_build_command_with_given_config_name(expected_config_name):
    test_data = CommonTestData()

    expected_command = f'cmake --build . --config {expected_config_name.value}'
    expected_cwd = test_data.BUILD_DIR

    test_data.sut.build(expected_config_name)
    test_data.command_runner_mock.run_command.assert_called_with(expected_command, expected_cwd)


def test_generate_configs_will_generate_all_config_files_through_config_generator():
    test_data = CommonTestData()
    expected_config = {
        'directory_name': test_data.CMAKE_CONFIG_DIR,
        'targets': {
            'beastengine': {},
            'sandbox': {},
            'lab': {},
        }
    }

    type(test_data.config_mock).cmake = expected_config

    expected_main_config_call = call(expected_config, test_data.CMAKE_CONFIG_DIR_FULL_PATH)
    expected_lib_target_call = call(expected_config['targets']['beastengine'], test_data.CMAKE_CONFIG_DIR_FULL_PATH)
    expected_exe_target_call = call(expected_config['targets']['sandbox'], test_data.CMAKE_CONFIG_DIR_FULL_PATH)
    expected_tests_target_call = call(expected_config['targets']['lab'], test_data.CMAKE_CONFIG_DIR_FULL_PATH)

    sut = CMake(
        test_data.command_runner_mock,
        test_data.cmake_config_creator_mock,
        test_data.config_mock,
        test_data.PROJECT_DIR,
        test_data.BUILD_DIR,
    )
    sut.generate_configs()

    test_data \
        .cmake_config_creator_mock \
        .generate_main_config \
        .assert_has_calls([expected_main_config_call])

    test_data\
        .cmake_config_creator_mock\
        .generate_target_config\
        .assert_has_calls([expected_lib_target_call, expected_exe_target_call, expected_tests_target_call])


def test_generate_main_config_will_generate_config_file_through_config_generator():
    test_data = CommonTestData()
    expected_config = {
        'directory_name': test_data.CMAKE_CONFIG_DIR,
        'targets': {}
    }
    type(test_data.config_mock).cmake = expected_config

    sut = CMake(
        test_data.command_runner_mock,
        test_data.cmake_config_creator_mock,
        test_data.config_mock,
        test_data.PROJECT_DIR,
        test_data.BUILD_DIR,
    )
    sut.generate_main_config()

    test_data\
        .cmake_config_creator_mock\
        .generate_main_config\
        .assert_called_with(expected_config, test_data.CMAKE_CONFIG_DIR_FULL_PATH)


def test_generate_targets_configs_will_generate_config_files_through_config_generator():
    test_data = CommonTestData()

    config = {
        'directory_name': test_data.CMAKE_CONFIG_DIR,
        'targets': {
            'beastengine': {},
            'sandbox': {},
            'lab': {},
        }
    }
    type(test_data.config_mock).cmake = config

    expected_lib_target_call = call(config['targets']['beastengine'], test_data.CMAKE_CONFIG_DIR_FULL_PATH)
    expected_exe_target_call = call(config['targets']['sandbox'], test_data.CMAKE_CONFIG_DIR_FULL_PATH)
    expected_tests_target_call = call(config['targets']['lab'], test_data.CMAKE_CONFIG_DIR_FULL_PATH)

    sut = CMake(
        test_data.command_runner_mock,
        test_data.cmake_config_creator_mock,
        test_data.config_mock,
        test_data.PROJECT_DIR,
        test_data.BUILD_DIR,
    )
    sut.generate_targets_configs()

    test_data \
        .cmake_config_creator_mock \
        .generate_target_config \
        .assert_has_calls([expected_lib_target_call, expected_exe_target_call, expected_tests_target_call])


def test_generate_target_config_will_generate_config_file_through_config_generator():
    test_data = CommonTestData()

    config = {
        'directory_name': test_data.CMAKE_CONFIG_DIR,
        'targets': {}
    }
    type(test_data.config_mock).cmake = config

    expected_target_config = config['targets']

    sut = CMake(
        test_data.command_runner_mock,
        test_data.cmake_config_creator_mock,
        test_data.config_mock,
        test_data.PROJECT_DIR,
        test_data.BUILD_DIR,
    )
    sut.generate_target_config(expected_target_config)

    test_data \
        .cmake_config_creator_mock \
        .generate_target_config \
        .assert_called_with(expected_target_config, test_data.CMAKE_CONFIG_DIR_FULL_PATH)
