#pragma once
#include <Config/Cell.h>
#include <Config/Common.h>
#include <Config/ImageSpace.h>
#include <Config/ObjectRef.h>
#include <Config/Templates.h>
#include <filesystem>
#include <format>
#include <rfl/json.hpp>
#include <string_view>
#include <vector>
#include <ClibUtil/editorID.hpp>
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

    class StatData : public REX::Singleton<StatData>
    {
    public:
        SKSE::RegistrationSet<RE::TESObjectCELL*> cellLoad{ "OnCellChange"sv };
        RE::TESRegion* lastRegion;
    };
    template <typename T>
        requires Named<T> && Patch<T>
    void LoadConfigFormID(typename T::Patch* form)
    {
        static std::vector<std::string> valid_files = RE::TESDataHandler::GetSingleton()->files |
                                              std::views::filter([](RE::TESFile* file)
                                                  {
            std::string sum(file->summary.c_str());
            return sum.contains("[Luma]"); }) |
                                              std::views::transform([](RE::TESFile* file)
                                                  {
                                                      return std::string(file->GetFilename());
                                                   }) |
                                              std::ranges::to<std::vector>();
        for (auto local_file : valid_files)
        {
            auto file_name = std::format("Luma/{}/{}/{}/{:06X}.json", T::Name, local_file, form->GetFile(0)->GetFilename(), form->GetLocalFormID());
            RE::BSResourceNiBinaryStream fileStream(file_name);
            if (fileStream.good())
            {
                if (fileStream.stream->totalSize > 0)
                {
    #    ifndef NDEBUG
                    logger::info("Loading file {}", file_name);
    #    endif
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
        static bool lumaDump = std::filesystem::exists(".lumadump");
        if (lumaDump)
        {
            T data = T::From(form);
            std::filesystem::path path(std::format("Data/SKSE/Luma/Dump/{}/{}", T::Name, form->GetFile(0)->GetFilename()));
            if (!std::filesystem::exists(path))
            {
                std::filesystem::create_directories(path);
            }
            rfl::json::save(std::format("Data/SKSE/Luma/Dump/{}/{}/{:06X}.json", T::Name, form->GetFile(0)->GetFilename(), form->GetLocalFormID()), data, 0);
        }
    }
}  // namespace MPL::Config
