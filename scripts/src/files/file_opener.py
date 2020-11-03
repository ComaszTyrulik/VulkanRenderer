from pathlib import Path


class FileOpener:
    class File:
        def __init__(self, file_path: str):
            file_mode_read_append_write = 'r+'
            self.__file = open(file_path, file_mode_read_append_write)

        def __del__(self):
            self.__file.close()

        def get_content(self):
            return self.__file.read()

        def replace_content(self, new_content):
            self.__file.truncate(0)
            self.__file.seek(0)
            self.__file.write(new_content)

    def open(self, file_path: str) -> File:
        return self.File(file_path)

    def create(self, file_path):
        Path(file_path).touch()
