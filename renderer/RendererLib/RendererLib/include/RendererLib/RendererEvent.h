//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDEREREVENT_H
#define RAMSES_RENDEREREVENT_H

#include "RendererAPI/Types.h"
#include "SceneAPI/SceneTypes.h"
#include "SceneAPI/SceneId.h"
#include "SceneAPI/RendererSceneState.h"
#include "SceneAPI/DataSlot.h"
#include "SceneAPI/SceneVersionTag.h"
#include "RendererLib/EMouseEventType.h"
#include "RendererLib/EKeyEventType.h"
#include "RendererLib/EKeyCode.h"
#include "RendererLib/DisplayConfig.h"
#include "Utils/LoggingUtils.h"
#include <chrono>

namespace ramses_internal
{
    enum class ERendererEventType
    {
        Invalid = 0,
        DisplayCreated,
        DisplayCreateFailed,
        DisplayDestroyed,
        DisplayDestroyFailed,
        ReadPixelsFromFramebuffer,
        ReadPixelsFromFramebufferFailed,
        OffscreenBufferCreated,
        OffscreenBufferCreateFailed,
        OffscreenBufferDestroyed,
        OffscreenBufferDestroyFailed,
        ExternalBufferCreated,
        ExternalBufferCreateFailed,
        ExternalBufferDestroyed,
        ExternalBufferDestroyFailed,
        ScenePublished,
        SceneStateChanged,

        // internal scene state event types
        SceneUnpublished,
        SceneSubscribed,
        SceneSubscribeFailed,
        SceneUnsubscribed,
        SceneUnsubscribedIndirect,
        SceneUnsubscribeFailed,
        SceneMapped,
        SceneMapFailed,
        SceneUnmapped,
        SceneUnmappedIndirect,
        SceneUnmapFailed,
        SceneShown,
        SceneShowFailed,
        SceneHidden,
        SceneHiddenIndirect,
        SceneHideFailed,

        SceneFlushed,
        SceneExpirationMonitoringEnabled,
        SceneExpirationMonitoringDisabled,
        SceneExpired,
        SceneRecoveredFromExpiration,
        SceneDataLinked,
        SceneDataLinkFailed,
        SceneDataBufferLinked,
        SceneDataBufferLinkFailed,
        SceneDataUnlinked,
        SceneDataUnlinkedAsResultOfClientSceneChange,
        SceneDataUnlinkFailed,
        SceneDataSlotProviderCreated,
        SceneDataSlotProviderDestroyed,
        SceneDataSlotConsumerCreated,
        SceneDataSlotConsumerDestroyed,
        WindowClosed,
        WindowKeyEvent,
        WindowMouseEvent,
        WindowMoveEvent,
        WindowResizeEvent,
        StreamSurfaceAvailable,
        StreamSurfaceUnavailable,
        ObjectsPicked,
        FrameTimingReport,
        NUMBER_OF_ELEMENTS
    };

