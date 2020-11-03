import os
from unittest.mock import MagicMock

from src.commandrunners.cmake.cmake import CMake
from src.commandrunners.conan import Conan
from src.beastengine.commands.init import Init


def test_constructor_will_remove_existing_build_directory(tmpdir):
    project_directory = tmpdir

    build_directory_name = 'build'
    build_directory = f'{project_directory}/{build_directory_name}'

    expected_directory_to_be_deleted = f'{build_directory}/ThisShouldBeDeleted'

    os.mkdir(build_directory)
    os.mkdir(expected_directory_to_be_deleted)

    conan_mock = MagicMock(Conan)
    cmake_mock = MagicMock(CMake)

    Init(project_directory, conan_mock, cmake_mock)

    assert os.path.isdir(expected_directory_to_be_deleted) is False


def test_constructor_will_recreate_build_directory(tmpdir):
    project_directory = tmpdir

    build_directory_name = 'build'
    build_directory = f'{project_directory}/{build_directory_name}'

    conan_mock = MagicMock(Conan)
    cmake_mock = MagicMock(CMake)

    Init(project_directory, conan_mock, cmake_mock)

    assert os.path.isdir(build_directory) is True


def test_constructor_will_call_conan_and_cmake(tmpdir):
    project_directory = tmpdir

    conan_mock = MagicMock(Conan)
    conan_mock.install = MagicMock()

    cmake_mock = MagicMock(CMake)
    cmake_mock.generate_configs = MagicMock()

    Init(project_directory, conan_mock, cmake_mock)
    conan_mock.install.assert_called_once()
    cmake_mock.generate_configs.assert_called_once()
