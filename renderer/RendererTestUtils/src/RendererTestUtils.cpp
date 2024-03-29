//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererTestUtils.h"
#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/IRendererEventHandler.h"
#include "RamsesRendererImpl.h"
#include "DisplayConfigImpl.h"
#include "RendererConfigImpl.h"
#include "Utils/Image.h"
#include "Utils/File.h"
#include "RendererAndSceneTestEventHandler.h"
#include "RendererAPI/IRenderBackend.h"
#include "ReadPixelCallbackHandler.h"

using namespace ramses_internal;

const float RendererTestUtils::DefaultMaxAveragePercentPerPixel = 0.2f;
std::optional<std::chrono::microseconds> RendererTestUtils::MaxFrameCallbackPollingTime;
std::optional<std::string> RendererTestUtils::WaylandDisplayForSystemCompositorController;
std::unique_ptr<ramses::RendererConfig> RendererTestUtils::defaultRendererConfig = std::make_unique<ramses::RendererConfig>();
std::unique_ptr<ramses::DisplayConfig> RendererTestUtils::defaultDisplayConfig = std::make_unique<ramses::DisplayConfig>();

namespace
{
    // If multiple platform backends are created at the same time, they have to have different id's
    // The numbers have special meaning, therefore the test uses 100, 101, 102, ... to make sure they
    // don't clash with existing applications
    const uint32_t firstIviSurfaceId = 10010;
}

void RendererTestUtils::SaveScreenshotForDisplay(ramses::RamsesRenderer& renderer, ramses::displayId_t displayId, ramses::displayBufferId_t displayBuffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const std::string& screenshotFileName)
{
    const Image screenshotBitmap = ReadPixelData(renderer, displayId, displayBuffer, x, y, width, height);
    screenshotBitmap.saveToFilePNG("./res/" + screenshotFileName + ".PNG");
}

bool RendererTestUtils::PerformScreenshotTestForDisplay(
    ramses::RamsesRenderer& renderer,
    ramses::displayId_t displayId,
    ramses::displayBufferId_t displayBuffer,
    uint32_t x, uint32_t y, uint32_t width, uint32_t height,
    const std::string& screenshotFileName,
    float maxAveragePercentErrorPerPixel,
    bool saveDiffOnError)
{
    const Image screenshotBitmap = ReadPixelData(renderer, displayId, displayBuffer, x, y, width, height);
    return CompareBitmapToImageInFile(screenshotBitmap, screenshotFileName, maxAveragePercentErrorPerPixel, saveDiffOnError);
}

bool RendererTestUtils::CompareBitmapToImageInFile(const Image& actualBitmap, const std::string& expectedScreenshotFileName, float maxAveragePercentErrorPerPixel, bool saveDiffOnError)
{
    Image expectedBitmap;
    expectedBitmap.loadFromFilePNG("./res/" + expectedScreenshotFileName + ".PNG");
    if (expectedBitmap.getWidth() != actualBitmap.getWidth() ||
        expectedBitmap.getHeight() != actualBitmap.getHeight())
    {
        printf("Screenshot comparison failed: size of expected image %u/%u does not match size of actual image %u/%u\n",
            expectedBitmap.getWidth(), expectedBitmap.getHeight(), actualBitmap.getWidth(), actualBitmap.getHeight());

        return false;
    }
    const Image diff = expectedBitmap.createDiffTo(actualBitmap);
    assert(diff.getNumberOfPixels() == actualBitmap.getNumberOfPixels());

    const auto sumOfPixelDiff = diff.getSumOfPixelValues();
    const uint32_t totalNumberOfPixels = diff.getNumberOfPixels();

    if (totalNumberOfPixels == 0)
    {
        assert(false);
        printf("Screenshot comparison failed: difference bitmap has no pixels!\n");
        return false;
    }

    const float averagePercentErrorPerPixel = (100.0f * (sumOfPixelDiff.x + sumOfPixelDiff.y + sumOfPixelDiff.z + sumOfPixelDiff.w) / 256.0f) / totalNumberOfPixels;

    if (averagePercentErrorPerPixel > maxAveragePercentErrorPerPixel)
    {
        if (saveDiffOnError)
        {
            printf("Screenshot comparison failed: %s (%ux%u)\n", expectedScreenshotFileName.c_str(), diff.getWidth(), diff.getHeight());
            printf(" - avg error per pixel %.2f, maximum allowed error is %.2f\n", averagePercentErrorPerPixel, maxAveragePercentErrorPerPixel);
            printf(" - total error R=%i, G=%i, B=%i, A=%i\n", sumOfPixelDiff.x, sumOfPixelDiff.y, sumOfPixelDiff.z, sumOfPixelDiff.w);
            printf(" - number of pixels different by more than 1 (one or more color channels): %u\n", diff.getNumberOfNonBlackPixels(1));
            printf(" - number of pixels different by more than 64 (one or more color channels): %u\n", diff.getNumberOfNonBlackPixels(64));
            printf(" - number of pixels different by more than 128 (one or more color channels): %u\n", diff.getNumberOfNonBlackPixels(128));
            printf(" - number of pixels different by 255 (one or more color channels): %u\n", diff.getNumberOfNonBlackPixels(254));

            const std::string screenShotPath{"./res/diffs"};
            File diffDirectory(screenShotPath);
            diffDirectory.createDirectory();

            actualBitmap.saveToFilePNG(screenShotPath + "/" + expectedScreenshotFileName + "_ACTUAL.PNG");
            diff.saveToFilePNG(screenShotPath + "/" + expectedScreenshotFileName + "_DIFF.PNG");

            //Create separate RGB and Alpha diffs
            std::pair<Image, Image> colorAndAlphaDiffImages = diff.createSeparateColorAndAlphaImages();
            colorAndAlphaDiffImages.first.saveToFilePNG(screenShotPath + "/" + expectedScreenshotFileName + "_DIFF_RGB.PNG");
            colorAndAlphaDiffImages.second.saveToFilePNG(screenShotPath + "/" + expectedScreenshotFileName + "_DIFF_ALPHA.PNG");
        }

        return false;
    }

    return true;
}

