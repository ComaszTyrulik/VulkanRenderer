from src.beastengine.commands.class_commands.class_files_helper import ClassFilesHelper
from src.config.config import Config
from src.config.target_config_manager import TargetConfigManager


class ClassRemove:
    def __init__(
        self,
        config: Config,
        target_config_manager: TargetConfigManager,
        class_files_helper: ClassFilesHelper,
        class_name: str,
        target_config
    ):
        headers_base_dir = target_config_manager.get_headers_base_directory(target_config, config.cmake)
        if class_files_helper.does_class_header_file_exist(class_name, headers_base_dir, target_config) is True:
            class_files_helper.remove_class_header_file(class_name, headers_base_dir)

            header_name = class_files_helper.get_header_filename(class_name)
            target_config_manager.remove_file_from_headers_list(header_name, target_config)

            config.update()

        sources_base_dir = target_config_manager.get_sources_base_directory(target_config, config.cmake)
        if class_files_helper.does_class_source_file_exist(class_name, sources_base_dir, target_config) is True:
            class_files_helper.remove_class_source_file(class_name, sources_base_dir)

            class_name = class_files_helper.get_source_filename(class_name)
            target_config_manager.remove_file_from_sources_list(class_name, target_config)

            config.update()
