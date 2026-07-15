#pragma once
#include <rfl/Flatten.hpp>
#include <vector>
namespace MPL::WeatherPatcher
{
    constexpr double kDefaultDalcAnchor = 80.0;
    constexpr double kDefaultSunlightAnchor = 255.0;
    constexpr double kStaticLightingTolerance = 0.05;

    enum struct PresetType : uint8_t
    {
        Brightness = 0,
        Compression = 1,
        Saturation = 2,
    };

    inline int PresetTypeToInt(PresetType type) { return static_cast<int>(type); }
    inline PresetType IntToPresetType(int type) { return static_cast<PresetType>(type); }
    inline std::string PresetTypeToString(PresetType type)
    {
        switch (type)
        {
        case PresetType::Brightness:
            return "Brightness";
        case PresetType::Compression:
            return "Compression";
        case PresetType::Saturation:
            return "Saturation";
        default:
            return "";
        }
    }

    struct WeatherBaseline
    {
        RE::BGSDirectionalAmbientLightingColors dalc[RE::TESWeather::ColorTime::kTotal];
        RE::Color ambient[RE::TESWeather::ColorTime::kTotal];
        RE::Color sunlight[RE::TESWeather::ColorTime::kTotal];
        RE::Color effectLighting[RE::TESWeather::ColorTime::kTotal];
    };

    struct BrightnessSettings
    {
        double ambientMultiplier = 1.0;
        double sunlightMultiplier = 1.0;
    };

    struct CompressionSettings
    {
        double ambientCompression = 0.0;
        double sunlightCompression = 0.0;
    };

    struct SaturationSettings
    {
        double multiplier = 1.0;
    };

    struct Anchors {
        double ambientAnchor = kDefaultDalcAnchor;
        double sunlightAnchor = kDefaultSunlightAnchor;
    };

    struct Settings
    {
        rfl::Flatten<Anchors> anchors;
        BrightnessSettings brightness;
        SaturationSettings saturation;
        CompressionSettings compression;
        std::vector<std::string> plugins;
    };

    typedef std::vector<RE::TESWeather*> SourceWeatherSet;
}  // namespace MPL::WeatherPatcher
