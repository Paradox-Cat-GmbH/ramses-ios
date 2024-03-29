//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <array>

#include "PlatformAbstraction/PlatformError.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/TextureSampler.h"
#include "ramses-client-api/TextureSamplerMS.h"
#include "ramses-client-api/RenderBuffer.h"
#include "ramses-client-api/RenderTargetDescription.h"
#include "ramses-client-api/RenderTarget.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/BlitPass.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/DataObject.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/TextureSamplerExternal.h"
#include "ramses-client-api/ArrayBuffer.h"
#include "ramses-client-api/Texture2DBuffer.h"
#include "ramses-client-api/PickableObject.h"
#include "ramses-client-api/ArrayResource.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/Effect.h"
#include "ramses-utils.h"

#include "ScenePersistationTest.h"
#include "CameraNodeImpl.h"
#include "AppearanceImpl.h"
#include "DataObjectImpl.h"
#include "GeometryBindingImpl.h"
#include "EffectImpl.h"
#include "SceneImpl.h"
#include "Texture2DImpl.h"
#include "TextureSamplerImpl.h"
#include "RenderGroupImpl.h"
#include "RenderPassImpl.h"
#include "BlitPassImpl.h"
#include "PickableObjectImpl.h"
#include "RenderBufferImpl.h"
#include "RenderTargetImpl.h"
#include "MeshNodeImpl.h"
#include "ArrayBufferImpl.h"
#include "Texture2DBufferImpl.h"

#include "Utils/File.h"
#include "SceneAPI/IScene.h"
#include "SceneAPI/BlitPass.h"
#include "SceneAPI/RenderGroupUtils.h"
#include "SceneAPI/RenderPass.h"
#include "Scene/ESceneActionId.h"
#include "Scene/ResourceChanges.h"
#include "Scene/SceneActionApplier.h"

#include "TestEffects.h"
#include "FileDescriptorHelper.h"
#include "UnsafeTestMemoryHelpers.h"
#include "RamsesObjectTestTypes.h"

#include <fstream>
#include <sys/stat.h>
#include <fcntl.h>

namespace ramses
{
    using namespace testing;

    TEST_F(ASceneLoadedFromFile, canWriteAndReadSceneFromFile)
    {
        EXPECT_EQ(StatusOK, m_scene.saveToFile("someTemporaryFile.ram", false));
        EXPECT_NE(nullptr, m_clientForLoading.loadSceneFromFile("someTemporaryFile.ram", false));
    }

    TEST_F(ASceneLoadedFromFile, canWriteAndReadSceneFromMemoryWithExplicitDeleter)
    {
        EXPECT_EQ(StatusOK, m_scene.saveToFile("someTemporaryFile.ram", false));

        ramses_internal::File f("someTemporaryFile.ram");
        size_t fileSize = 0;
        EXPECT_TRUE(f.getSizeInBytes(fileSize));

        std::unique_ptr<unsigned char[], void(*)(const unsigned char*)> data(new unsigned char[fileSize], [](const unsigned char* ptr) { delete[] ptr; });
        size_t numBytesRead = 0;
        EXPECT_TRUE(f.open(ramses_internal::File::Mode::ReadOnlyBinary));
        EXPECT_EQ(ramses_internal::EStatus::Ok, f.read(data.get(), fileSize, numBytesRead));
        EXPECT_NE(nullptr, m_clientForLoading.loadSceneFromMemory(std::move(data), fileSize, false));
    }

    TEST_F(ASceneLoadedFromFile, canWriteAndReadSceneFromMemoryWithImplicitDeleter)
    {
        EXPECT_EQ(StatusOK, m_scene.saveToFile("someTemporaryFile.ram", false));

        ramses_internal::File f("someTemporaryFile.ram");
        size_t fileSize = 0;
        EXPECT_TRUE(f.getSizeInBytes(fileSize));

        std::unique_ptr<unsigned char[]> data(new unsigned char[fileSize]);
        size_t numBytesRead = 0;
        EXPECT_TRUE(f.open(ramses_internal::File::Mode::ReadOnlyBinary));
        EXPECT_EQ(ramses_internal::EStatus::Ok, f.read(data.get(), fileSize, numBytesRead));
        EXPECT_NE(nullptr, RamsesUtils::LoadSceneFromMemory(m_clientForLoading, std::move(data), fileSize, false));
    }

    TEST_F(ASceneLoadedFromFile, canWriteAndReadSceneFromFileDescriptor)
    {
        EXPECT_EQ(StatusOK, m_scene.saveToFile("someTemporaryFile.ram", false));

        size_t fileSize = 0;
        {
            // write to a file with some offset
            ramses_internal::File inFile("someTemporaryFile.ram");
            EXPECT_TRUE(inFile.getSizeInBytes(fileSize));
            std::vector<unsigned char> data(fileSize);
            size_t numBytesRead = 0;
            EXPECT_TRUE(inFile.open(ramses_internal::File::Mode::ReadOnlyBinary));
            EXPECT_EQ(ramses_internal::EStatus::Ok, inFile.read(data.data(), fileSize, numBytesRead));

            ramses_internal::File outFile("someTemporaryFileWithOffset.ram");
            EXPECT_TRUE(outFile.open(ramses_internal::File::Mode::WriteOverWriteOldBinary));

            uint32_t zeroData = 0;
            EXPECT_TRUE(outFile.write(&zeroData, sizeof(zeroData)));
            EXPECT_TRUE(outFile.write(data.data(), data.size()));
            EXPECT_TRUE(outFile.write(&zeroData, sizeof(zeroData)));
        }

        const int fd = ramses_internal::FileDescriptorHelper::OpenFileDescriptorBinary ("someTemporaryFileWithOffset.ram");
        auto scene = m_clientForLoading.loadSceneFromFileDescriptor(fd, 4, fileSize, false);
        ASSERT_NE(nullptr, scene);
        EXPECT_EQ(123u, scene->getSceneId().getValue());
    }

    TEST_F(ASceneLoadedFromFile, canReadSceneFromFileDescriptorCustomSceneId)
    {
        const char* filename = "someTemporaryFile.ram";
        EXPECT_EQ(StatusOK, m_scene.saveToFile(filename, false));
        size_t fileSize = 0;
        {
            ramses_internal::File inFile(filename);
            EXPECT_TRUE(inFile.getSizeInBytes(fileSize));
        }
        {
            const int fdA = ramses_internal::FileDescriptorHelper::OpenFileDescriptorBinary(filename);
            auto sceneA = m_clientForLoading.loadSceneFromFileDescriptor(ramses::sceneId_t(335), fdA, 0, fileSize, false);
            ASSERT_TRUE(sceneA);
            EXPECT_EQ(335u, sceneA->getSceneId().getValue());
        }
        {
            const int fdB = ramses_internal::FileDescriptorHelper::OpenFileDescriptorBinary (filename);
            auto sceneB = m_clientForLoading.loadSceneFromFileDescriptor(ramses::sceneId_t(339), fdB, 0, fileSize, false);
            ASSERT_TRUE(sceneB);
            EXPECT_EQ(339u, sceneB->getSceneId().getValue());
        }
    }

    TEST_F(ASceneLoadedFromFile, errorsReadingSceneFromFileDescriptorCustomSceneId)
    {
        const char* filename = "someTemporaryFile.ram";
        EXPECT_EQ(StatusOK, m_scene.saveToFile(filename, false));
        size_t fileSize = 0;
        {
            ramses_internal::File inFile(filename);
            EXPECT_TRUE(inFile.getSizeInBytes(fileSize));
        }

        const int fd  = ramses_internal::FileDescriptorHelper::OpenFileDescriptorBinary(filename);

        static_cast<void>(m_clientForLoading.createScene(m_scene.getSceneId()));
        const auto duplicateSceneId = m_clientForLoading.loadSceneFromFileDescriptor(m_scene.getSceneId(), fd, 0, fileSize, false);
        const auto invalidSceneId = m_clientForLoading.loadSceneFromFileDescriptor(ramses::sceneId_t(), fd, 0, fileSize, false);
        const auto invalidFileDescriptor = m_clientForLoading.loadSceneFromFileDescriptor(ramses::sceneId_t(340), 0, 0, fileSize, false);
        const auto invalidFileSize = m_clientForLoading.loadSceneFromFileDescriptor(ramses::sceneId_t(341), fd, 0, 0, false);

        EXPECT_TRUE(duplicateSceneId == nullptr);
        EXPECT_TRUE(invalidSceneId == nullptr);
        EXPECT_TRUE(invalidFileDescriptor == nullptr);
        EXPECT_TRUE(invalidFileSize == nullptr);
    }

    TEST_F(ASceneLoadedFromFile, canReadWriteAScene)
    {
        const status_t status = m_scene.saveToFile("someTemporaryFile.ram", false);
        EXPECT_EQ(StatusOK, status);

        m_sceneLoaded = m_clientForLoading.loadSceneFromFile("someTemporaryFile.ram", {});
        ASSERT_TRUE(nullptr != m_sceneLoaded);

        ObjectTypeHistogram origSceneNumbers;
        ObjectTypeHistogram loadedSceneNumbers;
        fillObjectTypeHistogramFromScene(origSceneNumbers, m_scene);
        fillObjectTypeHistogramFromScene(loadedSceneNumbers, *m_sceneLoaded);
        EXPECT_PRED_FORMAT2(AssertHistogramEqual, origSceneNumbers, loadedSceneNumbers);

        ramses_internal::SceneSizeInformation origSceneSizeInfo = m_scene.m_impl.getIScene().getSceneSizeInformation();
        ramses_internal::SceneSizeInformation loadedSceneSizeInfo = m_sceneLoaded->m_impl.getIScene().getSceneSizeInformation();
        EXPECT_EQ(origSceneSizeInfo, loadedSceneSizeInfo);
    }

    TEST_F(ASceneLoadedFromFile, canReadWriteAPerspectiveCamera)
    {
        PerspectiveCamera* camera = this->m_scene.createPerspectiveCamera("my cam");
        camera->setFrustum(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f);
        camera->setViewport(1, -2, 3, 4);

        doWriteReadCycle();

        PerspectiveCamera* loadedCamera = this->getObjectForTesting<PerspectiveCamera>("my cam");
        EXPECT_EQ(camera->m_impl.getCameraHandle(), loadedCamera->m_impl.getCameraHandle());
        EXPECT_EQ(0.1f, loadedCamera->getLeftPlane());
        EXPECT_EQ(0.2f, loadedCamera->getRightPlane());
        EXPECT_EQ(0.3f, loadedCamera->getBottomPlane());
        EXPECT_EQ(0.4f, loadedCamera->getTopPlane());
        EXPECT_EQ(0.5f, loadedCamera->getNearPlane());
        EXPECT_EQ(0.6f, loadedCamera->getFarPlane());

        EXPECT_EQ(1, loadedCamera->getViewportX());
        EXPECT_EQ(-2, loadedCamera->getViewportY());
        EXPECT_EQ(3u, loadedCamera->getViewportWidth());
        EXPECT_EQ(4u, loadedCamera->getViewportHeight());

        EXPECT_EQ(StatusOK, loadedCamera->validate());

        m_sceneLoaded->destroy(*loadedCamera);
    }

