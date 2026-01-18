#pragma once
#include <filesystem>
#include <format>
#include <string_view>
#include <unordered_map>
namespace MPL::Config
{
    template <typename T>
    concept Named = requires(T t) {
        { T::Name } -> std::same_as<const std::string_view&>;
    };
    template <typename T>
    concept Patch = requires(T t) {
        typename T::Patch;
        { T::From(static_cast<typename T::Patch*>(nullptr)) } -> std::same_as<T>;
        t.Apply(static_cast<typename T::Patch*>(nullptr));
    };
    struct UMMPair
    {
        std::unordered_map<std::string, RE::FormID> modded;
        std::unordered_map<std::string, RE::FormID> unmodded;
    };
    struct ListPairs
    {
        std::unordered_map<RE::FormID, std::string_view> loaded;
    };
    class StatData : public REX::Singleton<StatData>
    {
    public:
        void PostProcess();
        UMMPair Templates;
        UMMPair Imagespaces;
        ListPairs Cells;
    };
    template <typename T>
        requires Named<T> && Patch<T>
    void LoadConfigFile(typename T::Patch* form, std::string_view a_mod)
    {
        if(std::filesystem::exists("expo")) {
            if(std::filesystem::exists(std::format("expo/{}", T::Name))) {
                if(!std::filesystem::exists(std::format("expo/{}/{}", T::Name, a_mod))) {
                    std::filesystem::create_directories(std::format("expo/{}/{}", T::Name, a_mod));
                }
                rfl::json::save(std::format("expo/{}/{}/{:06X}.json", T::Name, a_mod, form->GetLocalFormID()), T::From(form), rfl::json::pretty);
            }

        }
        auto* dh = RE::TESDataHandler::GetSingleton();
        for (auto lf : dh->files)
        {
            std::string sum(lf->summary.c_str());
            if (sum.contains("[Luma]"))
            {
                RE::BSResourceNiBinaryStream fileStream(std::format("Luma/{}/{}/{}/{:06X}.json", T::Name, lf->GetFilename(), a_mod, form->GetLocalFormID()));
                if (fileStream.good())
                {
                    if (fileStream.stream->totalSize > 0)
                    {
                        std::string contents;
                        contents.resize(fileStream.stream->totalSize);
                        fileStream.read(contents.data(), fileStream.stream->totalSize);
                        auto pch = rfl::json::read<T>(contents);
                        if (pch.has_value())
                        {
                            pch->Apply(form);
                        }
                        else
                        {
                            logger::info("Error {} {}:{:06X} {}", lf->GetFilename(), a_mod, form->GetLocalFormID(), pch.error().what());
                        }
                    }
                }
            }
        }
        auto luma_path = std::format("./Data/SKSE/Luma/{}", T::Name);
        if (std::filesystem::exists(luma_path))
        {
            for (auto dir : std::filesystem::directory_iterator(luma_path))
            {
                if (std::filesystem::is_directory(dir))
                {
                    auto pth = dir.path() / a_mod / std::format("{:6X}.json", form->GetLocalFormID());
                    if (std::filesystem::exists(pth))
                    {
                        auto pch = rfl::json::load<T>(pth.string(), 0);
                        if (pch.has_value())
                        {
                            pch->Apply(form);
                        }
                        else
                        {
                            logger::info("Error {} {}:{:06X} {}", pth.string(), a_mod, form->GetLocalFormID(), pch.error().what());
                        }
                    }
                }
            }
        }
    }
}  // namespace MPL::Config
