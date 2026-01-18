#pragma once
namespace MPL::Config::ImageSpace
{
    struct HDR
    {
        using Patch = RE::ImageSpaceBaseData::HDR;
        std::optional<float> eyeAdaptSpeed;
        std::optional<float> bloomBlurRadius;
        std::optional<float> bloomThreshold;
        std::optional<float> bloomScale;
        std::optional<float> receiveBloomThreshold;
        std::optional<float> white;
        std::optional<float> sunlightScale;
        std::optional<float> skyScale;
        std::optional<float> eyeAdaptStrength;
        void Apply(Patch* itm)
        {
            if (this->eyeAdaptSpeed) itm->eyeAdaptSpeed = *this->eyeAdaptSpeed;
            if (this->bloomBlurRadius) itm->bloomBlurRadius = *this->bloomBlurRadius;
            if (this->bloomThreshold) itm->bloomThreshold = *this->bloomThreshold;
            if (this->receiveBloomThreshold) itm->receiveBloomThreshold = *this->receiveBloomThreshold;
            if (this->white) itm->white = *this->white;
            if (this->sunlightScale) itm->sunlightScale = *this->sunlightScale;
            if (this->skyScale) itm->skyScale = *this->skyScale;
            if (this->eyeAdaptStrength) itm->eyeAdaptStrength = *this->eyeAdaptStrength;
        };
        static HDR From(HDR::Patch* itm)
        {
            HDR cpy{
                .eyeAdaptSpeed = itm->eyeAdaptSpeed,
                .bloomBlurRadius = itm->bloomBlurRadius,
                .bloomThreshold = itm->bloomThreshold,
                .bloomScale = itm->bloomScale,
                .receiveBloomThreshold = itm->receiveBloomThreshold,
                .white = itm->white,
                .sunlightScale = itm->sunlightScale,
                .skyScale = itm->skyScale,
                .eyeAdaptStrength = itm->eyeAdaptStrength,
            };
            return cpy;
        }
    };

    struct Cinematic
    {
        using Patch = RE::ImageSpaceBaseData::Cinematic;
        std::optional<float> saturation;
        std::optional<float> brightness;
        std::optional<float> contrast;
        void Apply(Patch* itm)
        {
            if (this->saturation) itm->saturation = *this->saturation;
            if (this->brightness) itm->brightness = *this->brightness;
            if (this->contrast) itm->contrast = *this->contrast;
        };
        static Cinematic From(Patch* itm)
        {
            Cinematic cpy{
                .saturation = itm->saturation,
                .brightness = itm->brightness,
                .contrast = itm->contrast,
            };
            return cpy;
        };
    };

    struct ColorF
    {
        using Patch = RE::ImageSpaceBaseData::Tint::ColorF;
        std::optional<float> red;
        std::optional<float> green;
        std::optional<float> blue;
        void Apply(Patch* itm)
        {
            if (this->red) itm->red = *this->red;
            if (this->green) itm->green = *this->green;
            if (this->blue) itm->blue = *this->blue;
        }
        static ColorF From(Patch* itm)
        {
            ColorF cpy{
                .red = itm->red,
                .green = itm->green,
                .blue = itm->blue,
            };
            return cpy;
        }
    };

    struct Tint
    {
        using Patch = RE::ImageSpaceBaseData::Tint;
        std::optional<float> amount;
        std::optional<ColorF> color;
        void Apply(Patch* itm)
        {
            if (this->amount) itm->amount = *this->amount;
            if (this->color) this->color->Apply(&itm->color);
        }
        static Tint From(Patch* itm)
        {
            Tint cpy{
                .amount = itm->amount,
                .color = ColorF::From(&itm->color),
            };
            return cpy;
        }
    };

    struct DepthOfField
    {
        using Patch = RE::ImageSpaceBaseData::DepthOfField;
        std::optional<float> strength;
        std::optional<float> distance;
        std::optional<float> range;
        void Apply(Patch* itm)
        {
            if (this->strength) itm->strength = *this->strength;
            if (this->distance) itm->distance = *this->distance;
            if (this->range) itm->range = *this->range;
        }
        static DepthOfField From(Patch* itm)
        {
            DepthOfField cpy{
                .strength = itm->strength,
                .distance = itm->distance,
                .range = itm->range,
            };
            return cpy;
        }
    };

    struct ImageSpaceBaseData
    {
        using Patch = RE::ImageSpaceBaseData;
        std::optional<HDR> hdr;
        std::optional<Cinematic> cinematic;
        std::optional<Tint> tint;
        std::optional<DepthOfField> depthOfField;
        void Apply(Patch* itm)
        {
            if (this->hdr) this->hdr->Apply(&itm->hdr);
            if (this->cinematic) this->cinematic->Apply(&itm->cinematic);
            if (this->tint) this->tint->Apply(&itm->tint);
            if (this->depthOfField) this->depthOfField->Apply(&itm->depthOfField);
        }
        static ImageSpaceBaseData From(Patch* itm)
        {
            ImageSpaceBaseData cpy{
                .hdr = HDR::From(&itm->hdr),
                .cinematic = Cinematic::From(&itm->cinematic),
                .tint = Tint::From(&itm->tint),
                .depthOfField = DepthOfField::From(&itm->depthOfField),
            };
            return cpy;
        }
    };

    struct ImageSpace
    {
        static constexpr std::string_view Name = "ImageSpace";
        using Patch = RE::TESImageSpace;
        std::optional<ImageSpaceBaseData> data;
        void Apply(ImageSpace::Patch* itm)
        {
            if (this->data) this->data->Apply(&itm->data);
        }
        static ImageSpace From(ImageSpace::Patch* itm)
        {
            ImageSpace cpy{
                .data = ImageSpaceBaseData::From(&itm->data)
            };
            return cpy;
        }
    };

}  // namespace MPL::Config::ImageSpace
