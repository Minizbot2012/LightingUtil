#pragma once
#include "RE/T/TESRegion.h"
#include <Config.h>
#include <MMSF_API.h>

namespace MPL::Papyrus
{
    inline std::string GetRegion(RE::StaticFunctionTag*, RE::TESObjectCELL* cl)
    {
        if (cl != nullptr)
        {
            auto sta = MPL::Config::StatData::GetSingleton();
            if (sta->g_sm == nullptr)
            {
                sta->g_sm = MPL::API::RequestMMSFAPI();
            }
            if (cl->extraList.HasType<RE::ExtraCellSkyRegion>())
            {
                auto dat = cl->extraList.GetByType<RE::ExtraCellSkyRegion>();
                if (dat->skyRegion != nullptr)
                {
#ifndef NDEBUG
                    logger::info("{:X}:{}", dat->skyRegion->GetLocalFormID(), dat->skyRegion->sourceFiles.array->front()->GetFilename());
#endif
                    return sta->g_sm->LookupEDIDForFormID(dat->skyRegion->formID);
                }
            }
            if (cl->IsExteriorCell())
            {
                auto sky = RE::Sky::GetSingleton();
                if (sky && sky->region)
                {
#ifndef NDEBUG
                    logger::info("{:X}:{}", sky->region->GetLocalFormID(), sky->region->sourceFiles.array->front()->GetFilename());
#endif
                    sta->lastRegion = sky->region;
                    return sta->g_sm->LookupEDIDForFormID(sky->region->formID);
                }
                else if (sta->lastRegion)
                {
                    return sta->g_sm->LookupEDIDForFormID(sta->lastRegion->formID);
                }
            }
        }
        return "";
    }

    inline void ChangeRegion(RE::StaticFunctionTag*, RE::TESObjectCELL* cl, std::string region)
    {
        if (cl != nullptr && !region.empty())
        {
            auto sta = MPL::Config::StatData::GetSingleton();
            if (sta->g_sm == nullptr)
            {
                sta->g_sm = MPL::API::RequestMMSFAPI();
            }
            if (cl->extraList.HasType<RE::ExtraCellSkyRegion>())
            {
                auto dat = cl->extraList.GetByType<RE::ExtraCellSkyRegion>();
                auto skr = sta->g_sm->LookupCachedForm(region)->As<RE::TESRegion>();
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
                auto skr = sta->g_sm->LookupCachedForm(region)->As<RE::TESRegion>();
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

    //REGION: EmittanceUtil

    // If the ref's emittance source is a Region (the standard "interior
    // weather" hookup), the region's cached emittanceColor lags one weather
    // behind on ForceWeather. Copy the live value out of
    // Sky::skyColor[kEffectLighting] (slot 9 — the engine's per-weather
    // "emittance" color, which CommonLib happens to expose under the name
    // kEffectLighting) so UpdateRefLight() picks up the new color.
    static void PatchRegionFromSky(RE::ExtraEmittanceSource* a_extra, RE::Sky* a_sky)
    {
        if (!a_extra || !a_sky)
        {
            return;
        }
        auto* source = a_extra->source;
        if (!source || !source->Is(RE::FormType::Region))
        {
            return;
        }
        auto* region = static_cast<RE::TESRegion*>(source);
        region->emittanceColor = a_sky->skyColor[RE::TESWeather::ColorTypes::kEffectLighting];
        region->currentWeather = a_sky->currentWeather;
    }

    // Calls TESObjectREFR::UpdateRefLight() on every loaded reference in the
    // cell that carries an ExtraEmittanceSource. That is the engine routine
    // which re-reads the emittance source's current effect color and pushes
    // it into the ref's 3D (light bulbs, window statics with an emittance
    // link, etc.).
    //
    // The engine normally only fans this out during a real weather transition
    // tick, so forced/instant weather changes leave emittances stale.
    inline void RefreshCellEmittances(RE::StaticFunctionTag*, RE::TESObjectCELL* a_cell)
    {
        if (!a_cell)
        {
            return;
        }

        auto* sky = RE::Sky::GetSingleton();
        std::uint32_t refreshed = 0;

        a_cell->ForEachReference([&](RE::TESObjectREFR* a_ref)
            {
            if (!a_ref || !a_ref->Is3DLoaded()) {
                return RE::BSContainer::ForEachResult::kContinue;
            }
            auto* extra = a_ref->extraList.GetByType<RE::ExtraEmittanceSource>();
            if (!extra) {
                return RE::BSContainer::ForEachResult::kContinue;
            }
            PatchRegionFromSky(extra, sky);
            a_ref->UpdateRefLight();
            ++refreshed;
            return RE::BSContainer::ForEachResult::kContinue; });

        logger::info("RefreshCellEmittances: cell {:08X} refreshed={}",
            a_cell->GetFormID(), refreshed);
    }

    // Single-ref variant for callers that already know the specific reference
    // they want to refresh (e.g. a window static added at runtime).
    inline void RefreshRefEmittance(RE::StaticFunctionTag*, RE::TESObjectREFR* a_ref)
    {
        if (!a_ref || !a_ref->Is3DLoaded())
        {
            return;
        }
        if (auto* extra = a_ref->extraList.GetByType<RE::ExtraEmittanceSource>())
        {
            PatchRegionFromSky(extra, RE::Sky::GetSingleton());
        }
        a_ref->UpdateRefLight();
    }

    //END REGION: EMITTANCEUTIL

    inline bool Bind(RE::BSScript::IVirtualMachine* vm)
    {
        vm->RegisterFunction("GetRegion", "CLUtil", GetRegion);
        vm->RegisterFunction("ChangeRegion", "CLUtil", ChangeRegion);
        vm->RegisterFunction("RegisterForCellloadForm", "CLUtil", RegisterForOnCellLoadForm);
        vm->RegisterFunction("RegisterForCellloadRef", "CLUtil", RegisterForOnCellLoadAlias);
        vm->RegisterFunction("RegisterForCellloadMgef", "CLUtil", RegisterForOnCellLoadMgef);
        //BEGIN REGION: Emittance Util VM REGISTER
        vm->RegisterFunction("RefreshCellEmittances", "EmittanceUtil", RefreshCellEmittances);
        vm->RegisterFunction("RefreshRefEmittance", "EmittanceUtil", RefreshRefEmittance);
        //END REGION: Emittance Util VM REGISTER
        return true;
    }
}  // namespace MPL::Papyrus
