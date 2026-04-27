#pragma once
#include <ClibUtil/editorID.hpp>
#include <Config/Cell.h>
#include <Config/Common.h>
#include <Config/ImageSpace.h>
#include <Config/ObjectRef.h>
#include <Config/Templates.h>
#include <format>
#include <rfl/json.hpp>
#include <rfl/json/write.hpp>
#include <string_view>
#include <vector>
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
                                                          { return std::string(file->GetFilename()); }) |
                                                      std::ranges::to<std::vector>();
        for (auto local_file : valid_files)
        {
            auto file_name = std::format("Luma/{}/{}/{}/{:06X}.json", T::Name, local_file, form->GetFile(0)->GetFilename(), form->GetLocalFormID());
            RE::BSResourceNiBinaryStream fileStream(file_name);
            if (fileStream.good())
            {
                if (fileStream.stream->totalSize > 0)
                {
#ifndef NDEBUG
                    logger::info("Loading file {}", file_name);
#endif
                    std::string contents;
                    contents.resize(fileStream.stream->totalSize);
                    fileStream.read(contents.data(), fileStream.stream->totalSize);
                    auto pch = rfl::json::read<T>(contents);
                    if (pch.has_value())
                    {
#ifndef NDEBUG
                        auto jso = rfl::json::write(*pch);
                        auto old = rfl::json::write(T::From(form));
                        logger::info("New: {}", jso);
                        logger::info("Old: {}", old);
#endif
                        pch->Apply(form);
#ifndef NDEBUG
                        auto post = rfl::json::write(T::From(form));
                        logger::info("Post: {}", post);
#endif
                    }

                    else
                    {
                        logger::info("Error {} {}", clib_util::editorID::get_editorID(form), pch.error().what());
                    }
                }
            }
        }
    }
}  // namespace MPL::Config
