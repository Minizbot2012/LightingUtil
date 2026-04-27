#include <Config.h>
#include <Hooks.h>
namespace MPL::Hooks
{
    struct LoadCELL
    {
        using Target = RE::TESObjectCELL;
        static inline void thunk(Target* a_ref)
        {
            func(a_ref);
            if (a_ref != nullptr && a_ref->sourceFiles.array != nullptr)
            {
#ifndef NDEBUG
                logger::info("Loading Cell {:06X}:{}", a_ref->GetLocalFormID(), a_ref->GetFile(0)->GetFilename());
#endif
                MPL::Config::LoadConfigFormID<MPL::Config::Cell::TESObjectCELL>(a_ref);
            }
        }
        static inline REL::Relocation<decltype(thunk)> func;
        static inline constexpr std::size_t index{ 0x13 };
    };

    struct InitIS
    {
        using Target = RE::TESImageSpace;
        static inline void thunk(Target* a_ref)
        {
            func(a_ref);
            if (a_ref != nullptr && a_ref->sourceFiles.array != nullptr)
            {
#ifndef NDEBUG
                logger::info("Loading IS {:06X}:{}", a_ref->GetLocalFormID(), a_ref->GetFile(0)->GetFilename());
#endif
                MPL::Config::LoadConfigFormID<MPL::Config::ImageSpace::TESImageSpace>(a_ref);
            }
        }
        static inline REL::Relocation<decltype(thunk)> func;
        static inline constexpr std::size_t index{ 0x13 };
    };

    struct InitObjRef
    {
        using Target = RE::TESObjectREFR;
        static inline void thunk(Target* a_ref)
        {
            func(a_ref);
            if (a_ref != nullptr && a_ref->sourceFiles.array != nullptr && Config::TESObjectREFR::IsValid(a_ref))
            {
#ifndef NDEBUG
                logger::info("Loading ObjRef {:06X}:{}", a_ref->GetLocalFormID(), a_ref->GetFile(0)->GetFilename());
#endif
                MPL::Config::LoadConfigFormID<MPL::Config::TESObjectREFR>(a_ref);
            }
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
            if (a_ref != nullptr && a_ref->sourceFiles.array != nullptr)
            {
#ifndef NDEBUG
                logger::info("Loading TMPL {:06X}:{}", a_ref->GetLocalFormID(), a_ref->GetFile(0)->GetFilename());
#endif
                MPL::Config::LoadConfigFormID<MPL::Config::Template::BGSLightingTemplate>(a_ref);
            }
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
            if (cl != nullptr && cl->sourceFiles.array != nullptr)
            {
#ifndef NDEBUG
                logger::info("Player changed cell to {:06X}:{}", cl->GetLocalFormID(), cl->GetFile(0)->GetFilename());
#endif
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
        stl::install_hook<InitObjRef>();
    }
}  // namespace MPL::Hooks
