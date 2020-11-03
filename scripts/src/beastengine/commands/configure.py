from src.commandrunners.cmake.cmake import CMake


class Configure:
    def __init__(self, cmake: CMake):
        cmake.generate_configs()
        cmake.configure()
