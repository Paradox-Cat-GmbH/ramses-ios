//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-renderer-api/DisplayConfig.h"
#include "DisplayConfigImpl.h"
#include "RendererConfigImpl.h"
#include "RendererTestUtils.h"
#include "WindowEventHandlerMock.h"
#include "Resource/EffectResource.h"
#include "Resource/ResourceTypes.h"
#include "RendererAPI/IRenderBackend.h"
#include "Platform_Base/Device_Base.h"
#include "PlatformFactory/PlatformFactory.h"
#include <memory>

using namespace testing;

namespace ramses_internal
{
    class ADevice : public Test
    {
    public:
        ADevice()
        {
            // Need to "read" the error state in Device_GL once, in order to reset the error state to NO_ERROR
            // This is needed because all tests share the same device to save time
            std::ignore = testDevice->isDeviceStatusHealthy();
        }

        static EffectResource* CreateTestEffectResource()
        {
            const std::string vertexShader(
                "#version 100                            \n"
                "                                        \n"
                "attribute vec3 a_position;              \n"
                "varying vec3 v_position;                \n"
                "                                        \n"
                "void main()                             \n"
                "{                                       \n"
                "    gl_Position = vec4(a_position, 1.0);\n"
                "    v_position = a_position;            \n"
                "}                                       \n"
            );

            const std::string fragmentShader(
                "#version 100                                                     \n"
                "                                                                 \n"
                "varying highp vec3 v_position;                                   \n"
                "                                                                 \n"
                "uniform highp float u_float;                                     \n"
                "uniform highp vec2 u_vec2;                                       \n"
                "                                                                 \n"
                "void main(void)                                                  \n"
                "{                                                                \n"
                "    gl_FragColor = vec4(v_position + vec3(u_vec2, 1.0), u_float);\n"
                "}                                                                \n"
            );

            EffectInputInformationVector uniformInputs;
            uniformInputs.push_back(EffectInputInformation("u_float", 1, EDataType::Float, EFixedSemantics::Invalid));
            uniformInputs.push_back(EffectInputInformation("u_vec2", 1, EDataType::Vector2F, EFixedSemantics::Invalid));

            EffectInputInformationVector attributeInputs;
            attributeInputs.push_back(EffectInputInformation("a_position", 1, EDataType::Vector3F, EFixedSemantics::Invalid));

            return new EffectResource(vertexShader, fragmentShader, "", {}, uniformInputs, attributeInputs, "test effect", ResourceCacheFlag_DoNotCache);
        }

        static void SetUpTestCase()
        {
            assert(nullptr == platform);

            ramses::RendererConfig rendererConfig = RendererTestUtils::CreateTestRendererConfig();
            ramses::DisplayConfig dispConfig = RendererTestUtils::CreateTestDisplayConfig(0u);

            PlatformFactory platformFactory;
            platform = platformFactory.createPlatform(rendererConfig.m_impl.get().getInternalRendererConfig(), dispConfig.m_impl.get().getInternalDisplayConfig());
            assert(nullptr != platform);

            eventHandler = new NiceMock<WindowEventHandlerMock>();

            ramses::DisplayConfig displayConfig = RendererTestUtils::CreateTestDisplayConfig(0);
            displayConfig.setWindowRectangle(0, 0, 16u, 16u);
            renderBackend = platform->createRenderBackend(displayConfig.m_impl.get().getInternalDisplayConfig(), *eventHandler);
            assert(nullptr != renderBackend);

            testDevice = &renderBackend->getDevice();
        }

        static void TearDownTestCase()
        {
            testDevice = nullptr;

            platform->destroyRenderBackend();
            renderBackend = nullptr;

            platform.reset();

            delete eventHandler;
            eventHandler = nullptr;
        }

    protected:
        static IDevice*                             testDevice;

    private:
        static std::unique_ptr<IPlatform>           platform;
        static IRenderBackend*                      renderBackend;
        static NiceMock<WindowEventHandlerMock>*    eventHandler;
    };

    std::unique_ptr<IPlatform>          ADevice::platform                    ;
    IDevice*                            ADevice::testDevice         = nullptr;
    IRenderBackend*                     ADevice::renderBackend      = nullptr;
    NiceMock<WindowEventHandlerMock>*   ADevice::eventHandler       = nullptr;


