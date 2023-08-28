//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/DisplayConfig.h"
#include "Utils/ThreadLocalLogForced.h"
#include "Utils/Warnings.h"

#include "RendererAPI/IWindowEventHandler.h"
#include "Platform_iOS_EAGL/Window_iOS.h"
#include <GLKit/GLKit.h>

namespace ramses_internal
{
    Window_iOS::Window_iOS(const DisplayConfig& displayConfig, IWindowEventHandler &windowEventHandler, UInt32 id)
        : Window_Base(displayConfig, windowEventHandler, id)
        , eaglDrawable(static_cast<EAGLDrawable*>(displayConfig.getIOSNativeWindow().getValue()))
    {
        LOG_INFO(CONTEXT_RENDERER, "Window_iOS::Window_iOS");
    }

    Window_iOS::~Window_iOS()
    {
        LOG_INFO(CONTEXT_RENDERER, "Window_iOS::~Window_iOS");
    }

    Bool Window_iOS::init()
    {
        return true;
    }

    Bool Window_iOS::setFullscreen(Bool fullscreen)
    {
        UNUSED(fullscreen);
        return true;
    }

    bool Window_iOS::setExternallyOwnedWindowSize(uint32_t width, uint32_t height)
    {
        m_width = width;
        m_height = height;
        return true;
    }

    void Window_iOS::handleEvents()
    {
    }
}