    TEST_F(ASceneLoadedFromFile, canReadWriteAnOrthographicCamera)
    {
        OrthographicCamera* camera = this->m_scene.createOrthographicCamera("my cam");
        camera->setFrustum(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f);
        camera->setViewport(1, -2, 3, 4);

        doWriteReadCycle();

        OrthographicCamera* loadedCamera = this->getObjectForTesting<OrthographicCamera>("my cam");

        EXPECT_EQ(camera->m_impl.getCameraHandle(), loadedCamera->m_impl.getCameraHandle());

        EXPECT_FLOAT_EQ(0.1f, loadedCamera->getLeftPlane());
        EXPECT_FLOAT_EQ(0.2f, loadedCamera->getRightPlane());
        EXPECT_FLOAT_EQ(0.3f, loadedCamera->getBottomPlane());
        EXPECT_FLOAT_EQ(0.4f, loadedCamera->getTopPlane());
        EXPECT_FLOAT_EQ(0.5f, loadedCamera->getNearPlane());
        EXPECT_FLOAT_EQ(0.6f, loadedCamera->getFarPlane());

        EXPECT_EQ(1, loadedCamera->getViewportX());
        EXPECT_EQ(-2, loadedCamera->getViewportY());
        EXPECT_EQ(3u, loadedCamera->getViewportWidth());
        EXPECT_EQ(4u, loadedCamera->getViewportHeight());

        EXPECT_EQ(StatusOK, loadedCamera->validate());

        m_sceneLoaded->destroy(*loadedCamera);
    }

    TEST_F(ASceneLoadedFromFile, canReadWriteAnEffect)
    {
        TestEffects::CreateTestEffectWithAllStages(this->m_scene, "eff");

        doWriteReadCycle();

        auto loadedEffect = this->getObjectForTesting<Effect>("eff");
        ASSERT_TRUE(loadedEffect);

        // check uniforms
        EXPECT_EQ(4u, loadedEffect->getUniformInputCount());
        UniformInput uniform;
        EXPECT_EQ(StatusOK, loadedEffect->getUniformInput(0u, uniform));
        EXPECT_STREQ("vs_uniform", uniform.getName());
        EXPECT_EQ(EDataType::Float, *uniform.getDataType());
        EXPECT_EQ(1u, uniform.getElementCount());

        EXPECT_EQ(StatusOK, loadedEffect->getUniformInput(1u, uniform));
        EXPECT_STREQ("colorRG", uniform.getName());
        EXPECT_EQ(EDataType::Vector2F, *uniform.getDataType());
        EXPECT_EQ(1u, uniform.getElementCount());

        EXPECT_EQ(StatusOK, loadedEffect->getUniformInput(2u, uniform));
        EXPECT_STREQ("colorBA", uniform.getName());
        EXPECT_EQ(EDataType::Float, *uniform.getDataType());
        EXPECT_EQ(2u, uniform.getElementCount());

        EXPECT_EQ(StatusOK, loadedEffect->getUniformInput(3u, uniform));
        EXPECT_STREQ("gs_uniform", uniform.getName());
        EXPECT_EQ(EDataType::Float, *uniform.getDataType());
        EXPECT_EQ(1u, uniform.getElementCount());

        // check attributes
        EXPECT_EQ(3u, loadedEffect->getAttributeInputCount());
        AttributeInput attrib;
        EXPECT_EQ(StatusOK, loadedEffect->getAttributeInput(0u, attrib));
        EXPECT_STREQ("a_position1", attrib.getName());
        EXPECT_EQ(EDataType::Vector3F, *attrib.getDataType());

        EXPECT_EQ(StatusOK, loadedEffect->getAttributeInput(1u, attrib));
        EXPECT_STREQ("a_position2", attrib.getName());
        EXPECT_EQ(EDataType::Float, *attrib.getDataType());

        EXPECT_EQ(StatusOK, loadedEffect->getAttributeInput(2u, attrib));
        EXPECT_STREQ("a_position3", attrib.getName());
        EXPECT_EQ(EDataType::Float, *attrib.getDataType());

        // GS
        EXPECT_TRUE(loadedEffect->hasGeometryShader());
        EDrawMode gsInputType = EDrawMode::Points;
        EXPECT_EQ(StatusOK, loadedEffect->getGeometryShaderInputType(gsInputType));
        EXPECT_EQ(EDrawMode::Lines, gsInputType);
    }

    TEST_F(ASceneLoadedFromFile, canReadWriteAnAppearance)
    {
        Effect* effect = TestEffects::CreateTestEffect(this->m_scene);
        Appearance* appearance = this->m_scene.createAppearance(*effect, "appearance");

        doWriteReadCycle();

        Appearance* loadedAppearance = this->getObjectForTesting<Appearance>("appearance");

        EXPECT_EQ(appearance->getEffect().m_impl.getLowlevelResourceHash(), loadedAppearance->getEffect().m_impl.getLowlevelResourceHash());
        EXPECT_EQ(appearance->m_impl.getRenderStateHandle(), loadedAppearance->m_impl.getRenderStateHandle());
        EXPECT_EQ(appearance->m_impl.getUniformDataInstance(), loadedAppearance->m_impl.getUniformDataInstance());
        EXPECT_EQ(appearance->m_impl.getIScene().getSceneId(), loadedAppearance->m_impl.getIScene().getSceneId());
    }

    TEST_F(ASceneLoadedFromFile, canReadWriteAnAppearanceWithGeometryShaderAndRestrictDrawMode)
    {
        const Effect* effect = TestEffects::CreateTestEffectWithAllStages(this->m_scene);
        this->m_scene.createAppearance(*effect, "appearance");

        doWriteReadCycle();

        Appearance* loadedAppearance = this->getObjectForTesting<Appearance>("appearance");

        EDrawMode drawMode = EDrawMode::Points;
        EXPECT_EQ(StatusOK, loadedAppearance->getDrawMode(drawMode));
        EXPECT_EQ(EDrawMode::Lines, drawMode);

        EDrawMode gsInputType = EDrawMode::Points;
        ASSERT_TRUE(loadedAppearance->getEffect().hasGeometryShader());
        EXPECT_EQ(StatusOK, loadedAppearance->getEffect().getGeometryShaderInputType(gsInputType));
        EXPECT_EQ(EDrawMode::Lines, gsInputType);

        // valid draw mode change
        EXPECT_EQ(StatusOK, loadedAppearance->setDrawMode(EDrawMode::LineStrip));

        // invalid draw mode change
        EXPECT_NE(StatusOK, loadedAppearance->setDrawMode(EDrawMode::Points));
    }

    TEST_F(ASceneLoadedFromFile, canReadWriteAnAppearanceWithGeometryShaderAndRestrictDrawMode_usingSameEffect)
    {
        const Effect* effect1 = TestEffects::CreateTestEffectWithAllStages(this->m_scene);
        const Effect* effect2 = TestEffects::CreateTestEffectWithAllStages(this->m_scene);
        this->m_scene.createAppearance(*effect1, "appearance1");
        this->m_scene.createAppearance(*effect2, "appearance2");

        doWriteReadCycle();

        Appearance* loadedAppearance1 = this->getObjectForTesting<Appearance>("appearance1");
        Appearance* loadedAppearance2 = this->getObjectForTesting<Appearance>("appearance2");

        // appearance 1
        EDrawMode drawMode = EDrawMode::Points;
        EDrawMode gsInputType = EDrawMode::Points;
        EXPECT_EQ(StatusOK, loadedAppearance1->getDrawMode(drawMode));
        EXPECT_EQ(EDrawMode::Lines, drawMode);
        ASSERT_TRUE(loadedAppearance1->getEffect().hasGeometryShader());
        EXPECT_EQ(StatusOK, loadedAppearance1->getEffect().getGeometryShaderInputType(gsInputType));
        EXPECT_EQ(EDrawMode::Lines, gsInputType);

        // appearance 2
        EXPECT_EQ(StatusOK, loadedAppearance1->getDrawMode(drawMode));
        EXPECT_EQ(EDrawMode::Lines, drawMode);
        ASSERT_TRUE(loadedAppearance1->getEffect().hasGeometryShader());
        EXPECT_EQ(StatusOK, loadedAppearance1->getEffect().getGeometryShaderInputType(gsInputType));
        EXPECT_EQ(EDrawMode::Lines, gsInputType);

        // valid draw mode change
        EXPECT_EQ(StatusOK, loadedAppearance1->setDrawMode(EDrawMode::LineStrip));
        EXPECT_EQ(StatusOK, loadedAppearance2->setDrawMode(EDrawMode::LineStrip));

        // invalid draw mode change
        EXPECT_NE(StatusOK, loadedAppearance1->setDrawMode(EDrawMode::Points));
        EXPECT_NE(StatusOK, loadedAppearance2->setDrawMode(EDrawMode::Points));
    }

    TEST_F(ASceneLoadedFromFile, keepingTrackOfsceneObjectIdsAndFindObjectByIdWork)
    {
        Effect* effect = TestEffects::CreateTestEffect(this->m_scene);

        const Appearance* appearance = this->m_scene.createAppearance(*effect, "appearance");
        const GeometryBinding* geometry = this->m_scene.createGeometryBinding(*effect, "geometry");

        const sceneObjectId_t appearanceIdBeforeSaveAndLoad = appearance->getSceneObjectId();
        const sceneObjectId_t geometryIdBeforeSaveAndLoad = geometry->getSceneObjectId();
        EXPECT_NE(appearanceIdBeforeSaveAndLoad, geometryIdBeforeSaveAndLoad);

        doWriteReadCycle();

        const Appearance& appearanceLoaded = RamsesObjectTypeUtils::ConvertTo<Appearance>(*this->m_scene.findObjectById(appearanceIdBeforeSaveAndLoad));
        EXPECT_EQ(appearanceIdBeforeSaveAndLoad, appearanceLoaded.getSceneObjectId());

        const GeometryBinding& geometryLoaded = RamsesObjectTypeUtils::ConvertTo<GeometryBinding>(*this->m_scene.findObjectById(geometryIdBeforeSaveAndLoad));
        EXPECT_EQ(geometryIdBeforeSaveAndLoad, geometryLoaded.getSceneObjectId());

        const Camera* camera = this->m_scene.createOrthographicCamera("camera");
        EXPECT_NE(appearanceIdBeforeSaveAndLoad, camera->getSceneObjectId());
        EXPECT_NE(geometryIdBeforeSaveAndLoad, camera->getSceneObjectId());
    }

    TEST_F(ASceneLoadedFromFile, canReadWriteAnAppearanceWithUniformValuesSetOrBound)
    {
        Effect* effect = TestEffects::CreateTestEffect(this->m_scene);
        Appearance* appearance = this->m_scene.createAppearance(*effect, "appearance");

        UniformInput fragColorR;
        UniformInput fragColorG;
        effect->findUniformInput("u_FragColorR", fragColorR);
        effect->findUniformInput("u_FragColorG", fragColorG);

        auto fragColorDataObject = this->m_scene.createDataObject(EDataType::Float);
        fragColorDataObject->setValue(123.f);

        appearance->setInputValue(fragColorR, 567.f);
        appearance->bindInput(fragColorG, *fragColorDataObject);

        doWriteReadCycle();

        Appearance* loadedAppearance = this->getObjectForTesting<Appearance>("appearance");
        const Effect& loadedEffect = loadedAppearance->getEffect();

        UniformInput fragColorROut;
        UniformInput fragColorGOut;
        loadedEffect.findUniformInput("u_FragColorR", fragColorROut);
        loadedEffect.findUniformInput("u_FragColorG", fragColorGOut);

        float resultR = 0.f;
        loadedAppearance->getInputValue(fragColorROut, resultR);
        EXPECT_FLOAT_EQ(567.f, resultR);

        ASSERT_TRUE(loadedAppearance->isInputBound(fragColorGOut));
        float resultG = 0.f;
        const DataObject& fragColorDataObjectOut = *loadedAppearance->getDataObjectBoundToInput(fragColorGOut);
        fragColorDataObjectOut.getValue(resultG);
        EXPECT_EQ(123.f, resultG);
    }

