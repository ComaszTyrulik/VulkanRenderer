import os
from pathlib import Path

from src.files.file_opener import FileOpener


class ClassFilesHelper:
    HEADER_FILE_EXTENSION = 'h'
    SOURCE_FILE_EXTENSION = 'cpp'
    CLASS_NAME_DIRECTORY_SEPARATOR = '/'

    def __init__(self, file_opener: FileOpener):
        self.file_opener = file_opener

    def does_class_header_file_exist(self, class_name: str, headers_base_dir: str, target_config):
        header_files = target_config['headers']['files']
        header_file = ClassFilesHelper.get_header_filename(class_name)
        header_file_path = f'{headers_base_dir}/{header_file}'

        return header_files.__contains__(header_file) and self.__path_exists(header_file_path)

    def does_class_source_file_exist(self, class_name: str, source_base_dir: str, target_config):
        source_files = target_config['sources']['files']
        source_file = ClassFilesHelper.get_source_filename(class_name)
        source_file_path = f'{source_base_dir}/{source_file}'

        return source_files.__contains__(source_file) and self.__path_exists(source_file_path)

    def create_class_header(self, class_name, headers_base_dir, namespace=None):
        header_filename = self.get_header_filename(class_name)
        header_file_path = f'{headers_base_dir}/{header_filename}'
        if self.__path_exists(header_file_path):
            raise FileExistsError(f'{header_file_path} file already exists!')

        if class_name.find(self.CLASS_NAME_DIRECTORY_SEPARATOR) != -1:
            class_sub_directories_path = self.__get_class_subdirectories_path(class_name)
            class_sub_directories_full_path = f'{headers_base_dir}/{class_sub_directories_path}'

            self.__create_file_sub_directories(class_sub_directories_full_path)

        self.__create_header_file(header_file_path, namespace)
        return header_filename

    def create_class_source(self, class_name, sources_base_dir, namespace=None):
        source_filename = self.get_source_filename(class_name)
        source_file_path = f'{sources_base_dir}/{source_filename}'
        if self.__path_exists(source_file_path):
            raise FileExistsError(f'{source_file_path} file already exists!')

        if class_name.find(self.CLASS_NAME_DIRECTORY_SEPARATOR) != -1:
            class_sub_directories_path = self.__get_class_subdirectories_path(class_name)
            class_sub_directories_full_path = f'{sources_base_dir}/{class_sub_directories_path}'

            self.__create_file_sub_directories(class_sub_directories_full_path)

        self.__create_source_file(source_file_path, namespace)
        return source_filename

    def remove_class_header_file(self, class_name: str, headers_base_dir: str):
        file_path = f'{headers_base_dir}/{self.get_header_filename(class_name)}'
        os.remove(file_path)

        if class_name.find(self.CLASS_NAME_DIRECTORY_SEPARATOR) == -1:
            return

        cwd = f'{headers_base_dir}'
        self.__remove_class_subdirectories(self.__get_class_subdirectories_path(class_name), cwd)

    def remove_class_source_file(self, class_name: str, sources_base_dir: str):
        file_path = f'{sources_base_dir}/{self.get_source_filename(class_name)}'
        os.remove(file_path)

        if class_name.find(self.CLASS_NAME_DIRECTORY_SEPARATOR) == -1:
            return

        cwd = f'{sources_base_dir}'
        self.__remove_class_subdirectories(self.__get_class_subdirectories_path(class_name), cwd)

    @staticmethod
    def get_header_filename(class_name):
        return f'{class_name}.{ClassFilesHelper.HEADER_FILE_EXTENSION}'

    @staticmethod
    def get_source_filename(class_name):
        return f'{class_name}.{ClassFilesHelper.SOURCE_FILE_EXTENSION}'

    def __path_exists(self, file_path):
        return Path(file_path).exists()

    def __create_file_sub_directories(self, class_sub_directories_path: str):
        if self.__path_exists(class_sub_directories_path) is False:
            os.makedirs(class_sub_directories_path)

    def __remove_class_subdirectories(self, path: str, cwd: str):
        full_path = f'{cwd}/{path}'

        # Remove subdirectory if it exists and isn't empty
        if os.path.exists(full_path) and os.path.isdir(full_path) and not os.listdir(full_path):
            os.rmdir(full_path)

        # Check if path without the deepest directory still contains any subdirectories and remove them if any
        if path.find(self.CLASS_NAME_DIRECTORY_SEPARATOR) != -1:
            last_occurrence = path.rfind(self.CLASS_NAME_DIRECTORY_SEPARATOR)
            self.__remove_class_subdirectories(path[:last_occurrence], cwd)

    def __create_header_file(self, header_file_path: str, namespace):
        file_content = '#pragma once'
        if namespace is not None:
            file_content += f'\n\nnamespace {namespace}\n{{\n\n}} // namespace {namespace}\n'

        self.file_opener.create(header_file_path)
        header_file = self.file_opener.open(header_file_path)
        header_file.replace_content(file_content)

    def __create_source_file(self, source_file_path: str, namespace):
        file_content = ''

        if namespace is not None:
            file_content += f'\n\nnamespace {namespace}\n{{\n\n}} // namespace {namespace}\n'

        self.file_opener.create(source_file_path)
        source_file = self.file_opener.open(source_file_path)
        source_file.replace_content(file_content)

    def __get_class_subdirectories_path(self, class_name):
        directories = class_name.split(self.CLASS_NAME_DIRECTORY_SEPARATOR)
        directories.pop()

        class_sub_directories_path = ''
        for directory in directories:
            class_sub_directories_path += f'{directory}/'

        return class_sub_directories_path[:-1]