    const std::array RendererEventTypeNames =
    {
        "Invalid",
        "DisplayCreated",
        "DisplayCreateFailed",
        "DisplayDestroyed",
        "DisplayDestroyFailed",
        "ReadPixelsFromFramebuffer",
        "ReadPixelsFromFramebufferFailed",
        "OffscreenBufferCreated",
        "OffscreenBufferCreateFailed",
        "OffscreenBufferDestroyed",
        "OffscreenBufferDestroyFailed",
        "ExternalBufferCreated",
        "ExternalBufferCreateFailed",
        "ExternalBufferDestroyed",
        "ExternalBufferDestroyFailed",
        "ScenePublished",
        "SceneStateChanged",
        "SceneUnpublished",
        "SceneSubscribed",
        "SceneSubscribeFailed",
        "SceneUnsubscribed",
        "SceneUnsubscribedAsResultOfUnpublish",
        "SceneUnsubscribeFailed",
        "SceneMapped",
        "SceneMapFailed",
        "SceneUnmapped",
        "SceneUnmappedAsResultOfUnpublish",
        "SceneUnmapFailed",
        "SceneShown",
        "SceneShowFailed",
        "SceneHidden",
        "SceneHiddenAsResultOfUnpublish",
        "SceneHideFailed",
        "SceneFlushed",
        "SceneExpirationMonitoringEnabled",
        "SceneExpirationMonitoringDisabled",
        "SceneExpired",
        "SceneRecoveredFromExpiration",
        "SceneDataLinked",
        "SceneDataLinkFailed",
        "SceneDataBufferLinked",
        "SceneDataBufferLinkFailed",
        "SceneDataUnlinked",
        "SceneDataUnlinkedAsResultOfClientSceneChange",
        "SceneDataUnlinkFailed",
        "SceneDataSlotProviderCreated",
        "SceneDataSlotProviderDestroyed",
        "SceneDataSlotConsumerCreated",
        "SceneDataSlotConsumerDestroyed",
        "WindowClosed",
        "WindowKeyEvent",
        "WindowMouseEvent",
        "WindowMoveEvent",
        "WindowResizeEvent",
        "StreamSurfaceAvailable",
        "StreamSurfaceUnavailable",
        "ObjectsPicked",
        "FrameTimingReport",
    };

    struct MouseEvent
    {
        EMouseEventType type = EMouseEventType_Invalid;
        glm::ivec2        pos;
    };

    struct WindowMoveEvent
    {
        int32_t        posX;
        int32_t        posY;
    };

    struct KeyEvent
    {
        EKeyEventType type = EKeyEventType_Invalid;
        uint32_t        modifier;
        EKeyCode      keyCode;
    };

    struct ResizeEvent
    {
        uint32_t width;
        uint32_t height;
    };

    struct FrameTimings
    {
        std::chrono::microseconds maximumLoopTimeWithinPeriod;
        std::chrono::microseconds averageLoopTimeWithinPeriod;
    };

    struct RendererEvent
    {
        RendererEvent(ERendererEventType type = ERendererEventType::Invalid, SceneId sId = {})  //NOLINT(google-explicit-constructor) for RendererEventVector creation convenience
            : eventType(type)
            , sceneId(sId)
        {
        }

        ERendererEventType          eventType;
        SceneId                     sceneId;
        RendererSceneState          state = RendererSceneState::Unavailable;
        DisplayHandle               displayHandle;
        DisplayConfig               displayConfig;
        UInt8Vector                 pixelData;
        SceneId                     providerSceneId;
        SceneId                     consumerSceneId;
        DataSlotId                  providerdataId;
        DataSlotId                  consumerdataId;
        OffscreenBufferHandle       offscreenBuffer;
        StreamBufferHandle          streamBuffer;
        ExternalBufferHandle        externalBuffer;
        SceneVersionTag             sceneVersionTag;
        MouseEvent                  mouseEvent;
        ResizeEvent                 resizeEvent;
        KeyEvent                    keyEvent;
        WindowMoveEvent             moveEvent;
        WaylandIviSurfaceId         streamSourceId;
        PickableObjectIds           pickedObjectIds;
        FrameTimings                frameTimings;
        int                         dmaBufferFD = -1;
        uint32_t                    dmaBufferStride = 0u;
        uint32_t                    textureGlId = 0u;
    };
    using RendererEventVector = std::vector<RendererEvent>;

    struct InternalSceneStateEvent
    {
        ERendererEventType type;
        SceneId sceneId;
    };
    using InternalSceneStateEvents = std::vector<InternalSceneStateEvent>;
}

MAKE_ENUM_CLASS_PRINTABLE(ramses_internal::ERendererEventType, "ERendererEventType", ramses_internal::RendererEventTypeNames, ramses_internal::ERendererEventType::NUMBER_OF_ELEMENTS);

#endif