    // Needed so that these tests can be blacklisted on drivers which don't support binary shaders
    class ADeviceSupportingBinaryShaders : public ADevice
    {
    };

    // Needed so that these tests can be blacklisted on drivers which don't support geometry shaders
    class ADeviceSupportingGeometryShaders : public ADevice
    {
    public:
        static EffectResource* CreateTestEffectResourceWithGeometryShader()
        {
            const std::string vertexShader(R"SHADER(
                #version 320 es

                in highp vec3 a_position;
                out highp vec3 v_position;

                void main()
                {
                    gl_Position = vec4(a_position, 1.0);
                    v_position = a_position;
                }
                )SHADER");

            const std::string fragmentShader(R"SHADER(
                #version 320 es

                in highp vec3 g_position;
                out highp vec4 color;

                uniform highp float u_float;
                uniform highp vec2 u_vec2;

                void main(void)
                {
                    color = vec4(g_position + vec3(u_vec2, 1.0), u_float);
                }
                )SHADER");

            const std::string geometryShader(R"SHADER(
                #version 320 es

                layout(points) in;
                layout(points, max_vertices = 1) out;
                uniform highp float u_geomFloat;

                in highp vec3 v_position[];
                out highp vec3 g_position;

                void main() {

                    gl_Position = vec4(u_geomFloat);
                    g_position = v_position[0];
                    EmitVertex();
                    EndPrimitive();
                }
                )SHADER");
            EffectInputInformationVector uniformInputs;
            uniformInputs.push_back(EffectInputInformation("u_float", 1, EDataType::Float, EFixedSemantics::Invalid));
            uniformInputs.push_back(EffectInputInformation("u_vec2", 1, EDataType::Vector2F, EFixedSemantics::Invalid));
            uniformInputs.push_back(EffectInputInformation("u_geomFloat", 1, EDataType::Float, EFixedSemantics::Invalid));

            EffectInputInformationVector attributeInputs;
            attributeInputs.push_back(EffectInputInformation("a_position", 1, EDataType::Vector3F, EFixedSemantics::Invalid));

            return new EffectResource(vertexShader, fragmentShader, geometryShader, EDrawMode::Points, uniformInputs, attributeInputs, "test effect", ResourceCacheFlag_DoNotCache);
        }
    };

    TEST_F(ADevice, CreatesShaderFromEffect)
    {
        ASSERT_TRUE(testDevice != nullptr);

        const std::unique_ptr<EffectResource> testEffect(CreateTestEffectResource());
        auto shaderGpuResource = testDevice->uploadShader(*testEffect);
        const DeviceResourceHandle handle = testDevice->registerShader(std::move(shaderGpuResource));
        EXPECT_TRUE(handle.isValid());

        testDevice->deleteShader(handle);
    }

    TEST_F(ADevice, RefusesToUploadEffectWithInvalidVertexShader)
    {
        const std::unique_ptr<EffectResource> templateEffect(CreateTestEffectResource());
        const EffectResource invalidEffect(
            "--this is some invalide shader source code--",
            templateEffect->getFragmentShader(),
            "", {},
            templateEffect->getUniformInputs(),
            templateEffect->getAttributeInputs(),
            "invalid effect", ResourceCacheFlag_DoNotCache);
        const auto shaderGpuResource = testDevice->uploadShader(invalidEffect);
        EXPECT_EQ(nullptr, shaderGpuResource);
    }

    TEST_F(ADevice, RefusesToUploadEffectWithInvalidFragmentShader)
    {
        const std::unique_ptr<EffectResource> templateEffect(CreateTestEffectResource());
        const EffectResource invalidEffect(
            templateEffect->getVertexShader(),
            "--this is some invalide shader source code--",
            "", {},
            templateEffect->getUniformInputs(),
            templateEffect->getAttributeInputs(),
            "invalid effect", ResourceCacheFlag_DoNotCache);
        const auto shaderGpuResource = testDevice->uploadShader(invalidEffect);
        EXPECT_EQ(nullptr, shaderGpuResource);
    }

