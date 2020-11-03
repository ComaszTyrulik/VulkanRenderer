import builtins

from mock import MagicMock

from src.config.target_config_manager import TargetConfigManager
from src.config.config import Config
from src.beastengine.commands.class_commands import class_add, class_files_helper


class CommonTestData:
    def __init__(self):
        self.config_mock = MagicMock(Config)
        self.config_mock.cmake = []

        self.target_config_manager_mock = MagicMock(TargetConfigManager)
        self.class_files_helper_mock = MagicMock(class_files_helper.ClassFilesHelper)

        self.headers_base_dir = 'headers/base/dir'
        self.sources_base_dir = 'sources/base/dir'

        self.print_mock = MagicMock()
        self.original_print = builtins.print

    def mock_get_headers_base_dir(self):
        self.target_config_manager_mock.get_headers_base_directory = MagicMock(return_value=self.headers_base_dir)

    def mock_get_sources_base_dir(self):
        self.target_config_manager_mock.get_sources_base_directory = MagicMock(return_value=self.sources_base_dir)


def test_constructor_will_create_class_and_add_created_files_to_list_of_target_headers_and_sources():
    expected_class_name = 'class_name'
    target_config = []
    namespace = None
    header_only = False
    source_only = False

    expected_header_name = f'{expected_class_name}.h'
    expected_source_name = f'{expected_class_name}.cpp'

    test_data = CommonTestData()

    test_data.mock_get_headers_base_dir()
    test_data.mock_get_sources_base_dir()

    test_data.class_files_helper_mock.create_class_header = MagicMock(return_value=expected_header_name)
    test_data.class_files_helper_mock.create_class_source = MagicMock(return_value=expected_source_name)

    test_data.target_config_manager_mock.add_file_to_headers_list = MagicMock()
    test_data.target_config_manager_mock.add_file_to_sources_list = MagicMock()

    class_add.ClassAdd(
        test_data.config_mock,
        test_data.target_config_manager_mock,
        test_data.class_files_helper_mock,
        target_config,
        expected_class_name,
        namespace,
        header_only,
        source_only
    )

    test_data.class_files_helper_mock.create_class_header.assert_called_with(expected_class_name, test_data.headers_base_dir, namespace)
    test_data.class_files_helper_mock.create_class_source.assert_called_with(expected_class_name, test_data.sources_base_dir, namespace)
    test_data.target_config_manager_mock.add_file_to_headers_list.assert_called_with(expected_header_name, target_config)
    test_data.target_config_manager_mock.add_file_to_sources_list.assert_called_with(expected_source_name, target_config)


def test_constructor_will_create_class_and_update_config():
    expected_class_name = 'class_name'
    target_config = []
    namespace = None
    header_only = False
    source_only = False

    test_data = CommonTestData()
    test_data.config_mock.update = MagicMock()

    class_add.ClassAdd(
        test_data.config_mock,
        test_data.target_config_manager_mock,
        test_data.class_files_helper_mock,
        target_config,
        expected_class_name,
        namespace,
        header_only,
        source_only
    )

    test_data.config_mock.update.assert_called_once()


def test_constructor_will_create_header_file_and_add_created_file_to_list_of_target_headers_when_header_only_argument_passed():
    expected_class_name = 'class_name'
    target_config = []
    namespace = None
    header_only = True
    source_only = False

    expected_header_name = f'{expected_class_name}.h'

    test_data = CommonTestData()

    test_data.mock_get_headers_base_dir()
    test_data.mock_get_sources_base_dir()

    test_data.class_files_helper_mock.create_class_header = MagicMock(return_value=expected_header_name)
    test_data.class_files_helper_mock.create_class_source = MagicMock()

    test_data.target_config_manager_mock.add_file_to_headers_list = MagicMock()
    test_data.target_config_manager_mock.add_file_to_sources_list = MagicMock()

    class_add.ClassAdd(
        test_data.config_mock,
        test_data.target_config_manager_mock,
        test_data.class_files_helper_mock,
        target_config,
        expected_class_name,
        namespace,
        header_only,
        source_only
    )

    test_data.class_files_helper_mock.create_class_header.assert_called_with(expected_class_name, test_data.headers_base_dir, namespace)
    test_data.target_config_manager_mock.add_file_to_headers_list.assert_called_with(expected_header_name, target_config)

    test_data.class_files_helper_mock.create_class_source.assert_not_called()
    test_data.target_config_manager_mock.add_file_to_sources_list.assert_not_called()


