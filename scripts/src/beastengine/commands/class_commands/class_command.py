import sys

from src.beastengine.commands.class_commands.class_path_show import ClassShowPaths
from src.beastengine.commands.class_commands.class_remove import ClassRemove
from src.beastengine.commands.class_commands.class_files_helper import ClassFilesHelper
from src.beastengine.commands.class_commands.class_add import ClassAdd
from src.functions import create_arguments_parser
from src.beastengine.beast_command_helper import BeastCommandHelper
from src.commandrunners.cmake.cmake import CMake
from src.config.config import Config
from src.config.target_config_manager import TargetConfigManager


class ClassCommand:
    PROGRAM_USAGE = '''{green}beast {class} <target> <command> [<args>]

{white}This command operates on header and source files relatively to the project source directory defined in CMake and additionally
relative to the 'config.cmake.target.[headers|sources.base_dir]' parameter.

{green}For the list of available targets, execute: {yellow}beast {config} --list_targets

{purple}Available commands{white}
 {green}{class_add}{white}           Adds new class to the given target
 {green}{class_remove}{white}        Removes existing class from the given target
 {green}{class_path_show}{white}          Displays information about base directory for headers and sources of given target

{yellow}Type "beast {class} <target> <command> --help" for more information on a specific class related command{white}
'''

    def __init__(
        self,
        config: Config,
        cmake: CMake,
        target_config_manager: TargetConfigManager,
        class_files_helper: ClassFilesHelper
    ):
        self.config = config
        self.cmake = cmake
        self.target_config_manager = target_config_manager
        self.class_files_helper = class_files_helper

        usage = BeastCommandHelper.format_text(self.PROGRAM_USAGE)
        parser = create_arguments_parser(usage=usage)
        parser.add_argument('target', help='target for which the files should be added', metavar='<target>')
        parser.add_argument(
            'command',
            help='command to execute',
            metavar='<command>',
            choices=[
                BeastCommandHelper.COMMAND_NAME_CLASS_ADD,
                BeastCommandHelper.COMMAND_NAME_CLASS_REMOVE,
                BeastCommandHelper.COMMAND_NAME_CLASS_PATH_SHOW,
            ]
        )

        command_line_arguments = parser.parse_args(sys.argv[2:4])
        target_name = command_line_arguments.target
        target = config.get_target_config_by_name(target_name)

        command = command_line_arguments.command
        if command == BeastCommandHelper.COMMAND_NAME_CLASS_ADD:
            self.add_class(target)
        elif command == BeastCommandHelper.COMMAND_NAME_CLASS_REMOVE:
            self.remove_class(target)
        elif command == BeastCommandHelper.COMMAND_NAME_CLASS_PATH_SHOW:
            self.show_class_paths(target)

        else:
            return

    def add_class(self, target):
        program_usage = '''{green}beast {class} <target> {class_add} <class_name> [<args>]

{white}This command creates single header and single source files under the headers and sources base directories of the given CMake target.
If class name contains slashes, it will create subdirectories inside base directory.
Eg. {yellow}beast {class} {class_add} subDir/myClass{white} will result in creation of the 'myClass.h' and 'myClass.cpp'
files under the 'baseDirectory/subDir' path.{white}
'''
        parser = create_arguments_parser(usage=BeastCommandHelper.format_text(program_usage))
        parser.add_argument('class_name', help='class to add', metavar='<class_name>')
        parser.add_argument('-n', '--namespace', help='namespace in which the class should reside', type=str)

        group = parser.add_mutually_exclusive_group()
        group.add_argument('-ho', '--header_only', help='create only header file and omit the source file', action='store_true')
        group.add_argument('-so', '--source_only', help='create only source file and omit the header file', action='store_true')

        command_line_arguments = parser.parse_args(sys.argv[4:])
        class_name = command_line_arguments.class_name

        namespace = None
        if command_line_arguments:
            namespace = command_line_arguments.namespace

        header_only = False
        if command_line_arguments.header_only:
            header_only = True

        source_only = False
        if command_line_arguments.source_only:
            source_only = True

        ClassAdd(
            self.config,
            self.target_config_manager,
            self.class_files_helper,
            target,
            class_name,
            namespace,
            header_only,
            source_only
        )

        self.cmake.generate_configs()
        self.cmake.configure()

    def remove_class(self, target):
        usage = '''{green}beast {class} <target> {class_remove} <class_name> [<args>]

{white}This command removes header and source files from the headers and sources base directories.
If class name contains slashes, it will also delete empty subdirectories inside base directory.
'''
        parser = create_arguments_parser(usage=BeastCommandHelper.format_text(usage))
        parser.add_argument('class_name', help='class to remove', metavar='<class_name>')

        command_line_arguments = parser.parse_args(sys.argv[4:])
        class_name = command_line_arguments.class_name

        ClassRemove(self.config, self.target_config_manager, self.class_files_helper, class_name, target)

        self.cmake.generate_configs()
        self.cmake.configure()

    def show_class_paths(self, target):
        usage = '''{green}beast {class} <target> {class_path_show} [<args>]

{white}Displays information about base directory for headers and sources of given target{white}
'''
        parser = create_arguments_parser(usage=BeastCommandHelper.format_text(usage))
        parser.parse_args(sys.argv[4:])

        ClassShowPaths(self.target_config_manager, self.config, target)
