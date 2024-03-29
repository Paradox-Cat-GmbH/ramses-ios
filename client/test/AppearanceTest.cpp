//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "ramses-client-api/TextureSampler.h"
#include "ramses-client-api/TextureCube.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/DataObject.h"
#include "ramses-client-api/TextureSamplerMS.h"
#include "ramses-client-api/RenderBuffer.h"
#include "ramses-client-api/TextureSamplerExternal.h"
#include "TestEffectCreator.h"
#include "ClientTestUtils.h"
#include "EffectImpl.h"
#include "DataObjectImpl.h"
#include "TextureSamplerImpl.h"
#include "AppearanceImpl.h"
#include "AppearanceUtils.h"

namespace ramses
{
    class AAppearanceTest : public ::testing::Test
    {
    public:

        static void SetUpTestCase()
        {
            sharedTestState = std::make_unique<TestEffectCreator>(false);
        }

        static void TearDownTestCase()
        {
            sharedTestState = nullptr;
        }

        void SetUp() override
        {
            EXPECT_TRUE(sharedTestState != nullptr);
            sharedTestState->recreateAppearence();
            appearance = sharedTestState->appearance;
        }

    protected:
        struct TextureInputInfo
        {
            UniformInput input;
            TextureSampler* sampler = nullptr;
            TextureSamplerMS* samplerMS = nullptr;
            TextureSamplerExternal* samplerExternal = nullptr;
            RenderBuffer* renderBuffer = nullptr;
            Texture2D* texture2D = nullptr;
            Texture3D* texture3D = nullptr;
            TextureCube* textureCube = nullptr;
        };

        void getTexture2DInputInfo(TextureInputInfo& info)
        {
            EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("texture2dInput", info.input));

            const uint8_t data[] = { 1, 2, 3 };
            const MipLevelData mipData(3u, data);
            Texture2D* texture = sharedTestState->getScene().createTexture2D(ETextureFormat::RGB8, 1u, 1u, 1u, &mipData, false);
            EXPECT_TRUE(texture != nullptr);
            info.texture2D = texture;

