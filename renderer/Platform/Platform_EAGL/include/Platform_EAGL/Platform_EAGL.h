//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORM_EGL_H
#define RAMSES_PLATFORM_EGL_H

#include "Platform_Base/Platform_Base.h"
#include "RendererLib/RendererConfig.h"
#include "RendererLib/DisplayConfig.h"
#include "Context_EAGL/Context_EAGL.h"
#include "Device_GL/Device_GL.h"


namespace ramses_internal
{
    template <typename WindowT>
    class Platform_EAGL : public Platform_Base
    {
    protected:
        explicit Platform_EAGL(const RendererConfig& rendererConfig)
            : Platform_Base(rendererConfig)
        {
        }

        virtual bool createContext(const DisplayConfig& displayConfig) override
        {
            m_context = createContextInternal(displayConfig, nullptr);
            return m_context != nullptr;
        }

        virtual bool createContextUploading() override
        {
            assert(m_context);
            m_contextUploading = createContextInternal(DisplayConfig{}, static_cast<Context_EAGL*>(m_context.get()));
            return m_contextUploading != nullptr;
        }

        virtual bool createDevice() override
        {
            assert(m_context);
            m_device = createDeviceInternal(*m_context, m_deviceExtension.get());
            return m_device != nullptr;
        }

        virtual bool createDeviceUploading() override
        {
            assert(m_contextUploading);
            m_deviceUploading = createDeviceInternal(*m_contextUploading, nullptr);
            return m_deviceUploading != nullptr;
        }

        bool createDeviceExtension(const DisplayConfig& displayConfig) override
        {
            const auto& platformRenderNode = displayConfig.getPlatformRenderNode();
            if(platformRenderNode == "")
                return true;

            return false;
        }

        /**
         * gets the platform specific default swap interval
         */
        virtual uint32_t getSwapInterval() const = 0;

    private:
        std::unique_ptr<IContext> createContextInternal(const DisplayConfig& displayConfig, Context_EAGL* sharedContext)
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
            auto device = std::make_unique<Device_GL>(context, uint8_t{ 3 }, uint8_t{ 0 }, true, deviceExtension);
            if (device->init())
                return device;

            return {};
        }
    };
}

#endif
