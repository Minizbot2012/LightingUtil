#include <Config.h>
#include <WeatherPatcher.h>
#include <algorithm>
#include <optional>

//TODO: Massive overhaul of this code.
namespace MPL::WeatherPatcher
{
    std::uint8_t ClampByte(double a_value)
    {
        return static_cast<std::uint8_t>(std::clamp(std::round(a_value), 0.0, 255.0));
    }

    double ClampAnchor(double a_value)
    {
        return std::clamp(a_value, 0.0, 255.0);
    }

    double Luminance(const RE::Color& a_color)
    {
        return 0.299 * a_color.red + 0.587 * a_color.green + 0.114 * a_color.blue;
    }

    void MultiplyColor(RE::Color& a_color, double a_multiplier)
    {
        double multiplier = std::max(0.0, a_multiplier);
        if (multiplier > 1.0)
        {
            const double maxChannel = std::max({ static_cast<double>(a_color.red), static_cast<double>(a_color.green), static_cast<double>(a_color.blue) });
            if (maxChannel > 0.0)
            {
                multiplier = std::min(multiplier, 255.0 / maxChannel);
            }
        }

        a_color.red = ClampByte(a_color.red * multiplier);
        a_color.green = ClampByte(a_color.green * multiplier);
        a_color.blue = ClampByte(a_color.blue * multiplier);
    }

    void CompressColor(RE::Color& a_color, double a_compress, double a_anchor)
    {
        const double luminance = Luminance(a_color);
        double compressedLuminance = 0.0;

        if (luminance > 0.0001 && a_anchor > 0.0001)
        {
            compressedLuminance = std::exp((1.0 - a_compress) * std::log(luminance) + a_compress * std::log(a_anchor));
        }

        double red = compressedLuminance;
        double green = compressedLuminance;
        double blue = compressedLuminance;

        if (luminance > 0.0001)
        {
            const double scale = compressedLuminance / luminance;
            red = a_color.red * scale;
            green = a_color.green * scale;
            blue = a_color.blue * scale;
        }

        const double maxChannel = std::max({ red, green, blue });
        if (maxChannel > 255.0)
        {
            const double scale = 255.0 / maxChannel;
            red *= scale;
            green *= scale;
            blue *= scale;
        }

        a_color.red = ClampByte(red);
        a_color.green = ClampByte(green);
        a_color.blue = ClampByte(blue);
    }

    void SaturateColor(RE::Color& a_color, double a_multiplier)
    {
        const double factor = std::max(0.0, a_multiplier);
        const double luminance = Luminance(a_color);
        const double red = luminance + ((a_color.red - luminance) * factor);
        const double green = luminance + ((a_color.green - luminance) * factor);
        const double blue = luminance + ((a_color.blue - luminance) * factor);

        a_color.red = ClampByte(red);
        a_color.green = ClampByte(green);
        a_color.blue = ClampByte(blue);
    }

    std::string ColorString(const RE::Color& a_color)
    {
        return std::format("{},{},{}", a_color.red, a_color.green, a_color.blue);
    }

    bool BrightnessIsActive(const BrightnessSettings a_settings)
    {
        return std::abs(a_settings.ambientMultiplier - 1.0) > 0.0001 ||
               std::abs(a_settings.sunlightMultiplier - 1.0) > 0.0001;
    }

    bool AmbientCompressionIsActive(const CompressionSettings a_settings)
    {
        return std::abs(a_settings.ambientCompression) > 0.0001;
    }

    bool SunlightCompressionIsActive(const CompressionSettings a_settings)
    {
        return std::abs(a_settings.sunlightCompression) > 0.0001;
    }

    bool CompressionIsActive(const CompressionSettings a_settings)
    {
        return AmbientCompressionIsActive(a_settings) || SunlightCompressionIsActive(a_settings);
    }

    bool MoonlightSaturationIsActive(const SaturationSettings a_settings)
    {
        return std::abs(a_settings.multiplier - 1.0) > 0.0001;
    }

