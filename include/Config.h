#pragma once
#include <Config/Cell.h>
#include <Config/Common.h>
#include <Config/ImageSpace.h>
#include <Config/Templates.h>
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
        SKSE::RegistrationSet<RE::TESObjectCELL*> cellLoad{ "OnCellChange"sv };
        void PostProcess();
        UMMPair Templates;
        UMMPair Imagespaces;
        ListPairs Cells;
    };

    template <typename T>
        requires Named<T> && Patch<T>
    void LoadConfigFile(typename T::Patch* form, std::string_view a_mod)
    {
        auto* dh = RE::TESDataHandler::GetSingleton();
        for (auto lf : dh->files)
        {
            std::string sum(lf->summary.c_str());
            if (sum.contains("[Luma]"))
            {
                auto file_name = std::format("Luma/{}/{}/{}/{:06X}.json", T::Name, lf->GetFilename(), a_mod, form->GetLocalFormID());
                RE::BSResourceNiBinaryStream fileStream(file_name);
                if (fileStream.good())
                {
                    if (fileStream.stream->totalSize > 0)
                    {
                        logger::info("Loading file {}", file_name);
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
                    auto file_name = dir.path() / a_mod / std::format("{:6X}.json", form->GetLocalFormID());
                    if (std::filesystem::exists(file_name))
                    {
                        logger::info("Loading file {}", file_name.string());
                        auto pch = rfl::json::load<T>(file_name.string(), 0);
                        if (pch.has_value())
                        {
                            logger::info("Applying file {}", file_name.string());
                            pch->Apply(form);
                        }
                        else
                        {
                            logger::info("Error {} {}:{:06X} {}", file_name.string(), a_mod, form->GetLocalFormID(), pch.error().what());
                        }
                    }
                }
            }
        }
    }
}  // namespace MPL::Config
