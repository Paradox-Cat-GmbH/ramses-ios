//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Platform/iOS/Window_iOS.h"
#include "internal/Platform/EAGL/Platform_EAGL.h"

namespace ramses::internal
{
    class Platform_iOS_EAGL : public Platform_EAGL<Window_iOS>
    {
    public:
        explicit Platform_iOS_EAGL(const RendererConfigData& rendererConfig);

    protected:
        virtual bool createWindow(const DisplayConfigData& displayConfig, IWindowEventHandler& windowEventHandler) override;
        virtual uint32_t getSwapInterval() const override;
    };
}
