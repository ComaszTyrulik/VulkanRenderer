from src.json_utils.json_manager import JSONManager


class Config:
    JSON_STR_INDENT = 4

    TARGET_NAME_LIB = 'lib'
    TARGET_NAME_TESTS = 'tests'

    json_config: dict
    cmake: dict

    def __init__(self, config_path, json_manager: JSONManager):
        self.config_path = config_path
        self.json_manager = json_manager
        self.json_config = json_manager.load_from_file(config_path)

        self.cmake = self.json_config['cmake_config']

    def __getitem__(self, item):
        return self.json_config[item]

    def update(self):
        self.json_manager.save_to_file(self.json_config, self.config_path, self.JSON_STR_INDENT)

    def get_target_config_by_name(self, target_name):
        try:
            return self.cmake['targets'][target_name]
        except KeyError:
            raise ValueError(f'"{target_name}" is not a valid target!')

    def list_targets_names(self):
        targets_names = []
        for target in self.cmake['targets']:
            targets_names.append(target)

        return targets_names