    TEST_F(ASceneLoadedFromFile, multipleAppearancesSharingSameEffectAreCorrectlyWrittenAndLoaded)
    {
        Effect* effect = TestEffects::CreateTestEffect(this->m_scene);
        const Appearance* appearance1 = m_scene.createAppearance(*effect, "appearance1");
        const Appearance* appearance2 = m_scene.createAppearance(*effect, "appearance2");

        // check data layout ref count on LL
        EXPECT_EQ(appearance1->m_impl.getUniformDataLayout(), appearance2->m_impl.getUniformDataLayout());
        const uint32_t numReferences = m_scene.m_impl.getIScene().getNumDataLayoutReferences(appearance1->m_impl.getUniformDataLayout());
        EXPECT_EQ(2u, numReferences);

        doWriteReadCycle();

        Appearance* loadedAppearance1 = this->getObjectForTesting<Appearance>("appearance1");
        Appearance* loadedAppearance2 = this->getObjectForTesting<Appearance>("appearance2");

        // check data layout ref count on LL in loaded scene
        EXPECT_EQ(loadedAppearance1->m_impl.getUniformDataLayout(), loadedAppearance2->m_impl.getUniformDataLayout());
        const uint32_t numReferencesLoaded = m_scene.m_impl.getIScene().getNumDataLayoutReferences(loadedAppearance1->m_impl.getUniformDataLayout());
        EXPECT_EQ(numReferences, numReferencesLoaded);

        m_sceneLoaded->destroy(*loadedAppearance2);
        m_sceneLoaded->destroy(*loadedAppearance1);
    }

    TEST_F(ASceneLoadedFromFile, canReadWriteGeometryBinding)
    {
        static const uint16_t inds[3] = { 0, 1, 2 };
        ArrayResource* const indices = this->m_scene.createArrayResource(3u, inds, ramses::ResourceCacheFlag_DoNotCache, "indices");

        Effect* effect = TestEffects::CreateTestEffectWithAttribute(this->m_scene);

        GeometryBinding* geometry = this->m_scene.createGeometryBinding(*effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);
        EXPECT_EQ(StatusOK, geometry->setIndices(*indices));

        doWriteReadCycle();

        const GeometryBinding* loadedGeometry = this->getObjectForTesting<GeometryBinding>("geometry");

        EXPECT_EQ(geometry->m_impl.getEffectHash(), loadedGeometry->m_impl.getEffectHash());
        EXPECT_EQ(geometry->m_impl.getAttributeDataLayout(), loadedGeometry->m_impl.getAttributeDataLayout());
        EXPECT_EQ(geometry->m_impl.getAttributeDataInstance(), loadedGeometry->m_impl.getAttributeDataInstance());
        EXPECT_EQ(geometry->m_impl.getIndicesCount(), loadedGeometry->m_impl.getIndicesCount());

        const Effect& loadedEffect = loadedGeometry->getEffect();
        EXPECT_EQ(effect->getResourceId(), loadedEffect.getResourceId());
        EXPECT_EQ(effect->m_impl.getLowlevelResourceHash(), loadedEffect.m_impl.getLowlevelResourceHash());
        EXPECT_EQ(effect->m_impl.getObjectRegistryHandle(), loadedEffect.m_impl.getObjectRegistryHandle());
        EXPECT_EQ(4u, loadedEffect.getAttributeInputCount());
        AttributeInput attributeInputOut;
        EXPECT_EQ(StatusOK, loadedEffect.findAttributeInput("a_position", attributeInputOut));
    }

    TEST_F(ASceneLoadedFromFile, canReadWriteAMeshNode)
    {
        MeshNode* meshNode = this->m_scene.createMeshNode("a meshnode");

        doWriteReadCycle();

        MeshNode* loadedMeshNode = this->getObjectForTesting<MeshNode>("a meshnode");

        EXPECT_EQ(meshNode->getAppearance(), loadedMeshNode->getAppearance());
        EXPECT_EQ(meshNode->getGeometryBinding(),   loadedMeshNode->getGeometryBinding());
        EXPECT_EQ(meshNode->getStartIndex(), loadedMeshNode->getStartIndex());
        EXPECT_EQ(meshNode->getIndexCount(), loadedMeshNode->getIndexCount());
        EXPECT_EQ(meshNode->m_impl.getFlattenedVisibility(), loadedMeshNode->m_impl.getFlattenedVisibility());
    }

    TEST_F(ASceneLoadedFromFile, canReadWriteAMeshNode_withVisibilityParent)
    {
        MeshNode* meshNode = this->m_scene.createMeshNode("a meshnode");

        Node* visibilityParent = this->m_scene.createNode("vis node");
        visibilityParent->setVisibility(EVisibilityMode::Invisible);
        visibilityParent->addChild(*meshNode);

        // Apply visibility state only with flush, not before
        EXPECT_EQ(meshNode->m_impl.getFlattenedVisibility(), EVisibilityMode::Visible);
        this->m_scene.flush();
        EXPECT_EQ(meshNode->m_impl.getFlattenedVisibility(), EVisibilityMode::Invisible);

        doWriteReadCycle();

        MeshNode* loadedMeshNode = this->getObjectForTesting<MeshNode>("a meshnode");
        EXPECT_EQ(loadedMeshNode->m_impl.getFlattenedVisibility(), EVisibilityMode::Invisible);
    }

    TEST_F(ASceneLoadedFromFile, canReadWriteAMeshNode_withVisibilityParentOff)
    {
        MeshNode* meshNode = this->m_scene.createMeshNode("a meshnode");

        Node* visibilityParent = this->m_scene.createNode("vis node");
        visibilityParent->setVisibility(EVisibilityMode::Off);
        visibilityParent->addChild(*meshNode);

        // Apply visibility state only with flush, not before
        EXPECT_EQ(meshNode->m_impl.getFlattenedVisibility(), EVisibilityMode::Visible);
        this->m_scene.flush();
        EXPECT_EQ(meshNode->m_impl.getFlattenedVisibility(), EVisibilityMode::Off);

        doWriteReadCycle();

        MeshNode* loadedMeshNode = this->getObjectForTesting<MeshNode>("a meshnode");
        EXPECT_EQ(loadedMeshNode->m_impl.getFlattenedVisibility(), EVisibilityMode::Off);
    }

    TEST_F(ASceneLoadedFromFile, canReadWriteAMeshNode_withValues)
    {
        Effect* effect = TestEffects::CreateTestEffect(this->m_scene);
        Appearance* appearance = this->m_scene.createAppearance(*effect, "appearance");

        GeometryBinding* geometry = this->m_scene.createGeometryBinding(*effect, "geometry");
        const uint16_t data = 0u;
        ArrayResource* indices = this->m_scene.createArrayResource(1u, &data, ramses::ResourceCacheFlag_DoNotCache, "indices");
        geometry->setIndices(*indices);

        MeshNode* meshNode = this->m_scene.createMeshNode("a meshnode");

        //EXPECT_EQ(StatusOK, meshNode.setIndexArray(indices));
        EXPECT_EQ(StatusOK, meshNode->setAppearance(*appearance));
        EXPECT_EQ(StatusOK, meshNode->setGeometryBinding(*geometry));
        EXPECT_EQ(StatusOK, meshNode->setStartIndex(456));
        EXPECT_EQ(StatusOK, meshNode->setIndexCount(678u));
        EXPECT_EQ(StatusOK, meshNode->m_impl.setFlattenedVisibility(EVisibilityMode::Off));
        doWriteReadCycle();

        MeshNode* loadedMeshNode = this->getObjectForTesting<MeshNode>("a meshnode");

        EXPECT_STREQ(meshNode->getAppearance()->getName(), loadedMeshNode->getAppearance()->getName());
        EXPECT_EQ(meshNode->getAppearance()->getSceneObjectId(), loadedMeshNode->getAppearance()->getSceneObjectId());
        EXPECT_STREQ(meshNode->getGeometryBinding()->getName(), loadedMeshNode->getGeometryBinding()->getName());
        EXPECT_EQ(meshNode->getGeometryBinding()->getSceneObjectId(), loadedMeshNode->getGeometryBinding()->getSceneObjectId());
        EXPECT_EQ(meshNode->getSceneObjectId(), loadedMeshNode->getSceneObjectId());
        EXPECT_EQ(456u, loadedMeshNode->getStartIndex());
        EXPECT_EQ(678u, loadedMeshNode->getIndexCount());
        EXPECT_EQ(loadedMeshNode->m_impl.getFlattenedVisibility(), EVisibilityMode::Off);
    }

    TEST_F(ASceneLoadedFromFile, canReadWriteANodeWithVisibility)
    {
        Node* visibilityNode = this->m_scene.createNode("a visibilitynode");

        visibilityNode->setVisibility(EVisibilityMode::Invisible);

        doWriteReadCycle();

        Node* loadedVisibilityNode = this->getObjectForTesting<Node>("a visibilitynode");

        EXPECT_EQ(loadedVisibilityNode->getVisibility(), EVisibilityMode::Invisible);
    }

    TEST_F(ASceneLoadedFromFile, canReadWriteARenderGroup)
    {
        RenderGroup* renderGroup = this->m_scene.createRenderGroup("a rendergroup");

        MeshNode* meshA = this->m_scene.createMeshNode("meshA");
        MeshNode* meshB = this->m_scene.createMeshNode("meshB");

        renderGroup->addMeshNode(*meshA, 1);
        renderGroup->addMeshNode(*meshB, 2);

        doWriteReadCycle();

        RenderGroup* loadedRenderGroup = this->getObjectForTesting<RenderGroup>("a rendergroup");
        const MeshNode* loadedMeshA = this->getObjectForTesting<MeshNode>("meshA");
        const MeshNode* loadedMeshB = this->getObjectForTesting<MeshNode>("meshB");

        EXPECT_STREQ(renderGroup->getName(), loadedRenderGroup->getName());
        EXPECT_EQ(renderGroup->getSceneObjectId(), loadedRenderGroup->getSceneObjectId());

        EXPECT_EQ(2u, loadedRenderGroup->m_impl.getAllMeshes().size());
        EXPECT_EQ(&loadedMeshA->m_impl, loadedRenderGroup->m_impl.getAllMeshes()[0]);
        EXPECT_EQ(&loadedMeshB->m_impl, loadedRenderGroup->m_impl.getAllMeshes()[1]);

        const auto& internalRg = m_sceneLoaded->m_impl.getIScene().getRenderGroup(renderGroup->m_impl.getRenderGroupHandle());
        ASSERT_TRUE(ramses_internal::RenderGroupUtils::ContainsRenderable(meshA->m_impl.getRenderableHandle(), internalRg));
        ASSERT_TRUE(ramses_internal::RenderGroupUtils::ContainsRenderable(meshB->m_impl.getRenderableHandle(), internalRg));
        EXPECT_EQ(1, ramses_internal::RenderGroupUtils::FindRenderableEntry(meshA->m_impl.getRenderableHandle(), internalRg)->order);
        EXPECT_EQ(2, ramses_internal::RenderGroupUtils::FindRenderableEntry(meshB->m_impl.getRenderableHandle(), internalRg)->order);
    }