    bool LightingCycleIsStatic(const RE::TESWeather* a_weather, RE::TESWeather::ColorType a_colorType)
    {
        double darkest = std::numeric_limits<double>::max();
        double brightest = 0.0;

        for (std::uint32_t time = 0; time < RE::TESWeather::ColorTime::kTotal; ++time)
        {
            const double luminance = Luminance(a_weather->colorData[a_colorType][time]);
            darkest = std::min(darkest, luminance);
            brightest = std::max(brightest, luminance);
        }

        const double tolerance = std::max(255.0 * kStaticLightingTolerance, brightest * kStaticLightingTolerance);
        return brightest - darkest <= tolerance;
    }

    bool ShouldSkipMoonlightSaturation(const RE::TESWeather* a_weather)
    {
        return LightingCycleIsStatic(a_weather, RE::TESWeather::ColorType::kSunlight) ||
               LightingCycleIsStatic(a_weather, RE::TESWeather::ColorType::kEffectLighting);
    }

    template <class Fn>
    void ForEachDALCColor(RE::BGSDirectionalAmbientLightingColors& a_dalc, Fn&& a_fn)
    {
        a_fn(a_dalc.specular);
        a_fn(a_dalc.directional.x.max);
        a_fn(a_dalc.directional.x.min);
        a_fn(a_dalc.directional.y.max);
        a_fn(a_dalc.directional.y.min);
        a_fn(a_dalc.directional.z.max);
        a_fn(a_dalc.directional.z.min);
    }

    void ApplyBrightness(RE::TESWeather* a_weather, const BrightnessSettings a_settings)
    {
        const bool ambientActive = std::abs(a_settings.ambientMultiplier - 1.0) > 0.0001;
        const bool sunlightActive = std::abs(a_settings.sunlightMultiplier - 1.0) > 0.0001;

        for (std::uint32_t time = 0; time < RE::TESWeather::ColorTime::kTotal; ++time)
        {
            if (ambientActive)
            {
                ForEachDALCColor(a_weather->directionalAmbientLightingColors[time], [&](RE::Color& a_color)
                    { MultiplyColor(a_color, a_settings.ambientMultiplier); });
                MultiplyColor(a_weather->colorData[RE::TESWeather::ColorType::kAmbient][time], a_settings.ambientMultiplier);
            }

            if (sunlightActive)
            {
                MultiplyColor(a_weather->colorData[RE::TESWeather::ColorType::kSunlight][time], a_settings.sunlightMultiplier);
                MultiplyColor(a_weather->colorData[RE::TESWeather::ColorType::kEffectLighting][time], a_settings.sunlightMultiplier);
            }
        }
    }

    double ScaleCompressionAnchor(double a_anchor, double a_brightnessMultiplier)
    {
        return ClampAnchor(a_anchor * std::max(0.0, a_brightnessMultiplier));
    }

    void ApplyCompression(RE::TESWeather* a_weather, const Settings a_settings)
    {
        const double ambientCompression = std::clamp(a_settings.compression.ambientCompression / 100.0, -1.0, 1.0);
        const double sunlightCompression = std::clamp(a_settings.compression.sunlightCompression / 100.0, -1.0, 1.0);
        const double ambientAnchor = ScaleCompressionAnchor(a_settings.anchors.get().ambientAnchor, a_settings.brightness.ambientMultiplier);
        const double sunlightAnchor = ScaleCompressionAnchor(a_settings.anchors.get().sunlightAnchor, a_settings.brightness.sunlightMultiplier);
        const bool ambientActive = AmbientCompressionIsActive(a_settings.compression);
        const bool sunlightActive = SunlightCompressionIsActive(a_settings.compression);

        for (std::uint32_t time = 0; time < RE::TESWeather::ColorTime::kTotal; ++time)
        {
            if (ambientActive)
            {
                auto& dalc = a_weather->directionalAmbientLightingColors[time];
                ForEachDALCColor(dalc, [&](RE::Color& a_color)
                    { CompressColor(a_color, ambientCompression, ambientAnchor); });

                a_weather->colorData[RE::TESWeather::ColorType::kAmbient][time] = dalc.directional.z.min;
            }

            if (sunlightActive)
            {
                auto& sunlight = a_weather->colorData[RE::TESWeather::ColorType::kSunlight][time];
                CompressColor(sunlight, sunlightCompression, sunlightAnchor);
                a_weather->colorData[RE::TESWeather::ColorType::kEffectLighting][time] = sunlight;
            }
        }
    }

