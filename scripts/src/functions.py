from argparse import ArgumentParser
from inspect import currentframe, getframeinfo
from pathlib import Path

from src.config.config import Config


def get_project_path():
    current_filename = getframeinfo(currentframe()).filename
    return Path(current_filename).resolve().parent.parent.parent.__str__().replace('\\', '/')


def get_build_dir_name():
    return 'build'


def get_build_dir_path():
    return f'{get_project_path()}/{get_build_dir_name()}'


def get_config_path():
    return f'{get_project_path()}/config/config.json'


def get_target_cmake_variables_full_file_path(cmake_dir_name: str, variables_config):
    return f'{get_project_path()}/{cmake_dir_name}/{variables_config["target_cmake_variables_file_path"]}'


def create_arguments_parser(program=None, usage=None, description=None, formatter_class=None):
    if formatter_class is None:
        parser = ArgumentParser(prog=program, usage=usage, description=description)
    else:
        parser = ArgumentParser(prog=program, usage=usage, description=description, formatter_class=formatter_class)

    return parser