    TEST_F(ASceneLoadedFromFile, canReadWriteANestedRenderGroup)
    {
        RenderGroup* renderGroup = this->m_scene.createRenderGroup("a rendergroup");
        RenderGroup* nestedRenderGroup = this->m_scene.createRenderGroup("a nested rendergroup");

        MeshNode* meshA = this->m_scene.createMeshNode("meshA");
        MeshNode* meshB = this->m_scene.createMeshNode("meshB");

        ASSERT_EQ(StatusOK, renderGroup->addMeshNode(*meshA, 1));
        ASSERT_EQ(StatusOK, nestedRenderGroup->addMeshNode(*meshB, 2));
        ASSERT_EQ(StatusOK, renderGroup->addRenderGroup(*nestedRenderGroup, 1));

        doWriteReadCycle();

        RenderGroup* loadedRenderGroup = this->getObjectForTesting<RenderGroup>("a rendergroup");
        RenderGroup* loadedNestedRenderGroup = this->getObjectForTesting<RenderGroup>("a nested rendergroup");
        const MeshNode* loadedMeshA = this->getObjectForTesting<MeshNode>("meshA");
        const MeshNode* loadedMeshB = this->getObjectForTesting<MeshNode>("meshB");

        EXPECT_STREQ(nestedRenderGroup->getName(), loadedNestedRenderGroup->getName());
        EXPECT_EQ(nestedRenderGroup->getSceneObjectId(), loadedNestedRenderGroup->getSceneObjectId());

        EXPECT_EQ(1u, loadedRenderGroup->m_impl.getAllMeshes().size());
        EXPECT_EQ(1u, loadedRenderGroup->m_impl.getAllRenderGroups().size());
        EXPECT_EQ(1u, loadedNestedRenderGroup->m_impl.getAllMeshes().size());

        EXPECT_EQ(&loadedMeshA->m_impl, loadedRenderGroup->m_impl.getAllMeshes()[0]);
        EXPECT_EQ(&loadedMeshB->m_impl, loadedNestedRenderGroup->m_impl.getAllMeshes()[0]);

        EXPECT_EQ(&loadedNestedRenderGroup->m_impl, loadedRenderGroup->m_impl.getAllRenderGroups()[0]);

        const auto& internalRg = m_sceneLoaded->m_impl.getIScene().getRenderGroup(renderGroup->m_impl.getRenderGroupHandle());
        ASSERT_TRUE(ramses_internal::RenderGroupUtils::ContainsRenderable(meshA->m_impl.getRenderableHandle(), internalRg));
        EXPECT_EQ(1, ramses_internal::RenderGroupUtils::FindRenderableEntry(meshA->m_impl.getRenderableHandle(), internalRg)->order);

        ASSERT_TRUE(ramses_internal::RenderGroupUtils::ContainsRenderGroup(nestedRenderGroup->m_impl.getRenderGroupHandle(), internalRg));
        EXPECT_EQ(1, ramses_internal::RenderGroupUtils::FindRenderGroupEntry(nestedRenderGroup->m_impl.getRenderGroupHandle(), internalRg)->order);

        const auto& internalRgNested = m_sceneLoaded->m_impl.getIScene().getRenderGroup(nestedRenderGroup->m_impl.getRenderGroupHandle());
        ASSERT_TRUE(ramses_internal::RenderGroupUtils::ContainsRenderable(meshB->m_impl.getRenderableHandle(), internalRgNested));
        EXPECT_EQ(2, ramses_internal::RenderGroupUtils::FindRenderableEntry(meshB->m_impl.getRenderableHandle(), internalRgNested)->order);
    }

    TEST_F(ASceneLoadedFromFile, canReadWriteABasicRenderPass)
    {
        const int32_t renderOrder = 1;

        RenderPass* renderPass = this->m_scene.createRenderPass("a renderpass");
        EXPECT_EQ(StatusOK, renderPass->setRenderOrder(renderOrder));
        EXPECT_EQ(StatusOK, renderPass->setEnabled(false));
        EXPECT_EQ(StatusOK, renderPass->setRenderOnce(true));

        doWriteReadCycle();

        RenderPass* loadedRenderPass = this->getObjectForTesting<RenderPass>("a renderpass");

        EXPECT_STREQ(renderPass->getName(), loadedRenderPass->getName());
        EXPECT_EQ(renderPass->getSceneObjectId(), loadedRenderPass->getSceneObjectId());
        EXPECT_EQ(renderOrder, loadedRenderPass->getRenderOrder());
        EXPECT_FALSE(loadedRenderPass->isEnabled());
        EXPECT_TRUE(loadedRenderPass->isRenderOnce());
    }

    TEST_F(ASceneLoadedFromFile, canReadWriteARenderPassWithACamera)
    {
        RenderPass* renderPass = this->m_scene.createRenderPass("a renderpass");

        PerspectiveCamera* perspCam = this->m_scene.createPerspectiveCamera("camera");
        perspCam->setFrustum(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f);
        perspCam->setViewport(0, 0, 100, 200);
        renderPass->setCamera(*perspCam);

        doWriteReadCycle();

        RenderPass* loadedRenderPass = this->getObjectForTesting<RenderPass>("a renderpass");
        const Camera* loadedCamera = this->getObjectForTesting<Camera>("camera");

        EXPECT_STREQ(renderPass->getName(), loadedRenderPass->getName());
        EXPECT_EQ(renderPass->getSceneObjectId(), loadedRenderPass->getSceneObjectId());
        EXPECT_EQ(loadedCamera, loadedRenderPass->getCamera());
        EXPECT_EQ(StatusOK, loadedCamera->validate());
    }

    TEST_F(ASceneLoadedFromFile, canReadWriteARenderPassWhichHasRenderGroups)
    {
        RenderPass* renderPass = this->m_scene.createRenderPass("a renderpass");
        RenderGroup* groupA = this->m_scene.createRenderGroup("groupA");
        RenderGroup* groupB = this->m_scene.createRenderGroup("groupB");
        renderPass->addRenderGroup(*groupA, 1);
        renderPass->addRenderGroup(*groupB, 2);

        doWriteReadCycle();

        RenderPass* loadedRenderPass = this->getObjectForTesting<RenderPass>("a renderpass");
        const RenderGroup* loadedMeshA = this->getObjectForTesting<RenderGroup>("groupA");
        const RenderGroup* loadedMeshB = this->getObjectForTesting<RenderGroup>("groupB");

        EXPECT_STREQ(renderPass->getName(), loadedRenderPass->getName());
        EXPECT_EQ(renderPass->getSceneObjectId(), loadedRenderPass->getSceneObjectId());
        EXPECT_EQ(2u, loadedRenderPass->m_impl.getAllRenderGroups().size());
        EXPECT_EQ(&loadedMeshA->m_impl, loadedRenderPass->m_impl.getAllRenderGroups()[0]);
        EXPECT_EQ(&loadedMeshB->m_impl, loadedRenderPass->m_impl.getAllRenderGroups()[1]);

        const ramses_internal::RenderPass& internalRP = m_sceneLoaded->m_impl.getIScene().getRenderPass(renderPass->m_impl.getRenderPassHandle());
        ASSERT_TRUE(ramses_internal::RenderGroupUtils::ContainsRenderGroup(groupA->m_impl.getRenderGroupHandle(), internalRP));
        ASSERT_TRUE(ramses_internal::RenderGroupUtils::ContainsRenderGroup(groupB->m_impl.getRenderGroupHandle(), internalRP));
        EXPECT_EQ(groupA->m_impl.getRenderGroupHandle(), ramses_internal::RenderGroupUtils::FindRenderGroupEntry(groupA->m_impl.getRenderGroupHandle(), internalRP)->renderGroup);
        EXPECT_EQ(groupB->m_impl.getRenderGroupHandle(), ramses_internal::RenderGroupUtils::FindRenderGroupEntry(groupB->m_impl.getRenderGroupHandle(), internalRP)->renderGroup);
        EXPECT_EQ(1, ramses_internal::RenderGroupUtils::FindRenderGroupEntry(groupA->m_impl.getRenderGroupHandle(), internalRP)->order);
        EXPECT_EQ(2, ramses_internal::RenderGroupUtils::FindRenderGroupEntry(groupB->m_impl.getRenderGroupHandle(), internalRP)->order);
    }

    TEST_F(ASceneLoadedFromFile, canReadWriteBlitPass)
    {
        const int32_t renderOrder = 1;
        const RenderBuffer* srcRenderBuffer = this->m_scene.createRenderBuffer(23, 42, ERenderBufferType::Depth, ERenderBufferFormat::Depth24, ERenderBufferAccessMode::WriteOnly, 0u, "src renderBuffer");
        const RenderBuffer* dstRenderBuffer = this->m_scene.createRenderBuffer(23, 42, ERenderBufferType::Depth, ERenderBufferFormat::Depth24, ERenderBufferAccessMode::WriteOnly, 0u, "dst renderBuffer");

        BlitPass* blitPass = this->m_scene.createBlitPass(*srcRenderBuffer, *dstRenderBuffer, "a blitpass");
        EXPECT_EQ(StatusOK, blitPass->setRenderOrder(renderOrder));
        EXPECT_EQ(StatusOK, blitPass->setEnabled(false));

        doWriteReadCycle();

        const BlitPass* loadedBlitPass = this->getObjectForTesting<BlitPass>("a blitpass");

        EXPECT_STREQ(blitPass->getName(), loadedBlitPass->getName());
        EXPECT_EQ(blitPass->getSceneObjectId(), loadedBlitPass->getSceneObjectId());
        EXPECT_EQ(renderOrder, loadedBlitPass->getRenderOrder());
        EXPECT_FALSE(loadedBlitPass->isEnabled());

        const ramses_internal::BlitPassHandle loadedBlitPassHandle = loadedBlitPass->m_impl.getBlitPassHandle();
        const ramses_internal::BlitPass& blitPassInternal = m_sceneLoaded->m_impl.getIScene().getBlitPass(loadedBlitPassHandle);
        EXPECT_EQ(renderOrder, blitPassInternal.renderOrder);
        EXPECT_FALSE(blitPassInternal.isEnabled);
        EXPECT_EQ(srcRenderBuffer->m_impl.getRenderBufferHandle(), blitPassInternal.sourceRenderBuffer);
        EXPECT_EQ(dstRenderBuffer->m_impl.getRenderBufferHandle(), blitPassInternal.destinationRenderBuffer);
        EXPECT_EQ(renderOrder, blitPassInternal.renderOrder);

        const ramses_internal::PixelRectangle& sourceRegion = blitPassInternal.sourceRegion;
        EXPECT_EQ(0u, sourceRegion.x);
        EXPECT_EQ(0u, sourceRegion.y);
        EXPECT_EQ(static_cast<int32_t>(srcRenderBuffer->getWidth()), sourceRegion.width);
        EXPECT_EQ(static_cast<int32_t>(srcRenderBuffer->getHeight()), sourceRegion.height);

        const ramses_internal::PixelRectangle& destinationRegion = blitPassInternal.destinationRegion;
        EXPECT_EQ(0u, destinationRegion.x);
        EXPECT_EQ(0u, destinationRegion.y);
        EXPECT_EQ(static_cast<int32_t>(dstRenderBuffer->getWidth()), destinationRegion.width);
        EXPECT_EQ(static_cast<int32_t>(dstRenderBuffer->getHeight()), destinationRegion.height);

        //client HL api
        {
            uint32_t sourceXOut;
            uint32_t sourceYOut;
            uint32_t destinationXOut;
            uint32_t destinationYOut;
            uint32_t widthOut;
            uint32_t heightOut;
            loadedBlitPass->getBlittingRegion(sourceXOut, sourceYOut, destinationXOut, destinationYOut, widthOut, heightOut);
            EXPECT_EQ(0u, sourceXOut);
            EXPECT_EQ(0u, sourceYOut);
            EXPECT_EQ(0u, destinationXOut);
            EXPECT_EQ(0u, destinationYOut);
            EXPECT_EQ(dstRenderBuffer->getWidth(), widthOut);
            EXPECT_EQ(dstRenderBuffer->getHeight(), heightOut);
        }

        EXPECT_EQ(srcRenderBuffer->m_impl.getRenderBufferHandle(), loadedBlitPass->getSourceRenderBuffer().m_impl.getRenderBufferHandle());
        EXPECT_EQ(srcRenderBuffer->m_impl.getObjectRegistryHandle(), loadedBlitPass->getSourceRenderBuffer().m_impl.getObjectRegistryHandle());

        EXPECT_EQ(dstRenderBuffer->m_impl.getRenderBufferHandle(), loadedBlitPass->getDestinationRenderBuffer().m_impl.getRenderBufferHandle());
        EXPECT_EQ(dstRenderBuffer->m_impl.getObjectRegistryHandle(), loadedBlitPass->getDestinationRenderBuffer().m_impl.getObjectRegistryHandle());
    }

