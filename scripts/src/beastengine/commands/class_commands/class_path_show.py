from src.config.config import Config
from src.config.target_config_manager import TargetConfigManager


class ClassShowPaths:
    def __init__(self, target_config_manager: TargetConfigManager, config: Config, target_config):
        headers_base_dir = target_config_manager.get_headers_base_directory(target_config, config.cmake)
        sources_base_dir = target_config_manager.get_sources_base_directory(target_config, config.cmake)

        print(f'Headers base directory: {headers_base_dir}\nSources base directory: {sources_base_dir}')
