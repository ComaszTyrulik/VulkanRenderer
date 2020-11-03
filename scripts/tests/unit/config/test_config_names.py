from src.config.config_names import BuildConfigNames


def test_available_names_will_return_valid_config_names():
    expected_configurations =\
        f'[{BuildConfigNames.CONFIG_DEBUG}, ' \
        f'{BuildConfigNames.CONFIG_RELEASE}, {BuildConfigNames.CONFIG_REL_WITH_DEBUG}, {BuildConfigNames.CONFIG_MIN_SIZE_REL}]'

    actual_configurations = BuildConfigNames.available_names()
    assert expected_configurations == actual_configurations


def test_all_configs_will_return_array_of_configurations():
    expected_configurations = [
        BuildConfigNames(BuildConfigNames.CONFIG_DEBUG),
        BuildConfigNames(BuildConfigNames.CONFIG_RELEASE),
        BuildConfigNames(BuildConfigNames.CONFIG_REL_WITH_DEBUG),
        BuildConfigNames(BuildConfigNames.CONFIG_MIN_SIZE_REL),
    ]
    actual_configurations = BuildConfigNames.all_configs()

    assert expected_configurations == actual_configurations
