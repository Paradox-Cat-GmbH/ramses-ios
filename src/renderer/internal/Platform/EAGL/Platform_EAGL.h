//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformBase/Platform_Base.h"
#include "internal/RendererLib/RendererConfigData.h"
#include "internal/RendererLib/DisplayConfigData.h"
#include "internal/Platform/EAGL/Context_EAGL.h"
#include "internal/Platform/OpenGL/Device_GL.h"
#include "internal/Core/Utils/LogMacros.h"
#include <optional>

namespace ramses::internal
{
    template <typename WindowT>
    class Platform_EAGL : public Platform_Base
    {
    protected:
        explicit Platform_EAGL(const RendererConfigData& rendererConfig)
            : Platform_Base(rendererConfig)
        {
        }

        bool createContext(const DisplayConfigData& displayConfig) override
        {
            m_context = createContextInternal(displayConfig, nullptr);
            return m_context != nullptr;
        }

        bool createContextUploading() override
        {
            assert(m_context);
            m_contextUploading = createContextInternal(DisplayConfigData{}, static_cast<Context_EAGL*>(m_context.get()));
            return m_contextUploading != nullptr;
        }

        bool createDevice() override
        {
            assert(m_context);
            m_device = createDeviceInternal(*m_context, m_deviceExtension.get());
            return m_device != nullptr;
        }

        bool createDeviceUploading() override
        {
            assert(m_contextUploading);
            m_deviceUploading = createDeviceInternal(*m_contextUploading, nullptr);
            return m_deviceUploading != nullptr;
        }

        bool createDeviceExtension(const DisplayConfigData& displayConfig) override
        {
            const auto platformRenderNode = displayConfig.getPlatformRenderNode();
            if (platformRenderNode.empty())
                 return true;

            return false;
        }

        /**
         * gets the platform specific default swap interval
         */
        [[nodiscard]] virtual uint32_t getSwapInterval() const = 0;

    private:
        std::unique_ptr<IContext> createContextInternal(const DisplayConfigData& displayConfig, Context_EAGL* sharedContext)
        {
            auto context = std::make_unique<Context_EAGL>(
                static_cast<CAEAGLLayer*>(displayConfig.getIOSNativeWindow().getValue()),
                sharedContext);

            if (context->init())
                return context;

            return {};
        }

        std::unique_ptr<Device_GL> createDeviceInternal(IContext& context, IDeviceExtension* deviceExtension)
        {
            auto device = std::make_unique<Device_GL>(context, deviceExtension);
            if (device->init())
                return device;

            return {};
        }
    };
}
