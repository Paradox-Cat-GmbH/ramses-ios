//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/Renderer.h"
#include "internal/RendererLib/PlatformInterface/IRenderBackend.h"
#include "internal/RendererLib/PlatformInterface/ISystemCompositorController.h"
#include "internal/RendererLib/PlatformInterface/IEmbeddedCompositor.h"
#include "internal/RendererLib/PlatformInterface/IWindow.h"
#include "internal/RendererLib/PlatformInterface/IContext.h"
#include "internal/RendererLib/PlatformInterface/IDevice.h"
#include "internal/RendererLib/RenderingContext.h"
#include "internal/RendererLib/RendererCachedScene.h"
#include "internal/RendererLib/DisplayController.h"
#include "internal/RendererLib/RendererLogContext.h"
#include "internal/RendererLib/DisplayConfigData.h"
#include "internal/RendererLib/RendererScenes.h"
#include "internal/RendererLib/DisplayEventHandler.h"
#include "internal/RendererLib/SceneExpirationMonitor.h"
#include "internal/RendererLib/PlatformBase/Platform_Base.h"
#include "internal/Core/Utils/LogMacros.h"
#include <algorithm>

namespace ramses::internal
{
    const glm::vec4 Renderer::DefaultClearColor = { 0.f, 0.f, 0.f, 1.f };

    Renderer::Renderer(
        DisplayHandle display,
        IPlatform& platform,
        const RendererScenes& rendererScenes,
        RendererEventCollector& eventCollector,
        const FrameTimer& frameTimer,
        SceneExpirationMonitor& expirationMonitor,
        RendererStatistics& rendererStatistics)
        : m_display(display)
        , m_platform(platform)
        , m_rendererScenes(rendererScenes)
        , m_displayEventHandler(m_display, eventCollector)
        , m_statistics(rendererStatistics)
        , m_frameTimer(frameTimer)
        , m_expirationMonitor(expirationMonitor)
    {
    }

    Renderer::~Renderer()
    {
        assert(!hasDisplayController());
    }

    void Renderer::registerOffscreenBuffer(DeviceResourceHandle bufferDeviceHandle, uint32_t width, uint32_t height, uint32_t sampleCount, bool isInterruptible)
    {
        assert(hasDisplayController());
        assert(!hasAnyBufferWithInterruptedRendering());
        m_displayBuffersSetup.registerDisplayBuffer(bufferDeviceHandle, { 0, 0, width, height }, DefaultClearColor, true, sampleCount, isInterruptible);
        // no need to re-render OB as long as no scene is assigned to it, OB is cleared at creation time
        m_displayBuffersSetup.setDisplayBufferToBeRerendered(bufferDeviceHandle, false);
    }

    void Renderer::unregisterOffscreenBuffer(DeviceResourceHandle bufferDeviceHandle)
    {
        assert(hasDisplayController());
        assert(!hasAnyBufferWithInterruptedRendering());
        m_displayBuffersSetup.unregisterDisplayBuffer(bufferDeviceHandle);
        m_statistics.untrackOffscreenBuffer(bufferDeviceHandle);
        m_screenshots.erase(bufferDeviceHandle);
    }

    const IDisplayController& Renderer::getDisplayController() const
    {
        assert(hasDisplayController());
        return *m_displayController;
    }

    IDisplayController& Renderer::getDisplayController()
    {
        assert(hasDisplayController());
        return *m_displayController;
    }

    bool Renderer::hasDisplayController() const
    {
        return m_displayController != nullptr;
    }

    DisplayEventHandler& Renderer::getDisplayEventHandler()
    {
        return m_displayEventHandler;
    }

