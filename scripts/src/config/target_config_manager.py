import re

from src.beastengine.commands.class_commands.target_cmake_vars_file_opener import TargetCMakeVarsFileOpener


class TargetConfigManager:
    EXCEPTION_MESSAGE_TEMPLATE = "'{}' variable could not be found in '{}' target cmake variables file!"
    EXCEPTION_MESSAGE_TEMPLATE += "Did you forget to add it to the target's variables file inside CMake?"

    def __init__(self, file_opener: TargetCMakeVarsFileOpener):
        self.file_opener = file_opener
        self.cmake_var_pattern = re.compile(r'\${[a-zA-Z0-9._-]+\}')

    def get_headers_base_directory(self, target_config, cmake_config):
        return self.__get_files_base_directory(target_config, cmake_config, target_config['headers'])

    def get_sources_base_directory(self, target_config, cmake_config):
        return self.__get_files_base_directory(target_config, cmake_config, target_config['sources'])

    def __get_files_base_directory(self, target_config, cmake_config, target_files):
        variables = self.file_opener.open(cmake_config, target_config)

        base_dir = target_files['base_dir']
        if not base_dir:
            return ''

        matches = self.cmake_var_pattern.findall(base_dir)
        for match in matches:
            if not variables.__contains__(match):
                raise ValueError(self.EXCEPTION_MESSAGE_TEMPLATE.replace(match, target_config['name']))

            base_dir = base_dir.replace(match, variables[match])

        return base_dir

    def add_file_to_headers_list(self, header_name, target_config):
        target_config['headers']['files'].append(header_name)

    def add_file_to_sources_list(self, source_name, target_config):
        target_config['sources']['files'].append(source_name)

    def remove_file_from_headers_list(self, header_name, target_config):
        target_config['headers']['files'].remove(header_name)

    def remove_file_from_sources_list(self, source_name, target_config):
        target_config['sources']['files'].remove(source_name)