    void ApplyMoonlight(RE::TESWeather* a_weather, const SaturationSettings a_settings)
    {
        auto& sunlight = a_weather->colorData[RE::TESWeather::ColorType::kSunlight][RE::TESWeather::ColorTime::kNight];
        const auto before = sunlight;
        SaturateColor(sunlight, a_settings.multiplier);
        a_weather->colorData[RE::TESWeather::ColorType::kEffectLighting][RE::TESWeather::ColorTime::kNight] = sunlight;
        logger::info(
            "Moonlight saturation {} night Sunlight {} -> {}",
            WeatherName(a_weather),
            ColorString(before),
            ColorString(sunlight));
    }

    void CaptureBaselineIfNeeded(RE::TESWeather* a_weather)
    {
        auto sta = MPL::Config::StatData::GetSingleton();
        if (sta->weatherBaselines.contains(a_weather))
        {
            return;
        }

        WeatherBaseline baseline;
        for (std::uint32_t time = 0; time < RE::TESWeather::ColorTime::kTotal; ++time)
        {
            baseline.dalc[time] = a_weather->directionalAmbientLightingColors[time];
            baseline.ambient[time] = a_weather->colorData[RE::TESWeather::ColorType::kAmbient][time];
            baseline.sunlight[time] = a_weather->colorData[RE::TESWeather::ColorType::kSunlight][time];
            baseline.effectLighting[time] = a_weather->colorData[RE::TESWeather::ColorType::kEffectLighting][time];
        }
        sta->weatherBaselines.emplace(a_weather, baseline);
    }

    void RestoreBaseline(RE::TESWeather* a_weather)
    {
        auto sta = MPL::Config::StatData::GetSingleton();
        auto it = sta->weatherBaselines.find(a_weather);
        if (it == sta->weatherBaselines.end())
        {
            return;
        }

        const auto baseline = it->second;
        for (std::uint32_t time = 0; time < RE::TESWeather::ColorTime::kTotal; ++time)
        {
            a_weather->directionalAmbientLightingColors[time] = baseline.dalc[time];
            a_weather->colorData[RE::TESWeather::ColorType::kAmbient][time] = baseline.ambient[time];
            a_weather->colorData[RE::TESWeather::ColorType::kSunlight][time] = baseline.sunlight[time];
            a_weather->colorData[RE::TESWeather::ColorType::kEffectLighting][time] = baseline.effectLighting[time];
        }
    }

    std::string WeatherName(const RE::TESWeather* a_weather)
    {
        const auto editorID = a_weather->GetFormEditorID();
        if (editorID && editorID[0] != '\0')
        {
            return editorID;
        }

        return "<no editor ID>";
    }

    std::optional<Settings> LoadSettingsFile(std::filesystem::path a_fileName)
    {
        auto parsed = rfl::json::load<Settings>(a_fileName.string());
        if (parsed.has_value())
        {
            logger::info("Loaded {}", a_fileName.string());
            return std::optional(*parsed);
        }
        return std::nullopt;
    }

    std::string ProfileNameFromKey(const std::string& a_profileName)
    {
        auto profileName = std::filesystem::path(a_profileName).filename().string();
        if (profileName.empty())
        {
            return kDefaultSettingsProfileName;
        }

        const auto extension = std::filesystem::path(profileName).extension().string();
        if (Config::IEquals(extension, ".esp") || Config::IEquals(extension, ".esm") || Config::IEquals(extension, ".esl"))
        {
            profileName = std::filesystem::path(profileName).stem().string();
        }

        return profileName.empty() ? std::string(kDefaultSettingsProfileName) : profileName;
    }

