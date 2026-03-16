#pragma once
#include <ClibUtil/editorID.hpp>
#include <filesystem>
#include <format>
#include <rfl.hpp>
#include <rfl/json.hpp>
#include <string_view>
namespace MPL::Config
{
    struct LiteForm
    {
        RE::FormID formID;
        template<class T>
        T* Get() { return RE::TESForm::LookupByID<T>(formID); };
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
        //void PostProcess();
    };

    template <typename T>
        requires Named<T> && Patch<T>
    void LoadConfigFormID(typename T::Patch* form)
    {
        auto* dh = RE::TESDataHandler::GetSingleton();
        for (auto lf : dh->files)
        {
            std::string sum(lf->summary.c_str());
            if (sum.contains("[Luma]"))
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
        }
        auto luma_path = std::format("./Data/SKSE/Luma/{}", T::Name);
        if (std::filesystem::exists(luma_path))
        {
            for (auto dir : std::filesystem::directory_iterator(luma_path))
            {
                if (std::filesystem::is_directory(dir))
                {
                    auto file_name = dir.path() / std::format("{}/{:06X}.json", form->GetFile(0)->GetFilename(), form->GetLocalFormID());
                    if (std::filesystem::exists(file_name))
                    {
                        logger::info("Loading file {}", file_name.string());
                        auto pch = rfl::json::load<T>(file_name.string());
                        if (pch.has_value())
                        {
                            logger::info("Applying file {}", file_name.string());
                            pch->Apply(form);
                        }
                        else
                        {
                            logger::info("Error {} {}", file_name.string(), pch.error().what());
                        }
                    }
                }
            }
        }
    }
    template <typename T>
        requires Named<T> && Patch<T>
    void LoadConfigEDID(typename T::Patch* form)
    {
        std::string edid;
        edid = form->GetFormEditorID();
        if(edid == "") {
            edid = clib_util::editorID::get_editorID(form);
        }
        auto* dh = RE::TESDataHandler::GetSingleton();
        for (auto lf : dh->files)
        {
            std::string sum(lf->summary.c_str());
            if (sum.contains("[Luma]"))
            {
                auto file_name = std::format("Luma/{}/{}/{}.json", T::Name, lf->GetFilename(), edid);
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
                            logger::info("Error {} {}", form->GetFormEditorID(), pch.error().what());
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
                        auto pch = rfl::json::load<T>(file_name.string());
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

namespace rfl
{
    template <>
    struct Reflector<MPL::Config::LiteForm>
    {
        using ReflType = std::string;
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
