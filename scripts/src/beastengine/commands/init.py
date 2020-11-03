import os
import shutil
import colorama

from src.commandrunners.cmake.cmake import CMake
from src.commandrunners.conan import Conan
from src.functions import get_build_dir_name


class Init:
    def __init__(self, project_dir: str, conan: Conan, cmake: CMake):
        build_dir_path = f'{project_dir}/{get_build_dir_name()}'

        # Remove build directory if exists
        if os.path.isdir(build_dir_path):
            print(f'{colorama.Fore.YELLOW}Removing "{build_dir_path}" directory')
            shutil.rmtree(build_dir_path)

        # Create build directory
        print(f'{colorama.Fore.YELLOW}Recreating "{build_dir_path}" directory{colorama.Fore.WHITE}\n')
        os.mkdir(build_dir_path)

        conan.install()
        cmake.generate_configs()