    TEST_F(ADevice, RefusesToUploadEffectWithUnlinkableShaders)
    {
        const std::unique_ptr<EffectResource> templateEffect(CreateTestEffectResource());
        const std::string fragmentShader(
            "#version 100                                                     \n"
            "                                                                 \n"
            "varying highp float v_position; /* interface mismatch */         \n"
            "                                                                 \n"
            "uniform highp float u_float;                                     \n"
            "uniform highp vec2 u_vec2;                                       \n"
            "                                                                 \n"
            "void main(void)                                                  \n"
            "{                                                                \n"
            "    gl_FragColor = vec4(vec3(v_position) + vec3(u_vec2, 1.0), u_float);\n"
            "}                                                                \n"
            );
        const EffectResource invalidEffect(
            templateEffect->getVertexShader(),
            fragmentShader,
            "", {},
            templateEffect->getUniformInputs(),
            templateEffect->getAttributeInputs(),
            "invalid effect", ResourceCacheFlag_DoNotCache);
        const auto shaderGpuResource = testDevice->uploadShader(invalidEffect);
        EXPECT_EQ(nullptr, shaderGpuResource);
    }

    TEST_F(ADevice, SetsConstantsForAllPossibleDataTypesOnShader)
    {
        ASSERT_TRUE(testDevice != nullptr);

        const std::unique_ptr<EffectResource> templateEffect(CreateTestEffectResource());
        const std::string fragmentShader(
            "#version 100                                                     \n"
            "                                                                 \n"
            "varying highp vec3 v_position;                                   \n"
            "                                                                 \n"
            "uniform highp int u_int;                                         \n"
            "uniform highp float u_float;                                     \n"
            "uniform highp vec2 u_vec2;                                       \n"
            "uniform highp vec3 u_vec3;                                       \n"
            "uniform highp vec4 u_vec4;                                       \n"
            "uniform highp ivec2 u_ivec2;                                     \n"
            "uniform highp ivec3 u_ivec3;                                     \n"
            "uniform highp ivec4 u_ivec4;                                     \n"
            "uniform highp mat2 u_mat2;                                       \n"
            "uniform highp mat3 u_mat3;                                       \n"
            "uniform highp mat4 u_mat4;                                       \n"
            "                                                                 \n"
            "void main(void)                                                  \n"
            "{                                                                \n"
            "    gl_FragColor = vec4(v_position + vec3(u_vec2, 1.0), u_float);\n"
            "}                                                                \n"
            );
        EffectInputInformationVector uniformInputs;
        uniformInputs.push_back(EffectInputInformation("u_int", 1, EDataType::Int32, EFixedSemantics::Invalid));
        uniformInputs.push_back(EffectInputInformation("u_float", 1, EDataType::Float, EFixedSemantics::Invalid));

        uniformInputs.push_back(EffectInputInformation("u_vec2", 1, EDataType::Vector2F, EFixedSemantics::Invalid));
        uniformInputs.push_back(EffectInputInformation("u_vec3", 1, EDataType::Vector3F, EFixedSemantics::Invalid));
        uniformInputs.push_back(EffectInputInformation("u_vec4", 1, EDataType::Vector4F, EFixedSemantics::Invalid));

        uniformInputs.push_back(EffectInputInformation("u_ivec2", 1, EDataType::Vector2I, EFixedSemantics::Invalid));
        uniformInputs.push_back(EffectInputInformation("u_ivec3", 1, EDataType::Vector3I, EFixedSemantics::Invalid));
        uniformInputs.push_back(EffectInputInformation("u_ivec4", 1, EDataType::Vector4I, EFixedSemantics::Invalid));

        uniformInputs.push_back(EffectInputInformation("u_mat2", 1, EDataType::Matrix22F, EFixedSemantics::Invalid));
        uniformInputs.push_back(EffectInputInformation("u_mat3", 1, EDataType::Matrix33F, EFixedSemantics::Invalid));
        uniformInputs.push_back(EffectInputInformation("u_mat4", 1, EDataType::Matrix44F, EFixedSemantics::Invalid));

        const EffectResource testEffect(
            templateEffect->getVertexShader(),
            fragmentShader,
            "", {},
            uniformInputs,
            templateEffect->getAttributeInputs(),
            "uniform test effect", ResourceCacheFlag_DoNotCache);
        auto shaderGpuResource = testDevice->uploadShader(testEffect);
        ASSERT_NE(nullptr, shaderGpuResource);
        const DeviceResourceHandle handle = testDevice->registerShader(std::move(shaderGpuResource));
        ASSERT_TRUE(handle.isValid());

        testDevice->activateShader(handle);

        const int32_t intValue = 5;
        const float floatValue = 5.0f;
        const glm::vec2 vec2Value(5.0f);
        const glm::vec3 vec3Value(5.0f);
        const glm::vec4 vec4Value(5.0f);
        const glm::ivec2 vec2iValue(5);
        const glm::ivec3 vec3iValue(5);
        const glm::ivec4 vec4iValue(5);
        const glm::mat2 mat22Value(1.0f, 2.0f, 3.0f, 4.0f);
        const glm::mat3 mat33Value(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f);
        const glm::mat4 mat44Value(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f);

        EXPECT_TRUE(testDevice->isDeviceStatusHealthy()); // check that there were no errors (GL_NO_ERROR in case of OpenGL) - triggers an internal assert otherwise
        testDevice->setConstant(DataFieldHandle(testEffect.getUniformDataFieldHandleByName("u_int")), 1, &intValue);
        EXPECT_TRUE(testDevice->isDeviceStatusHealthy());
        testDevice->setConstant(DataFieldHandle(testEffect.getUniformDataFieldHandleByName("u_float")), 1, &floatValue);
        EXPECT_TRUE(testDevice->isDeviceStatusHealthy());

        testDevice->setConstant(DataFieldHandle(testEffect.getUniformDataFieldHandleByName("u_vec2")), 1, &vec2Value);
        EXPECT_TRUE(testDevice->isDeviceStatusHealthy());
        testDevice->setConstant(DataFieldHandle(testEffect.getUniformDataFieldHandleByName("u_vec3")), 1, &vec3Value);
        EXPECT_TRUE(testDevice->isDeviceStatusHealthy());
        testDevice->setConstant(DataFieldHandle(testEffect.getUniformDataFieldHandleByName("u_vec4")), 1, &vec4Value);
        EXPECT_TRUE(testDevice->isDeviceStatusHealthy());

        testDevice->setConstant(DataFieldHandle(testEffect.getUniformDataFieldHandleByName("u_ivec2")), 1, &vec2iValue);
        EXPECT_TRUE(testDevice->isDeviceStatusHealthy());
        testDevice->setConstant(DataFieldHandle(testEffect.getUniformDataFieldHandleByName("u_ivec3")), 1, &vec3iValue);
        EXPECT_TRUE(testDevice->isDeviceStatusHealthy());
        testDevice->setConstant(DataFieldHandle(testEffect.getUniformDataFieldHandleByName("u_ivec4")), 1, &vec4iValue);
        EXPECT_TRUE(testDevice->isDeviceStatusHealthy());

        testDevice->setConstant(DataFieldHandle(testEffect.getUniformDataFieldHandleByName("u_mat2")), 1, &mat22Value);
        EXPECT_TRUE(testDevice->isDeviceStatusHealthy());
        testDevice->setConstant(DataFieldHandle(testEffect.getUniformDataFieldHandleByName("u_mat3")), 1, &mat33Value);
        EXPECT_TRUE(testDevice->isDeviceStatusHealthy());
        testDevice->setConstant(DataFieldHandle(testEffect.getUniformDataFieldHandleByName("u_mat4")), 1, &mat44Value);
        EXPECT_TRUE(testDevice->isDeviceStatusHealthy());

        testDevice->deleteShader(handle);
    }

