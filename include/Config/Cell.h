#pragma once
#include <Config/Templates.h>
#include <optional>
namespace MPL::Config::Cell
{
    struct Flags
    {
        using Patch = REX::EnumSet<RE::TESObjectCELL::Flag, uint16_t>;
        std::optional<bool> IsInteriorCell;
        std::optional<bool> HasWater;
        std::optional<bool> CanTravelFromHere;
        std::optional<bool> NoLODWater;
        std::optional<bool> HasTempData;
        std::optional<bool> PublicArea;
        std::optional<bool> HandChanged;
        std::optional<bool> ShowSky;
        std::optional<bool> UsesSkyLighting;
        std::optional<bool> WarnToLeave;
        std::optional<bool> Unknown9;
        std::optional<bool> Unknown10;
        std::optional<bool> Unknown11;
        std::optional<bool> Unknown12;
        std::optional<bool> Unknown13;
        std::optional<bool> Unknown14;
        std::optional<bool> SunlightShadows;
        void Apply(Patch* itm)
        {
            if (IsInteriorCell) itm->set(*this->IsInteriorCell, RE::TESObjectCELL::Flag::kIsInteriorCell);
            if (HasWater) itm->set(*this->HasWater, RE::TESObjectCELL::Flag::kHasWater);
            if (CanTravelFromHere) itm->set(*this->CanTravelFromHere, RE::TESObjectCELL::Flag::kCanTravelFromHere);
            if (NoLODWater) itm->set(*this->NoLODWater, RE::TESObjectCELL::Flag::kNoLODWater);
            if (HasTempData) itm->set(*this->HasTempData, RE::TESObjectCELL::Flag::kHasTempData);
            if (PublicArea) itm->set(*this->PublicArea, RE::TESObjectCELL::Flag::kPublicArea);
            if (HandChanged) itm->set(*this->HandChanged, RE::TESObjectCELL::Flag::kHandChanged);
            if (ShowSky) itm->set(*this->ShowSky, RE::TESObjectCELL::Flag::kShowSky);
            if (UsesSkyLighting) itm->set(*this->UsesSkyLighting, RE::TESObjectCELL::Flag::kUseSkyLighting);
            if (WarnToLeave) itm->set(*this->WarnToLeave, RE::TESObjectCELL::Flag::kWarnToLeave);
            if (Unknown10) itm->set(*this->Unknown10, static_cast<RE::TESObjectCELL::Flag>(1 << 10));
            if (Unknown11) itm->set(*this->Unknown11, static_cast<RE::TESObjectCELL::Flag>(1 << 11));
            if (Unknown12) itm->set(*this->Unknown12, static_cast<RE::TESObjectCELL::Flag>(1 << 12));
            if (Unknown13) itm->set(*this->Unknown13, static_cast<RE::TESObjectCELL::Flag>(1 << 13));
            if (Unknown14) itm->set(*this->Unknown14, static_cast<RE::TESObjectCELL::Flag>(1 << 14));
            if (SunlightShadows) itm->set(*this->SunlightShadows, static_cast<RE::TESObjectCELL::Flag>(1 << 15));
        }
        static Flags From(Patch* itm)
        {
            Flags cpy{
                .IsInteriorCell = (*itm & RE::TESObjectCELL::Flag::kIsInteriorCell).underlying() != 0,
                .HasWater = (*itm & RE::TESObjectCELL::Flag::kHasWater).underlying() != 0,
                .CanTravelFromHere = (*itm & RE::TESObjectCELL::Flag::kCanTravelFromHere).underlying() != 0,
                .NoLODWater = (*itm & RE::TESObjectCELL::Flag::kNoLODWater).underlying() != 0,
                .HasTempData = (*itm & RE::TESObjectCELL::Flag::kHasTempData).underlying() != 0,
                .PublicArea = (*itm & RE::TESObjectCELL::Flag::kPublicArea).underlying() != 0,
                .HandChanged = (*itm & RE::TESObjectCELL::Flag::kHandChanged).underlying() != 0,
                .ShowSky = (*itm & RE::TESObjectCELL::Flag::kShowSky).underlying() != 0,
                .UsesSkyLighting = (*itm & RE::TESObjectCELL::Flag::kUseSkyLighting).underlying() != 0,
                .WarnToLeave = (*itm & RE::TESObjectCELL::Flag::kWarnToLeave).underlying() != 0,
                .Unknown10 = (itm->underlying() & 1 << 10) != 0,
                .Unknown11 = (itm->underlying() & 1 << 11) != 0,
                .Unknown12 = (itm->underlying() & 1 << 12) != 0,
                .Unknown13 = (itm->underlying() & 1 << 13) != 0,
                .Unknown14 = (itm->underlying() & 1 << 14) != 0,
                .SunlightShadows = (itm->underlying() & 1 << 15) != 0,
            };
            return cpy;
        };
    };
    struct Cell
    {
        static constexpr std::string_view Name = "Cell";
        using Patch = RE::TESObjectCELL;
        std::optional<Flags> flags;
        std::optional<std::string> skylight;
        std::optional<MPL::Config::Template::INTERIOR_DATA> lighting;
        std::optional<std::string> lightTemplate;
        std::optional<std::string> imagespace;
        void Apply(Patch* itm)
        {
            if (flags) this->flags->Apply(&itm->cellFlags);
            if (this->skylight)
            {
                RE::ExtraCellSkyRegion* sky = nullptr;
                if (itm->extraList.HasType<RE::ExtraCellSkyRegion>())
                {
                    sky = itm->extraList.GetByType<RE::ExtraCellSkyRegion>();
                }
                else
                {
                    sky = RE::BSExtraData::Create<RE::ExtraCellSkyRegion>();
                    itm->extraList.Add(sky);
                }
                auto spos = this->skylight->find(":");
                if (spos != std::string::npos)
                {
                    sky->skyRegion = RE::TESForm::LookupByID<RE::TESRegion>(RE::TESDataHandler::GetSingleton()->LookupFormID(strtoul(this->skylight->substr(0, spos).c_str(), nullptr, 16), this->skylight->substr(spos + 1).c_str()));
                }
                else
                {
                    sky->skyRegion = RE::TESForm::LookupByEditorID<RE::TESRegion>(*this->skylight);
                }
            }
            if (this->lighting && itm->IsInteriorCell()) this->lighting->Apply(itm->cellData.interior);
            if (this->lightTemplate)
            {
                auto spos = this->lightTemplate->find(":");
                if (spos != std::string::npos)
                {
                    itm->lightingTemplate = RE::TESForm::LookupByID<RE::BGSLightingTemplate>(RE::TESDataHandler::GetSingleton()->LookupFormID(strtoul(this->lightTemplate->substr(0, spos).c_str(), nullptr, 16), this->lightTemplate->substr(spos + 1).c_str()));
                }
                else
                {
                    itm->lightingTemplate = RE::TESForm::LookupByEditorID<RE::BGSLightingTemplate>(*this->lightTemplate);
                }
            }
            if (this->imagespace)
            {
                RE::ExtraCellImageSpace* is;
                if (itm->extraList.HasType<RE::ExtraCellImageSpace>())
                {
                    is = itm->extraList.GetByType<RE::ExtraCellImageSpace>();
                }
                else
                {
                    is = RE::BSExtraData::Create<RE::ExtraCellImageSpace>();
                    itm->extraList.Add(is);
                }
                auto spos = this->imagespace->find(":");
                if (spos != std::string::npos)
                {
                    is->imageSpace = RE::TESForm::LookupByID<RE::TESImageSpace>(RE::TESDataHandler::GetSingleton()->LookupFormID(strtoul(this->imagespace->substr(0, spos).c_str(), nullptr, 16), this->imagespace->substr(spos + 1).c_str()));
                }
                else
                {
                    is->imageSpace = RE::TESForm::LookupByEditorID<RE::TESImageSpace>(*this->imagespace);
                }
            }
        }
        static Cell From(Patch* itm)
        {
            Cell cpy{
                .flags = Flags::From(&itm->cellFlags),
                .skylight = std::nullopt,
            };
            if (itm->IsInteriorCell())
            {
                cpy.lighting = Template::INTERIOR_DATA::From(itm->cellData.interior);
                cpy.lighting->ambient->alpha = {};
                cpy.lighting->fogColorFar->alpha = {};
                cpy.lighting->fogColorNear->alpha = {};
                cpy.lighting->directional->alpha = {};
            }
            if (itm->lightingTemplate != NULL)
            {
                cpy.lightTemplate = std::format("{:06X}:{}", itm->lightingTemplate->GetLocalFormID(), itm->lightingTemplate->GetFile(0)->GetFilename());
            }
            if (itm->extraList.HasType<RE::ExtraCellImageSpace>())
            {
                auto* dat = itm->extraList.GetByType<RE::ExtraCellImageSpace>();
                if (dat->imageSpace != NULL)
                    cpy.imagespace = std::format("{:06X}:{}", dat->imageSpace->GetLocalFormID(), dat->imageSpace->GetFile(0)->GetFilename());
            }
            if (itm->extraList.HasType<RE::ExtraCellSkyRegion>())
            {
                auto* dat = itm->extraList.GetByType<RE::ExtraCellSkyRegion>();
                if (dat->skyRegion != NULL)
                    cpy.skylight = std::format("{:06X}:{}", dat->skyRegion->GetLocalFormID(), dat->skyRegion->GetFile(0)->GetFilename());
            }
            return cpy;
        }
    };
}  // namespace MPL::Config::Cell
