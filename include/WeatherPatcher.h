#pragma once
#include <Config/Weathers.h>
#include <filesystem>
namespace MPL::WeatherPatcher
{
    static const std::filesystem::path kSettingsRoot("./Data/Luma/Weather");
    constexpr auto kDefaultSettingsProfileName = "Helios";

    void CaptureBaselineIfNeeded(RE::TESWeather*);
    std::string WeatherName(const RE::TESWeather* a_weather);

    void ApplyDataLoaded();
    void ApplySettings(std::string&);
    bool SaveSettings(std::string&);
    bool ResetSettingsToDefault(std::string&);

    float GetAmbientMultiplier(std::string&);
    void SetAmbientMultiplier(std::string&, float);
    float GetSunlightMultiplier(std::string&);
    void SetSunlightMultiplier(std::string&, float);
    float GetAmbientCompression(std::string&);
    void SetAmbientCompression(std::string&, float);
    float GetSunlightCompression(std::string&);
    void SetSunlightCompression(std::string&, float);
    float GetMoonlightSaturationMultiplier(std::string&);
    void SetMoonlightSaturationMultiplier(std::string&, float);

    std::vector<std::string> GetPresets(std::string&, PresetType);
    void LoadPreset(std::string&, PresetType, std::string&);

    void OnSKSEMessage(SKSE::MessagingInterface::Message*);
};  // namespace MPL::WeatherPatcher
