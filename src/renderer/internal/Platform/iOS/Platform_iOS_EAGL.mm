//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/iOS/Platform_iOS_EAGL.h"

namespace ramses::internal
{
    Platform_iOS_EAGL::Platform_iOS_EAGL(const RendererConfigData& rendererConfig)
        : Platform_EAGL<Window_iOS>(rendererConfig)
    {
    }

    bool Platform_iOS_EAGL::createWindow(const DisplayConfigData& displayConfig, IWindowEventHandler& windowEventHandler)
    {
        auto window = std::make_unique<Window_iOS>(displayConfig, windowEventHandler, 0u);
        if (window->init())
        {
            m_window = std::move(window);
            return true;
        }

        return false;
    }

    uint32_t Platform_iOS_EAGL::getSwapInterval() const
    {
        return 1u;
    }
}
