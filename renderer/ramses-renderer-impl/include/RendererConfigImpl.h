//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERCONFIGIMPL_H
#define RAMSES_RENDERERCONFIGIMPL_H

#include "RendererLib/RendererConfig.h"
#include "StatusObjectImpl.h"

namespace ramses
{
    class IBinaryShaderCache;
    class IRendererResourceCache;

    class RendererConfigImpl : public StatusObjectImpl
    {
    public:
        RendererConfigImpl();

        status_t enableSystemCompositorControl();

        status_t setSystemCompositorWaylandDisplay(std::string_view waylandDisplay);
        std::string_view getSystemCompositorWaylandDisplay() const;

        status_t setFrameCallbackMaxPollTime(uint64_t waitTimeInUsec);

        status_t setBinaryShaderCache(IBinaryShaderCache& cache);
        IBinaryShaderCache* getBinaryShaderCache() const;

        status_t setRendererResourceCache(IRendererResourceCache& cache);
        IRendererResourceCache* getRendererResourceCache() const;

        status_t setOffscreenBufferDoubleBufferingEnabled(bool isDoubleBuffered);
        bool isOffscreenBufferDoubleBufferingEnabled() const;

        status_t setRenderThreadLoopTimingReportingPeriod(std::chrono::milliseconds period);
        std::chrono::milliseconds getRenderThreadLoopTimingReportingPeriod() const;

        //impl methods
        const ramses_internal::RendererConfig& getInternalRendererConfig() const;

    private:
        ramses_internal::RendererConfig    m_internalConfig;
        IBinaryShaderCache*                m_binaryShaderCache;
        IRendererResourceCache*            m_rendererResourceCache;
    };
}

#endif
