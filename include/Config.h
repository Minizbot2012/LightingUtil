#pragma once
#include <ClibUtil/editorID.hpp>
#include <Config/Cell.h>
#include <Config/Common.h>
#include <Config/ImageSpace.h>
#include <Config/Templates.h>
#include <filesystem>
#include <format>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
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
        std::unordered_set<RE::FormID> loaded;
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
    void LoadConfigFile(typename T::Patch* form)
    {
        auto* dh = RE::TESDataHandler::GetSingleton();
        for (auto lf : dh->files)
        {
            std::string sum(lf->summary.c_str());
            if (sum.contains("[Luma]"))
            {
                auto file_name = std::format("Luma/{}/{}/{}.json", T::Name, lf->GetFilename(), clib_util::editorID::get_editorID(form));
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
                            logger::info("Error {} {}", clib_util::editorID::get_editorID(form), pch.error().what());
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
                    auto file_name = dir.path() / std::format("{}.json", clib_util::editorID::get_editorID(form));
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
                            logger::info("Error {} {} {}", file_name.string(), clib_util::editorID::get_editorID(form), pch.error().what());
                        }
                    }
                }
            }
        }
    }
}  // namespace MPL::Config