    TEST_F(ASceneLoadedFromFile, canReadWritePickableObject)
    {
        const EDataType geometryBufferDataType = EDataType::Vector3F;
        const ArrayBuffer* geometryBuffer = this->m_scene.createArrayBuffer(geometryBufferDataType, 3u, "geometryBuffer");

        const int32_t viewPort_x = 1;
        const int32_t viewPort_y = 2;
        const uint32_t viewPort_width = 200;
        const uint32_t viewPort_height = 300;
        PerspectiveCamera* pickableCamera = m_scene.createPerspectiveCamera("pickableCamera");
        pickableCamera->setFrustum(-1.4f, 1.4f, -1.4f, 1.4f, 1.f, 100.f);
        pickableCamera->setViewport(viewPort_x, viewPort_y, viewPort_width, viewPort_height);

        const pickableObjectId_t id(2);
        PickableObject* pickableObject = this->m_scene.createPickableObject(*geometryBuffer, id, "PickableObject");
        EXPECT_EQ(StatusOK, pickableObject->setCamera(*pickableCamera));
        EXPECT_EQ(StatusOK, pickableObject->setEnabled(false));

        doWriteReadCycle();

        const PickableObject* loadedPickableObject = this->getObjectForTesting<PickableObject>("PickableObject");

        EXPECT_STREQ(pickableObject->getName(), loadedPickableObject->getName());
        EXPECT_EQ(pickableObject->getSceneObjectId(), loadedPickableObject->getSceneObjectId());
        EXPECT_EQ(id, loadedPickableObject->getPickableObjectId());
        EXPECT_FALSE(loadedPickableObject->isEnabled());
        EXPECT_EQ(this->getObjectForTesting<PerspectiveCamera>("pickableCamera"), loadedPickableObject->getCamera());
        EXPECT_EQ(this->getObjectForTesting<ArrayBuffer>("geometryBuffer"), &loadedPickableObject->getGeometryBuffer());

        const ramses_internal::PickableObjectHandle loadedPickableObjectPassHandle = loadedPickableObject->m_impl.getPickableObjectHandle();
        const ramses_internal::PickableObject& pickableObjectInternal = m_sceneLoaded->m_impl.getIScene().getPickableObject(loadedPickableObjectPassHandle);
        EXPECT_EQ(id.getValue(), pickableObjectInternal.id.getValue());
        EXPECT_FALSE(pickableObjectInternal.isEnabled);
        EXPECT_EQ(geometryBuffer->m_impl.getDataBufferHandle(), pickableObjectInternal.geometryHandle);
        EXPECT_EQ(pickableCamera->m_impl.getCameraHandle(), pickableObjectInternal.cameraHandle);

        EXPECT_EQ(geometryBuffer->m_impl.getDataBufferHandle(), loadedPickableObject->getGeometryBuffer().m_impl.getDataBufferHandle());
        EXPECT_EQ(geometryBuffer->m_impl.getObjectRegistryHandle(), loadedPickableObject->getGeometryBuffer().m_impl.getObjectRegistryHandle());

        EXPECT_EQ(pickableCamera->m_impl.getCameraHandle(), loadedPickableObject->getCamera()->m_impl.getCameraHandle());
        EXPECT_EQ(pickableCamera->m_impl.getObjectRegistryHandle(), loadedPickableObject->getCamera()->m_impl.getObjectRegistryHandle());
    }

    TEST_F(ASceneLoadedFromFile, canReadWriteRenderBuffer)
    {
        RenderBuffer* renderBuffer = this->m_scene.createRenderBuffer(23, 42, ERenderBufferType::Depth, ERenderBufferFormat::Depth24, ERenderBufferAccessMode::WriteOnly, 4u, "a renderTarget");

        doWriteReadCycle();

        RenderBuffer* loadedRenderBuffer = this->getObjectForTesting<RenderBuffer>("a renderTarget");

        EXPECT_STREQ(renderBuffer->getName(), loadedRenderBuffer->getName());
        EXPECT_EQ(renderBuffer->getSceneObjectId(), loadedRenderBuffer->getSceneObjectId());
        EXPECT_EQ(renderBuffer->getWidth(), loadedRenderBuffer->getWidth());
        EXPECT_EQ(renderBuffer->getHeight(), loadedRenderBuffer->getHeight());
        EXPECT_EQ(renderBuffer->getBufferType(), loadedRenderBuffer->getBufferType());
        EXPECT_EQ(renderBuffer->getBufferFormat(), loadedRenderBuffer->getBufferFormat());
        EXPECT_EQ(renderBuffer->getAccessMode(), loadedRenderBuffer->getAccessMode());
        EXPECT_EQ(renderBuffer->getSampleCount(), loadedRenderBuffer->getSampleCount());

        EXPECT_EQ(renderBuffer->m_impl.getRenderBufferHandle(), loadedRenderBuffer->m_impl.getRenderBufferHandle());
    }

    TEST_F(ASceneLoadedFromFile, canReadWriteARenderPassWithARenderTargetAndCamera)
    {
        RenderPass* renderPass = this->m_scene.createRenderPass("a renderpass");

        RenderBuffer* renderBuffer = this->m_scene.createRenderBuffer(23u, 42u, ERenderBufferType::Depth, ERenderBufferFormat::Depth24, ERenderBufferAccessMode::ReadWrite, 0u, "a renderBuffer");
        RenderTargetDescription rtDesc;
        rtDesc.addRenderBuffer(*renderBuffer);
        RenderTarget* renderTarget = this->m_scene.createRenderTarget(rtDesc, "target");

        OrthographicCamera* orthoCam = this->m_scene.createOrthographicCamera("camera");
        orthoCam->setFrustum(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f);
        orthoCam->setViewport(0, 0, 100, 200);

        renderPass->setCamera(*orthoCam);
        renderPass->setRenderTarget(renderTarget);

        doWriteReadCycle();

        const RenderPass* loadedRenderPass = this->getObjectForTesting<RenderPass>("a renderpass");
        const RenderTarget* loadedRenderTarget = this->getObjectForTesting<RenderTarget>("target");
        const OrthographicCamera* loadedCamera = this->getObjectForTesting<OrthographicCamera>("camera");

        EXPECT_EQ(loadedRenderTarget, loadedRenderPass->getRenderTarget());
        EXPECT_EQ(loadedCamera, loadedRenderPass->getCamera());
        EXPECT_EQ(StatusOK, loadedRenderTarget->validate());
        EXPECT_EQ(StatusOK, loadedCamera->validate());
    }

