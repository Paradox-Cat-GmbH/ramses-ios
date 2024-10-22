//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/macOS/Platform_macOS_EGL.h"

namespace ramses::internal
{
    Platform_macOS_EGL::Platform_macOS_EGL(const RendererConfigData& rendererConfig)
        : Platform_EGL<Window_Apple>(rendererConfig)
    {
    }

    bool Platform_macOS_EGL::createWindow(const DisplayConfigData& displayConfig, IWindowEventHandler& windowEventHandler)
    {
        auto window = std::make_unique<Window_Apple>(displayConfig, windowEventHandler, 0u);
        if (window->init())
        {
            m_window = std::move(window);
            return true;
        }

        return false;
    }

    uint32_t Platform_macOS_EGL::getSwapInterval() const
    {
        return 1u;
    }
}
