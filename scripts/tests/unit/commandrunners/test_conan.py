from unittest.mock import call, MagicMock

from src.commandrunners.command_runner import CommandRunner
from src.commandrunners.conan import Conan
from src.config.config_names import BuildConfigNames


def test_install_will_execute_proper_command_with_each_config_name():
    command_runner_mock = MagicMock(CommandRunner)
    command_runner_mock.run_command = MagicMock()

    expected_build_dir = 'build/dir'

    expected_debug_command = f'conan install .. -g cmake_multi --build=missing -s build_type={BuildConfigNames.CONFIG_DEBUG}'
    expected_release_command = f'conan install .. -g cmake_multi --build=missing -s build_type={BuildConfigNames.CONFIG_RELEASE}'
    expected_rel_with_debug_command = f'conan install .. -g cmake_multi --build=missing -s build_type={BuildConfigNames.CONFIG_REL_WITH_DEBUG}'
    expected_min_size_rel_command = f'conan install .. -g cmake_multi --build=missing -s build_type={BuildConfigNames.CONFIG_MIN_SIZE_REL}'

    expected_debug_call = call(expected_debug_command, expected_build_dir)
    expected_release_call = call(expected_release_command, expected_build_dir)
    expected_rel_with_debug_call = call(expected_rel_with_debug_command, expected_build_dir)
    expected_min_size_rel_call = call(expected_min_size_rel_command, expected_build_dir)

    sut = Conan(command_runner_mock, expected_build_dir)
    sut.install()

    command_runner_mock\
        .run_command\
        .assert_has_calls(
            [
                expected_debug_call,
                expected_release_call,
                expected_rel_with_debug_call,
                expected_min_size_rel_call
            ]
        )
