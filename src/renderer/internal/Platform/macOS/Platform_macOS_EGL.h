//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Platform/macOS/Window_Apple.h"
#include "internal/Platform/EGL/Platform_EGL.h"

namespace ramses::internal
{
    class Platform_macOS_EGL : public Platform_EGL<Window_Apple>
    {
    public:
        explicit Platform_macOS_EGL(const RendererConfigData& rendererConfig);

    protected:
        virtual bool createWindow(const DisplayConfigData& displayConfig, IWindowEventHandler& windowEventHandler) override;
        virtual uint32_t getSwapInterval() const override;
    };
}
