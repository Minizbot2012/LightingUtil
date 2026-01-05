#include <Config.h>
namespace MPL::Config
{
    void StatData::PostProcess()
    {
        for (auto [ed, id] : this->Templates.modded)
        {
            if (this->Templates.unmodded.contains(ed))
            {
#ifdef DEBUG
                logger::info("Overriding template {}", ed);
#endif
                RE::BGSLightingTemplate* um = RE::TESForm::LookupByID<RE::BGSLightingTemplate>(this->Templates.unmodded.at(ed));
                RE::BGSLightingTemplate* nw = RE::TESForm::LookupByID<RE::BGSLightingTemplate>(this->Templates.modded.at(ed));
                um->data = nw->data;
                um->directionalAmbientLightingColors = nw->directionalAmbientLightingColors;
            }
        }
        this->Templates.modded.clear();
        this->Templates.unmodded.clear();
        for (auto [ed, id] : this->Imagespaces.modded)
        {
            if (this->Imagespaces.unmodded.contains(ed))
            {
#ifdef DEBUG
                logger::info("Overriding imagespace {}", ed);
#endif
                RE::TESImageSpace* um = RE::TESForm::LookupByID<RE::TESImageSpace>(this->Imagespaces.unmodded.at(ed));
                RE::TESImageSpace* nw = RE::TESForm::LookupByID<RE::TESImageSpace>(this->Imagespaces.modded.at(ed));
                um->data = nw->data;
            }
        }
        this->Imagespaces.modded.clear();
        this->Imagespaces.unmodded.clear();
    }
}  // namespace MPL::Config
