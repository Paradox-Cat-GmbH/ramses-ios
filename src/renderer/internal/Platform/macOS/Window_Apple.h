//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WINDOW_APPLE_H
#define RAMSES_WINDOW_APPLE_H

#include "internal/RendererLib/PlatformBase/Window_Base.h"
#include "ramses/renderer/Types.h"
#include "internal/RendererLib/PlatformInterface/IWindowEventHandler.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

class OSWindow;
    
namespace ramses::internal
{

    class Window_Apple : public Window_Base
    {
    public: 
        Window_Apple(const DisplayConfigData& displayConfig, IWindowEventHandler& windowEventHandler, uint32_t id);
        ~Window_Apple() override;

        virtual bool init() override;

        void handleEvents() override;
        bool setExternallyOwnedWindowSize(uint32_t width, uint32_t height) override;

        bool hasTitle() const override
        {
            return !m_fullscreen;
        }

        EGLNativeDisplayType getNativeDisplayHandle() const;
        EGLNativeWindowType getNativeWindowHandle() const;

        // public as it is used by tests
        static EKeyCode convertKeySymbolIntoRamsesKeyCode(uint32_t virtualKeyCode);
        bool setFullscreen(bool fullscreen) override;
    
    private:
        OSWindow* mWindow;
        MacOSNativeWindowPtr mExternallyOwnedWindow;
    };
}

#endif
