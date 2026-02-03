#include <ClibUtil/editorID.hpp>
#include <Hooking.h>
#include <Hooks.h>
namespace MPL::Hooks
{
    struct LoadLT
    {
        using Target = RE::BGSLightingTemplate;
        static inline void thunk(Target* a_ref, RE::TESFile* a_mod)
        {
            func(a_ref, a_mod);
            std::string sum(a_mod->summary.c_str());
            auto sta = MPL::Config::StatData::GetSingleton();
            if (sum.contains("[Luma]"))
            {
                sta->Templates.modded.insert(std::make_pair(a_ref->GetFormEditorID(), a_ref->GetFormID()));
#ifdef DEBUG
                logger::info("Loaded Modded Template {} {}:{:06X}", a_ref->GetFormEditorID(), a_mod->GetFilename(), a_ref->GetLocalFormID());
#endif
            }
            else
            {
                if (!sta->Templates.unmodded.contains(a_ref->GetFormEditorID()))
                {
                    sta->Templates.unmodded.insert(std::make_pair(a_ref->GetFormEditorID(), a_ref->GetFormID()));
#ifdef DEBUG
                    logger::info("Loaded Unmodded Template {} {}:{:06X}", a_ref->GetFormEditorID(), a_mod->GetFilename(), a_ref->GetLocalFormID());
#endif
                }
            }
        }
        static inline REL::Relocation<decltype(thunk)> func;
        static inline constexpr std::size_t index{ 0x6 };
    };

    struct LoadIMG
    {
        using Target = RE::TESImageSpace;
        static inline void thunk(Target* a_ref, RE::TESFile* a_mod)
        {
            func(a_ref, a_mod);
            std::string sum(a_mod->summary.c_str());
            auto sta = MPL::Config::StatData::GetSingleton();
            if (sum.contains("[Luma]"))
            {
                sta->Imagespaces.modded.insert(std::make_pair(clib_util::editorID::get_editorID(a_ref), a_ref->GetFormID()));
#ifdef DEBUG
                logger::info("Loaded Modded Imagespace {} {}:{:06X}", clib_util::editorID::get_editorID(a_ref), a_mod->GetFilename(), a_ref->GetLocalFormID());
#endif
            }
            else
            {
                if (!sta->Imagespaces.unmodded.contains(clib_util::editorID::get_editorID(a_ref)))
                {
                    sta->Imagespaces.unmodded.insert(std::make_pair(clib_util::editorID::get_editorID(a_ref), a_ref->GetFormID()));
#ifdef DEBUG
                    logger::info("Loaded Unmodded Imagespace {} {}:{:06X}", clib_util::editorID::get_editorID(a_ref), a_mod->GetFilename(), a_ref->GetLocalFormID());
#endif
                }
            }
        }
        static inline REL::Relocation<decltype(thunk)> func;
        static inline constexpr std::size_t index{ 0x6 };
    };

    struct LoadCELL
    {
        using Target = RE::TESObjectCELL;
        static inline void thunk(Target* a_ref, RE::TESFile* a_mod)
        {
            func(a_ref, a_mod);
#ifdef DEBUG
            logger::info("{}:{:08X}", a_ref->GetFile(0)->GetFilename(), a_ref->GetFormID());
#endif
            auto sta = MPL::Config::StatData::GetSingleton();
            if (!sta->Cells.loaded.contains(a_ref->GetFormID()))
            {
                sta->Cells.loaded.insert(std::make_pair(a_ref->GetFormID(), a_mod->GetFilename()));
            }
        }
        static inline REL::Relocation<decltype(thunk)> func;
        static inline constexpr std::size_t index{ 0x6 };
    };

    void Install()
    {
        stl::install_hook<LoadLT>();
        stl::install_hook<LoadIMG>();
        stl::install_hook<LoadCELL>();
    }
}  // namespace MPL::Hooks