            TextureSampler* sampler = sharedTestState->getScene().createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Linear, *texture);
            EXPECT_TRUE(sampler != nullptr);
            info.sampler = sampler;
        }

        void getTexture2DMSInputInfo(TextureInputInfo& info)
        {
            EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("texture2dMSInput", info.input));

            RenderBuffer* renderBuffer = sharedTestState->getScene().createRenderBuffer(2u, 2u, ERenderBufferType::Color, ERenderBufferFormat::RGB8, ERenderBufferAccessMode::ReadWrite, 4u);
            EXPECT_TRUE(renderBuffer != nullptr);
            info.renderBuffer = renderBuffer;

            TextureSamplerMS* samplerMS = sharedTestState->getScene().createTextureSamplerMS(*renderBuffer, "renderBuffer");
            EXPECT_TRUE(samplerMS != nullptr);
            info.samplerMS = samplerMS;
        }

        void getTexture3DInputInfo(TextureInputInfo& info)
        {
            EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("texture3dInput", info.input));

            const uint8_t data[] = { 1, 2, 3 };
            const MipLevelData mipData(3u, data);
            Texture3D* texture = sharedTestState->getScene().createTexture3D(ETextureFormat::RGB8, 1u, 1u, 1u, 1u, &mipData, false);
            EXPECT_TRUE(texture != nullptr);
            info.texture3D = texture;

            TextureSampler* sampler = sharedTestState->getScene().createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Linear, *texture);
            EXPECT_TRUE(sampler != nullptr);
            info.sampler = sampler;
        }

        void getTextureCubeInputInfo(TextureInputInfo& info)
        {
            EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("textureCubeInput", info.input));

            const uint8_t data[] = { 1, 2, 3 };
            const CubeMipLevelData mipData(3u, data, data, data, data, data, data);
            TextureCube* texture = sharedTestState->getScene().createTextureCube(ETextureFormat::RGB8, 1u, 1u, &mipData, false);
            EXPECT_TRUE(texture != nullptr);
            info.textureCube = texture;

            TextureSampler* sampler = sharedTestState->getScene().createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Linear, *texture);
            EXPECT_TRUE(sampler != nullptr);
            info.sampler = sampler;
        }

        void getTextureExternalInputInfo(TextureInputInfo& info)
        {
            EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("textureExternalInput", info.input));

            TextureSamplerExternal* sampler = sharedTestState->getScene().createTextureSamplerExternal(ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Linear);
            EXPECT_TRUE(sampler != nullptr);
            info.samplerExternal = sampler;
        }

        static std::unique_ptr<TestEffectCreator> sharedTestState;
        Appearance* appearance;
    };

    std::unique_ptr<TestEffectCreator> AAppearanceTest::sharedTestState;

    class AAppearanceTestWithSemanticUniforms : public AAppearanceTest
    {
    public:
        static void SetUpTestCase()
        {
            sharedTestState = std::make_unique<TestEffectCreator>(true);
        }
    };

    TEST_F(AAppearanceTest, getsTheSameEffectUsedToCreateIt)
    {
        const Effect& effect = appearance->getEffect();
        EXPECT_EQ(&effect, sharedTestState->effect);

        const ramses_internal::DataLayout uniformLayout = sharedTestState->getInternalScene().getDataLayout(appearance->m_impl.getUniformDataLayout());
        const ramses_internal::ResourceContentHash& effectHashFromUniformLayout = uniformLayout.getEffectHash();
        EXPECT_EQ(appearance->getEffect().m_impl.getLowlevelResourceHash(), effectHashFromUniformLayout);
    }

    TEST_F(AAppearanceTest, setGetBlendingFactors)
    {
        status_t stat = appearance->setBlendingFactors(EBlendFactor::One, EBlendFactor::SrcAlpha, EBlendFactor::OneMinusSrcAlpha, EBlendFactor::DstAlpha);
        EXPECT_EQ(StatusOK, stat);
        EBlendFactor srcColor = EBlendFactor::Zero;
        EBlendFactor destColor = EBlendFactor::Zero;
        EBlendFactor srcAlpha = EBlendFactor::Zero;
        EBlendFactor destAlpha = EBlendFactor::Zero;
        stat = appearance->getBlendingFactors(srcColor, destColor, srcAlpha, destAlpha);
        EXPECT_EQ(StatusOK, stat);
        EXPECT_EQ(EBlendFactor::One, srcColor);
        EXPECT_EQ(EBlendFactor::SrcAlpha, destColor);
        EXPECT_EQ(EBlendFactor::OneMinusSrcAlpha, srcAlpha);
        EXPECT_EQ(EBlendFactor::DstAlpha, destAlpha);
    }

    TEST_F(AAppearanceTest, setGetBlendingColor)
    {
        vec4f color{ std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
        //default values
        status_t stat = appearance->getBlendingColor(color);
        EXPECT_EQ(StatusOK, stat);
        EXPECT_EQ(color, vec4f(0.f, 0.f, 0.f, 0.f));

        const vec4f colorToSet{ 0.1f, 0.2f, 0.3f, 0.4f };
        stat = appearance->setBlendingColor(colorToSet);
        EXPECT_EQ(StatusOK, stat);

        stat = appearance->getBlendingColor(color);
        EXPECT_EQ(StatusOK, stat);
        EXPECT_EQ(colorToSet, color);
    }

    TEST_F(AAppearanceTest, setGetDepthWrite)
    {
        EDepthWrite depthWriteMode = EDepthWrite::Disabled;
        EXPECT_EQ(StatusOK, appearance->setDepthWrite(EDepthWrite::Enabled));
        EXPECT_EQ(StatusOK, appearance->getDepthWriteMode(depthWriteMode));
        EXPECT_EQ(EDepthWrite::Enabled, depthWriteMode);

        EXPECT_EQ(StatusOK, appearance->setDepthWrite(EDepthWrite::Disabled));
        EXPECT_EQ(StatusOK, appearance->getDepthWriteMode(depthWriteMode));
        EXPECT_EQ(EDepthWrite::Disabled, depthWriteMode);

        EXPECT_EQ(StatusOK, appearance->setDepthWrite(EDepthWrite::Enabled));
        EXPECT_EQ(StatusOK, appearance->getDepthWriteMode(depthWriteMode));
        EXPECT_EQ(EDepthWrite::Enabled, depthWriteMode);
    }

    TEST_F(AAppearanceTest, setGetDepthFunction)
    {
        EDepthFunc depthFunc = EDepthFunc::Disabled;
        EXPECT_EQ(StatusOK, appearance->getDepthFunction(depthFunc));
        EXPECT_EQ(EDepthFunc::LessEqual, depthFunc);

        EXPECT_EQ(StatusOK, appearance->setDepthFunction(EDepthFunc::GreaterEqual));
        EXPECT_EQ(StatusOK, appearance->getDepthFunction(depthFunc));
        EXPECT_EQ(EDepthFunc::GreaterEqual, depthFunc);
    }

    TEST_F(AAppearanceTest, setGetScissorTest)
    {
        EScissorTest mode = EScissorTest::Disabled;
        EXPECT_EQ(StatusOK, appearance->setScissorTest(EScissorTest::Enabled, 1, 2, 3u, 4u));
        EXPECT_EQ(StatusOK, appearance->getScissorTestState(mode));
        EXPECT_EQ(EScissorTest::Enabled, mode);

        int16_t x = 0;
        int16_t y = 0;
        uint16_t width = 0u;
        uint16_t height = 0;
        EXPECT_EQ(StatusOK, appearance->getScissorRegion(x, y, width, height));
        EXPECT_EQ(1, x);
        EXPECT_EQ(2, y);
        EXPECT_EQ(3u, width);
        EXPECT_EQ(4u, height);
    }

    TEST_F(AAppearanceTest, setGetStencilFunc)
    {
        status_t stat = appearance->setStencilFunction(EStencilFunc::Equal, 2u, 0xef);
        EXPECT_EQ(StatusOK, stat);
        EStencilFunc func = EStencilFunc::Disabled;
        uint8_t ref = 0;
        uint8_t mask = 0;
        stat = appearance->getStencilFunction(func, ref, mask);
        EXPECT_EQ(StatusOK, stat);
        EXPECT_EQ(EStencilFunc::Equal, func);
        EXPECT_EQ(2u, ref);
        EXPECT_EQ(0xef, mask);
    }

    TEST_F(AAppearanceTest, setGetStencilOperation)
    {
        status_t stat = appearance->setStencilOperation(EStencilOperation::Decrement, EStencilOperation::Increment, EStencilOperation::DecrementWrap);
        EXPECT_EQ(StatusOK, stat);
        EStencilOperation sfail = EStencilOperation::Zero;
        EStencilOperation dpfail = EStencilOperation::Zero;
        EStencilOperation dppass = EStencilOperation::Zero;
        stat = appearance->getStencilOperation(sfail, dpfail, dppass);
        EXPECT_EQ(StatusOK, stat);
        EXPECT_EQ(EStencilOperation::Decrement, sfail);
        EXPECT_EQ(EStencilOperation::Increment, dpfail);
        EXPECT_EQ(EStencilOperation::DecrementWrap, dppass);
    }

    TEST_F(AAppearanceTest, setGetBlendOperations)
    {
        EXPECT_EQ(StatusOK, appearance->setBlendingOperations(EBlendOperation::Subtract, EBlendOperation::Max));
        EBlendOperation opColor = EBlendOperation::Disabled;
        EBlendOperation opAlpha = EBlendOperation::Disabled;
        EXPECT_EQ(StatusOK, appearance->getBlendingOperations(opColor, opAlpha));
        EXPECT_EQ(EBlendOperation::Subtract, opColor);
        EXPECT_EQ(EBlendOperation::Max, opAlpha);
    }

    TEST_F(AAppearanceTest, setGetCullMode)
    {
        ECullMode mode = ECullMode::Disabled;
        EXPECT_EQ(StatusOK, appearance->setCullingMode(ECullMode::FrontFacing));
        EXPECT_EQ(StatusOK, appearance->getCullingMode(mode));
        EXPECT_EQ(ECullMode::FrontFacing, mode);
        EXPECT_EQ(StatusOK, appearance->setCullingMode(ECullMode::Disabled));
        EXPECT_EQ(StatusOK, appearance->getCullingMode(mode));
        EXPECT_EQ(ECullMode::Disabled, mode);
    }

    TEST_F(AAppearanceTest, hasDrawModeTrianglesByDefault)
    {
        EDrawMode mode;
        EXPECT_EQ(StatusOK, appearance->getDrawMode(mode));
        EXPECT_EQ(EDrawMode::Triangles, mode);
    }

    TEST_F(AAppearanceTest, setGetDrawMode)
    {
        EDrawMode mode = EDrawMode::Lines;
        EXPECT_EQ(StatusOK, appearance->setDrawMode(EDrawMode::Points));
        EXPECT_EQ(StatusOK, appearance->getDrawMode(mode));
        EXPECT_EQ(EDrawMode::Points, mode);
    }

    TEST_F(AAppearanceTest, setGetColorWriteMask)
    {
        bool writeR = false;
        bool writeG = false;
        bool writeB = false;
        bool writeA = false;
        EXPECT_EQ(StatusOK, appearance->setColorWriteMask(true, false, true, false));
        EXPECT_EQ(StatusOK, appearance->getColorWriteMask(writeR, writeG, writeB, writeA));
        EXPECT_TRUE(writeR);
        EXPECT_FALSE(writeG);
        EXPECT_TRUE(writeB);
        EXPECT_FALSE(writeA);
        EXPECT_EQ(StatusOK, appearance->setColorWriteMask(false, true, false, true));
        EXPECT_EQ(StatusOK, appearance->getColorWriteMask(writeR, writeG, writeB, writeA));
        EXPECT_FALSE(writeR);
        EXPECT_TRUE(writeG);
        EXPECT_FALSE(writeB);
        EXPECT_TRUE(writeA);
    }

    TEST_F(AAppearanceTest, reportsErrorWhenGetSetMismatchingInputTypeScalar)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("integerInput", inputObject));

        const float value = 42;
        float getValue = 0;

        EXPECT_NE(StatusOK, appearance->setInputValue(inputObject, value));
        EXPECT_NE(StatusOK, appearance->getInputValue(inputObject, getValue));
    }

    TEST_F(AAppearanceTest, reportsErrorWhenGetSetMismatchingInputTypeArray)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("integerInput", inputObject));

        const float values[] = { 42, 43, 44, 45, 46, 47 };
        float getValues[6];

        EXPECT_NE(StatusOK, appearance->setInputValue(inputObject, 6u, values));
        EXPECT_NE(StatusOK, appearance->getInputValue(inputObject, 6u, getValues));
    }

    TEST_F(AAppearanceTest, reportsErrorWhenGetSetMismatchingInputTypeTexture)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("integerInput", inputObject));

        const uint8_t texData[] = { 1, 2, 3 };
        const MipLevelData mipData(3u, texData);

        Texture2D* texture = sharedTestState->getScene().createTexture2D(ETextureFormat::RGB8, 1u, 1u, 1u, &mipData, false);
        ASSERT_TRUE(texture != nullptr);
        TextureSampler* textureSampler = sharedTestState->getScene().createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Linear, *texture);
        ASSERT_TRUE(textureSampler != nullptr);

        EXPECT_NE(StatusOK, appearance->setInputTexture(inputObject, *textureSampler));

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*textureSampler));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*texture));
    }

    TEST_F(AAppearanceTest, reportsErrorWhenSetInputTextureFromADifferentScene)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("texture2dInput", inputObject));

        const uint8_t texData[] = { 1, 2, 3 };
        const MipLevelData mipData(3u, texData);

        ramses::Scene& anotherScene = *sharedTestState->getClient().createScene(sceneId_t(1u));
        Texture2D* texture = anotherScene.createTexture2D(ETextureFormat::RGB8, 1u, 1u, 1u, &mipData, false);
        ASSERT_TRUE(texture != nullptr);
        TextureSampler* textureSampler = anotherScene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Linear, *texture);
        ASSERT_TRUE(textureSampler != nullptr);

        EXPECT_NE(StatusOK, appearance->setInputTexture(inputObject, *textureSampler));

        EXPECT_EQ(StatusOK, anotherScene.destroy(*texture));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(anotherScene));
    }

    TEST_F(AAppearanceTest, getsSamplerSetToUniformInput)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("texture2dInput", inputObject));

        const uint8_t texData[] = { 1, 2, 3 };
        const MipLevelData mipData(3u, texData);
        Texture2D* texture = sharedTestState->getScene().createTexture2D(ETextureFormat::RGB8, 1u, 1u, 1u, &mipData, false);
        ASSERT_TRUE(texture != nullptr);
        TextureSampler* textureSampler = sharedTestState->getScene().createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Linear, *texture);
        ASSERT_TRUE(textureSampler != nullptr);

        EXPECT_EQ(StatusOK, appearance->setInputTexture(inputObject, *textureSampler));

        const TextureSampler* actualSampler = nullptr;
        EXPECT_EQ(StatusOK, appearance->getInputTexture(inputObject, actualSampler));
        EXPECT_EQ(textureSampler, actualSampler);

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*textureSampler));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*texture));
    }

    TEST_F(AAppearanceTest, getsNullSamplerIfNoneSetToUniformInput)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("texture2dInput", inputObject));

        const TextureSampler* actualSampler = nullptr;
        EXPECT_EQ(StatusOK, appearance->getInputTexture(inputObject, actualSampler));
        EXPECT_EQ(nullptr, actualSampler);
    }

    TEST_F(AAppearanceTest, getsNullSamplerMSIfNoneSetToUniformInput)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("texture2dMSInput", inputObject));

        const TextureSamplerMS* actualSampler = nullptr;
        EXPECT_EQ(StatusOK, appearance->getInputTextureMS(inputObject, actualSampler));
        EXPECT_EQ(nullptr, actualSampler);
    }

    TEST_F(AAppearanceTest, getsNullSamplerExternalIfNoneSetToUniformInput)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("textureExternalInput", inputObject));

        const TextureSamplerExternal* actualSampler = nullptr;
        EXPECT_EQ(StatusOK, appearance->getInputTextureExternal(inputObject, actualSampler));
        EXPECT_EQ(nullptr, actualSampler);
    }

    TEST_F(AAppearanceTest, failsToGetSamplerSetToUniformIfInputHasWrongType)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("integerInput", inputObject));

        const TextureSampler* actualSampler = nullptr;
        EXPECT_NE(StatusOK, appearance->getInputTexture(inputObject, actualSampler));
        EXPECT_EQ(nullptr, actualSampler);
    }

    TEST_F(AAppearanceTest, failsToGetSamplerMSIfInputHasWrongType)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("texture2dInput", inputObject));

        const TextureSamplerMS* actualSampler = nullptr;
        EXPECT_NE(StatusOK, appearance->getInputTextureMS(inputObject, actualSampler));
        EXPECT_EQ(nullptr, actualSampler);
    }

    TEST_F(AAppearanceTest, failsToGetSamplerExternalIfInputHasWrongType)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("texture2dInput", inputObject));

        const TextureSamplerExternal* actualSampler = nullptr;
        EXPECT_NE(StatusOK, appearance->getInputTextureExternal(inputObject, actualSampler));
        EXPECT_EQ(nullptr, actualSampler);
    }

    /// Int32
    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeInt32)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("integerInput", inputObject));

        int32_t value = 42;
        int32_t& valueR = value;
        const int32_t& valueCR = value;
        auto valueM = value;
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, value));
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, valueR));
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, valueCR));
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, std::move(valueM)));

        int32_t getValue = 0;
        EXPECT_EQ(StatusOK, appearance->getInputValue(inputObject, getValue));
        EXPECT_EQ(value, getValue);
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeInt32Array)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("integerInputArray", inputObject));

        const int32_t value = 42;
        int32_t values[] = { value, value * 2, value * 3 };
        int32_t getValues[3] = { 0 };

        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, 3u, values));
        EXPECT_EQ(StatusOK, appearance->getInputValue(inputObject, 3u, getValues));
        EXPECT_EQ(values, absl::MakeSpan(getValues));

        EXPECT_NE(StatusOK, appearance->setInputValue(inputObject, 0u, values));
        EXPECT_NE(StatusOK, appearance->setInputValue(inputObject, 11u, values));
        EXPECT_NE(StatusOK, appearance->getInputValue(inputObject, 0u, values));
        EXPECT_NE(StatusOK, appearance->getInputValue(inputObject, 11u, values));
    }

    /// Float
    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeFloat)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("floatInput", inputObject));

        float value = 42;
        float& valueR = value;
        const float& valueCR = value;
        auto valueM = value;
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, value));
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, valueR));
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, valueCR));
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, std::move(valueM)));

        float getValue = 0;
        EXPECT_EQ(StatusOK, appearance->getInputValue(inputObject, getValue));
        EXPECT_EQ(value, getValue);
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeFloatArray)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("floatInputArray", inputObject));

        const float value = 42;
        const float values[] = { value, value * 2, value * 3 };
        float getValues[3] = { 0 };

        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, 3u, values));
        EXPECT_EQ(StatusOK, appearance->getInputValue(inputObject, 3u, getValues));
        EXPECT_EQ(values, absl::MakeSpan(getValues));

        EXPECT_NE(StatusOK, appearance->setInputValue(inputObject, 0u, values));
        EXPECT_NE(StatusOK, appearance->setInputValue(inputObject, 11u, values));
        EXPECT_NE(StatusOK, appearance->getInputValue(inputObject, 0u, getValues));
        EXPECT_NE(StatusOK, appearance->getInputValue(inputObject, 11u, getValues));
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeVector2i)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("vec2iInput", inputObject));

        vec2i value{ 42, 24 };
        vec2i& valueR = value;
        const vec2i& valueCR = value;
        auto valueM = value;
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, value));
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, valueR));
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, valueCR));
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, std::move(valueM)));

        vec2i getValue;
        EXPECT_EQ(StatusOK, appearance->getInputValue(inputObject, getValue));
        EXPECT_EQ(value, getValue);
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeVector2iArray)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("vec2iInputArray", inputObject));

        const std::vector<vec2i> values = { vec2i{42, 43}, vec2i{44, 45}, vec2i{46, 47} };
        std::vector<vec2i> getValues(values.size());

        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, 3u, values.data()));
        EXPECT_EQ(StatusOK, appearance->getInputValue(inputObject, 3u, getValues.data()));
        EXPECT_EQ(values, getValues);

        EXPECT_NE(StatusOK, appearance->setInputValue(inputObject, 0u, values.data()));
        EXPECT_NE(StatusOK, appearance->setInputValue(inputObject, 11u, values.data()));
        EXPECT_NE(StatusOK, appearance->getInputValue(inputObject, 0u, getValues.data()));
        EXPECT_NE(StatusOK, appearance->getInputValue(inputObject, 11u, getValues.data()));
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeVector3i)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("vec3iInput", inputObject));

        vec3i value{ 42, 24, 4422 };
        vec3i& valueR = value;
        const vec3i& valueCR = value;
        auto valueM = value;
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, value));
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, valueR));
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, valueCR));
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, std::move(valueM)));

        vec3i getValue;
        EXPECT_EQ(StatusOK, appearance->getInputValue(inputObject, getValue));
        EXPECT_EQ(value, getValue);
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeVector3iArray)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("vec3iInputArray", inputObject));

        const std::vector<vec3i> values = { vec3i{42, 43, 444}, vec3i{44, 45, 555}, vec3i{46, 47, 666} };
        std::vector<vec3i> getValues(values.size());

        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, 3u, values.data()));
        EXPECT_EQ(StatusOK, appearance->getInputValue(inputObject, 3u, getValues.data()));
        EXPECT_EQ(values, getValues);

        EXPECT_NE(StatusOK, appearance->setInputValue(inputObject, 0u, values.data()));
        EXPECT_NE(StatusOK, appearance->setInputValue(inputObject, 11u, values.data()));
        EXPECT_NE(StatusOK, appearance->getInputValue(inputObject, 0u, getValues.data()));
        EXPECT_NE(StatusOK, appearance->getInputValue(inputObject, 11u, getValues.data()));
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeVector4i)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("vec4iInput", inputObject));

        vec4i value{ 42, 24, 44, 22 };
        vec4i& valueR = value;
        const vec4i& valueCR = value;
        auto valueM = value;
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, value));
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, valueR));
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, valueCR));
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, std::move(valueM)));

        vec4i getValue;
        EXPECT_EQ(StatusOK, appearance->getInputValue(inputObject, getValue));
        EXPECT_EQ(value, getValue);
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeVector4iArray)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("vec4iInputArray", inputObject));

        const std::vector<vec4i> values = { vec4i{42, 43, 444, 555}, vec4i{44, 45, 666, 777}, vec4i{46, 47, 888, 999} };
        std::vector<vec4i> getValues(values.size());

        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, 3u, values.data()));
        EXPECT_EQ(StatusOK, appearance->getInputValue(inputObject, 3u, getValues.data()));
        EXPECT_EQ(values, getValues);

        EXPECT_NE(StatusOK, appearance->setInputValue(inputObject, 0u, values.data()));
        EXPECT_NE(StatusOK, appearance->setInputValue(inputObject, 11u, values.data()));
        EXPECT_NE(StatusOK, appearance->getInputValue(inputObject, 0u, getValues.data()));
        EXPECT_NE(StatusOK, appearance->getInputValue(inputObject, 11u, getValues.data()));
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeVector2f)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("vec2fInput", inputObject));

        vec2f value{ 42.f, 24.f };
        vec2f& valueR = value;
        const vec2f& valueCR = value;
        auto valueM = value;
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, value));
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, valueR));
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, valueCR));
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, std::move(valueM)));

        vec2f getValue;
        EXPECT_EQ(StatusOK, appearance->getInputValue(inputObject, getValue));
        EXPECT_EQ(value, getValue);
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeVector2fArray)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("vec2fInputArray", inputObject));

        const std::vector<vec2f> values = { vec2f{42.f, 43.f}, vec2f{44.f, 45.f}, vec2f{46.f, 47.f} };
        std::vector<vec2f> getValues(values.size());

        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, 3u, values.data()));
        EXPECT_EQ(StatusOK, appearance->getInputValue(inputObject, 3u, getValues.data()));
        EXPECT_EQ(values, getValues);

        EXPECT_NE(StatusOK, appearance->setInputValue(inputObject, 0u, values.data()));
        EXPECT_NE(StatusOK, appearance->setInputValue(inputObject, 11u, values.data()));
        EXPECT_NE(StatusOK, appearance->getInputValue(inputObject, 0u, getValues.data()));
        EXPECT_NE(StatusOK, appearance->getInputValue(inputObject, 11u, getValues.data()));
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeVector3f)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("vec3fInput", inputObject));

        vec3f value{ 42.f, 24.f, 44.f };
        vec3f& valueR = value;
        const vec3f& valueCR = value;
        auto valueM = value;
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, value));
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, valueR));
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, valueCR));
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, std::move(valueM)));

        vec3f getValue;
        EXPECT_EQ(StatusOK, appearance->getInputValue(inputObject, getValue));
        EXPECT_EQ(value, getValue);
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeVector3fArray)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("vec3fInputArray", inputObject));

        const std::vector<vec3f> values = { vec3f{42.f, 43.f, 444.f}, vec3f{44.f, 45.f, 666.f}, vec3f{46.f, 47.f, 888.f} };
        std::vector<vec3f> getValues(values.size());

        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, 3u, values.data()));
        EXPECT_EQ(StatusOK, appearance->getInputValue(inputObject, 3u, getValues.data()));
        EXPECT_EQ(values, getValues);

        EXPECT_NE(StatusOK, appearance->setInputValue(inputObject, 0u, values.data()));
        EXPECT_NE(StatusOK, appearance->setInputValue(inputObject, 11u, values.data()));
        EXPECT_NE(StatusOK, appearance->getInputValue(inputObject, 0u, getValues.data()));
        EXPECT_NE(StatusOK, appearance->getInputValue(inputObject, 11u, getValues.data()));
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeVector4f)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("vec4fInput", inputObject));

        vec4f value{ 42.f, 24.f, 44.f, 55.f };
        vec4f& valueR = value;
        const vec4f& valueCR = value;
        auto valueM = value;
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, value));
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, valueR));
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, valueCR));
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, std::move(valueM)));

        vec4f getValue;
        EXPECT_EQ(StatusOK, appearance->getInputValue(inputObject, getValue));
        EXPECT_EQ(value, getValue);
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeVector4fArray)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("vec4fInputArray", inputObject));

        const std::vector<vec4f> values = { vec4f{42.f, 43.f, 444.f, 555.f}, vec4f{44.f, 45.f, 666.f, 777.f}, vec4f{46.f, 47.f, 888.f, 999.f} };
        std::vector<vec4f> getValues(values.size());

        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, 3u, values.data()));
        EXPECT_EQ(StatusOK, appearance->getInputValue(inputObject, 3u, getValues.data()));
        EXPECT_EQ(values, getValues);

        EXPECT_NE(StatusOK, appearance->setInputValue(inputObject, 0u, values.data()));
        EXPECT_NE(StatusOK, appearance->setInputValue(inputObject, 11u, values.data()));
        EXPECT_NE(StatusOK, appearance->getInputValue(inputObject, 0u, getValues.data()));
        EXPECT_NE(StatusOK, appearance->getInputValue(inputObject, 11u, getValues.data()));
    }

    /// matrix22f
    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeMatrix22f)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("matrix22fInput", inputObject));

        matrix22f value{ 42.f, 43.f, 44.f, 45.f };
        matrix22f& valueR = value;
        const matrix22f& valueCR = value;
        auto valueM = value;
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, value));
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, valueR));
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, valueCR));
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, std::move(valueM)));

        matrix22f getValue;
        EXPECT_EQ(StatusOK, appearance->getInputValue(inputObject, getValue));
        EXPECT_EQ(value, getValue);
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeMatrix22fArray)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("matrix22fInputArray", inputObject));

        const std::vector<matrix22f> values =
        {
            matrix22f{42.f, 43.f, 44.f, 45.f},
            matrix22f{46.f, 47.f, 48.f, 49.f},
            matrix22f{50.f, 51.f, 52.f, 53.f}
        };
        std::vector<matrix22f> getValues(values.size());

        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, 3u, values.data()));
        EXPECT_EQ(StatusOK, appearance->getInputValue(inputObject, 3u, getValues.data()));
        EXPECT_EQ(values, getValues);

        EXPECT_NE(StatusOK, appearance->setInputValue(inputObject, 0u, values.data()));
        EXPECT_NE(StatusOK, appearance->setInputValue(inputObject, 11u, values.data()));
        EXPECT_NE(StatusOK, appearance->getInputValue(inputObject, 0u, getValues.data()));
        EXPECT_NE(StatusOK, appearance->getInputValue(inputObject, 11u, getValues.data()));
    }

    /// Matrix33f
    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeMatrix33f)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("matrix33fInput", inputObject));

        matrix33f value{ 42.f, 43.f, 44.f, 45.f, 46.f, 47.f, 48.f, 49.f, 50.f };
        matrix33f& valueR = value;
        const matrix33f& valueCR = value;
        auto valueM = value;
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, value));
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, valueR));
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, valueCR));
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, std::move(valueM)));

        matrix33f getValue;
        EXPECT_EQ(StatusOK, appearance->getInputValue(inputObject, getValue));
        EXPECT_EQ(value, getValue);
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeMatrix33fArray)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("matrix33fInputArray", inputObject));

        const std::vector<matrix33f> values =
        {
            matrix33f{42.f, 43.f, 44.f, 45.f, 11.f, 22.f, 33.f, 44.f, 55.f},
            matrix33f{46.f, 47.f, 48.f, 49.f, 66.f, 77.f, 88.f, 99.f, 10.f},
            matrix33f{50.f, 51.f, 52.f, 53.f, 20.f, 30.f, 40.f, 50.f, 60.f}
        };
        std::vector<matrix33f> getValues(values.size());

        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, 3u, values.data()));
        EXPECT_EQ(StatusOK, appearance->getInputValue(inputObject, 3u, getValues.data()));
        EXPECT_EQ(values, getValues);

        EXPECT_NE(StatusOK, appearance->setInputValue(inputObject, 0u, values.data()));
        EXPECT_NE(StatusOK, appearance->setInputValue(inputObject, 11u, values.data()));
        EXPECT_NE(StatusOK, appearance->getInputValue(inputObject, 0u, getValues.data()));
        EXPECT_NE(StatusOK, appearance->getInputValue(inputObject, 11u, getValues.data()));
    }

    /// Matrix44f
    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeMatrix44f)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("matrix44fInput", inputObject));

        matrix44f value{ 42.f, 43.f, 44.f, 45.f, 46.f, 47.f, 48.f, 49.f, 50.f, 51.f, 52.f, 53.f, 54.f, 55.f, 56.f, 57.f };
        matrix44f& valueR = value;
        const matrix44f& valueCR = value;
        auto valueM = value;
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, value));
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, valueR));
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, valueCR));
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, std::move(valueM)));

        matrix44f getValue;
        EXPECT_EQ(StatusOK, appearance->getInputValue(inputObject, getValue));
        EXPECT_EQ(value, getValue);
    }

    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeMatrix44fArray)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("matrix44fInputArray", inputObject));

        const std::vector<matrix44f> values =
        {
            matrix44f{42.f, 43.f, 44.f, 45.f, 11.f, 22.f, 33.f, 44.f, 55.f, 1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f},
            matrix44f{46.f, 47.f, 48.f, 49.f, 66.f, 77.f, 88.f, 99.f, 10.f, 8.f, 9.f, 10.f, 11.f, 12.f, 13.f, 14.f},
            matrix44f{50.f, 51.f, 52.f, 53.f, 20.f, 30.f, 40.f, 50.f, 60.f, 15.f, 16.f, 17.f, 18.f, 19.f, 20.f, 21.f}
        };
        std::vector<matrix44f> getValues(values.size());

        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, 3u, values.data()));
        EXPECT_EQ(StatusOK, appearance->getInputValue(inputObject, 3u, getValues.data()));
        EXPECT_EQ(values, getValues);

        EXPECT_NE(StatusOK, appearance->setInputValue(inputObject, 0u, values.data()));
        EXPECT_NE(StatusOK, appearance->setInputValue(inputObject, 11u, values.data()));
        EXPECT_NE(StatusOK, appearance->getInputValue(inputObject, 0u, getValues.data()));
        EXPECT_NE(StatusOK, appearance->getInputValue(inputObject, 11u, getValues.data()));
    }

    /// Texture2D
    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeTexture2D)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("texture2dInput", inputObject));

        const uint8_t texData[] = { 1, 2, 3 };
        const MipLevelData mipData(3u, texData);

        Texture2D* texture = sharedTestState->getScene().createTexture2D(ETextureFormat::RGB8, 1u, 1u, 1u, &mipData, false);
        ASSERT_TRUE(texture != nullptr);
        TextureSampler* textureSampler = sharedTestState->getScene().createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Linear, *texture);
        ASSERT_TRUE(textureSampler != nullptr);

        EXPECT_EQ(StatusOK, appearance->setInputTexture(inputObject, *textureSampler));

        const TextureSampler* actualSampler = nullptr;
        EXPECT_EQ(StatusOK, appearance->getInputTexture(inputObject, actualSampler));
        EXPECT_EQ(textureSampler, actualSampler);

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*textureSampler));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*texture));
    }

    /// Texture2DMS
    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeTexture2DMS)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("texture2dMSInput", inputObject));

        RenderBuffer* renderBuffer = sharedTestState->getScene().createRenderBuffer(2u, 2u, ERenderBufferType::Color, ERenderBufferFormat::RGB8, ERenderBufferAccessMode::ReadWrite, 4u);
        TextureSamplerMS* textureSampler = sharedTestState->getScene().createTextureSamplerMS(*renderBuffer, "renderBuffer");
        ASSERT_TRUE(textureSampler != nullptr);

        EXPECT_EQ(textureSampler->m_impl.getTextureDataType(), ramses_internal::EDataType::TextureSampler2DMS);
        EXPECT_EQ(StatusOK, appearance->setInputTexture(inputObject, *textureSampler));

        const TextureSamplerMS* actualSampler = nullptr;
        EXPECT_EQ(StatusOK, appearance->getInputTextureMS(inputObject, actualSampler));
        EXPECT_EQ(textureSampler, actualSampler);

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*textureSampler));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*renderBuffer));
    }

    /// TextureCube
    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeTextureCube)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("textureCubeInput", inputObject));

        const uint8_t texData[] = { 1, 2, 3 };
        const CubeMipLevelData mipData(3u, texData, texData, texData, texData, texData, texData);

        TextureCube* texture = sharedTestState->getScene().createTextureCube(ETextureFormat::RGB8, 1u, 1u, &mipData, false);
        ASSERT_TRUE(texture != nullptr);
        TextureSampler* textureSampler = sharedTestState->getScene().createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, *texture);
        ASSERT_TRUE(textureSampler != nullptr);

        EXPECT_EQ(StatusOK, appearance->setInputTexture(inputObject, *textureSampler));

        const TextureSampler* actualSampler = nullptr;
        EXPECT_EQ(StatusOK, appearance->getInputTexture(inputObject, actualSampler));
        EXPECT_EQ(textureSampler, actualSampler);

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*textureSampler));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*texture));
    }

    /// TextureExternal
    TEST_F(AAppearanceTest, canHandleUniformInputsOfTypeTextureExternal)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("textureExternalInput", inputObject));

        TextureSamplerExternal* textureSampler = sharedTestState->getScene().createTextureSamplerExternal(ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear);
        ASSERT_TRUE(textureSampler != nullptr);

        EXPECT_EQ(textureSampler->m_impl.getTextureDataType(), ramses_internal::EDataType::TextureSamplerExternal);
        EXPECT_EQ(StatusOK, appearance->setInputTexture(inputObject, *textureSampler));

        const TextureSamplerExternal* actualSampler = nullptr;
        EXPECT_EQ(StatusOK, appearance->getInputTextureExternal(inputObject, actualSampler));
        EXPECT_EQ(textureSampler, actualSampler);

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*textureSampler));
    }

    /// Binding data objects
    TEST_F(AAppearanceTest, uniformInputIsNotBoundInitially)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("floatInput", inputObject));
        EXPECT_FALSE(appearance->isInputBound(inputObject));
    }

    TEST_F(AAppearanceTest, canBindDataObjectToUniformInput)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("floatInput", inputObject));

        auto dataObject = sharedTestState->getScene().createDataObject(EDataType::Float);
        ASSERT_TRUE(dataObject != nullptr);

        EXPECT_EQ(StatusOK, appearance->bindInput(inputObject, *dataObject));
        EXPECT_TRUE(appearance->isInputBound(inputObject));
        EXPECT_EQ(dataObject->m_impl.getDataReference(), appearance->getDataObjectBoundToInput(inputObject)->m_impl.getDataReference());

        EXPECT_EQ(StatusOK, appearance->unbindInput(inputObject));
        EXPECT_FALSE(appearance->isInputBound(inputObject));
        EXPECT_EQ(nullptr, appearance->getDataObjectBoundToInput(inputObject));
    }

    TEST_F(AAppearanceTest, failsToSetOrGetValueIfInputBound)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("floatInput", inputObject));
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, 666.f));

        auto dataObject = sharedTestState->getScene().createDataObject(EDataType::Float);
        ASSERT_TRUE(dataObject != nullptr);
        dataObject->setValue(333.f);

        EXPECT_EQ(StatusOK, appearance->bindInput(inputObject, *dataObject));

        const float setValue = 0.111f;
        float value = 0.f;
        EXPECT_NE(StatusOK, appearance->setInputValue(inputObject, setValue));
        EXPECT_EQ(StatusOK, dataObject->getValue(value));
        EXPECT_FLOAT_EQ(333.f, value); // failed setter does not modify data object
        value = 0.f;
        EXPECT_NE(StatusOK, appearance->getInputValue(inputObject, value));
        EXPECT_FLOAT_EQ(0.f, value); // failed getter does not modify out parameter

        EXPECT_EQ(StatusOK, appearance->unbindInput(inputObject));

        EXPECT_EQ(StatusOK, appearance->getInputValue(inputObject, value));
        EXPECT_EQ(666.f, value); // failed setter did not modify previously set value
    }

    TEST_F(AAppearanceTest, failsToBindDataObjectToArrayUniformInput)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("floatInputArray", inputObject));

        auto dataObject = sharedTestState->getScene().createDataObject(EDataType::Float);
        ASSERT_TRUE(dataObject != nullptr);

        EXPECT_NE(StatusOK, appearance->bindInput(inputObject, *dataObject));
    }

    TEST_F(AAppearanceTest, failsToBindDataObjectFromADifferentScene)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("floatInput", inputObject));

        ramses::Scene& anotherScene = *sharedTestState->getClient().createScene(sceneId_t(1u));
        auto dataObject = anotherScene.createDataObject(EDataType::Float);
        ASSERT_TRUE(dataObject != nullptr);

        EXPECT_NE(StatusOK, appearance->bindInput(inputObject, *dataObject));

        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(anotherScene));
    }

    TEST_F(AAppearanceTest, failsToBindDataObjectOfMismatchingType)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("vec4fInput", inputObject));

        auto dataObject = sharedTestState->getScene().createDataObject(EDataType::Float);
        ASSERT_TRUE(dataObject != nullptr);

        EXPECT_NE(StatusOK, appearance->bindInput(inputObject, *dataObject));
        EXPECT_FALSE(appearance->isInputBound(inputObject));
    }

    TEST_F(AAppearanceTestWithSemanticUniforms, failsToBindDataObjectIfInputHasSemantics)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("matrix44fInput", inputObject));

        auto dataObject = sharedTestState->getScene().createDataObject(EDataType::Matrix44F);
        ASSERT_TRUE(dataObject != nullptr);

        EXPECT_NE(StatusOK, appearance->bindInput(inputObject, *dataObject));
    }

    TEST_F(AAppearanceTest, unbindingDataObjectFallsBackToPreviouslySetValue)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("floatInput", inputObject));
        EXPECT_EQ(StatusOK, appearance->setInputValue(inputObject, 666.f));

        auto dataObject = sharedTestState->getScene().createDataObject(EDataType::Float);
        ASSERT_TRUE(dataObject != nullptr);

        EXPECT_EQ(StatusOK, appearance->bindInput(inputObject, *dataObject));
        dataObject->setValue(13.f);

        EXPECT_EQ(StatusOK, appearance->unbindInput(inputObject));
        float value = 0.f;
        EXPECT_EQ(StatusOK, appearance->getInputValue(inputObject, value));
        EXPECT_FLOAT_EQ(666.f, value);
    }

    TEST_F(AAppearanceTest, failsToUnbindIfInputIsNotBound)
    {
        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("floatInput", inputObject));

        EXPECT_NE(StatusOK, appearance->unbindInput(inputObject));
    }

    /// Validation
    TEST_F(AAppearanceTest, reportsErrorWhenValidatedWithInvalidTextureSampler)
    {
        TextureInputInfo texture2DInputInfo;
        getTexture2DInputInfo(texture2DInputInfo);

        TextureInputInfo texture2DMSInputInfo;
        getTexture2DMSInputInfo(texture2DMSInputInfo);

        TextureInputInfo texture3DInputInfo;
        getTexture3DInputInfo(texture3DInputInfo);

        TextureInputInfo textureCubeInputInfo;
        getTextureCubeInputInfo(textureCubeInputInfo);

        TextureInputInfo textureExternalInfo;
        getTextureExternalInputInfo(textureExternalInfo);

        Appearance* newAppearance = sharedTestState->getScene().createAppearance(*sharedTestState->effect, "New Appearance");
        ASSERT_TRUE(nullptr != newAppearance);

        EXPECT_EQ(StatusOK, newAppearance->setInputTexture(texture2DInputInfo.input, *texture2DInputInfo.sampler));
        EXPECT_EQ(StatusOK, newAppearance->setInputTexture(texture2DMSInputInfo.input, *texture2DMSInputInfo.samplerMS));
        EXPECT_EQ(StatusOK, newAppearance->setInputTexture(texture3DInputInfo.input, *texture3DInputInfo.sampler));
        EXPECT_EQ(StatusOK, newAppearance->setInputTexture(textureCubeInputInfo.input, *textureCubeInputInfo.sampler));
        EXPECT_EQ(StatusOK, newAppearance->setInputTexture(textureExternalInfo.input, *textureExternalInfo.samplerExternal));
        EXPECT_EQ(StatusOK, newAppearance->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*texture2DInputInfo.sampler));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*texture2DMSInputInfo.samplerMS));
        EXPECT_NE(StatusOK, newAppearance->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*newAppearance));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*texture2DInputInfo.texture2D));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*texture2DMSInputInfo.renderBuffer));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*textureCubeInputInfo.sampler));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*textureCubeInputInfo.textureCube));
    }

    TEST_F(AAppearanceTest, reportsErrorWhenValidatedWithInvalidTexture)
    {
        TextureInputInfo texture2DInputInfo;
        getTexture2DInputInfo(texture2DInputInfo);

        TextureInputInfo texture2DMSInputInfo;
        getTexture2DMSInputInfo(texture2DMSInputInfo);

        TextureInputInfo texture3DInputInfo;
        getTexture3DInputInfo(texture3DInputInfo);

        TextureInputInfo textureCubeInputInfo;
        getTextureCubeInputInfo(textureCubeInputInfo);

        TextureInputInfo textureExternalInfo;
        getTextureExternalInputInfo(textureExternalInfo);

        Appearance* newAppearance = sharedTestState->getScene().createAppearance(*sharedTestState->effect, "New Appearance");
        ASSERT_TRUE(nullptr != newAppearance);
        EXPECT_EQ(StatusOK, newAppearance->setInputTexture(texture2DInputInfo.input, *texture2DInputInfo.sampler));
        EXPECT_EQ(StatusOK, newAppearance->setInputTexture(texture2DMSInputInfo.input, *texture2DMSInputInfo.samplerMS));
        EXPECT_EQ(StatusOK, newAppearance->setInputTexture(texture3DInputInfo.input, *texture3DInputInfo.sampler));
        EXPECT_EQ(StatusOK, newAppearance->setInputTexture(textureCubeInputInfo.input, *textureCubeInputInfo.sampler));
        EXPECT_EQ(StatusOK, newAppearance->setInputTexture(textureExternalInfo.input, *textureExternalInfo.samplerExternal));
        EXPECT_EQ(StatusOK, newAppearance->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*texture2DInputInfo.texture2D));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*texture2DMSInputInfo.renderBuffer));
        EXPECT_NE(StatusOK, appearance->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*newAppearance));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*texture2DInputInfo.sampler));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*texture2DMSInputInfo.samplerMS));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*textureCubeInputInfo.sampler));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*textureCubeInputInfo.textureCube));
    }

    TEST_F(AAppearanceTest, reportsErrorWhenValidatedWithBoundDataObjectThatWasDestroyed)
    {
        TextureInputInfo texture2DInputInfo;
        getTexture2DInputInfo(texture2DInputInfo);

        TextureInputInfo texture2DMSInputInfo;
        getTexture2DMSInputInfo(texture2DMSInputInfo);

        TextureInputInfo texture3DInputInfo;
        getTexture3DInputInfo(texture3DInputInfo);

        TextureInputInfo textureCubeInputInfo;
        getTextureCubeInputInfo(textureCubeInputInfo);

        TextureInputInfo textureExternalInfo;
        getTextureExternalInputInfo(textureExternalInfo);

        Appearance* newAppearance = sharedTestState->getScene().createAppearance(*sharedTestState->effect, "New Appearance");
        ASSERT_TRUE(nullptr != newAppearance);
        EXPECT_EQ(StatusOK, newAppearance->setInputTexture(texture2DInputInfo.input, *texture2DInputInfo.sampler));
        EXPECT_EQ(StatusOK, newAppearance->setInputTexture(texture2DMSInputInfo.input, *texture2DMSInputInfo.samplerMS));
        EXPECT_EQ(StatusOK, newAppearance->setInputTexture(texture3DInputInfo.input, *texture3DInputInfo.sampler));
        EXPECT_EQ(StatusOK, newAppearance->setInputTexture(textureCubeInputInfo.input, *textureCubeInputInfo.sampler));
        EXPECT_EQ(StatusOK, newAppearance->setInputTexture(textureExternalInfo.input, *textureExternalInfo.samplerExternal));
        EXPECT_EQ(StatusOK, newAppearance->validate());

        UniformInput inputObject;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findUniformInput("floatInput", inputObject));

        auto dataObject = sharedTestState->getScene().createDataObject(EDataType::Float);
        ASSERT_TRUE(dataObject != nullptr);

        EXPECT_EQ(StatusOK, newAppearance->bindInput(inputObject, *dataObject));
        EXPECT_EQ(StatusOK, newAppearance->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*dataObject));
        EXPECT_NE(StatusOK, newAppearance->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*newAppearance));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*texture2DInputInfo.sampler));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*texture2DInputInfo.texture2D));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*texture2DMSInputInfo.samplerMS));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*texture2DMSInputInfo.renderBuffer));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*textureCubeInputInfo.sampler));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*textureCubeInputInfo.textureCube));
    }

    TEST_F(AAppearanceTest, failsWhenWrongBlendingOperationsSet)
    {
        status_t stat = appearance->setBlendingOperations(EBlendOperation::Subtract, EBlendOperation::ReverseSubtract);
        EXPECT_EQ(StatusOK, stat);

        stat = appearance->setBlendingOperations(EBlendOperation::Add, EBlendOperation::Disabled);
        EXPECT_NE(StatusOK, stat);

        stat = appearance->setBlendingOperations(EBlendOperation::Disabled, EBlendOperation::Add);
        EXPECT_NE(StatusOK, stat);

        EBlendOperation opColor = EBlendOperation::Disabled;
        EBlendOperation opAlpha = EBlendOperation::Disabled;
        EXPECT_EQ(StatusOK, appearance->getBlendingOperations(opColor, opAlpha));
        EXPECT_EQ(EBlendOperation::Subtract, opColor);
        EXPECT_EQ(EBlendOperation::ReverseSubtract, opAlpha);
    }

    class AnAppearanceWithGeometryShader : public AAppearanceTest
    {
    public:

        static void SetUpTestCase()
        {
            sharedTestState = std::make_unique<TestEffectCreator>(false, true);
        }
    };

    TEST_F(AnAppearanceWithGeometryShader, HasInitialDrawModeOfGeometryShadersRequirement)
    {
        EDrawMode mode;
        EXPECT_EQ(StatusOK, appearance->getDrawMode(mode));
        EXPECT_EQ(EDrawMode::Lines, mode);
    }

    TEST_F(AnAppearanceWithGeometryShader, RefusesToChangeDrawingMode_WhenIncompatibleToGeometryShader)
    {
        // Shader uses lines, can't change to incompatible types
        EXPECT_NE(StatusOK, appearance->setDrawMode(EDrawMode::Points));
        EXPECT_NE(StatusOK, appearance->setDrawMode(EDrawMode::Triangles));
        EXPECT_NE(StatusOK, appearance->setDrawMode(EDrawMode::TriangleFan));
        EDrawMode mode;
        EXPECT_EQ(StatusOK, appearance->getDrawMode(mode));
        EXPECT_EQ(EDrawMode::Lines, mode);
    }

    TEST_F(AnAppearanceWithGeometryShader, AllowsChangingDrawMode_IfNewModeIsStillCompatible)
    {
        // Shader uses lines, change to line strip is ok - still produces lines for the geometry stage
        EXPECT_EQ(StatusOK, appearance->setDrawMode(EDrawMode::LineStrip));
        EDrawMode mode;
        EXPECT_EQ(StatusOK, appearance->getDrawMode(mode));
        EXPECT_EQ(EDrawMode::LineStrip, mode);
    }

    TEST(AnAppearanceUtils, ChecksValidGeometryShaderModes)
    {
        // valid combinations of appearance draw mode (first) and GS input type (second)
        const std::initializer_list<std::pair<EDrawMode, EDrawMode>> validDrawModeToGSModeCombinations = {
            { EDrawMode::Points, EDrawMode::Points },
            { EDrawMode::Lines, EDrawMode::Lines },
            { EDrawMode::LineStrip, EDrawMode::Lines },
            { EDrawMode::LineLoop, EDrawMode::Lines },
            { EDrawMode::Triangles, EDrawMode::Triangles },
            { EDrawMode::TriangleStrip, EDrawMode::Triangles },
            { EDrawMode::TriangleFan, EDrawMode::Triangles }
        };

        for (int drawModeVal = 0; drawModeVal <= static_cast<int>(EDrawMode::LineStrip); ++drawModeVal)
        {
            for (int gsInputTypeVal = 0; gsInputTypeVal <= static_cast<int>(EDrawMode::LineStrip); ++gsInputTypeVal)
            {
                const auto drawMode = static_cast<EDrawMode>(drawModeVal);
                const auto gsInputType = static_cast<EDrawMode>(gsInputTypeVal);

                // gs input mode restricted to only these types
                if (gsInputType != EDrawMode::Points && gsInputType != EDrawMode::Lines && gsInputType != EDrawMode::Triangles)
                    continue;

                const auto it = std::find_if(validDrawModeToGSModeCombinations.begin(), validDrawModeToGSModeCombinations.end(), [&](const auto& modePair) {
                    return modePair.first == drawMode && modePair.second == gsInputType;
                    });
                const bool isValid = (it != validDrawModeToGSModeCombinations.end());

                EXPECT_EQ(isValid, AppearanceUtils::GeometryShaderCompatibleWithDrawMode(gsInputType, drawMode));
            }
        }
    }
}