ramses::displayId_t RendererTestUtils::CreateDisplayImmediate(ramses::RamsesRenderer& renderer, const ramses::DisplayConfig& displayConfig)
{
    const ramses::displayId_t displayId = renderer.createDisplay(displayConfig);
    renderer.flush();
    ramses::RendererAndSceneTestEventHandler eventHandler(renderer);
    if (eventHandler.waitForDisplayCreation(displayId))
    {
        return displayId;
    }

    assert(false && "Display construction failed or timed out!");
    return ramses::displayId_t::Invalid();
}

void RendererTestUtils::DestroyDisplayImmediate(ramses::RamsesRenderer& renderer, ramses::displayId_t displayId)
{
    renderer.destroyDisplay(displayId);
    renderer.flush();
    ramses::RendererAndSceneTestEventHandler eventHandler(renderer);
    eventHandler.registerAlreadyCreatedDisplay(displayId);
    if (!eventHandler.waitForDisplayDestruction(displayId))
    {
        assert(false && "Display destruction failed or timed out!");
    }
}

ramses::RendererConfig RendererTestUtils::CreateTestRendererConfig()
{
    ramses::RendererConfig rendererConfig(*defaultRendererConfig);
    ramses_internal::RendererConfig& internalRendererConfig = const_cast<ramses_internal::RendererConfig&>(rendererConfig.m_impl.get().getInternalRendererConfig());

    if (WaylandDisplayForSystemCompositorController.has_value())
    {
        internalRendererConfig.enableSystemCompositorControl();
        internalRendererConfig.setWaylandDisplayForSystemCompositorController(WaylandDisplayForSystemCompositorController.value());
    }

    if(MaxFrameCallbackPollingTime.has_value())
        internalRendererConfig.setFrameCallbackMaxPollTime(MaxFrameCallbackPollingTime.value());

    return rendererConfig;
}

void RendererTestUtils::SetWaylandDisplayForSystemCompositorControllerForAllTests(const std::string& wd)
{
    WaylandDisplayForSystemCompositorController = wd;
}

void RendererTestUtils::SetDefaultConfigForAllTests(const ramses::RendererConfig& rendererConfig, const ramses::DisplayConfig& displayConfig)
{
    defaultRendererConfig = std::make_unique<ramses::RendererConfig>(rendererConfig);
    defaultDisplayConfig = std::make_unique<ramses::DisplayConfig>(displayConfig);
}

void RendererTestUtils::SetMaxFrameCallbackPollingTimeForAllTests(std::chrono::microseconds time)
{
    MaxFrameCallbackPollingTime = time;
}

ramses::DisplayConfig RendererTestUtils::CreateTestDisplayConfig(uint32_t iviSurfaceIdOffset, bool iviWindowStartVisible)
{
    ramses::DisplayConfig displayConfig(*defaultDisplayConfig);
    displayConfig.setWaylandIviSurfaceID(ramses::waylandIviSurfaceId_t(firstIviSurfaceId + iviSurfaceIdOffset));

    if(iviWindowStartVisible)
        displayConfig.setWindowIviVisible();

    return displayConfig;
}

Image RendererTestUtils::ReadPixelData(
    ramses::RamsesRenderer& renderer,
    ramses::displayId_t displayId,
    ramses::displayBufferId_t displayBuffer,
    uint32_t x,
    uint32_t y,
    uint32_t width,
    uint32_t height)
{
    const ramses::status_t status = renderer.readPixels(displayId, displayBuffer, x, y, width, height);
    assert(status == ramses::StatusOK);
    UNUSED(status);
    renderer.flush();

    constexpr std::chrono::seconds timeoutTime{ 10u };
    const auto startTime = std::chrono::steady_clock::now();
    ReadPixelCallbackHandler callbackHandler;
    while (true)
    {
        if (!renderer.m_impl.isThreaded())
            renderer.doOneLoop();

        renderer.dispatchEvents(callbackHandler);
        if (callbackHandler.m_pixelDataRead)
            break;

        if ((std::chrono::steady_clock::now() - startTime) > timeoutTime)
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererTestUtils::ReadPixelData: ran out of time!");
            return Image{};
        }
        std::this_thread::sleep_for(std::chrono::milliseconds{ 5 });
    }

    assert(callbackHandler.m_pixelData.size() == width * height * 4u);

    // flip image vertically so that the layout read from frame buffer (bottom-up)
    // is converted to layout normally used in image files (top-down)
    return Image(width, height, callbackHandler.m_pixelData.cbegin(), callbackHandler.m_pixelData.cend(), true);
}