    void Renderer::createDisplayContext(const DisplayConfigData& displayConfig)
    {
        LOG_TRACE(CONTEXT_PROFILING, "Renderer::createDisplayContext start creating display");

        assert(!m_displayController);
        m_displayController.reset(createDisplayControllerFromConfig(displayConfig));
        if (!m_displayController)
        {
            LOG_ERROR(CONTEXT_RENDERER, "RamsesRenderer::createDisplayContext: failed to create a display controller");
            return;
        }

        m_frameBufferDeviceHandle = m_displayController->getDisplayBuffer();
        m_displayBuffersSetup.registerDisplayBuffer(m_frameBufferDeviceHandle, { 0, 0, m_displayController->getDisplayWidth(), m_displayController->getDisplayHeight() },
            DefaultClearColor, false, displayConfig.getAntialiasingSampleCount(), false);
        setClearColor(m_frameBufferDeviceHandle, displayConfig.getClearColor());

        LOG_TRACE(CONTEXT_PROFILING, "RamsesRenderer::createDisplayContext finished creating display");
    }

    void Renderer::destroyDisplayContext()
    {
        assert(hasDisplayController());
        assert(!hasAnyBufferWithInterruptedRendering());

        if (m_platform.getSystemCompositorController() != nullptr)
            systemCompositorDestroyIviSurface(m_displayController->getRenderBackend().getWindow().getWaylandIviSurfaceID());

        m_displayController.reset();
        m_platform.destroyRenderBackend();
    }

    const DisplaySetup& Renderer::getDisplaySetup() const
    {
        assert(hasDisplayController());
        return m_displayBuffersSetup;
    }

