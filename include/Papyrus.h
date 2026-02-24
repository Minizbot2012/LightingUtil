#pragma once
#include <ClibUtil/editorID.hpp>
#include <Config.h>
namespace MPL::Papyrus
{
    std::string GetRegion(RE::StaticFunctionTag*, RE::TESObjectCELL* cl)
    {
        if (cl != nullptr)
        {
            if (cl->extraList.HasType<RE::ExtraCellSkyRegion>())
            {
                auto dat = cl->extraList.GetByType<RE::ExtraCellSkyRegion>();
                if (dat->skyRegion != nullptr)
                {
                    logger::info("{:X}:{}", dat->skyRegion->GetLocalFormID(), dat->skyRegion->sourceFiles.array->front()->GetFilename());
                    return clib_util::editorID::get_editorID(dat->skyRegion);
                }
            }
        }
        return "";
    }

    void ChangeRegion(RE::StaticFunctionTag*, RE::TESObjectCELL* cl, std::string region)
    {
        if (cl != nullptr)
        {
            if (cl->extraList.HasType<RE::ExtraCellSkyRegion>())
            {
                auto dat = cl->extraList.GetByType<RE::ExtraCellSkyRegion>();
                dat->skyRegion = RE::TESForm::LookupByEditorID<RE::TESRegion>(region);
                if (dat->skyRegion != nullptr)
                {
                    logger::info("{:X}:{}", dat->skyRegion->GetLocalFormID(), dat->skyRegion->sourceFiles.array->front()->GetFilename());
                }
                else
                {
                    logger::warn("Sky region for {} IS NULL", region);
                }
            }
            else
            {
                auto dat = RE::BSExtraData::Create<RE::ExtraCellSkyRegion>();
                dat->skyRegion = RE::TESForm::LookupByEditorID<RE::TESRegion>(region);
                if (dat->skyRegion != nullptr)
                {
                    logger::info("Adding sky region {:X}:{}", dat->skyRegion->GetLocalFormID(), dat->skyRegion->sourceFiles.array->front()->GetFilename());
                    cl->extraList.Add(dat);
                }
                else
                {
                    logger::warn("Sky region for {} IS NULL", region);
                }
            }
        }
    }
    void RegisterForOnCellLoadForm(RE::StaticFunctionTag*, RE::TESForm* listener)
    {
        MPL::Config::StatData::GetSingleton()->cellLoad.Register(listener);
    }
    void RegisterForOnCellLoadAlias(RE::StaticFunctionTag*, RE::BGSRefAlias* listener)
    {
        MPL::Config::StatData::GetSingleton()->cellLoad.Register(listener);
    }
    void RegisterForOnCellLoadMgef(RE::StaticFunctionTag*, RE::ActiveEffect* listener)
    {
        MPL::Config::StatData::GetSingleton()->cellLoad.Register(listener);
    }
    bool Bind(RE::BSScript::IVirtualMachine* vm)
    {
        vm->RegisterFunction("GetRegion", "CLUtil", GetRegion);
        vm->RegisterFunction("ChangeRegion", "CLUtil", ChangeRegion);
        vm->RegisterFunction("RegisterForCellloadForm", "CLUtil", RegisterForOnCellLoadForm);
        vm->RegisterFunction("RegisterForCellloadRef", "CLUtil", RegisterForOnCellLoadAlias);
        vm->RegisterFunction("RegisterForCellloadMgef", "CLUtil", RegisterForOnCellLoadMgef);
        return true;
    }
}  // namespace MPL::Papyrus