    std::optional<Settings> LoadSettings(std::string& a_sourceFile)
    {
        auto sta = MPL::Config::StatData::GetSingleton();
        if (auto it = sta->settingsCache.find(ProfileNameFromKey(a_sourceFile)); it != sta->settingsCache.end())
        {
            return it->second;
        }

        auto settings = LoadSettingsFile(kSettingsRoot / ProfileNameFromKey(a_sourceFile) / "userSettings.json");
        if (settings.has_value())
        {
            if (auto defaults = LoadSettingsFile(kSettingsRoot / ProfileNameFromKey(a_sourceFile) / "defaultSettings.json"); defaults && !defaults->plugins.empty())
            {
                settings->plugins = defaults->plugins;
            }
            logger::info("Using userSettings.json for {}", ProfileNameFromKey(a_sourceFile));
            sta->settingsCache[ProfileNameFromKey(a_sourceFile)] = *settings;
            return *settings;
        }
        else
        {
            settings = LoadSettingsFile(kSettingsRoot / ProfileNameFromKey(a_sourceFile) / "defaultSettings.json");
            if (settings.has_value())
            {
                logger::info("Using defaultSettings.json defaults for {}", a_sourceFile);
                sta->settingsCache[ProfileNameFromKey(a_sourceFile)] = *settings;
                return *settings;
            }
        }

        return std::nullopt;
    }

    Settings& GetOrCreateSettings(std::string& a_sourceFile)
    {
        auto sta = MPL::Config::StatData::GetSingleton();
        if (auto loaded = LoadSettings(a_sourceFile); loaded.has_value())
        {
            return sta->settingsCache[ProfileNameFromKey(a_sourceFile)];
        }

        auto [it, inserted] = sta->settingsCache.try_emplace(ProfileNameFromKey(a_sourceFile), Settings{});
        if (inserted)
        {
            logger::info("Created default WeatherUtil settings for {}", ProfileNameFromKey(a_sourceFile));
        }
        return it->second;
    }

    std::filesystem::path SettingsWritePath(const std::string& a_profileName)
    {
        return std::filesystem::current_path() / "Data" / "Luma" / "Weather" / ProfileNameFromKey(a_profileName) / "userSettings.json";
    }

    std::vector<std::string> TargetPluginsForProfile(const std::string& a_profileName, const Settings a_settings)
    {
        std::vector<std::string> result;
        for (const auto& pluginName : a_settings.plugins)
        {
            if (!pluginName.empty() && !std::ranges::any_of(result, [&](const std::string& name)
                                           { return Config::IEquals(name, pluginName); }))
            {
                result.push_back(pluginName);
            }
        }

        if (result.empty())
        {
            result.push_back(a_profileName);
        }

        return result;
    }

    SourceWeatherSet* FindWeatherSet(std::unordered_map<std::string, SourceWeatherSet>& a_weatherSets, const std::string& a_sourceFile)
    {
        if (a_weatherSets.contains(a_sourceFile))
        {
            return std::addressof(a_weatherSets[a_sourceFile]);
        }

        return nullptr;
    }

    std::vector<std::string> EnumerateSettingsProfiles(const std::unordered_map<std::string, SourceWeatherSet>& a_weatherSets)
    {
        (void) a_weatherSets;
        std::vector<std::string> result;
        const auto root = std::filesystem::current_path() / "Data" / "Luma" / "Weather";
        std::error_code ec;
        for (const auto& entry : std::filesystem::directory_iterator(root, ec))
        {
            if (ec)
            {
                break;
            }

            if (!entry.is_directory(ec))
            {
                continue;
            }

            const auto settingsPath = entry.path() / "defaultSettings.json";
            const auto userSettingsPath = entry.path() / "userSettings.json";
            if (std::filesystem::is_regular_file(settingsPath, ec) || std::filesystem::is_regular_file(userSettingsPath, ec))
            {
                auto file_name = entry.path().filename().string();
                if (!std::ranges::any_of(result, [&](const std::string& name)
                        { return Config::IEquals(name, file_name); }))
                {
                    result.push_back(file_name);
                }
            }
        }

        if (result.empty())
        {
            result.push_back(kDefaultSettingsProfileName);
        }

        return result;
    }

