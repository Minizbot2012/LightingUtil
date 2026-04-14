#pragma once
#include <ClibUtil/editorID.hpp>
#include <Config.h>
namespace MPL::Papyrus
{
    inline std::string GetRegion(RE::StaticFunctionTag*, RE::TESObjectCELL* cl)
    {
        if (cl != nullptr)
        {
            if (cl->extraList.HasType<RE::ExtraCellSkyRegion>())
            {
                auto dat = cl->extraList.GetByType<RE::ExtraCellSkyRegion>();
                if (dat->skyRegion != nullptr)
                {
#ifdef DEBUG
                    logger::info("{:X}:{}", dat->skyRegion->GetLocalFormID(), dat->skyRegion->sourceFiles.array->front()->GetFilename());
#endif
                    return clib_util::editorID::get_editorID(dat->skyRegion);
                }
            }
            if (cl->IsExteriorCell())
            {
                auto sta = MPL::Config::StatData::GetSingleton();
                auto sky = RE::Sky::GetSingleton();
                if (sky && sky->region)
                {
#ifdef DEBUG
                    logger::info("{:X}:{}", sky->region->GetLocalFormID(), sky->region->sourceFiles.array->front()->GetFilename());
#endif
                    sta->lastRegion = sky->region;
                    return clib_util::editorID::get_editorID(sky->region);
                }
                else if (sta->lastRegion)
                {
                    return clib_util::editorID::get_editorID(sta->lastRegion);
                }
            }
        }
        return "";
    }

    inline void ChangeRegion(RE::StaticFunctionTag*, RE::TESObjectCELL* cl, std::string region)
    {
        if (cl != nullptr && !region.empty())
        {
            if (cl->extraList.HasType<RE::ExtraCellSkyRegion>())
            {
                auto dat = cl->extraList.GetByType<RE::ExtraCellSkyRegion>();
                auto skr = RE::TESForm::LookupByEditorID<RE::TESRegion>(region);
                if (skr != nullptr)
                {
                    dat->skyRegion = skr;
                    logger::info("{:X}:{}", dat->skyRegion->GetLocalFormID(), dat->skyRegion->sourceFiles.array->front()->GetFilename());
                }
                else
                {
                    logger::warn("Sky region for {} IS nullptr", region);
                }
            }
            else
            {
                auto skr = RE::TESForm::LookupByEditorID<RE::TESRegion>(region);
                if (skr != nullptr)
                {
                    auto dat = RE::BSExtraData::Create<RE::ExtraCellSkyRegion>();
                    dat->skyRegion = skr;
                    logger::info("Adding sky region {:X}:{}", dat->skyRegion->GetLocalFormID(), dat->skyRegion->sourceFiles.array->front()->GetFilename());
                    cl->extraList.Add(dat);
                }
                else
                {
                    logger::warn("Sky region for {} IS nullptr", region);
                }
            }
        }
    }
    inline void RegisterForOnCellLoadForm(RE::StaticFunctionTag*, RE::TESForm* listener)
    {
        MPL::Config::StatData::GetSingleton()->cellLoad.Register(listener);
    }
    inline void RegisterForOnCellLoadAlias(RE::StaticFunctionTag*, RE::BGSRefAlias* listener)
    {
        MPL::Config::StatData::GetSingleton()->cellLoad.Register(listener);
    }
    inline void RegisterForOnCellLoadMgef(RE::StaticFunctionTag*, RE::ActiveEffect* listener)
    {
        MPL::Config::StatData::GetSingleton()->cellLoad.Register(listener);
    }
    inline bool Bind(RE::BSScript::IVirtualMachine* vm)
    {
        vm->RegisterFunction("GetRegion", "CLUtil", GetRegion);
        vm->RegisterFunction("ChangeRegion", "CLUtil", ChangeRegion);
        vm->RegisterFunction("RegisterForCellloadForm", "CLUtil", RegisterForOnCellLoadForm);
        vm->RegisterFunction("RegisterForCellloadRef", "CLUtil", RegisterForOnCellLoadAlias);
        vm->RegisterFunction("RegisterForCellloadMgef", "CLUtil", RegisterForOnCellLoadMgef);
        return true;
    }
}  // namespace MPL::Papyrus
