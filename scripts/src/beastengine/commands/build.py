from src.beastengine.beast_command_helper import BeastCommandHelper
from src.commandrunners.cmake.cmake import CMake
from src.config.config_names import BuildConfigNames


class Build:
    NO_CONFIG_INFO_MESSAGE_TEMPLATE =\
        "{yellow}No configuration specified, building for all configurations {reset}"
    INVALID_CONFIG_ERROR_MESSAGE_TEMPLATE =\
        "{red}'{config}' is not a valid configuration!\n{yellow}The available configurations are: {configs}"

    def __init__(self, cmake: CMake, config_name: str):
        self.cmake = cmake

        if not config_name:
            info_message = BeastCommandHelper.format_text(self.NO_CONFIG_INFO_MESSAGE_TEMPLATE)
            print(info_message)

            for config in BuildConfigNames.all_configs():
                self.cmake.build(config)
            return

        try:
            config_name = BuildConfigNames.from_string(config_name)
            self.cmake.build(config_name)
        except ValueError:
            substitution_map = {'config': config_name, 'configs': BuildConfigNames.available_names()}
            message = BeastCommandHelper.format_text(self.INVALID_CONFIG_ERROR_MESSAGE_TEMPLATE, substitution_map)
            raise ValueError(message)
