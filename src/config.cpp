#include "config.hpp"
#include <fstream>
#include <iostream>
#include <pwd.h>
#include <unistd.h>

Config::Config() :
    json({}),
    json_state({})
{
    const char *homedir = getenv("HOME");
    if (homedir == nullptr) {
        homedir = getpwuid(getuid())->pw_dir;
    }
    app_path.assign(homedir).append(".keybr");
    if (!std::filesystem::exists(app_path)) {
        std::filesystem::create_directory(app_path);
    }
    if (auto conf = get_config_filepath(); std::filesystem::exists(conf)) {
        std::ifstream(conf) >> json;
    }
    else {
        json["deck"] = "Vocabulary Profile";
        json["card_model"] = "Main en-GB";
        json["cambridge_dictionary"] = "english-russian";
    }
    if (auto conf = get_state_filepath(); std::filesystem::exists(conf)) {
        std::ifstream(conf) >> json_state;
    }
}

bool Config::is_sound_enabled() const
{
    return sound_enabled;
}

void Config::set_sound_enabled(bool value)
{
    sound_enabled = value;
}

Config::~Config()
{
    try {
        std::ofstream(get_config_filepath()) << std::setw(4) << json;
        std::ofstream(get_state_filepath()) << std::setw(4) << json_state;
    }
    catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

Config &Config::instance()
{
    static Config _instance;
    return _instance;
}

std::filesystem::path Config::get_app_path() const
{
    return app_path;
}

std::string Config::get_vocabulary_profile_filepath() const
{
    return get_app_path().append("vocabulary_profile.db");
}

std::string Config::get_kindle_db_filepath() const
{
    return "/Volumes/Kindle/system/vocabulary/vocab.db";
}

std::string Config::get_config_filepath() const
{
    return get_app_path().append("vocabulary_builder_config.json");
}

std::string Config::get_state_filepath() const
{
    return get_app_path().append("vocabulary_builder_state.json");
}
