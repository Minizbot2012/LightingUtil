#include <WeatherPatcher.h>
#include <Hooking.h>
#include <Config.h>
#include <Hooks.h>
namespace MPL::Hooks
{
    struct InitCell
    {
        using Target = RE::TESObjectCELL;
        static inline constexpr VariantIndex index = VariantIndex(0x13);
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
        static void post_hook()
        {
            logger::info("Installed InitCell Hook");
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct InitIS
    {
        using Target = RE::TESImageSpace;
        static inline constexpr VariantIndex index = VariantIndex(0x13);
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
        static void post_hook()
        {
            logger::info("Installed InitImagespace Hook");
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct InitREFR
    {
        using Target = RE::TESObjectREFR;
        static inline constexpr VariantIndex index = VariantIndex(0x13);
        static inline void thunk(Target* a_ref)
        {
            func(a_ref);
            if (a_ref != nullptr && a_ref->sourceFiles.array != nullptr)
            {
#ifndef NDEBUG
                logger::info("Loading ObjRef {:06X}:{}", a_ref->GetLocalFormID(), a_ref->GetFile(0)->GetFilename());
#endif
                MPL::Config::LoadConfigFormID<MPL::Config::TESObjectREFR>(a_ref);
            }
        }
        static void post_hook()
        {
            logger::info("Installed InitREFR Hook");
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct InitTMPL
    {
        using Target = RE::BGSLightingTemplate;
        static inline constexpr VariantIndex index = VariantIndex(0x13);
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
        static void post_hook()
        {
            logger::info("Installed InitTempl Hook");
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct InitLGHT
    {
        using Target = RE::TESObjectLIGH;
        static inline constexpr VariantIndex index = VariantIndex(0x13);
        static inline void thunk(Target* a_ref)
        {
            func(a_ref);
            if (a_ref != nullptr && a_ref->sourceFiles.array != nullptr)
            {
#ifndef NDEBUG
                logger::info("Loading Light {:06X}:{}", a_ref->GetLocalFormID(), a_ref->GetFile(0)->GetFilename());
#endif
                MPL::Config::LoadConfigFormID<MPL::Config::TESObjectLIGH>(a_ref);
            }
        }
        static void post_hook()
        {
            logger::info("Installed InitLGHT Hook");
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct InitWorldspace
    {
        using Target = RE::TESWorldSpace;
        static inline constexpr VariantIndex index = VariantIndex(0x13);
        static inline void thunk(Target* a_ref)
        {
            func(a_ref);
            if (a_ref != nullptr && a_ref->sourceFiles.array != nullptr && !a_ref->sourceFiles.array->empty())
            {
#ifndef NDEBUG
                logger::info("Loading WorldSpace {:06X}:{}", a_ref->GetLocalFormID(), a_ref->GetFile(0)->GetFilename());
#endif
                Config::LoadConfigFormID<MPL::Config::Worldspace::TESWorldSpace>(a_ref);
            }
        }
        static void post_hook()
        {
            logger::info("Installed InitWorldspace Hook");
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct CellChange
    {
        using Target = RE::PlayerCharacter;
        static inline constexpr VariantIndex index = VariantIndex(0x98, 0x98, 0x99);
        static inline void thunk(RE::PlayerCharacter* a_ref, const RE::TESObjectCELL* cl)
        {
            func(a_ref, cl);
            if (cl != nullptr)
            {
                MPL::Config::StatData::GetSingleton()->cellLoad.QueueEvent(cl);
            }
        }
        static void post_hook()
        {
            logger::info("Installed CellChange Hook");
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
    void Install()
    {
        stl::install_hook<InitCell>();
        stl::install_hook<InitIS>();
        stl::install_hook<InitREFR>();
        stl::install_hook<InitTMPL>();
        stl::install_hook<InitLGHT>();
        stl::install_hook<InitWorldspace>();
        stl::install_hook<CellChange>();
    }
}  // namespace MPL::Hooks
