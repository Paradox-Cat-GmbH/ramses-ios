//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

@class EAGLContext;
@class CAEAGLLayer;

#include "internal/RendererLib/PlatformBase/Context_Base.h"

namespace ramses::internal
{
    class Context_EAGL : public Context_Base
    {
    public:
        using Generic_EAGLNativeDisplayType = int;
        using Generic_EAGLNativeWindowType = void*;

        Context_EAGL(CAEAGLLayer* m_drawable, Context_EAGL* sharedContext = nullptr);
        ~Context_EAGL() override;

        bool init();

        bool swapBuffers() override;
        bool enable() override;
        bool disable() override;

        [[nodiscard]] GlProcLoadFunc getGlProcLoadFunc() const override;

    private:
        EAGLContext* m_context;
        EAGLContext* m_sharedContext;
        CAEAGLLayer* m_drawable;
        DeviceResourceHandle m_framebufferResource;
        
        // The OpenGL names for the framebuffer and renderbuffer used to render to this view
        uint32_t _colorRenderbuffer;
        uint32_t _depthRenderbuffer;
        uint32_t _defaultFrameBuffer;
    };

}
