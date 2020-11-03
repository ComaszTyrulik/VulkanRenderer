from src.beastengine.commands.class_commands.class_files_helper import ClassFilesHelper
from src.config.config import Config
from src.config.target_config_manager import TargetConfigManager


class ClassAdd:
    PROGRAM_USAGE = '''{green}beast {class} {class_add} <target> <class_name> [<args>]

{white}This command creates single header and single source files under the headers and sources base directories of the given CMake target.
If class name contains slashes, it will create subdirectories inside base directory.
Eg. {yellow}beast {class} {class_add} subDir/myClass{white} will result in creation of the 'myClass.h' and 'myClass.cpp'
files under the 'baseDirectory/subDir' path.{white}
'''

    CLASS_SOURCE_FILE_EXISTS_ERROR_MESSAGE_TEMPLATE = "'{}' source file already exists!"
    CLASS_HEADER_FILE_EXISTS_ERROR_MESSAGE_TEMPLATE = "'{}' header file already exists!"
    CLASS_EXISTS_ERROR_MESSAGE_TEMPLATE = "'{}' class already exists!"

    def __init__(
            self,
            config: Config,
            target_config_manager: TargetConfigManager,
            class_files_helper: ClassFilesHelper,
            target_config,
            class_name: str,
            namespace=None,
            header_only: bool = False,
            source_only: bool = False
    ):
        self.target_config_manager = target_config_manager
        self.class_files_helper = class_files_helper

        if header_only == source_only:
            self.create_header(class_name, target_config, config.cmake, namespace)
            self.create_source(class_name, target_config, config.cmake, namespace)
        elif header_only is True:
            self.create_header(class_name, target_config, config.cmake, namespace)
        elif source_only is True:
            self.create_source(class_name, target_config, config.cmake, namespace)

        config.update()

    def create_header(self, class_name, target_config, cmake_config, namespace):
        header_name = self.class_files_helper.create_class_header(
            class_name,
            self.target_config_manager.get_headers_base_directory(target_config, cmake_config),
            namespace
        )
        self.target_config_manager.add_file_to_headers_list(header_name, target_config)

    def create_source(self, class_name, target_config, cmake_config, namespace):
        source_name = self.class_files_helper.create_class_source(
            class_name,
            self.target_config_manager.get_sources_base_directory(target_config, cmake_config),
            namespace
        )
        self.target_config_manager.add_file_to_sources_list(source_name, target_config)
