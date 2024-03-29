//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EMESSAGEID_H
#define RAMSES_EMESSAGEID_H

#include "Collections/IOutputStream.h"
#include "Collections/IInputStream.h"
#include "Utils/LoggingUtils.h"
#include <type_traits>

namespace ramses_internal
{
    enum class EMessageId : uint32_t
    {
        PublishScene = 1,
        UnpublishScene,
        SubscribeScene,
        UnsubscribeScene,

        ConnectionDescriptionMessage,
        ConnectorAddressExchange,
        InputEvent,
        SendSceneUpdate,

        RendererEvent,

        Alive,

        // scene
        CreateScene,

        NUM_ELEMENTS
    };

    const std::array EMessageIdNames = {
        "INVALID_0",  // needed because EMessageId starts at 1

        "PublishScene",
        "UnpublishScene",
        "SubscribeScene",
        "UnsubscribeScene",
        "ConnectionDescriptionMessage",
        "ConnectorAddressExchange",
        "InputEvent",
        "SendSceneUpdate",
        "RendererEvent",
        "Alive",
        "CreateScene",
    };

    inline IOutputStream& operator<<(IOutputStream& outputStream, EMessageId messageId)
    {
        return outputStream << static_cast<std::underlying_type_t<EMessageId>>(messageId);
    }

    inline IInputStream& operator>>(IInputStream& inputStream, EMessageId& messageId)
    {
        std::underlying_type_t<EMessageId> val;
        inputStream >> val;
        messageId = static_cast<EMessageId>(val);
        return inputStream;
    }
}

MAKE_ENUM_CLASS_PRINTABLE(ramses_internal::EMessageId,
                          "EMessageId",
                          ramses_internal::EMessageIdNames,
                          ramses_internal::EMessageId::NUM_ELEMENTS);

#endif
