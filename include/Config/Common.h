#pragma once
#include <Config/Forms.h>
#include <locale>
#include <rfl.hpp>

namespace MPL::Config
{
    struct ColorF
    {
        std::optional<uint32_t> red;
        std::optional<uint32_t> green;
        std::optional<uint32_t> blue;
        using Patch = RE::ImageSpaceBaseData::Tint::ColorF;
        void Apply(Patch* itm)
        {
            if (this->red) itm->red = ((float)*this->red)/255.0;
            if (this->green) itm->green = ((float)*this->green)/255.0;
            if (this->blue) itm->blue = ((float)*this->blue/255.0);
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

    struct Color
    {
        std::optional<uint8_t> alpha;
        std::optional<uint8_t> red;
        std::optional<uint8_t> green;
        std::optional<uint8_t> blue;
        using Patch = RE::Color;
        void Apply(Patch* itm)
        {
            if (this->alpha) itm->alpha = *this->alpha;
            if (this->red) itm->red = *this->red;
            if (this->green) itm->green = *this->green;
            if (this->blue) itm->blue = *this->blue;
        };
        static Color From(Patch* itm)
        {
            {
                Color col{ .alpha = itm->alpha,
                    .red = itm->red,
                    .green = itm->green,
                    .blue = itm->blue };
                return col;
            }
        }
    };
}  // namespace MPL::Config
