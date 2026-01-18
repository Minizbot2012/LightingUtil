#pragma once
namespace MPL::Config
{
    struct Color
    {
        using Patch = RE::Color;
        std::optional<uint8_t> alpha;
        std::optional<uint8_t> red;
        std::optional<uint8_t> green;
        std::optional<uint8_t> blue;
        void Apply(Patch* itm) {
            if (this->alpha) itm->alpha = *this->alpha;
            if (this->red) itm->red = *this->red;
            if (this->green) itm->green = *this->green;
            if (this->blue) itm->blue = *this->blue;
        };
        static Color From(Patch* itm) {
            {
                Color col{ .alpha = itm->alpha, .red = itm->red, .green = itm->green, .blue = itm->blue };
                return col;
            }
        };
    };
}  // namespace MPL::Config
