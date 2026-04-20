#pragma once
#include <ClibUtil/editorID.hpp>
#include <cstring>
#include <filesystem>
#include <format>
#include <ranges>
#include <rfl.hpp>
#include <rfl/json.hpp>
#include <rfl/json/save.hpp>
#include <string_view>
namespace MPL::Config
{
    struct LiteForm
    {
        RE::FormID formID;
        template <class T>
        T* Get() const
        {
            return formID != 0x0 ? RE::TESForm::LookupByID<T>(formID) : nullptr;
        };
        static LiteForm FromID(RE::FormID id) { return { .formID = id }; };
    };

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
        static auto filtered_files = RE::TESDataHandler::GetSingleton()->files | std::views::filter([](RE::TESFile* file)
                                        { std::string sum(file->summary.c_str());
                                            return sum.contains("[Luma]"); });
        static std::vector<RE::TESFile*> valid_files(filtered_files.begin(), filtered_files.end());
        for (auto lf : valid_files)
        {
            auto file_name = std::format("Luma/{}/{}/{}/{:06X}.json", T::Name, lf->GetFilename(), form->GetFile(0)->GetFilename(), form->GetLocalFormID());
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

namespace rfl
{
    template <>
    struct Reflector<MPL::Config::LiteForm>
    {
        using ReflType = std::string;
        static ReflType from(const MPL::Config::LiteForm& v)
        {
            if (v.formID == 0x0)
            {
                return "null";
            }
            auto form = v.Get<RE::TESForm>();
            if (form->sourceFiles.array != nullptr)
            {
                return "null";
            }
            if (auto edid = form->GetFormEditorID(); strcmp(edid, "") != 0)
            {
                return std::string(form->GetFormEditorID());
            }
            return format("{:06X}:{}", form->GetLocalFormID(), form->GetFile(0)->GetFilename());
        }
        static MPL::Config::LiteForm to(const ReflType& v)
        {
            if (v == "null")
            {
                return MPL::Config::LiteForm::FromID(0x0);
            }
            auto loc = v.find(":");
            if (loc != std::string::npos)
            {
                auto lfid = strtoul(v.substr(0, loc).c_str(), nullptr, 16);
                auto file = v.substr(loc + 1);
                auto* dh = RE::TESDataHandler::GetSingleton();
                return MPL::Config::LiteForm::FromID(dh->LookupFormID(lfid, file));
            }
            else
            {
                auto* frm = RE::TESForm::LookupByEditorID(v);
                if (frm)
                {
                    return MPL::Config::LiteForm::FromID(frm->formID);
                }
                else
                {
                    return MPL::Config::LiteForm::FromID(0x0);
                }
            }
        }
    };
}  // namespace rfl
