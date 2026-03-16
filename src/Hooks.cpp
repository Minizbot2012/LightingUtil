#include <Config.h>
#include <Config/ImageSpace.h>
#include <Config/Templates.h>
#include <Config/Cell.h>
#include <Hooking.h>
#include <Hooks.h>
#include <RE/B/BGSLightingTemplate.h>
#include <RE/T/TESImageSpace.h>
namespace MPL::Hooks
{
    struct LoadCELL
    {
        using Target = RE::TESObjectCELL;
        static inline bool thunk(Target* a_ref, RE::TESFile* a_mod)
        {
            auto res = func(a_ref, a_mod);
#ifdef DEBUG
            logger::info("Loading Cell {}:{:06X}", a_ref->GetFile(0)->GetFilename(), a_ref->GetLocalFormID());
#endif
            MPL::Config::LoadConfigEDID<MPL::Config::Cell::Cell>(a_ref);
            MPL::Config::LoadConfigFormID<MPL::Config::Cell::Cell>(a_ref);
            return res;
        }
        static inline REL::Relocation<decltype(thunk)> func;
        static inline constexpr std::size_t index{ 0x6 };
    };

    struct InitIS
    {
        using Target = RE::TESImageSpace;
        static inline void thunk(Target* a_ref)
        {
            func(a_ref);
#ifdef DEBUG
            logger::info("Loading IS {}:{:06X}", a_ref->GetFile(0)->GetFilename(), a_ref->GetLocalFormID());
#endif
            MPL::Config::LoadConfigEDID<MPL::Config::ImageSpace::ImageSpace>(a_ref);
            MPL::Config::LoadConfigFormID<MPL::Config::ImageSpace::ImageSpace>(a_ref);
        }
        static inline REL::Relocation<decltype(thunk)> func;
        static inline constexpr std::size_t index{ 0x13 };
    };

    struct InitTMPL
    {
        using Target = RE::BGSLightingTemplate;
        static inline void thunk(Target* a_ref)
        {
            func(a_ref);
#ifdef DEBUG
            logger::info("Loading TMPL {}:{:06X}", a_ref->GetFile(0)->GetFilename(), a_ref->GetLocalFormID());
#endif
            MPL::Config::LoadConfigEDID<MPL::Config::Template::LightingTemplate>(a_ref);
            MPL::Config::LoadConfigFormID<MPL::Config::Template::LightingTemplate>(a_ref);
        }
        static inline REL::Relocation<decltype(thunk)> func;
        static inline constexpr std::size_t index{ 0x13 };
    };

    struct InitCell
    {
        using Target = RE::PlayerCharacter;
        static inline void thunk(Target* a_ref, RE::TESObjectCELL* cl)
        {
            func(a_ref, cl);
            if (cl != nullptr)
            {
                auto* sta = MPL::Config::StatData::GetSingleton();
                sta->cellLoad.QueueEvent(cl);
            }
        }
        static inline REL::Relocation<decltype(thunk)> func;
        static inline constexpr std::size_t index{ 0x98 };
    };
    void Install()
    {
        stl::install_hook<LoadCELL>();
        stl::install_hook<InitCell>();
        stl::install_hook<InitIS>();
        stl::install_hook<InitTMPL>();
    }
}  // namespace MPL::Hooks
