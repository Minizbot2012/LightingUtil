#pragma once
#include <cstdint>
namespace MPL::API
{
    class ServiceMap
    {
    public:
        virtual RE::FormID LookupFormIDForEDID(std::string) = 0;
        virtual std::string LookupEDIDForFormID(RE::FormID) = 0;
        virtual RE::TESForm* LookupCachedForm(std::string) = 0;
        virtual RE::TESForm* AllocateForm(std::string, RE::FormType) = 0;
    };
    struct MMSFMessage
    {
        enum message_type : uint32_t
        {
            kMessage_GetInterface = 'MMSF'
        };
        ServiceMap* API;
    };

    [[nodiscard]] inline ServiceMap* RequestMMSFAPI()
    {
        MMSFMessage message;
        SKSE::GetMessagingInterface()->Dispatch(MMSFMessage::kMessage_GetInterface, &message, sizeof(MMSFMessage), nullptr);
        if (message.API != nullptr)
        {
            return message.API;
        }
        else
        {
            return nullptr;
        }
    }
}  // namespace MPL::API
