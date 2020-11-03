import json

from src.files.file_opener import FileOpener


class JSONManager:
    def __init__(self, file_opener: FileOpener):
        self.file_opener = file_opener

    def load_from_file(self, file_path: str):
        json_file = self.file_opener.open(file_path)
        return json.loads(json_file.get_content())

    def save_to_file(self, json_object: dict, file_path: str, indent: int):
        json_string = json.dumps(json_object, indent=indent)

        json_file = self.file_opener.open(file_path)
        json_file.replace_content(json_string)
