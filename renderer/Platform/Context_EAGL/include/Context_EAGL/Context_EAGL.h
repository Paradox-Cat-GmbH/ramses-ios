//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CONTEXT_EAGL_H
#define RAMSES_CONTEXT_EAGL_H
/*
#include <EGL/egl.h>

#undef Bool // Xlib.h (included from EGL/egl.h) defines Bool as int - this collides with ramses_internal::Bool
#undef Status
#undef None
*/
#include "Platform_Base/Context_Base.h"

@class EAGLContext;
@class CAEAGLLayer;

namespace ramses_internal
{
    class Context_EAGL : public Context_Base
    {
    public:
        using Generic_EAGLNativeDisplayType = int;
        using Generic_EAGLNativeWindowType = void*;

        Context_EAGL(CAEAGLLayer* m_drawable, Context_EAGL* sharedContext = nullptr);
        ~Context_EAGL() override;

        Bool init();

        bool swapBuffers() override;
        bool enable() override;
        bool disable() override;

        void* getProcAddress(const char* name) const override;
        
        DeviceResourceHandle getFramebufferResource() const override;

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

#endif
