from enum import Enum


class BuildConfigNames(Enum):
    CONFIG_DEBUG = 'Debug'
    CONFIG_RELEASE = 'Release'
    CONFIG_REL_WITH_DEBUG = 'RelWithDebInfo'
    CONFIG_MIN_SIZE_REL = 'MinSizeRel'

    @staticmethod
    def from_string(name: str):
        return BuildConfigNames(name)

    @staticmethod
    def available_names():
        names = '['
        for name, member in BuildConfigNames.__members__.items():
            names += member.value
            names += ', '

        names = names[:-2]
        names += ']'
        return names

    @staticmethod
    def all_configs():
        configs = []
        for name, member in BuildConfigNames.__members__.items():
            configs.append(member)

        return configs

    def __str__(self):
        return self.value
