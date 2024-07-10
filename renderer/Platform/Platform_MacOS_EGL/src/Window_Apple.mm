//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Platform_MacOS_EGL/Window_Apple.h"
#include "Platform_MacOS_EGL/OSWindow.h"
#include "Platform_MacOS_EGL/OSXWindow.h"
#include "RendererLib/DisplayConfig.h"
#include "RendererLib/EKeyModifier.h"
#include "RendererAPI/IWindowEventHandler.h"
#include "Utils/ThreadLocalLogForced.h"
#include "Utils/Warnings.h"


void runOnUIThread(dispatch_block_t block) {
    
    if ([NSThread isMainThread]) {
        block();
    } else {
        dispatch_semaphore_t sema = dispatch_semaphore_create(0);
        CFRunLoopPerformBlock(CFRunLoopGetMain(), kCFRunLoopCommonModes, ^{
            block();
            dispatch_semaphore_signal(sema);
        });
        dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
        dispatch_release(sema);
    }
}
namespace ramses_internal
{

    Window_Apple::Window_Apple(const DisplayConfig& displayConfig, IWindowEventHandler &windowEventHandler, UInt32 id)
        : Window_Base(displayConfig, windowEventHandler, id),
            mExternallyOwnedWindow(displayConfig.getX11WindowHandle())
    {
        if(!mExternallyOwnedWindow.isValid()) {
            mWindow = new OSXWindow(nullptr);
        } else {
            void* p = (void*)mExternallyOwnedWindow.getValue();
            NSView * v = (NSView*)p;
            mWindow = new OSXWindow(v);
        }
                
        LOG_DEBUG(CONTEXT_RENDERER, "Window_Apple::Window_Apple");
    }

    Window_Apple::~Window_Apple()
    {
        LOG_DEBUG(CONTEXT_RENDERER, "Window_Apple::~Window_Apple");
        
        runOnUIThread(^(void) {
            mWindow->destroy();
            mWindow = nullptr;
        });
    }

    Bool Window_Apple::init()
    {
        LOG_DEBUG(CONTEXT_RENDERER, "Window_Apple::initAppleWindowAndGetNativeHandles XOpenDisplay");
        
        runOnUIThread(^(void) {
            mWindow->initialize(m_windowName.stdRef(), m_width, m_height);
            mWindow->setVisible(true);
        });

        if (m_fullscreen)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Fullscreen mode is not supported on this platform");
        }

        LOG_INFO(CONTEXT_RENDERER, "Created Apple Window, size " << m_width << " by " << m_height << " pixels");
        return true;
    }

    Bool Window_Apple::setFullscreen(Bool fullscreen)
    {
        if (fullscreen)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Fullscreen mode is not supported on this platform");
            return false;
        }

        return true;
    }

    EGLNativeDisplayType Window_Apple::getNativeDisplayHandle() const
    {
        return mWindow->getNativeDisplay();
    }

    EGLNativeWindowType Window_Apple::getNativeWindowHandle() const
    {
        return mWindow->getNativeWindow();
    }

    void Window_Apple::handleEvents()
    {
        LOG_TRACE(CONTEXT_RENDERER, "Updating Apple window");
        if(!mExternallyOwnedWindow.isValid()) {
            runOnUIThread(^(void) {
                mWindow->messageLoopOnce();
            });
        }
    }

    bool Window_Apple::setExternallyOwnedWindowSize(uint32_t width, uint32_t height)
    {
        if(mExternallyOwnedWindow.isValid()) {
            mWindow->resize(width, height);
            return true;
        }
        LOG_ERROR(CONTEXT_RENDERER, "Window_Apple::setExternallyOwnedWindowSize: Apple window is not externally owned!");
        return false;
    }

    void Window_Apple::setTitle(const String& title)
    {
        // TODO
    }
}