    TEST_F(ADevice, IsHealthyAfterSettingNonExistingUniformOnShader)
    {
        const std::unique_ptr<EffectResource> templateEffect(CreateTestEffectResource());

        EffectInputInformationVector uniformInputs = templateEffect->getUniformInputs();
        uniformInputs.push_back(EffectInputInformation("u_NonExistingFloat", 1, EDataType::Float, EFixedSemantics::Invalid));

        const EffectResource testEffect(
            templateEffect->getVertexShader(),
            templateEffect->getFragmentShader(),
            "", {},
            uniformInputs,
            templateEffect->getAttributeInputs(),
            "test effect", ResourceCacheFlag_DoNotCache);

        auto shaderGpuResource = testDevice->uploadShader(testEffect);
        ASSERT_NE(nullptr, shaderGpuResource);
        const DeviceResourceHandle handle = testDevice->registerShader(std::move(shaderGpuResource));
        EXPECT_TRUE(handle.isValid());

        testDevice->activateShader(handle);

        const float floatValue = 5.0f;

        EXPECT_TRUE(testDevice->isDeviceStatusHealthy());
        testDevice->setConstant(DataFieldHandle(testEffect.getUniformDataFieldHandleByName("u_NonExistingFloat")), 1, &floatValue);

        // Currently, RAMSES silently ignores (== does not try to set) uniforms which are not declared in the shader
        // This test verifies that behavior (i.e. device does not crash or report error)
        // Should RAMSES become more strict and don't check uniforms existence, this EXPECTation should be false
        EXPECT_TRUE(testDevice->isDeviceStatusHealthy());

        testDevice->deleteShader(handle);
    }

