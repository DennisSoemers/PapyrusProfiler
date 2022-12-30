#pragma once

// Thanks to: https://github.com/powerof3/CLibUtil
namespace string {
    template <class T>
    T to_num(const std::string& a_str, bool a_hex = false) {
        const int base = a_hex ? 16 : 10;

        if constexpr (std::is_same_v<T, double>) {
            return static_cast<T>(std::stod(a_str, nullptr));
        } else if constexpr (std::is_same_v<T, float>) {
            return static_cast<T>(std::stof(a_str, nullptr));
        } else if constexpr (std::is_same_v<T, std::int64_t>) {
            return static_cast<T>(std::stol(a_str, nullptr, base));
        } else if constexpr (std::is_same_v<T, std::uint64_t>) {
            return static_cast<T>(std::stoull(a_str, nullptr, base));
        } else if constexpr (std::is_signed_v<T>) {
            return static_cast<T>(std::stoi(a_str, nullptr, base));
        } else {
            return static_cast<T>(std::stoul(a_str, nullptr, base));
        }
    }

    inline std::vector<std::string> split(const std::string& a_str, std::string_view a_delimiter) {
        auto range = a_str | std::ranges::views::split(a_delimiter) |
                     std::ranges::views::transform([](auto&& r) { return std::string_view(r); });
        return {range.begin(), range.end()};
    }

    // https://stackoverflow.com/a/35452044
    inline std::string join(const std::vector<std::string>& a_vec, std::string_view a_delimiter) {
        return std::accumulate(a_vec.begin(), a_vec.end(), std::string{},
                               [a_delimiter](const auto& str1, const auto& str2) {
                                   return str1.empty() ? str2 : str1 + a_delimiter.data() + str2;
                               });
    }
};  // namespace string

class Settings {
public:
    [[nodiscard]] static Settings* GetSingleton();

    void Load();

    struct PapyrusProfilerSettings {
        void Load(CSimpleIniA& a_ini);

        std::string startupConfig{""};
    } papyrusProfilerSettings;

private:
    Settings() = default;
    Settings(const Settings&) = delete;
    Settings(Settings&&) = delete;
    ~Settings() = default;

    Settings& operator=(const Settings&) = delete;
    Settings& operator=(Settings&&) = delete;

    struct detail {
        // Thanks to: https://github.com/powerof3/CLibUtil
        template <class T>
        static T& get_value(CSimpleIniA& a_ini, T& a_value, const char* a_section, const char* a_key,
                            const char* a_comment, const char* a_delimiter = R"(|)") {
            if constexpr (std::is_same_v<T, bool>) {
                a_value = a_ini.GetBoolValue(a_section, a_key, a_value);
                a_ini.SetBoolValue(a_section, a_key, a_value, a_comment);
            } else if constexpr (std::is_floating_point_v<T>) {
                a_value = static_cast<float>(a_ini.GetDoubleValue(a_section, a_key, a_value));
                a_ini.SetDoubleValue(a_section, a_key, a_value, a_comment);
            } else if constexpr (std::is_enum_v<T>) {
                a_value = string::template to_num<T>(
                    a_ini.GetValue(a_section, a_key, std::to_string(std::to_underlying(a_value)).c_str()));
                a_ini.SetValue(a_section, a_key, std::to_string(std::to_underlying(a_value)).c_str(), a_comment);
            } else if constexpr (std::is_arithmetic_v<T>) {
                a_value = string::template to_num<T>(a_ini.GetValue(a_section, a_key, std::to_string(a_value).c_str()));
                a_ini.SetValue(a_section, a_key, std::to_string(a_value).c_str(), a_comment);
            } else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
                a_value = string::split(a_ini.GetValue(a_section, a_key, string::join(a_value, a_delimiter).c_str()),
                                        a_delimiter);
                a_ini.SetValue(a_section, a_key, string::join(a_value, a_delimiter).c_str(), a_comment);
            } else {
                a_value = a_ini.GetValue(a_section, a_key, a_value.c_str());
                a_ini.SetValue(a_section, a_key, a_value.c_str(), a_comment);
            }
            return a_value;
        }
    };
};