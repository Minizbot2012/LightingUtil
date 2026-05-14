#include <Config.h>
#include <Hooks.h>
#include <Papyrus.h>
#include <Plugin.h>
#include <REL/Version.h>
#include <SKSE/API.h>
void Serialize(SKSE::SerializationInterface* ser)
{
    MPL::Config::StatData::GetSingleton()->cellLoad.Save(ser, 'CLCH', 1);
}

void Deserialize(SKSE::SerializationInterface* ser)
{
    uint32_t type;
    uint32_t version;
    uint32_t len;
    while (ser->GetNextRecordInfo(type, version, len))
    {
        switch (type)
        {
        case 'CLCH':
            MPL::Config::StatData::GetSingleton()->cellLoad.Load(ser);
            break;
        default:
            break;
        }
    }
}

void Revert(SKSE::SerializationInterface* ser)
{
    MPL::Config::StatData::GetSingleton()->cellLoad.Revert(ser);
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
    SKSE::Init(a_skse);
    logger::info("Game version : {}", a_skse->RuntimeVersion().string());
    MPL::Hooks::Install();
    SKSE::GetPapyrusInterface()->Register(MPL::Papyrus::Bind);
    auto ser = SKSE::GetSerializationInterface();
    ser->SetUniqueID('LUMA');
    ser->SetSaveCallback(Serialize);
    ser->SetLoadCallback(Deserialize);
    ser->SetRevertCallback(Revert);
    return true;
};
SKSEPluginInfo(
    .Version = REL::Version{ MPL::Plugin::MAJOR, MPL::Plugin::MINOR, MPL::Plugin::PATCH, 0 },
    .Name = "LumaUtil"sv,
    .Author = "mini"sv,
    .SupportEmail = ""sv,
    .StructCompatibility = SKSE::StructCompatibility::Independent,
    .RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary
)