def test_constructor_will_create_header_file_and_update_config_when_header_only_argument_passed():
    expected_class_name = 'class_name'
    target_config = []
    namespace = None
    header_only = True
    source_only = False

    test_data = CommonTestData()
    test_data.config_mock.update = MagicMock()

    class_add.ClassAdd(
        test_data.config_mock,
        test_data.target_config_manager_mock,
        test_data.class_files_helper_mock,
        target_config,
        expected_class_name,
        namespace,
        header_only,
        source_only
    )

    test_data.config_mock.update.assert_called_once()


def test_constructor_will_create_source_file_and_add_created_file_to_list_of_target_sources_when_source_only_argument_passed():
    expected_class_name = 'class_name'
    target_config = []
    namespace = None
    header_only = False
    source_only = True

    expected_source_name = f'{expected_class_name}.cpp'

    test_data = CommonTestData()
    test_data.config_mock.get_target_config_by_name = MagicMock(return_value=target_config)

    test_data.mock_get_headers_base_dir()
    test_data.mock_get_sources_base_dir()

    test_data.class_files_helper_mock.create_class_header = MagicMock()
    test_data.class_files_helper_mock.create_class_source = MagicMock(return_value=expected_source_name)

    test_data.target_config_manager_mock.add_file_to_headers_list = MagicMock()
    test_data.target_config_manager_mock.add_file_to_sources_list = MagicMock()

    class_add.ClassAdd(
        test_data.config_mock,
        test_data.target_config_manager_mock,
        test_data.class_files_helper_mock,
        target_config,
        expected_class_name,
        namespace,
        header_only,
        source_only
    )

    test_data.class_files_helper_mock.create_class_source.assert_called_with(expected_class_name, test_data.sources_base_dir, namespace)
    test_data.target_config_manager_mock.add_file_to_sources_list.assert_called_with(expected_source_name, target_config)

    test_data.class_files_helper_mock.create_class_header.assert_not_called()
    test_data.target_config_manager_mock.add_file_to_headers_list.assert_not_called()


def test_constructor_will_create_source_file_and_update_config_when_source_only_argument_passed():
    expected_class_name = 'class_name'
    target_config = []
    namespace = None
    header_only = False
    source_only = True

    test_data = CommonTestData()
    test_data.config_mock.update = MagicMock()

    class_add.ClassAdd(
        test_data.config_mock,
        test_data.target_config_manager_mock,
        test_data.class_files_helper_mock,
        target_config,
        expected_class_name,
        namespace,
        header_only,
        source_only
    )

    test_data.config_mock.update.assert_called_once()


def test_constructor_will_create_class_and_add_created_files_to_list_of_target_files_when_header_only_and_source_only_parameters_are_both_true():
    expected_class_name = 'class_name'
    target_config = []
    namespace = None
    header_only = True
    source_only = True

    expected_header_name = f'{expected_class_name}.h'
    expected_source_name = f'{expected_class_name}.cpp'

    test_data = CommonTestData()
    test_data.config_mock.get_target_config_by_name = MagicMock(return_value=target_config)

    test_data.mock_get_headers_base_dir()
    test_data.mock_get_sources_base_dir()

    test_data.class_files_helper_mock.create_class_header = MagicMock(return_value=expected_header_name)
    test_data.class_files_helper_mock.create_class_source = MagicMock(return_value=expected_source_name)

    test_data.target_config_manager_mock.add_file_to_headers_list = MagicMock()
    test_data.target_config_manager_mock.add_file_to_sources_list = MagicMock()

    class_add.ClassAdd(
        test_data.config_mock,
        test_data.target_config_manager_mock,
        test_data.class_files_helper_mock,
        target_config,
        expected_class_name,
        namespace,
        header_only,
        source_only
    )

    test_data.class_files_helper_mock.create_class_header.assert_called_with(expected_class_name, test_data.headers_base_dir, namespace)
    test_data.class_files_helper_mock.create_class_source.assert_called_with(expected_class_name, test_data.sources_base_dir, namespace)
    test_data.target_config_manager_mock.add_file_to_headers_list.assert_called_with(expected_header_name, target_config)
    test_data.target_config_manager_mock.add_file_to_sources_list.assert_called_with(expected_source_name, target_config)