    std::unordered_map<std::string, SourceWeatherSet> BuildSourceWeatherSets(RE::TESDataHandler* a_dataHandler)
    {
        std::unordered_map<std::string, SourceWeatherSet> result;
        for (auto* weather : a_dataHandler->GetFormArray<RE::TESWeather>())
        {
            if (!weather || weather->sourceFiles.array == nullptr)
            {
                continue;
            }

            const auto* sourceFile = weather->GetFile();
            if (!sourceFile)
            {
                continue;
            }
            std::string sourceName(sourceFile->GetFilename());
            if (!result.contains(sourceName))
            {
                result.emplace(sourceName, SourceWeatherSet{ weather });
            }
            else
            {
                result[sourceName].push_back(weather);
            }
        }
        return result;
    }

    void RefreshCurrentWeatherIfNeeded(bool a_changedCurrentWeather)
    {
        if (!a_changedCurrentWeather)
        {
            return;
        }

        auto* sky = RE::Sky::GetSingleton();
        if (sky && sky->currentWeather)
        {
            sky->ForceWeather(sky->currentWeather, true);
            sky->ReleaseWeatherOverride();
        }
    }

    std::size_t ApplySettingsToWeatherSet(
        const std::string& a_sourceFile,
        const Settings a_settings,
        SourceWeatherSet a_weatherSet,
        bool& a_changedCurrentWeather)
    {
        auto* sky = RE::Sky::GetSingleton();
        std::size_t patched = 0;
        const bool brightnessActive = BrightnessIsActive(a_settings.brightness);
        const bool compressionActive = CompressionIsActive(a_settings.compression);
        const bool moonlightSaturationActive = MoonlightSaturationIsActive(a_settings.saturation);

        for (auto* weather : a_weatherSet)
        {
            if (!weather)
            {
                continue;
            }

            CaptureBaselineIfNeeded(weather);
            RestoreBaseline(weather);

            const bool skipMoonlightSaturation = moonlightSaturationActive && ShouldSkipMoonlightSaturation(weather);
            const bool moonlightSaturationApplied = moonlightSaturationActive && !skipMoonlightSaturation;
            const bool changedWeather = brightnessActive || compressionActive || moonlightSaturationApplied;

            if (!changedWeather)
            {
                if (skipMoonlightSaturation)
                {
                    logger::info(
                        "Skipped moonlight saturation for static weather {} ({:08X}, {:06X}:{})",
                        WeatherName(weather),
                        weather->GetFormID(),
                        weather->GetLocalFormID(),
                        a_sourceFile);
                }
                continue;
            }

            if (brightnessActive)
            {
                ApplyBrightness(weather, a_settings.brightness);
            }
            if (compressionActive)
            {
                ApplyCompression(weather, a_settings);
            }
            if (moonlightSaturationApplied)
            {
                ApplyMoonlight(weather, a_settings.saturation);
            }

            logger::info(
                "Patched weather {} ({:08X}, {:06X}:{}){}{}{}",
                WeatherName(weather),
                weather->GetFormID(),
                weather->GetLocalFormID(),
                a_sourceFile,
                brightnessActive ? " brightness" : "",
                compressionActive ? " compression" : "",
                moonlightSaturationApplied ? std::format(" moonlightSaturation({:.2f}x)", a_settings.saturation.multiplier) : "");

            a_changedCurrentWeather = a_changedCurrentWeather || (sky && sky->currentWeather == weather);
            ++patched;
        }

        return patched;
    }

