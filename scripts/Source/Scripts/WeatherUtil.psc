Scriptname WeatherUtil Hidden

Float Function GetAmbientMultiplier(String pluginName) Global Native
Function SetAmbientMultiplier(String pluginName, Float value) Global Native

Float Function GetSunlightMultiplier(String pluginName) Global Native
Function SetSunlightMultiplier(String pluginName, Float value) Global Native

Float Function GetAmbientCompression(String pluginName) Global Native
Function SetAmbientCompression(String pluginName, Float value) Global Native

Float Function GetSunlightCompression(String pluginName) Global Native
Function SetSunlightCompression(String pluginName, Float value) Global Native

Float Function GetMoonlightSaturationMultiplier(String pluginName) Global Native
Function SetMoonlightSaturationMultiplier(String pluginName, Float value) Global Native

Function ApplySettings(String pluginName) Global Native
Function SaveSettings(String pluginName) Global Native
Bool Function ResetSettingsToDefault(String pluginName) Global Native

int Property Brightness = 0 AutoReadOnly
int Property Compression = 1 AutoReadOnly
int Property Saturation = 2 AutoReadOnly

String[] Function GetPresets(String pluginName, int presetType) Global Native
Function LoadPreset(String pluginName, int presetType, String presetName) Global Native