    TEST_F(ASceneLoadedFromFile, canReadWriteRenderTarget)
    {
        const RenderBuffer& rb = *m_scene.createRenderBuffer(16u, 8u, ERenderBufferType::Color, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        RenderTargetDescription rtDesc;
        rtDesc.addRenderBuffer(rb);

        RenderTarget* renderTarget = this->m_scene.createRenderTarget(rtDesc, "a renderTarget");

        doWriteReadCycle();

        RenderTarget* loadedRenderTarget = this->getObjectForTesting<RenderTarget>("a renderTarget");

        EXPECT_STREQ(renderTarget->getName(), loadedRenderTarget->getName());
        EXPECT_EQ(renderTarget->getSceneObjectId(), loadedRenderTarget->getSceneObjectId());
        EXPECT_EQ(renderTarget->getWidth(), loadedRenderTarget->getWidth());
        EXPECT_EQ(renderTarget->getHeight(), loadedRenderTarget->getHeight());

        EXPECT_EQ(renderTarget->m_impl.getRenderTargetHandle(), loadedRenderTarget->m_impl.getRenderTargetHandle());
    }

    TEST_F(ASceneLoadedFromFile, canReadWriteIndexDataBuffer)
    {
        ArrayBuffer& buffer = *m_scene.createArrayBuffer(EDataType::UInt32, 6u, "indexDB");
        buffer.updateData(3u, 2u, std::array<uint32_t, 2>{ {6, 7} }.data());

        doWriteReadCycle();

        const ArrayBuffer* loadedBuffer = this->getObjectForTesting<ArrayBuffer>("indexDB");

        EXPECT_STREQ(buffer.getName(), loadedBuffer->getName());
        EXPECT_EQ(buffer.m_impl.getDataBufferHandle(), loadedBuffer->m_impl.getDataBufferHandle());
        EXPECT_EQ(6 * sizeof(uint32_t), m_scene.m_impl.getIScene().getDataBuffer(loadedBuffer->m_impl.getDataBufferHandle()).data.size());
        EXPECT_EQ(5 * sizeof(uint32_t), m_scene.m_impl.getIScene().getDataBuffer(loadedBuffer->m_impl.getDataBufferHandle()).usedSize);

        const ramses_internal::Byte* loadedDataBufferData = m_scene.m_impl.getIScene().getDataBuffer(loadedBuffer->m_impl.getDataBufferHandle()).data.data();
        EXPECT_EQ(6u, ramses_internal::UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<uint32_t>(loadedDataBufferData, 3));
        EXPECT_EQ(7u, ramses_internal::UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<uint32_t>(loadedDataBufferData, 4));

        EXPECT_EQ(6u, loadedBuffer->getMaximumNumberOfElements());
        EXPECT_EQ(5u, loadedBuffer->getUsedNumberOfElements());
        EXPECT_EQ(EDataType::UInt32, loadedBuffer->getDataType());
        std::array<uint32_t, 6> bufferDataOut;
        EXPECT_EQ(StatusOK, loadedBuffer->getData(bufferDataOut.data(), static_cast<uint32_t>(bufferDataOut.size() * sizeof(uint32_t))));
        EXPECT_EQ(6u, bufferDataOut[3]);
        EXPECT_EQ(7u, bufferDataOut[4]);
    }

    TEST_F(ASceneLoadedFromFile, canReadWriteTexture2DBuffer)
    {
        Texture2DBuffer& buffer = *m_scene.createTexture2DBuffer(ETextureFormat::RGBA8, 3, 4, 2, "textureBuffer");
        buffer.updateData(0, 0, 0, 2, 2, std::array<uint32_t, 4>{ {12, 23, 34, 56} }.data());
        buffer.updateData(1, 0, 0, 1, 1, std::array<uint32_t, 1>{ {78} }.data());

        doWriteReadCycle();

        const Texture2DBuffer* loadedBuffer = this->getObjectForTesting<Texture2DBuffer>("textureBuffer");

        EXPECT_STREQ(buffer.getName(), loadedBuffer->getName());
        EXPECT_EQ(buffer.getSceneObjectId(), loadedBuffer->getSceneObjectId());
        EXPECT_EQ(buffer.m_impl.getTextureBufferHandle(), loadedBuffer->m_impl.getTextureBufferHandle());

        //iscene
        const ramses_internal::TextureBuffer& loadedInternalBuffer = m_scene.m_impl.getIScene().getTextureBuffer(loadedBuffer->m_impl.getTextureBufferHandle());
        ASSERT_EQ(2u, loadedInternalBuffer.mipMaps.size());
        EXPECT_EQ(3u, loadedInternalBuffer.mipMaps[0].width);
        EXPECT_EQ(4u, loadedInternalBuffer.mipMaps[0].height);
        EXPECT_EQ(1u, loadedInternalBuffer.mipMaps[1].width);
        EXPECT_EQ(2u, loadedInternalBuffer.mipMaps[1].height);
        EXPECT_EQ(56u, ramses_internal::TextureBuffer::GetMipMapDataSizeInBytes(loadedInternalBuffer));
        EXPECT_EQ(ramses_internal::ETextureFormat::RGBA8, loadedInternalBuffer.textureFormat);

        const ramses_internal::Byte* loadedBufferDataMip0 = loadedInternalBuffer.mipMaps[0].data.data();
        const ramses_internal::Byte* loadedBufferDataMip1 = loadedInternalBuffer.mipMaps[1].data.data();
        EXPECT_EQ(12u, ramses_internal::UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<uint32_t>(loadedBufferDataMip0, 0));
        EXPECT_EQ(23u, ramses_internal::UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<uint32_t>(loadedBufferDataMip0, 1));
        EXPECT_EQ(34u, ramses_internal::UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<uint32_t>(loadedBufferDataMip0, 3 * 1 + 0));
        EXPECT_EQ(56u, ramses_internal::UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<uint32_t>(loadedBufferDataMip0, 3 * 1 + 1));
        EXPECT_EQ(78u, ramses_internal::UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<uint32_t>(loadedBufferDataMip1, 0));

        //client API
        EXPECT_EQ(2u, loadedBuffer->getMipLevelCount());
        uint32_t mipLevelWidthOut;
        uint32_t mipLevelHeightOut;
        EXPECT_EQ(StatusOK, loadedBuffer->getMipLevelSize(0u, mipLevelWidthOut, mipLevelHeightOut));
        EXPECT_EQ(3u, mipLevelWidthOut);
        EXPECT_EQ(4u, mipLevelHeightOut);
        EXPECT_EQ(StatusOK, loadedBuffer->getMipLevelSize(1u, mipLevelWidthOut, mipLevelHeightOut));
        EXPECT_EQ(1u, mipLevelWidthOut);
        EXPECT_EQ(2u, mipLevelHeightOut);

        EXPECT_EQ(ETextureFormat::RGBA8, loadedBuffer->getTexelFormat());
        std::array<uint32_t, 12> bufferForMip0;
        std::array<uint32_t, 2> bufferForMip1;
        EXPECT_EQ(StatusOK, loadedBuffer->getMipLevelData(0u, bufferForMip0.data(), static_cast<uint32_t>(bufferForMip0.size() * sizeof(uint32_t))));
        EXPECT_EQ(StatusOK, loadedBuffer->getMipLevelData(1u, bufferForMip1.data(), static_cast<uint32_t>(bufferForMip1.size() * sizeof(uint32_t))));
        EXPECT_EQ(12u, bufferForMip0[0]);
        EXPECT_EQ(23u, bufferForMip0[1]);
        EXPECT_EQ(34u, bufferForMip0[3 * 1 + 0]);
        EXPECT_EQ(56u, bufferForMip0[3 * 1 + 1]);
        EXPECT_EQ(78u, bufferForMip1[0]);
    }

    TEST_F(ASceneLoadedFromFile, canReadWriteANode)
    {
        //generic node cannot be created, therefore using group node
        Node* grandParent = this->m_scene.createNode("node1");
        Node* parent = this->m_scene.createNode("node2");
        Node* child = this->m_scene.createNode("node3");

        grandParent->addChild(*parent);
        child->setParent(*parent);

        child->setTranslation({1, 2, 3});
        child->setVisibility(EVisibilityMode::Invisible);
        child->setRotation({1, 2, 3}, ERotationType::Euler_XYX);
        child->setScaling({1, 2, 3});

        doWriteReadCycle();

        Node* loadedGrandParent = this->getObjectForTesting<Node>("node1");
        Node* loadedParent = this->getObjectForTesting<Node>("node2");
        Node* loadedChild = this->getObjectForTesting<Node>("node3");
        ASSERT_TRUE(nullptr != loadedGrandParent);
        ASSERT_TRUE(nullptr != loadedParent);
        ASSERT_TRUE(nullptr != loadedChild);
        EXPECT_EQ(loadedParent, loadedChild->getParent());
        EXPECT_EQ(loadedGrandParent, loadedParent->getParent());
        EXPECT_EQ(loadedParent, loadedGrandParent->getChild(0u));
        EXPECT_EQ(1u, loadedGrandParent->getChildCount());
        EXPECT_EQ(1u, loadedParent->getChildCount());
        EXPECT_EQ(0u, loadedChild->getChildCount());

        vec3f translation;
        EXPECT_EQ(StatusOK, loadedChild->getTranslation(translation));
        EXPECT_FLOAT_EQ(1, translation.x);
        EXPECT_FLOAT_EQ(2, translation.y);
        EXPECT_FLOAT_EQ(3, translation.z);

        EXPECT_EQ(loadedChild->getVisibility(), EVisibilityMode::Invisible);

        vec3f rotation;
        EXPECT_EQ(StatusOK, loadedChild->getRotation(rotation));
        EXPECT_FLOAT_EQ(1, rotation.x);
        EXPECT_FLOAT_EQ(2, rotation.y);
        EXPECT_FLOAT_EQ(3, rotation.z);
        EXPECT_EQ(ERotationType::Euler_XYX, loadedChild->getRotationType());

        vec3f scale;
        EXPECT_EQ(StatusOK, loadedChild->getScaling(scale));
        EXPECT_FLOAT_EQ(1, scale.x);
        EXPECT_FLOAT_EQ(2, scale.y);
        EXPECT_FLOAT_EQ(3, scale.z);
    }

    TEST_F(ASceneLoadedFromFile, canReadWriteANodeWithTranslation)
    {
        Node* node = this->m_scene.createNode("translate node 1");
        Node* child = this->m_scene.createNode("groupnode child");

        node->setTranslation({1, 2, 3});
        node->addChild(*child);

        doWriteReadCycle();

        Node* loadedTranslateNode = getObjectForTesting<Node>("translate node 1");
        Node* loadedChild = getObjectForTesting<Node>("groupnode child");

        ASSERT_TRUE(nullptr != loadedTranslateNode);
        ASSERT_TRUE(nullptr != loadedChild);

        EXPECT_EQ(1u, node->getChildCount());
        EXPECT_EQ(loadedChild, loadedTranslateNode->getChild(0u));
        vec3f translation;
        EXPECT_EQ(StatusOK, node->getTranslation(translation));
        EXPECT_FLOAT_EQ(1, translation.x);
        EXPECT_FLOAT_EQ(2, translation.y);
        EXPECT_FLOAT_EQ(3, translation.z);
    }

    TEST_F(ASceneLoadedFromFile, canReadWriteANodeWithRotation)
    {
        Node* node = this->m_scene.createNode("rotate node 1");
        Node* child = this->m_scene.createNode("groupnode child");

        node->setRotation({1, 2, 3}, ERotationType::Euler_ZYX);
        child->setParent(*node);
        doWriteReadCycle();

        Node* loadedRotateNode = getObjectForTesting<Node>("rotate node 1");
        Node* loadedChild = getObjectForTesting<Node>("groupnode child");

        ASSERT_TRUE(nullptr != loadedRotateNode);
        ASSERT_TRUE(nullptr != loadedChild);

        EXPECT_EQ(1u, loadedRotateNode->getChildCount());
        EXPECT_EQ(loadedChild, loadedRotateNode->getChild(0u));
        vec3f rotation;
        EXPECT_EQ(StatusOK, loadedRotateNode->getRotation(rotation));
        EXPECT_FLOAT_EQ(1, rotation.x);
        EXPECT_FLOAT_EQ(2, rotation.y);
        EXPECT_FLOAT_EQ(3, rotation.z);
        EXPECT_EQ(ERotationType::Euler_ZYX, loadedRotateNode->getRotationType());
    }

    TEST_F(ASceneLoadedFromFile, canReadWriteANodeWithScaling)
    {
        Node* node = this->m_scene.createNode("scale node");
        Node* child = this->m_scene.createNode("groupnode child");

        node->setScaling({1, 2, 3});
        child->setParent(*node);
        doWriteReadCycle();

        Node* loadedScaleNode = getObjectForTesting<Node>("scale node");
        Node* loadedChild = getObjectForTesting<Node>("groupnode child");

        ASSERT_TRUE(nullptr != loadedScaleNode);
        ASSERT_TRUE(nullptr != loadedChild);

        EXPECT_EQ(1u, loadedScaleNode->getChildCount());
        EXPECT_EQ(loadedChild, loadedScaleNode->getChild(0u));
        vec3f scale;
        EXPECT_EQ(StatusOK, loadedScaleNode->getScaling(scale));
        EXPECT_FLOAT_EQ(1, scale.x);
        EXPECT_FLOAT_EQ(2, scale.y);
        EXPECT_FLOAT_EQ(3, scale.z);
    }

    TEST_F(ASceneLoadedFromFile, canReadWriteATextureSampler)
    {
        const ETextureAddressMode wrapUMode = ETextureAddressMode::Mirror;
        const ETextureAddressMode wrapVMode = ETextureAddressMode::Repeat;
        const ETextureSamplingMethod minSamplingMethod = ETextureSamplingMethod::Linear_MipMapNearest;
        const ETextureSamplingMethod magSamplingMethod = ETextureSamplingMethod::Linear;
        const uint8_t data[4] = { 0u };
        const MipLevelData mipLevelData(sizeof(data), data);
        Texture2D* texture = this->m_scene.createTexture2D(ETextureFormat::RGBA8, 1u, 1u, 1, &mipLevelData, false, {}, ramses::ResourceCacheFlag_DoNotCache, "texture");

        TextureSampler* sampler = this->m_scene.createTextureSampler(wrapUMode, wrapVMode, minSamplingMethod, magSamplingMethod, *texture, 8u, "sampler");
        ASSERT_TRUE(nullptr != sampler);

        doWriteReadCycle();

        TextureSampler* loadedSampler = getObjectForTesting<TextureSampler>("sampler");
        ASSERT_TRUE(nullptr != loadedSampler);

        EXPECT_EQ(wrapUMode, loadedSampler->getWrapUMode());
        EXPECT_EQ(wrapVMode, loadedSampler->getWrapVMode());
        EXPECT_EQ(minSamplingMethod, loadedSampler->getMinSamplingMethod());
        EXPECT_EQ(magSamplingMethod, loadedSampler->getMagSamplingMethod());
        EXPECT_EQ(8u, loadedSampler->getAnisotropyLevel());
        EXPECT_EQ(texture->m_impl.getLowlevelResourceHash(), this->m_sceneLoaded->m_impl.getIScene().getTextureSampler(loadedSampler->m_impl.getTextureSamplerHandle()).textureResource);
        EXPECT_EQ(ERamsesObjectType::Texture2D, loadedSampler->m_impl.getTextureType());
    }

    TEST_F(ASceneLoadedFromFile, canReadWriteATextureSamplerMS)
    {
        RenderBuffer* renderBuffer = m_scene.createRenderBuffer(4u, 4u, ERenderBufferType::Color, ERenderBufferFormat::RGB8, ERenderBufferAccessMode::ReadWrite, 4u);

        TextureSamplerMS* sampler = this->m_scene.createTextureSamplerMS(*renderBuffer, "sampler");
        ASSERT_TRUE(nullptr != sampler);

        doWriteReadCycle();

        TextureSamplerMS* loadedSampler = getObjectForTesting<TextureSamplerMS>("sampler");
        ASSERT_TRUE(nullptr != loadedSampler);

        EXPECT_EQ(ERamsesObjectType::RenderBuffer, loadedSampler->m_impl.getTextureType());
    }

    TEST_F(ASceneLoadedFromFile, canReadWriteATextureSamplerExternal)
    {
        TextureSamplerExternal* sampler = this->m_scene.createTextureSamplerExternal(ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, "sampler");
        ASSERT_TRUE(nullptr != sampler);

        doWriteReadCycle();

        TextureSamplerExternal* loadedSampler = getObjectForTesting<TextureSamplerExternal>("sampler");
        ASSERT_TRUE(nullptr != loadedSampler);

        EXPECT_EQ(ERamsesObjectType::TextureSamplerExternal, loadedSampler->m_impl.getTextureType());
    }

    TEST_F(ASceneLoadedFromFile, canReadWriteSceneId)
    {
        const sceneId_t sceneId = ramses::sceneId_t(1ULL << 63);
        ramses::Scene& mScene(*client.createScene(sceneId));

        const status_t status = mScene.saveToFile("someTempararyFile.ram", false);
        EXPECT_EQ(StatusOK, status);

        m_sceneLoaded = m_clientForLoading.loadSceneFromFile("someTempararyFile.ram");
        ASSERT_TRUE(nullptr != m_sceneLoaded);

        EXPECT_EQ(sceneId, m_sceneLoaded->getSceneId());
    }

    TEST_F(ASceneLoadedFromFile, defaultsToLocalAndRemotePublicationMode)
    {
        const sceneId_t sceneId(81);
        EXPECT_EQ(StatusOK, client.createScene(sceneId)->saveToFile("someTempararyFile.ram", false));

        m_sceneLoaded = m_clientForLoading.loadSceneFromFile("someTempararyFile.ram");
        ASSERT_TRUE(nullptr != m_sceneLoaded);
        EXPECT_EQ(EScenePublicationMode::LocalAndRemote, m_sceneLoaded->m_impl.getPublicationModeSetFromSceneConfig());
    }

    // TODO(tobias) add to store this option to file format as soon as changes are allowed
    TEST_F(ASceneLoadedFromFile, DISABLED_respectsPublicationModeSetOnCreatingFileBeforeSave)
    {
        const sceneId_t sceneId(81);
        SceneConfig config;
        config.setPublicationMode(ramses::EScenePublicationMode::LocalOnly);
        EXPECT_EQ(StatusOK, client.createScene(sceneId, config)->saveToFile("someTempararyFile.ram", false));

        m_sceneLoaded = m_clientForLoading.loadSceneFromFile("someTempararyFile.ram");
        ASSERT_TRUE(nullptr != m_sceneLoaded);
        EXPECT_EQ(EScenePublicationMode::LocalOnly, m_sceneLoaded->m_impl.getPublicationModeSetFromSceneConfig());
    }

    TEST_F(ASceneLoadedFromFile, canOverwritePublicationModeForLoadedFiles)
    {
        const sceneId_t sceneId(80);
        EXPECT_EQ(StatusOK, client.createScene(sceneId)->saveToFile("someTempararyFile.ram", false));

        m_sceneLoaded = m_clientForLoading.loadSceneFromFile("someTempararyFile.ram", true);
        ASSERT_TRUE(nullptr != m_sceneLoaded);
        EXPECT_EQ(EScenePublicationMode::LocalOnly, m_sceneLoaded->m_impl.getPublicationModeSetFromSceneConfig());
    }

    TEST_F(ASceneLoadedFromFile, reportsErrorWhenSavingSceneToFileWithInvalidFileName)
    {
        ramses::status_t status = m_scene.saveToFile("?Euler_ZYX:/dummyFile", false);
        EXPECT_NE(ramses::StatusOK, status);
    }

    TEST_F(ASceneLoadedFromFile, reportsErrorWhenSavingSceneToFileWithNoFileName)
    {
        ramses::status_t status = m_scene.saveToFile({}, false);
        EXPECT_NE(ramses::StatusOK, status);
    }

    TEST_F(ASceneLoadedFromFile, overwritesExistingFileWhenSavingSceneToIt)
    {
        {
            ramses_internal::File existingFile("dummyFile.dat");
            existingFile.createFile();
        }

        ramses::status_t status = m_scene.saveToFile("dummyFile.dat", false);
        EXPECT_EQ(ramses::StatusOK, status);

        {
            ramses_internal::File fileShouldBeOverwritten("dummyFile.dat");
            EXPECT_TRUE(fileShouldBeOverwritten.open(ramses_internal::File::Mode::ReadOnly));
            size_t fileSize = 0;
            EXPECT_TRUE(fileShouldBeOverwritten.getSizeInBytes(fileSize));
            EXPECT_NE(0u, fileSize);
        }

        EXPECT_TRUE(ramses_internal::File("dummyFile.dat").remove());
    }

    TEST_F(ASceneLoadedFromFile, doesNotLoadSceneFromFileWithInvalidFileName)
    {
        ramses::Scene* scene = client.loadSceneFromFile("?Euler_ZYX:/dummyFile");
        EXPECT_TRUE(nullptr == scene);
    }

    TEST_F(ASceneLoadedFromFile, doesNotLoadSceneFromFileWithoutFileName)
    {
        EXPECT_EQ(nullptr, client.loadSceneFromFile({}));
        EXPECT_EQ(nullptr, client.loadSceneFromFile(""));
    }

    TEST_F(ASceneLoadedFromFile, doesNotLoadSceneFromInvalidMemory)
    {
        auto deleter = [](const unsigned char* ptr) { delete[] ptr; };
        EXPECT_EQ(nullptr, client.loadSceneFromMemory(std::unique_ptr<unsigned char[], void(*)(const unsigned char*)>(nullptr, deleter), 1, false));
        EXPECT_EQ(nullptr, client.loadSceneFromMemory(std::unique_ptr<unsigned char[], void(*)(const unsigned char*)>(new unsigned char[1], deleter), 0, false));

        EXPECT_EQ(nullptr, RamsesUtils::LoadSceneFromMemory(client, std::unique_ptr<unsigned char[]>(nullptr), 1, false));
        EXPECT_EQ(nullptr, RamsesUtils::LoadSceneFromMemory(client, std::unique_ptr<unsigned char[]>(new unsigned char[1]), 0, false));
    }

    TEST_F(ASceneLoadedFromFile, doesNotLoadSceneFromInvalidFileDescriptor)
    {
        EXPECT_EQ(nullptr, client.loadSceneFromFileDescriptor(-1, 0, 1, false));
        EXPECT_EQ(nullptr, client.loadSceneFromFileDescriptor(1, 0, 0, false));
    }

    TEST_F(ASceneLoadedFromFile, doesNotLoadSceneFromUnexistingFile)
    {
        ramses::Scene* scene = client.loadSceneFromFile("ZEGETWTWAGTGSDGEg_thisfilename_in_this_directory_should_not_exist_DSAFDSFSTEZHDXHB");
        EXPECT_TRUE(nullptr == scene);
    }

    TEST_F(ASceneLoadedFromFile, canHandleAllZeroFileOnSceneLoad)
    {
        const char* filename = "allzerofile.dat";
        {
            ramses_internal::File file(filename);
            EXPECT_TRUE(file.open(ramses_internal::File::Mode::WriteNew));
            std::vector<char> zerovector(4096);
            EXPECT_TRUE(file.write(&zerovector[0], zerovector.size()));
            file.close();
        }

        ramses::Scene* scene = client.loadSceneFromFile(filename);
        EXPECT_TRUE(scene == nullptr);
    }

    TEST_F(ASceneLoadedFromFile, cannotLoadSameFileTwice)
    {
        const sceneId_t sceneId = ramses::sceneId_t(1ULL << 63);
        ramses::Scene* scene = client.createScene(sceneId);
        const status_t status = scene->saveToFile("someTempararyFile.ram", false);
        EXPECT_EQ(StatusOK, status);

        m_sceneLoaded = m_clientForLoading.loadSceneFromFile("someTempararyFile.ram");
        EXPECT_NE(nullptr, m_sceneLoaded);

        EXPECT_EQ(nullptr, m_clientForLoading.loadSceneFromFile("someTempararyFile.ram"));
    }

    TEST_F(ASceneLoadedFromFile, cannotLoadScenesWithSameSceneIdTwice)
    {
        const sceneId_t sceneId = ramses::sceneId_t(1ULL << 63);

        {
            ramses::Scene* scene = client.createScene(sceneId);
            const status_t status = scene->saveToFile("someTempararyFile.ram", false);
            EXPECT_EQ(StatusOK, status);
            client.destroy(*scene);
        }
        {
            ramses::Scene* scene = client.createScene(sceneId);
            const status_t status = scene->saveToFile("someTempararyFile_2.ram", false);
            EXPECT_EQ(StatusOK, status);
            client.destroy(*scene);
        }

        m_sceneLoaded = m_clientForLoading.loadSceneFromFile("someTempararyFile.ram");
        EXPECT_NE(nullptr, m_sceneLoaded);

        EXPECT_EQ(nullptr, m_clientForLoading.loadSceneFromFile("someTempararyFile_2.ram"));
    }

    TEST_F(ASceneLoadedFromFile, cannotLoadSceneWithMismatchingFeatureLevel)
    {
        saveSceneWithFeatureLevelToFile(EFeatureLevel(99), "someTemporaryFile.ram");
        EXPECT_EQ(nullptr, m_clientForLoading.loadSceneFromFile("someTemporaryFile.ram"));
    }

    TEST_F(ASceneLoadedFromFile, canGetFeatureLevelFromSceneFile)
    {
        saveSceneWithFeatureLevelToFile(EFeatureLevel_Latest, "someTemporaryFile.ram");

        EFeatureLevel featureLevel = EFeatureLevel_01;
        EXPECT_TRUE(RamsesClient::GetFeatureLevelFromFile("someTemporaryFile.ram", featureLevel));
        EXPECT_EQ(EFeatureLevel_Latest, featureLevel);
    }

    TEST_F(ASceneLoadedFromFile, failsToGetFeatureLevelFromFileWithUnknownFeatureLevel)
    {
        saveSceneWithFeatureLevelToFile(EFeatureLevel(99), "someTemporaryFile.ram");

        EFeatureLevel featureLevel = EFeatureLevel_01;
        EXPECT_FALSE(RamsesClient::GetFeatureLevelFromFile("someTemporaryFile.ram", featureLevel));
    }

    TEST_F(ASceneLoadedFromFile, failsToGetFeatureLevelFromNonexistingFile)
    {
        EFeatureLevel featureLevel = EFeatureLevel_01;
        EXPECT_FALSE(RamsesClient::GetFeatureLevelFromFile("doesnt.Exist", featureLevel));
    }

    TEST_F(ASceneLoadedFromFile, canGetFeatureLevelFromSceneFileViaFileDescriptor)
    {
        saveSceneWithFeatureLevelToFile(EFeatureLevel_Latest, "someTemporaryFile.ram");

        size_t fileSize = 0;
        {
            // write to a file with some offset
            ramses_internal::File inFile("someTemporaryFile.ram");
            EXPECT_TRUE(inFile.getSizeInBytes(fileSize));
            std::vector<unsigned char> data(fileSize);
            size_t numBytesRead = 0;
            EXPECT_TRUE(inFile.open(ramses_internal::File::Mode::ReadOnlyBinary));
            EXPECT_EQ(ramses_internal::EStatus::Ok, inFile.read(data.data(), fileSize, numBytesRead));

            ramses_internal::File outFile("someTemporaryFileWithOffset.ram");
            EXPECT_TRUE(outFile.open(ramses_internal::File::Mode::WriteOverWriteOldBinary));

            uint32_t zeroData = 0;
            EXPECT_TRUE(outFile.write(&zeroData, sizeof(zeroData)));
            EXPECT_TRUE(outFile.write(data.data(), data.size()));
            EXPECT_TRUE(outFile.write(&zeroData, sizeof(zeroData)));
        }
        const int fd = ramses_internal::FileDescriptorHelper::OpenFileDescriptorBinary("someTemporaryFileWithOffset.ram");

        EFeatureLevel featureLevel = EFeatureLevel_01;
        EXPECT_TRUE(RamsesClient::GetFeatureLevelFromFile(fd, 4u, fileSize, featureLevel));
        EXPECT_EQ(EFeatureLevel_Latest, featureLevel);
    }

    TEST_F(ASceneLoadedFromFile, failsToGetFeatureLevelFromInvalidFileDescriptor)
    {
        EFeatureLevel featureLevel = EFeatureLevel_01;
        EXPECT_FALSE(RamsesClient::GetFeatureLevelFromFile(-1, 0u, 10u, featureLevel));
        EXPECT_FALSE(RamsesClient::GetFeatureLevelFromFile(1, 0u, 0u, featureLevel));
    }

    TEST_F(ASceneLoadedFromFile, canReadWriteTransformDataSlot)
    {
        Node* node = this->m_scene.createNode("node");

        EXPECT_EQ(StatusOK, this->m_scene.m_impl.createTransformationDataConsumer(*node, dataConsumerId_t(2u)));
        ASSERT_EQ(1u, this->m_scene.m_impl.getIScene().getDataSlotCount());

        ramses_internal::DataSlotHandle slotHandle(0u);
        EXPECT_TRUE(this->m_scene.m_impl.getIScene().isDataSlotAllocated(slotHandle));

        doWriteReadCycle();

        const Node* nodeLoaded = this->getObjectForTesting<Node>("node");
        ramses_internal::NodeHandle nodeHandle = nodeLoaded->m_impl.getNodeHandle();
        ASSERT_EQ(1u, this->m_sceneLoaded->m_impl.getIScene().getDataSlotCount());

        EXPECT_TRUE(this->m_sceneLoaded->m_impl.getIScene().isDataSlotAllocated(slotHandle));
        EXPECT_EQ(nodeHandle, this->m_sceneLoaded->m_impl.getIScene().getDataSlot(slotHandle).attachedNode);
        EXPECT_EQ(ramses_internal::DataSlotId(2u), this->m_sceneLoaded->m_impl.getIScene().getDataSlot(slotHandle).id);
        EXPECT_EQ(ramses_internal::EDataSlotType_TransformationConsumer, this->m_sceneLoaded->m_impl.getIScene().getDataSlot(slotHandle).type);
    }

    TEST_F(ASceneLoadedFromFile, canReadWriteDataObject)
    {
        float setValue = 5.0f;
        auto data = this->m_scene.createDataObject(EDataType::Float, "floatData");

        EXPECT_EQ(StatusOK, data->setValue(setValue));

        doWriteReadCycle();

        const auto loadedData = this->getObjectForTesting<DataObject>("floatData");
        ASSERT_TRUE(loadedData);
        float loadedValue = 0.0f;
        EXPECT_EQ(EDataType::Float, loadedData->getDataType());
        EXPECT_EQ(StatusOK, loadedData->getValue(loadedValue));
        EXPECT_EQ(setValue, loadedValue);
    }

    TEST_F(ASceneLoadedFromFile, canReadWriteSceneReferences)
    {
        constexpr ramses::sceneId_t referencedSceneId(444);
        auto sr1 = this->m_scene.createSceneReference(referencedSceneId, "scene ref");
        sr1->requestState(RendererSceneState::Ready);

        constexpr ramses::sceneId_t referencedSceneId2(555);
        auto sr2 = this->m_scene.createSceneReference(referencedSceneId2, "scene ref2");
        sr2->requestState(RendererSceneState::Rendered);

        doWriteReadCycle();

        const SceneReference* loadedSceneRef = this->getObjectForTesting<SceneReference>("scene ref");
        ASSERT_TRUE(loadedSceneRef);
        EXPECT_EQ(referencedSceneId, loadedSceneRef->getReferencedSceneId());
        EXPECT_EQ(ramses::RendererSceneState::Ready, loadedSceneRef->getRequestedState());

        const SceneReference* loadedSceneRef2 = this->getObjectForTesting<SceneReference>("scene ref2");
        ASSERT_TRUE(loadedSceneRef2);
        EXPECT_EQ(referencedSceneId2, loadedSceneRef2->getReferencedSceneId());
        EXPECT_EQ(ramses::RendererSceneState::Rendered, loadedSceneRef2->getRequestedState());
    }

    TEST_F(ASceneLoadedFromFile, savesLLResourceOnlyOnceIfTwoHLResourcesReferToIt)
    {
        std::vector<uint16_t> inds(300);
        std::iota(inds.begin(), inds.end(), static_cast<uint16_t>(0u));
        this->m_scene.createArrayResource(300u, inds.data(), ramses::ResourceCacheFlag_DoNotCache, "indices");
        this->m_scene.createArrayResource(300u, inds.data(), ramses::ResourceCacheFlag_DoNotCache, "indices");
        this->m_scene.createArrayResource(300u, inds.data(), ramses::ResourceCacheFlag_DoNotCache, "indices");
        this->m_scene.createArrayResource(300u, inds.data(), ramses::ResourceCacheFlag_DoNotCache, "indices2");
        this->m_scene.createArrayResource(300u, inds.data(), ramses::ResourceCacheFlag_DoNotCache, "indices2");
        this->m_scene.createArrayResource(300u, inds.data(), ramses::ResourceCacheFlag_DoNotCache, "indices2");

        doWriteReadCycle();
        std::ifstream in("someTemporaryFile.ram", std::ifstream::ate | std::ifstream::binary);
        auto size = in.tellg();
        EXPECT_GT(1600, size) << "scene file size exceeds allowed max size. verify that LL resource is saved only once before adapting this number";
    }

    template <typename T>
    struct TestHelper
    {
        static T* create(ASceneLoadedFromFileTemplated<T>* fixture, ramses::RamsesClient&, ramses::Scene&)
        {
            return &fixture->template createObject<T>("a node");
        }
    };

    TYPED_TEST_SUITE(ASceneLoadedFromFileTemplated, NodeTypes);
    TYPED_TEST(ASceneLoadedFromFileTemplated, canReadWriteAllNodes)
    {
        auto node = TestHelper<TypeParam>::create(this, this->client, this->m_scene);

        node->setVisibility(EVisibilityMode::Invisible);

        auto child = &this->template createObject<ramses::Node>("child");
        auto parent = &this->template createObject<ramses::Node>("parent");

        node->setTranslation({1, 2, 3});
        node->setRotation({4, 5, 6}, ERotationType::Euler_XZX);
        node->setScaling({7, 8, 9});
        node->addChild(*child);
        node->setParent(*parent);

        this->m_scene.flush();

        this->doWriteReadCycle();

        const auto loadedSuperNode = this->template getObjectForTesting<TypeParam>("a node");
        const auto loadedChild = this->template getObjectForTesting<ramses::Node>("child");
        const auto loadedParent = this->template getObjectForTesting<ramses::Node>("parent");

        ASSERT_TRUE(nullptr != loadedSuperNode);
        ASSERT_TRUE(nullptr != loadedChild);
        ASSERT_TRUE(nullptr != loadedParent);

        ASSERT_EQ(1u, loadedSuperNode->getChildCount());
        EXPECT_EQ(loadedChild, loadedSuperNode->getChild(0u));
        EXPECT_EQ(loadedParent, loadedSuperNode->getParent());
        vec3f value;
        EXPECT_EQ(StatusOK, loadedSuperNode->getTranslation(value));
        EXPECT_FLOAT_EQ(1, value.x);
        EXPECT_FLOAT_EQ(2, value.y);
        EXPECT_FLOAT_EQ(3, value.z);
        EXPECT_EQ(StatusOK, loadedSuperNode->getRotation(value));
        EXPECT_FLOAT_EQ(4, value.x);
        EXPECT_FLOAT_EQ(5, value.y);
        EXPECT_FLOAT_EQ(6, value.z);
        EXPECT_EQ(ERotationType::Euler_XZX, loadedSuperNode->getRotationType());
        EXPECT_EQ(StatusOK, loadedSuperNode->getScaling(value));
        EXPECT_FLOAT_EQ(7, value.x);
        EXPECT_FLOAT_EQ(8, value.y);
        EXPECT_FLOAT_EQ(9, value.z);

        EXPECT_EQ(loadedSuperNode->getVisibility(), EVisibilityMode::Invisible);
    }

    TEST_F(ASceneLoadedFromFile, compressedFileIsSmallerThanUncompressedWhenUsingSaveSceneToFile)
    {
        Scene* scene = client.createScene(sceneId_t(1));
        const std::vector<uint16_t> data(1000u, 0u);
        EXPECT_TRUE(scene->createArrayResource(static_cast<uint32_t>(data.size()), data.data()));

        EXPECT_EQ(StatusOK, scene->saveToFile("testscene.ramscene", false));

        ramses_internal::File file("testscene.ramscene");
        EXPECT_TRUE(file.exists());
        size_t uncompressedFileSize = 0;
        EXPECT_TRUE(file.getSizeInBytes(uncompressedFileSize));

        EXPECT_EQ(StatusOK, scene->saveToFile("testscene.ramscene", true));

        ramses_internal::File file2("testscene.ramscene");
        EXPECT_TRUE(file2.exists());
        size_t compressedFileSize = 0;
        EXPECT_TRUE(file2.getSizeInBytes(compressedFileSize));

        EXPECT_GT(uncompressedFileSize, compressedFileSize);
    }

    TEST_F(ASceneLoadedFromFile, savedFilesAreConsistent)
    {
        for (const auto& name : { "ts1.ramscene", "ts2.ramscene", "ts3.ramscene", "ts4.ramscene", "ts5.ramscene", "ts6.ramscene" })
        {
            EXPECT_EQ(StatusOK, this->m_scene.saveToFile(name, false));
        }

        for (const auto& name : { "ts2.ramscene", "ts3.ramscene", "ts4.ramscene", "ts5.ramscene", "ts6.ramscene" })
        {
            EXPECT_TRUE(ClientTestUtils::CompareBinaryFiles("ts1.ramscene", name));
        }
    }

    TEST_F(ASceneLoadedFromFile, closesSceneFileAndLowLevelResourceWhenDestroyed)
    {
        const status_t status = m_scene.saveToFile("someTemporaryFile.ram", false);
        EXPECT_EQ(StatusOK, status);

        m_sceneLoaded = m_clientForLoading.loadSceneFromFile("someTemporaryFile.ram", {});
        ASSERT_TRUE(nullptr != m_sceneLoaded);

        const ramses_internal::SceneFileHandle handle = m_sceneLoaded->m_impl.getSceneFileHandle();
        EXPECT_TRUE(m_clientForLoading.m_impl.getClientApplication().hasResourceFile(handle));
        m_clientForLoading.destroy(*m_sceneLoaded);

        // scene gets destroyed asynchronously, so we can't just test after the destroy
        // unfortunately there is no callback, but I don't want to skip the test
        // => wait for it to happen in finite time, we don't test for performance here
        uint32_t ticks = 60000u;
        for (; ticks > 0; --ticks)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            if (!m_clientForLoading.m_impl.getClientApplication().hasResourceFile(handle))
                break;
        }
        EXPECT_GT(ticks, 0u);
    }
}
