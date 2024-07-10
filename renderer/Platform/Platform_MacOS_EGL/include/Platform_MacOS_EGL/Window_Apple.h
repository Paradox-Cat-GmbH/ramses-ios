//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WINDOW_APPLE_H
#define RAMSES_WINDOW_APPLE_H

#include "Platform_Base/Window_Base.h"
#include "RendererAPI/IWindowEventHandler.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

    class OSWindow;
    
namespace ramses_internal
{

    class Window_Apple : public Window_Base
    {
    public: 
        Window_Apple(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler, UInt32 id);
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
        static EKeyCode convertKeySymbolIntoRamsesKeyCode(UInt32 virtualKeyCode);
    private:
        Bool setFullscreen(Bool fullscreen) override;
        void setTitle(const String& title) override;

        OSWindow* mWindow;
        X11WindowHandle mExternallyOwnedWindow;
    };
}

#endif
