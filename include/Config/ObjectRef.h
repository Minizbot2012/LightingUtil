#pragma once
#include <Config.h>
#include <optional>
namespace MPL::Config
{
    struct ExtraRoomRefData
    {
        using Patch = RE::ExtraRoomRefData;
        using TopLevel = RE::TESObjectREFR;
        std::optional<MPL::Config::LiteForm> imageSpace;
        std::optional<MPL::Config::LiteForm> lightingTemplate;
        void Apply(Patch* itm)
        {
            if (this->imageSpace) itm->data->imageSpace = this->imageSpace->Get<RE::TESImageSpace>();
            if (this->lightingTemplate) itm->data->lightingTemplate = this->lightingTemplate->Get<RE::BGSLightingTemplate>();
        }
        static ExtraRoomRefData From(Patch* itm)
        {
            ExtraRoomRefData cpy;
            if (itm->data->imageSpace) cpy.imageSpace = MPL::Config::LiteForm::FromID(cpy.imageSpace->formID);
            if (itm->data->lightingTemplate) cpy.lightingTemplate = MPL::Config::LiteForm::FromID(cpy.imageSpace->formID);
            return cpy;
        }
        static bool IsValid(TopLevel* itm) {
            return itm->data.objectReference->formID = 0x15;
        }
    };

    // This is initially for setting up RoomBounds / Templates
    struct TESObjectREFR
    {
        using Patch = RE::TESObjectREFR;
        static constexpr std::string_view Name = "ObjectReference";
        std::optional<ExtraRoomRefData> roomBound;
        void Apply(Patch* itm)
        {
            if (ExtraRoomRefData::IsValid(itm))
            {
                if (itm->extraList.HasType<RE::ExtraRoomRefData>())
                {
                    auto rrd = itm->extraList.GetByType<RE::ExtraRoomRefData>();
                    this->roomBound->Apply(rrd);
                }
                else
                {
                    auto erd = RE::ExtraRoomRefData::Create<RE::ExtraRoomRefData>();
                    this->roomBound->Apply(erd);
                    itm->extraList.Add(erd);
                }
            }
        }
        static TESObjectREFR From(Patch* itm)
        {
            TESObjectREFR cpy;
            if (ExtraRoomRefData::IsValid(itm) && itm->extraList.HasType<RE::ExtraRoomRefData>())
            {
                cpy.roomBound = ExtraRoomRefData::From(itm->extraList.GetByType<RE::ExtraRoomRefData>());
            }
            return cpy;
        }
        static bool IsValid(Patch* itm) {
            return itm->data.objectReference != nullptr && (ExtraRoomRefData::IsValid(itm));
        }
    };
}  // namespace MPL::Config
