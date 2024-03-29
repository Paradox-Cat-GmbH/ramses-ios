//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Platform_iOS_EGL/Platform_iOS_EGL.h"
#include "RendererLib/RendererConfig.h"
#include "Context_EGL/Context_EGL.h"
#include "Platform_Base/EmbeddedCompositor_Dummy.h"

namespace ramses_internal
{

    Platform_iOS_EGL::Platform_iOS_EGL(const RendererConfig& rendererConfig)
        : Platform_EGL<Window_iOS>(rendererConfig)
    {
    }

    bool Platform_iOS_EGL::createWindow(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler)
    {
        auto window = std::make_unique<Window_iOS>(displayConfig, windowEventHandler, 0u);
        if (window->init())
        {
            m_window = std::move(window);
            return true;
        }

        return false;
    }

    uint32_t Platform_iOS_EGL::getSwapInterval() const
    {
        return 1u;
    }
}