    void ApplyDataLoaded()
    {
        auto* dataHandler = RE::TESDataHandler::GetSingleton();
        if (!dataHandler)
        {
            logger::warn("TESDataHandler is unavailable; WeatherUtil settings were not applied");
            return;
        }

        auto weatherSets = BuildSourceWeatherSets(dataHandler);
        if (weatherSets.empty())
        {
            logger::info("No weather records found");
            return;
        }

        bool changedCurrentWeather = false;
        std::size_t configsApplied = 0;
        std::size_t weathersApplied = 0;

        for (auto& profileName : EnumerateSettingsProfiles(weatherSets))
        {
            auto settings = LoadSettings(profileName);
            if (!settings)
            {
                continue;
            }

            std::size_t profileWeathersApplied = 0;
            for (const auto& sourceFile : TargetPluginsForProfile(profileName, *settings))
            {
                auto* weatherSet = FindWeatherSet(weatherSets, sourceFile);
                if (!weatherSet)
                {
                    logger::warn("WeatherUtil profile {} targets {}, but no loaded weather records were found", profileName, sourceFile);
                    continue;
                }

                const auto patched = ApplySettingsToWeatherSet(sourceFile, *settings, *weatherSet, changedCurrentWeather);
                profileWeathersApplied += patched;
                weathersApplied += patched;
                logger::info("Applied WeatherUtil profile {} to {} weather(s) from {}", profileName, patched, sourceFile);
            }

            ++configsApplied;
            logger::info("Applied WeatherUtil profile {} to {} total weather(s)", profileName, profileWeathersApplied);
        }

        RefreshCurrentWeatherIfNeeded(changedCurrentWeather);
        logger::info("WeatherUtil applied {} config(s) to {} weather record(s)", configsApplied, weathersApplied);
    }

    void ApplySettings(std::string& a_pluginName)
    {
        auto* dataHandler = RE::TESDataHandler::GetSingleton();
        if (!dataHandler)
        {
            logger::warn("TESDataHandler is unavailable; WeatherUtil settings for {} were not applied", a_pluginName);
            return;
        }

        auto allWeatherSets = BuildSourceWeatherSets(dataHandler);
        auto settings = GetOrCreateSettings(a_pluginName);
        bool changedCurrentWeather = false;
        std::size_t patched = 0;
        for (const auto& sourceFile : TargetPluginsForProfile(a_pluginName, settings))
        {
            auto* weatherSet = FindWeatherSet(allWeatherSets, sourceFile);
            if (!weatherSet)
            {
                logger::warn("WeatherUtil profile {} targets {}, but no loaded weather records were found", a_pluginName, sourceFile);
                continue;
            }

            patched += ApplySettingsToWeatherSet(sourceFile, settings, *weatherSet, changedCurrentWeather);
        }

        RefreshCurrentWeatherIfNeeded(changedCurrentWeather);
        logger::info("WeatherUtil MCM applied {} weather record(s) from profile {}", patched, a_pluginName);
    }

    bool SaveSettings(std::string& a_pluginName)
    {
        auto settings = GetOrCreateSettings(a_pluginName);
        const auto path = SettingsWritePath(a_pluginName);
        std::error_code ec;
        std::filesystem::create_directories(path.parent_path(), ec);
        if (ec)
        {
            logger::warn("Failed to create WeatherUtil settings directory {}: {}", path.parent_path().string(), ec.message());
            return false;
        }
        auto res = rfl::json::save(path.string(), settings, rfl::json::pretty);
        if (!res.has_value())
        {
            logger::warn("Failed to save WeatherUtil settings to {}: {}", path.string(), res.error().what());
            return false;
        }
        logger::info("Saved WeatherUtil settings to {}", path.string());
        return true;
    }

    bool ResetSettingsToDefault(std::string& a_pluginName)
    {
        auto sta = MPL::Config::StatData::GetSingleton();
        auto defaults = LoadSettingsFile(kSettingsRoot / ProfileNameFromKey(a_pluginName) / "defaultSettings.json");
        if (!defaults)
        {
            logger::warn("Cannot reset WeatherUtil settings for {}; settings.json was not found or could not be read", a_pluginName);
            return false;
        }

        sta->settingsCache[ProfileNameFromKey(a_pluginName)] = *defaults;
        logger::info("Reset WeatherUtil settings for {} from settings.json", a_pluginName);
        ApplySettings(a_pluginName);
        return true;
    }

    float GetAmbientMultiplier(std::string& a_pluginName)
    {
        return static_cast<float>(GetOrCreateSettings(a_pluginName).brightness.ambientMultiplier);
    }

    void SetAmbientMultiplier(std::string& a_pluginName, float a_value)
    {
        auto& settings = GetOrCreateSettings(a_pluginName);
        settings.brightness.ambientMultiplier = a_value;
    }