    TEST_F(ADeviceSupportingBinaryShaders, DownloadsBinaryDataForUploadedShader)
    {
        const std::unique_ptr<EffectResource> testEffect(CreateTestEffectResource());
        auto shaderGpuResource = testDevice->uploadShader(*testEffect);
        ASSERT_NE(nullptr, shaderGpuResource);
        const DeviceResourceHandle handle = testDevice->registerShader(std::move(shaderGpuResource));
        EXPECT_TRUE(handle.isValid());

        UInt8Vector binaryShader;
        BinaryShaderFormatID binaryShaderFormat;
        EXPECT_TRUE(testDevice->getBinaryShader(handle, binaryShader, binaryShaderFormat));
        EXPECT_TRUE(binaryShader.size() != 0);
        EXPECT_TRUE(binaryShaderFormat != BinaryShaderFormatID::Invalid());

        testDevice->deleteShader(handle);
    }

    TEST_F(ADeviceSupportingBinaryShaders, UploadsShaderFromBinaryData)
    {
        const std::unique_ptr<EffectResource> testEffect(CreateTestEffectResource());
        auto shaderGpuResource = testDevice->uploadShader(*testEffect);
        ASSERT_NE(nullptr, shaderGpuResource);
        const DeviceResourceHandle handle = testDevice->registerShader(std::move(shaderGpuResource));
        EXPECT_TRUE(handle.isValid());

        UInt8Vector binaryShader;
        BinaryShaderFormatID binaryShaderFormat;
        EXPECT_TRUE(testDevice->getBinaryShader(handle, binaryShader, binaryShaderFormat));
        EXPECT_TRUE(binaryShader.size() != 0);
        EXPECT_TRUE(binaryShaderFormat != BinaryShaderFormatID::Invalid());

        const DeviceResourceHandle handleBinary = testDevice->uploadBinaryShader(*testEffect, &binaryShader.front(), static_cast<uint32_t>(binaryShader.size()), binaryShaderFormat);
        EXPECT_TRUE(handle != handleBinary);

        testDevice->deleteShader(handle);
        testDevice->deleteShader(handleBinary);
    }

    TEST_F(ADeviceSupportingBinaryShaders, RefusesToCreateShaderFromEffectWhenPassingInvalidBinaryData)
    {
        ResourceBlob data(0);
        BinaryShaderFormatID binaryShaderFormat;

        const std::unique_ptr<EffectResource> testEffect(CreateTestEffectResource());
        const DeviceResourceHandle handle = testDevice->uploadBinaryShader(*testEffect, data.data(), static_cast<uint32_t>(data.size()), binaryShaderFormat);

        EXPECT_FALSE(handle.isValid());
    }

    TEST_F(ADeviceSupportingBinaryShaders, Confidence_SetsDataOnBinaryShader)
    {
        const std::unique_ptr<EffectResource> testEffect(CreateTestEffectResource());
        auto shaderGpuResource = testDevice->uploadShader(*testEffect);
        ASSERT_NE(nullptr, shaderGpuResource);
        const DeviceResourceHandle handle = testDevice->registerShader(std::move(shaderGpuResource));
        ASSERT_TRUE(handle.isValid());

        UInt8Vector binaryShader;
        BinaryShaderFormatID binaryShaderFormat;
        EXPECT_TRUE(testDevice->getBinaryShader(handle, binaryShader, binaryShaderFormat));
        EXPECT_TRUE(binaryShader.size() != 0);
        EXPECT_TRUE(binaryShaderFormat != BinaryShaderFormatID::Invalid());

        const DeviceResourceHandle handleBinary = testDevice->uploadBinaryShader(*testEffect, &binaryShader.front(), static_cast<uint32_t>(binaryShader.size()), binaryShaderFormat);
        EXPECT_TRUE(handle != handleBinary);

        testDevice->activateShader(handleBinary);

        const float floatValue = 5.0f;
        const glm::vec2 vec2Value(5.0f);

        // This will trigger internal asserts if the shader is not correct/loaded properly
        testDevice->setConstant(DataFieldHandle(testEffect->getUniformDataFieldHandleByName("u_float")), 1, &floatValue);
        testDevice->setConstant(DataFieldHandle(testEffect->getUniformDataFieldHandleByName("u_vec2")), 1, &vec2Value);

        testDevice->deleteShader(handle);
        testDevice->deleteShader(handleBinary);
    }