    void Renderer::systemCompositorListIviSurfaces() const
    {
        if (m_platform.getSystemCompositorController())
        {
            m_platform.getSystemCompositorController()->listIVISurfaces();
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "RamsesRenderer::systemCompositorListIviSurfaces: system compositor is not available");
        }
    }

    void Renderer::systemCompositorSetIviSurfaceVisibility(WaylandIviSurfaceId surfaceId, bool visibility) const
    {
        if (m_platform.getSystemCompositorController())
        {
            m_platform.getSystemCompositorController()->setSurfaceVisibility(surfaceId, visibility);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "RamsesRenderer::systemCompositorSetSurfaceVisibility: system compositor is not available");
        }
    }

    void Renderer::systemCompositorSetIviSurfaceOpacity(WaylandIviSurfaceId surfaceId, float opacity) const
    {
        if (m_platform.getSystemCompositorController())
        {
            m_platform.getSystemCompositorController()->setSurfaceOpacity(surfaceId, opacity);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "RamsesRenderer::systemCompositorSetOpacity: system compositor is not available");
        }
    }

    void Renderer::systemCompositorSetIviSurfaceDestRectangle(WaylandIviSurfaceId surfaceId, int32_t x, int32_t y, int32_t width, int32_t height) const
    {
        if (m_platform.getSystemCompositorController())
        {
            m_platform.getSystemCompositorController()->setSurfaceDestinationRectangle(surfaceId, x, y, width, height);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "RamsesRenderer::systemCompositorSetRectangle: system compositor is not available");
        }
    }

    void Renderer::systemCompositorSetIviLayerVisibility(WaylandIviLayerId layerId, bool visibility) const
    {
        if (m_platform.getSystemCompositorController())
        {
            m_platform.getSystemCompositorController()->setLayerVisibility(layerId, visibility);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "RamsesRenderer::systemCompositorSetLayerVisibility: system compositor is not available");
        }
    }

    void Renderer::systemCompositorScreenshot(std::string_view fileName, int32_t screenIviId) const
    {
        if (m_platform.getSystemCompositorController())
        {
            m_platform.getSystemCompositorController()->doScreenshot(fileName, screenIviId);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "RamsesRenderer::systemCompositorScreenshot: system compositor is not available");
        }
    }

    bool Renderer::systemCompositorAddIviSurfaceToIviLayer(WaylandIviSurfaceId surfaceId, WaylandIviLayerId layerId) const
    {
        if (m_platform.getSystemCompositorController())
        {
            return m_platform.getSystemCompositorController()->addSurfaceToLayer(surfaceId, layerId);
        }
        LOG_ERROR(CONTEXT_RENDERER, "RamsesRenderer::systemCompositorAddSurfaceToLayer: system compositor is not available");

        return true;
    }

    void Renderer::systemCompositorRemoveIviSurfaceFromIviLayer(WaylandIviSurfaceId surfaceId, WaylandIviLayerId layerId) const
    {
        if (m_platform.getSystemCompositorController())
        {
            m_platform.getSystemCompositorController()->removeSurfaceFromLayer(surfaceId, layerId);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "RamsesRenderer::systemCompositorRemoveSurfaceFromLayer: system compositor is not available");
        }
    }

    void Renderer::systemCompositorDestroyIviSurface(WaylandIviSurfaceId surfaceId) const
    {
        if (m_platform.getSystemCompositorController())
        {
            m_platform.getSystemCompositorController()->destroySurface(surfaceId);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "RamsesRenderer::systemCompositorDestroySurface: system compositor is not available");
        }
    }

    void Renderer::handleDisplayEvents()
    {
        m_displayController->handleWindowEvents();
        const bool canRenderFrame = m_displayController->canRenderNewFrame();
        if (canRenderFrame != m_canRenderFrame)
        {
            m_canRenderFrame = canRenderFrame;
            if (canRenderFrame)
            {
                LOG_INFO(CONTEXT_RENDERER, "Renderer::doOneRenderLoop start rendering frames on display");
            }
            else
            {
                LOG_INFO(CONTEXT_RENDERER, "Renderer::doOneRenderLoop will skip frames because window is not ready for rendering on display");
            }
        }
    }

    bool Renderer::renderToFramebuffer()
    {
        assert(m_canRenderFrame);
        const DisplayBufferInfo& displayBufferInfo = m_displayBuffersSetup.getDisplayBuffer(m_frameBufferDeviceHandle);
        if (!displayBufferInfo.needsRerender)
        {
            // notify clients even if nothing rendered but frame was consumed
            m_displayController->getEmbeddedCompositingManager().notifyClients();
            return false;
        }


        RenderingContext renderContext;
        renderContext.displayBufferDeviceHandle = m_frameBufferDeviceHandle;
        renderContext.viewportWidth = displayBufferInfo.viewport.width;
        renderContext.viewportHeight = displayBufferInfo.viewport.height;
        renderContext.displayBufferClearPending = displayBufferInfo.clearFlags;
        renderContext.displayBufferClearColor = displayBufferInfo.clearColor;
        renderContext.displayBufferDepthDiscard = false; // discarding is not meant for default framebuffer, see Device_GL::discardDepthStencil()

        // FB was marked for re-render but has no shown scenes -> clear it
        const auto& assignedScenes = displayBufferInfo.scenes;
        const bool hasAnyShownScene = std::any_of(std::cbegin(assignedScenes), std::cend(assignedScenes), [](const auto& s) { return s.shown; });
        if (!hasAnyShownScene)
            m_displayController->clearBuffer(m_frameBufferDeviceHandle, displayBufferInfo.clearFlags, displayBufferInfo.clearColor);

        for (const auto& sceneInfo : assignedScenes)
        {
            if (sceneInfo.shown)
            {
                const RendererCachedScene& scene = m_rendererScenes.getScene(sceneInfo.sceneId);
                m_displayController->renderScene(scene, renderContext, nullptr);
                onSceneWasRendered(scene);
            }
        }

        processScheduledScreenshots(m_frameBufferDeviceHandle);

        m_displayBuffersSetup.setDisplayBufferToBeRerendered(m_frameBufferDeviceHandle, false);

        return true;
    }

    void Renderer::renderToOffscreenBuffers()
    {
        assert(m_canRenderFrame);

        const auto& displayBuffersToRender = m_displayBuffersSetup.getNonInterruptibleOffscreenBuffersToRender();
        if (displayBuffersToRender.empty())
            return;

        for (const auto displayBuffer : displayBuffersToRender)
        {
            const auto& displayBufferInfo = m_displayBuffersSetup.getDisplayBuffer(displayBuffer);

            RenderingContext renderContext;
            renderContext.displayBufferDeviceHandle = displayBuffer;
            renderContext.viewportWidth = displayBufferInfo.viewport.width;
            renderContext.viewportHeight = displayBufferInfo.viewport.height;
            renderContext.displayBufferClearPending = displayBufferInfo.clearFlags;
            renderContext.displayBufferClearColor = displayBufferInfo.clearColor;

            m_tempScenesToRender.clear();
            for (const auto& assignedScene : displayBufferInfo.scenes)
            {
                if (assignedScene.shown)
                    m_tempScenesToRender.push_back(assignedScene.sceneId);
            }

            // OB was marked for re-render but has no shown scenes -> clear it
            if (m_tempScenesToRender.empty())
                m_displayController->clearBuffer(displayBuffer, displayBufferInfo.clearFlags, displayBufferInfo.clearColor);

            for (const auto& sceneId : m_tempScenesToRender)
            {
                // offscreen buffer depth component may be discarded after last scene rendered into it and it is not kept for next frame (clear is enabled)
                if (sceneId == m_tempScenesToRender.back()
                    && displayBufferInfo.clearFlags.isSet(EClearFlag::Depth) && displayBufferInfo.clearFlags.isSet(EClearFlag::Stencil))
                    renderContext.displayBufferDepthDiscard = true;

                const RendererCachedScene& scene = m_rendererScenes.getScene(sceneId);
                m_displayController->renderScene(scene, renderContext, nullptr);
                onSceneWasRendered(scene);
            }

            processScheduledScreenshots(displayBuffer);

            m_statistics.offscreenBufferSwapped(displayBuffer, false);
            m_displayBuffersSetup.setDisplayBufferToBeRerendered(displayBuffer, false);
        }
    }

    void Renderer::renderToInterruptibleOffscreenBuffers()
    {
        assert(m_canRenderFrame);

        const auto& displayBuffersToRender = m_displayBuffersSetup.getInterruptibleOffscreenBuffersToRender(m_rendererInterruptState.getInterruptedDisplayBuffer());
        if (displayBuffersToRender.empty())
            return;

        for (const auto displayBuffer : displayBuffersToRender)
        {
            const auto& displayBufferInfo = m_displayBuffersSetup.getDisplayBuffer(displayBuffer);

            RenderingContext renderContext;
            renderContext.displayBufferDeviceHandle = displayBuffer;
            renderContext.viewportWidth = displayBufferInfo.viewport.width;
            renderContext.viewportHeight = displayBufferInfo.viewport.height;
            renderContext.displayBufferClearPending = EClearFlag::None; // set clear flags accordingly below only if not in interrupted state

            const auto& assignedScenes = displayBufferInfo.scenes;
            if (!m_rendererInterruptState.isInterrupted(displayBuffer))
            {
                // remove buffer from list of buffers to re-render as soon as we start rendering into it (even if it gets interrupted later on)
                m_displayBuffersSetup.setDisplayBufferToBeRerendered(displayBuffer, false);
                // clear buffer only if we just started rendering into it and no scene shown to keep expected behavior (no scene -> clear buffer)
                const bool hasAnyShownScene = std::any_of(std::cbegin(assignedScenes), std::cend(assignedScenes), [](const auto& s) { return s.shown; });
                if (!hasAnyShownScene)
                {
                    m_displayController->clearBuffer(displayBuffer, displayBufferInfo.clearFlags, displayBufferInfo.clearColor);
                }
                else
                {
                    renderContext.displayBufferClearPending = displayBufferInfo.clearFlags;
                    renderContext.displayBufferClearColor = displayBufferInfo.clearColor;
                }
            }

            for (const auto& sceneInfo : assignedScenes)
            {
                if (!sceneInfo.shown)
                    continue;

                // if there was any rendering interrupted, skip till get to the interrupted scene
                const SceneId sceneId = sceneInfo.sceneId;
                if (m_rendererInterruptState.isInterrupted() && !m_rendererInterruptState.isInterrupted(displayBuffer, sceneId))
                    continue;

                const RendererCachedScene& scene = m_rendererScenes.getScene(sceneId);
                renderContext.renderFrom = m_rendererInterruptState.getExecutorState();
                const SceneRenderExecutionIterator interruptState = m_displayController->renderScene(scene, renderContext, &m_frameTimer);

                if (RendererInterruptState::IsInterrupted(interruptState))
                {
                    m_rendererInterruptState = RendererInterruptState{ displayBuffer, sceneId, interruptState };
                    LOG_TRACE(CONTEXT_PROFILING, "Renderer::renderToInterruptibleOffscreenBuffers interrupted rendering to OB {}, scene {}", displayBuffer.asMemoryHandle(), sceneId.getValue());
                    m_statistics.offscreenBufferInterrupted(displayBuffer);
                    break;
                }
                m_rendererInterruptState = RendererInterruptState{};

                onSceneWasRendered(scene);
                LOG_TRACE(CONTEXT_PROFILING, "Renderer::renderToInterruptibleOffscreenBuffers scene fully rendered to interruptible OB {}, scene {}", displayBuffer.asMemoryHandle(), sceneId.getValue());
            }

            if (m_rendererInterruptState.isInterrupted())
                break;

            processScheduledScreenshots(displayBuffer);

            m_displayController->getRenderBackend().getDevice().swapDoubleBufferedRenderTarget(displayBuffer);
            m_statistics.offscreenBufferSwapped(displayBuffer, true);
            LOG_TRACE(CONTEXT_PROFILING, "Renderer::renderToInterruptibleOffscreenBuffers interruptible OB {} swapped", displayBuffer.asMemoryHandle());

            //re-render framebuffer in next frame to reflect (finished!) changes in OB
            m_displayBuffersSetup.setDisplayBufferToBeRerendered(m_frameBufferDeviceHandle, true);
        }
    }

    void Renderer::doOneRenderLoop()
    {
        if (!hasDisplayController())
            return;

        LOG_TRACE(CONTEXT_PROFILING, "Renderer::doOneRenderLoop begin");


        m_profilerStatistics.startRegion(FrameProfilerStatistics::ERegion::HandleDisplayEvents);
        {
            m_traceId = 100;
            handleDisplayEvents();
        }
        m_profilerStatistics.endRegion(FrameProfilerStatistics::ERegion::HandleDisplayEvents);

        m_profilerStatistics.startRegion(FrameProfilerStatistics::ERegion::DrawScenes);
        bool swapBuffers = false;
        if (m_canRenderFrame)
        {
            m_traceId = 104;
            // FRAMEBUFFER AND OFFSCREEN BUFFERS
            LOG_TRACE(CONTEXT_PROFILING, "Renderer::doOneRenderLoop begin frame to offscreen buffers");
            renderToOffscreenBuffers();
            LOG_TRACE(CONTEXT_PROFILING, "Renderer::doOneRenderLoop finished frame to offscreen buffers");

            LOG_TRACE(CONTEXT_PROFILING, "Renderer::doOneRenderLoop begin frame to backbuffer");
            swapBuffers = renderToFramebuffer();
            LOG_TRACE(CONTEXT_PROFILING, "Renderer::doOneRenderLoop finished frame to backbuffer");

            // INTERRUPTIBLE OFFSCREEN BUFFERS
            LOG_TRACE(CONTEXT_PROFILING, "Renderer::doOneRenderLoop begin frame to interruptible offscreen buffers");
            renderToInterruptibleOffscreenBuffers();
            LOG_TRACE(CONTEXT_PROFILING, "Renderer::doOneRenderLoop finished frame to interruptible offscreen buffers");
        }
        m_profilerStatistics.endRegion(FrameProfilerStatistics::ERegion::DrawScenes);

        // SWAP BUFFERS
        m_profilerStatistics.startRegion(FrameProfilerStatistics::ERegion::SwapBuffersAndNotifyClients);
        if (swapBuffers)
        {
            m_traceId = 105;
            m_displayController->swapBuffers();
            m_traceId = 106;
            m_statistics.framebufferSwapped();
            m_traceId = 107;
            m_displayController->getEmbeddedCompositingManager().notifyClients();
            LOG_TRACE(CONTEXT_PROFILING, "Renderer::doOneRenderLoop swapBuffers");
        }
        m_profilerStatistics.endRegion(FrameProfilerStatistics::ERegion::SwapBuffersAndNotifyClients);

        LOG_TRACE(CONTEXT_PROFILING, "Renderer::doOneRenderLoop end");
    }

    void Renderer::onSceneWasRendered(const RendererCachedScene& scene)
    {
        scene.markAllRenderOncePassesAsRendered();
        m_expirationMonitor.onRendered(scene.getSceneId());
        m_statistics.sceneRendered(scene.getSceneId());
    }

    void Renderer::assignSceneToDisplayBuffer(SceneId sceneId, DeviceResourceHandle buffer, int32_t globalSceneOrder)
    {
        assert(hasDisplayController());
        assert(m_rendererScenes.hasScene(sceneId));

        std::ignore = getBufferSceneIsAssignedTo(sceneId); // TODO: remove or assert
        m_displayBuffersSetup.assignSceneToDisplayBuffer(sceneId, buffer, globalSceneOrder);
    }

    void Renderer::unassignScene(SceneId sceneId)
    {
        assert(m_rendererScenes.hasScene(sceneId));
        m_displayBuffersSetup.unassignScene(sceneId);
    }

    void Renderer::setSceneShown(SceneId sceneId, bool show)
    {
        assert(m_rendererScenes.hasScene(sceneId));
        assert(getBufferSceneIsAssignedTo(sceneId).isValid());
        m_displayBuffersSetup.setSceneShown(sceneId, show);
    }

    void Renderer::markBufferWithSceneForRerender(SceneId sceneId)
    {
        const auto displayBuffer = getBufferSceneIsAssignedTo(sceneId);
        assert(displayBuffer.isValid());
        m_displayBuffersSetup.setDisplayBufferToBeRerendered(displayBuffer, true);
    }

    DeviceResourceHandle Renderer::getBufferSceneIsAssignedTo(SceneId sceneId) const
    {
        return m_displayBuffersSetup.findDisplayBufferSceneIsAssignedTo(sceneId);
    }

    bool Renderer::isSceneAssignedToInterruptibleOffscreenBuffer(SceneId sceneId) const
    {
        const auto displayBuffer = getBufferSceneIsAssignedTo(sceneId);
        assert(displayBuffer.isValid());

        return m_displayBuffersSetup.getDisplayBuffer(displayBuffer).isInterruptible;
    }

    int32_t Renderer::getSceneGlobalOrder(SceneId sceneId) const
    {
        const auto displayBuffer = getBufferSceneIsAssignedTo(sceneId);
        assert(displayBuffer.isValid());

        const auto& assignedScenes = m_displayBuffersSetup.getDisplayBuffer(displayBuffer).scenes;
        const auto it = std::find_if(assignedScenes.cbegin(), assignedScenes.cend(), [sceneId](const auto& a) { return a.sceneId == sceneId; });
        assert(it != assignedScenes.cend());

        return it->globalSceneOrder;
    }

    RendererStatistics& Renderer::getStatistics()
    {
        return m_statistics;
    }

    FrameProfilerStatistics& Renderer::getProfilerStatistics()
    {
        return m_profilerStatistics;
    }

    bool Renderer::hasSystemCompositorController() const
    {
        return m_platform.getSystemCompositorController() != nullptr;
    }

    IDisplayController* Renderer::createDisplayControllerFromConfig(const DisplayConfigData& config)
    {
        IRenderBackend* renderBackend = m_platform.createRenderBackend(config, m_displayEventHandler);
        if (nullptr == renderBackend)
            return nullptr;

        if (m_platform.getSystemCompositorController())
        {
            const WaylandIviSurfaceId rendererSurfaceIVIID(config.getWaylandIviSurfaceID());
            const WaylandIviLayerId rendererLayerIVIID(config.getWaylandIviLayerID());

            if (!systemCompositorAddIviSurfaceToIviLayer(rendererSurfaceIVIID, rendererLayerIVIID))
                return nullptr;

            systemCompositorSetIviSurfaceVisibility(rendererSurfaceIVIID, config.getStartVisibleIvi());
            systemCompositorSetIviSurfaceDestRectangle(rendererSurfaceIVIID,
                config.getWindowPositionX(),
                config.getWindowPositionY(),
                static_cast<int32_t>(config.getDesiredWindowWidth()),
                static_cast<int32_t>(config.getDesiredWindowHeight()));
        }

        // Enable the context of the render backend that was just created
        // The initialization of display controller below needs active context
        renderBackend->getContext().enable();

        const uint32_t numSamples = config.getAntialiasingSampleCount();

        return new DisplayController(*renderBackend, numSamples);
    }

    void Renderer::setClearFlags(DeviceResourceHandle bufferDeviceHandle, ClearFlags clearFlags)
    {
        assert(hasDisplayController());
        m_displayBuffersSetup.setClearFlags(bufferDeviceHandle, clearFlags);
    }

    void Renderer::setClearColor(DeviceResourceHandle bufferDeviceHandle, const glm::vec4& clearColor)
    {
        assert(hasDisplayController());
        m_displayBuffersSetup.setClearColor(bufferDeviceHandle, clearColor);
    }

    bool Renderer::setExternallyOwnedWindowSize(uint32_t width, uint32_t height)
    {
        assert(hasDisplayController());
        IRenderBackend& renderBackend = m_displayController->getRenderBackend();
        IWindow& window = renderBackend.getWindow();

        const bool result = window.setExternallyOwnedWindowSize(width, height);
        if(result)
            m_displayBuffersSetup.setDisplayBufferSize(m_displayController->getDisplayBuffer(), width, height);

        return result;
    }

    void Renderer::scheduleScreenshot(DeviceResourceHandle renderTargetHandle, ScreenshotInfo&& screenshot)
    {
        assert(hasDisplayController());
        if (m_screenshots.count(renderTargetHandle) != 0u)
            LOG_WARN(CONTEXT_RENDERER, "Renderer::scheduleScreenshot: will overwrite previous screenshot request that was not executed yet (buffer={})", renderTargetHandle);

        m_screenshots[renderTargetHandle] = std::move(screenshot);

        // re-render all buffers that the screenshot might depend on
        for (const auto& buffer : m_displayBuffersSetup.getDisplayBuffers())
            m_displayBuffersSetup.setDisplayBufferToBeRerendered(buffer.first, true);
    }

    void Renderer::processScheduledScreenshots(DeviceResourceHandle renderTargetHandle)
    {
        assert(hasDisplayController());
        auto it = m_screenshots.find(renderTargetHandle);
        if (it == m_screenshots.end())
            return;

        ScreenshotInfo& screenshot = it->second;
        assert(screenshot.rectangle.width > 0u && screenshot.rectangle.height > 0u);
        if (!screenshot.pixelData.empty())
            return;

        m_displayController->readPixels(renderTargetHandle, screenshot.rectangle.x, screenshot.rectangle.y, screenshot.rectangle.width, screenshot.rectangle.height, screenshot.pixelData);
        assert(!screenshot.pixelData.empty());
    }

    std::vector<std::pair<DeviceResourceHandle, ScreenshotInfo>> Renderer::dispatchProcessedScreenshots()
    {
        std::vector<std::pair<DeviceResourceHandle, ScreenshotInfo>> result;
        for (auto& it : m_screenshots)
        {
            const auto rtHandle = it.first;
            auto& screenshot = it.second;
            if (!screenshot.pixelData.empty())
                result.emplace_back(rtHandle, std::move(screenshot));
        }

        for (const auto& it : result)
            m_screenshots.erase(it.first);

        return result;
    }

    bool Renderer::hasAnyBufferWithInterruptedRendering() const
    {
        return m_rendererInterruptState.isInterrupted();
    }

    void Renderer::resetRenderInterruptState()
    {
        LOG_TRACE(CONTEXT_PROFILING, "Renderer::resetRenderInterruptState");
        m_rendererInterruptState = RendererInterruptState{};
    }

    void Renderer::updateSystemCompositorController() const
    {
         if (m_platform.getSystemCompositorController())
        {
            LOG_TRACE(CONTEXT_PROFILING, "Renderer::updateSystemCompositorController update system compositor controller");
            m_platform.getSystemCompositorController()->update();
        }
    }
}