    float GetSunlightMultiplier(std::string& a_pluginName)
    {
        return static_cast<float>(GetOrCreateSettings(a_pluginName).brightness.sunlightMultiplier);
    }

    void SetSunlightMultiplier(std::string& a_pluginName, float a_value)
    {
        auto& settings = GetOrCreateSettings(a_pluginName);
        settings.brightness.sunlightMultiplier = a_value;
    }

    float GetAmbientCompression(std::string& a_pluginName)
    {
        return static_cast<float>(GetOrCreateSettings(a_pluginName).compression.ambientCompression);
    }

    void SetAmbientCompression(std::string& a_pluginName, float a_value)
    {
        auto& settings = GetOrCreateSettings(a_pluginName);
        settings.compression.ambientCompression = a_value;
    }

    float GetSunlightCompression(std::string& a_pluginName)
    {
        return static_cast<float>(GetOrCreateSettings(a_pluginName).compression.sunlightCompression);
    }

    void SetSunlightCompression(std::string& a_pluginName, float a_value)
    {
        auto& settings = GetOrCreateSettings(a_pluginName);
        settings.compression.sunlightCompression = a_value;
    }

    float GetMoonlightSaturationMultiplier(std::string& a_pluginName)
    {
        return static_cast<float>(GetOrCreateSettings(a_pluginName).saturation.multiplier);
    }

    void SetMoonlightSaturationMultiplier(std::string& a_pluginName, float a_value)
    {
        auto& settings = GetOrCreateSettings(a_pluginName);
        settings.saturation.multiplier = a_value;
    }

    std::vector<std::string> GetPresets(std::string& a_pluginName, PresetType preset_type)
    {
        std::vector<std::string> presets;
        for (auto& preset : std::filesystem::directory_iterator(kSettingsRoot / ProfileNameFromKey(a_pluginName) / PresetTypeToString(preset_type)))
        {
            if (preset.is_regular_file() && preset.path().extension() == ".json")
            {
                presets.push_back(preset.path().stem().string());
            }
        }
        return presets;
    }

    void LoadPreset(std::string& a_pluginName, PresetType presetType, std::string& a_presetName)
    {
        auto& settings = GetOrCreateSettings(a_pluginName);
        auto presetPath = kSettingsRoot / ProfileNameFromKey(a_pluginName) / PresetTypeToString(presetType) / std::format("{}.json", a_presetName);
        if (std::filesystem::exists(presetPath))
        {
            switch (presetType)
            {
            case MPL::WeatherPatcher::PresetType::Brightness:
                {
                    auto brightnessResult = rfl::json::load<BrightnessSettings>(presetPath.string(), 0);
                    if (brightnessResult)
                    {
                        logger::info("Loaded brightness preset {} for {}", a_presetName, a_pluginName);
                        settings.brightness = brightnessResult.value();
                    }
                    break;
                }
            case MPL::WeatherPatcher::PresetType::Compression:
                {
                    auto compressionResult = rfl::json::load<CompressionSettings>(presetPath.string(), 0);
                    if (compressionResult)
                    {
                        logger::info("Loaded compression preset {} for {}", a_presetName, a_pluginName);
                        settings.compression = compressionResult.value();
                    }
                    break;
                }
            case MPL::WeatherPatcher::PresetType::Saturation:
                {
                    auto saturationResult = rfl::json::load<SaturationSettings>(presetPath.string(), 0);
                    if (saturationResult)
                    {
                        logger::info("Loaded saturation preset {} for {}", a_presetName, a_pluginName);
                        settings.saturation = saturationResult.value();
                    }
                    break;
                }
            }
        }
        SaveSettings(a_pluginName);
        ApplySettings(a_pluginName);
    }

    void OnSKSEMessage(SKSE::MessagingInterface::Message* a_message)
    {
        if (a_message && a_message->type == SKSE::MessagingInterface::kDataLoaded)
        {
            ApplyDataLoaded();
        }
    }
}  // namespace MPL::WeatherPatcher