    TEST_F(ADeviceSupportingGeometryShaders, CreatesShaderFromEffect)
    {
        ASSERT_TRUE(testDevice != nullptr);

        const std::unique_ptr<EffectResource> testEffect(CreateTestEffectResourceWithGeometryShader());
        auto shaderGpuResource = testDevice->uploadShader(*testEffect);
        ASSERT_NE(nullptr, shaderGpuResource);
        const DeviceResourceHandle handle = testDevice->registerShader(std::move(shaderGpuResource));
        EXPECT_TRUE(handle.isValid());

        testDevice->deleteShader(handle);
    }

    TEST_F(ADeviceSupportingGeometryShaders, RefusesToUploadEffectWithInvalidGeometryShader)
    {
        const std::unique_ptr<EffectResource> templateEffect(CreateTestEffectResourceWithGeometryShader());
        const EffectResource invalidEffect(
            templateEffect->getVertexShader(),
            templateEffect->getFragmentShader(),
            "--this is some invalid shader source code--",
            EDrawMode::Lines,
            templateEffect->getUniformInputs(),
            templateEffect->getAttributeInputs(),
            "invalid effect", ResourceCacheFlag_DoNotCache);
        const auto shaderGpuResource = testDevice->uploadShader(invalidEffect);
        EXPECT_EQ(nullptr, shaderGpuResource);
    }

    TEST_F(ADeviceSupportingGeometryShaders, RefusesToUploadEffectWithUnlinkableShaders)
    {
        const std::unique_ptr<EffectResource> templateEffect(CreateTestEffectResourceWithGeometryShader());

        const std::string geometryShader(R"SHADER(
                #version 320 es

                layout(points) in;
                layout(points, max_vertices = 1) out;
                uniform highp float u_geomFloat;

                uniform highp vec4 u_float; //interface mismatch

                void main() {

                    gl_Position = vec4(u_geomFloat * u_float);
                    EmitVertex();
                    EndPrimitive();
                }
                )SHADER");
        const EffectResource invalidEffect(
            templateEffect->getVertexShader(),
            templateEffect->getFragmentShader(),
            geometryShader,
            EDrawMode::Points,
            templateEffect->getUniformInputs(),
            templateEffect->getAttributeInputs(),
            "invalid effect", ResourceCacheFlag_DoNotCache);
        const auto shaderGpuResource = testDevice->uploadShader(invalidEffect);
        EXPECT_EQ(nullptr, shaderGpuResource);
    }

    TEST_F(ADeviceSupportingGeometryShaders, SetsConstantsOnShader)
    {
        ASSERT_TRUE(testDevice != nullptr);

        const std::unique_ptr<EffectResource> testEffect(CreateTestEffectResourceWithGeometryShader());
        auto shaderGpuResource = testDevice->uploadShader(*testEffect);
        ASSERT_NE(nullptr, shaderGpuResource);
        const DeviceResourceHandle handle = testDevice->registerShader(std::move(shaderGpuResource));
        EXPECT_TRUE(handle.isValid());

        testDevice->activateShader(handle);
        EXPECT_TRUE(testDevice->isDeviceStatusHealthy()); // check that there were no errors (GL_NO_ERROR in case of OpenGL) - triggers an internal assert otherwise
        float floatValue = 10.0;
        testDevice->setConstant(DataFieldHandle(testEffect->getUniformDataFieldHandleByName("u_geomFloat")), 1, &floatValue);
        EXPECT_TRUE(testDevice->isDeviceStatusHealthy());

        testDevice->deleteShader(handle);
    }
}
