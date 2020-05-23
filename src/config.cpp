#include "config.h"
#include <unistd.h>
#include <pwd.h>
#include <fstream>
#include <iostream>


Config::Config() :
    json({})
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
    // TODO: use real path to kindle's db file
//    return ("/Volumes/Kindle/system/vocabulary/vocab.db");
    return get_app_path().append("vocab.db");
}

std::string Config::get_config_filepath() const
{
    return get_app_path().append("vocabulary_builder_config.json");
}

std::string Config::get_state_filepath() const
{
    return get_app_path().append("vocabulary_builder_state.json");
}
