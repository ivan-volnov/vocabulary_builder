#ifndef CONFIG_H
#define CONFIG_H


#include <filesystem>
#include <libs/json.hpp>

class Config
{
public:
    ~Config();
    Config(const Config &) = delete;
    Config &operator=(const Config &) = delete;

    static Config &instance();


    template<typename T>
    static T get(const std::string &key)
    {
        auto &value = instance().json[key];
        if (value.is_null()) {
            value = T{};
        }
        return value.get<T>();
    }

    template<typename T>
    static T get(const std::string &key, const std::string &inner_key)
    {
        auto &value = instance().json[key][inner_key];
        if (value.is_null()) {
            value = T{};
        }
        return value.get<T>();
    }

    template<typename T>
    static void set(const std::string &key, const T &value)
    {
        instance().json[key] = value;
    }

    template<typename T>
    static void set(const std::string &key, const std::string &inner_key, const T &value)
    {
        instance().json[key][inner_key] = value;
    }


    template<typename T>
    static T get_state(const std::string &key)
    {
        auto &value = instance().json_state[key];
        if (value.is_null()) {
            value = T{};
        }
        return value.get<T>();
    }

    template<typename T>
    static T get_state(const std::string &key, const std::string &inner_key)
    {
        auto &value = instance().json_state[key][inner_key];
        if (value.is_null()) {
            value = T{};
        }
        return value.get<T>();
    }

    template<typename T>
    static void set_state(const std::string &key, const T &value)
    {
        instance().json_state[key] = value;
    }

    template<typename T>
    static void set_state(const std::string &key, const std::string &inner_key, const T &value)
    {
        instance().json_state[key][inner_key] = value;
    }


    std::filesystem::path get_app_path() const;

    std::string get_vocabulary_profile_filepath() const;
    std::string get_kindle_db_filepath() const;
    std::string get_config_filepath() const;
    std::string get_state_filepath() const;

    bool is_sound_enabled() const;
    void set_sound_enabled(bool value);

private:
    Config();
    std::filesystem::path app_path;
    bool sound_enabled = false;
    nlohmann::json json;
    nlohmann::json json_state;
};

#endif // CONFIG_H
