#include <Config.h>
#include <Hooks.h>
#include <Plugin.h>
#include <Papyrus.h>
#include <REL/Version.h>

#ifdef SKYRIM_AE
extern "C" DLLEXPORT auto SKSEPlugin_Version = []()
{
    SKSE::PluginVersionData v;
    v.PluginVersion({ MPL::Plugin::MAJOR, MPL::Plugin::MINOR, MPL::Plugin::PATCH });
    v.PluginName(MPL::Plugin::PROJECT.data());
    v.AuthorName("Mini");
    v.UsesAddressLibrary();
    v.UsesNoStructs();
    return v;
}();
#else

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
    a_info->infoVersion = SKSE::PluginInfo::kVersion;
    a_info->name = MPL::Plugin::PROJECT.data();
    a_info->version = REL::Version(MPL::Plugin::MAJOR, MPL::Plugin::MINOR, MPL::Plugin::PATCH).pack();
    if (a_skse->IsEditor())
    {
        return false;
    }
    auto ver = a_skse->RuntimeVersion();
#    ifdef SKYRIM_VR
    if (ver > SKSE::RUNTIME_VR_1_4_15_1)
#    else
    if (ver > SKSE::RUNTIME_SSE_1_5_97)
#    endif
    {
        return false;
    }
    return true;
};
#endif

void msg_cb(SKSE::MessagingInterface::Message* msg)
{
    auto sta = MPL::Config::StatData::GetSingleton();
    switch (msg->type)
    {
    case SKSE::MessagingInterface::kDataLoaded:
        sta->PostProcess();
        break;
    default:
        break;
    }
}

void Serialize(SKSE::SerializationInterface* ser)
{
    MPL::Config::StatData::GetSingleton()->cellLoad.Save(ser, 'CLCH', 0x1);
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

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
    SKSE::Init(a_skse);
    logger::info("Game version : {}", a_skse->RuntimeVersion().string());
    MPL::Hooks::Install();
    SKSE::GetMessagingInterface()->RegisterListener(msg_cb);
    SKSE::GetPapyrusInterface()->Register(MPL::Papyrus::Bind);
    auto ser = SKSE::GetSerializationInterface();
    ser->SetUniqueID('CLTL');
    ser->SetSaveCallback(Serialize);
    ser->SetLoadCallback(Deserialize);
    ser->SetRevertCallback(Revert);
    return true;
}
