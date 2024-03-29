//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/BlitPass.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/TextureSampler.h"
#include "ramses-client-api/TextureSamplerMS.h"
#include "ramses-client-api/RenderBuffer.h"
#include "ramses-client-api/RenderTarget.h"
#include "ramses-client-api/ArrayResource.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/Texture3D.h"
#include "ramses-client-api/TextureCube.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/DataObject.h"
#include "ramses-client-api/ArrayBuffer.h"
#include "ramses-client-api/Texture2DBuffer.h"
#include "ramses-client-api/PickableObject.h"

#include "RamsesClientImpl.h"
#include "SceneImpl.h"
#include "ClientTestUtils.h"
#include "RamsesObjectTestTypes.h"
#include "NodeImpl.h"
#include "MeshNodeImpl.h"
#include "CameraNodeImpl.h"
#include "GeometryBindingImpl.h"
#include "RenderGroupImpl.h"
#include "RenderPassImpl.h"
#include "BlitPassImpl.h"
#include "RenderBufferImpl.h"
#include "RenderTargetImpl.h"
#include "TextureSamplerImpl.h"
#include "DataObjectImpl.h"
#include "ArrayBufferImpl.h"
#include "Texture2DBufferImpl.h"
#include "PickableObjectImpl.h"
#include "SceneReferenceImpl.h"
#include "Texture2DImpl.h"
#include "Texture3DImpl.h"
#include "TextureCubeImpl.h"
#include "EffectImpl.h"

#include "RamsesObjectTypeTraits.h"
#include "RamsesObjectTypeUtils.h"

namespace ramses
{
    using namespace testing;

    template <typename ObjectType>
    class SceneOwnershipTest : public LocalTestClientWithScene, public testing::Test
    {
    public:
        SceneOwnershipTest() : LocalTestClientWithScene()
        {
        }

        void expectNoFrameworkObjectsAllocated()
        {
            m_creationHelper.destroyAdditionalAllocatedSceneObjects();
            const ramses_internal::IScene& scene = this->m_scene.m_impl.getIScene();

            for (ramses_internal::NodeHandle i(0); i < scene.getNodeCount(); ++i)
            {
                EXPECT_FALSE(scene.isNodeAllocated(i));
            }

            for (ramses_internal::CameraHandle i(0); i < scene.getCameraCount(); ++i)
            {
                EXPECT_FALSE(scene.isCameraAllocated(i));
            }

            for (ramses_internal::TransformHandle i(0); i < scene.getTransformCount(); ++i)
            {
                EXPECT_FALSE(scene.isTransformAllocated(i));
            }

            for (ramses_internal::RenderableHandle i(0); i < scene.getRenderableCount(); ++i)
            {
                EXPECT_FALSE(scene.isRenderableAllocated(i));
            }

            for (ramses_internal::RenderStateHandle i(0); i < scene.getRenderStateCount(); ++i)
            {
                EXPECT_FALSE(scene.isRenderStateAllocated(i));
            }

            for (ramses_internal::DataLayoutHandle i(0); i < scene.getDataLayoutCount(); ++i)
            {
                EXPECT_FALSE(scene.isDataLayoutAllocated(i));
            }

            for (ramses_internal::DataInstanceHandle i(0); i < scene.getDataInstanceCount(); ++i)
            {
                EXPECT_FALSE(scene.isDataInstanceAllocated(i));
            }

            for (ramses_internal::RenderPassHandle i(0); i < scene.getRenderPassCount(); ++i)
            {
                EXPECT_FALSE(scene.isRenderPassAllocated(i));
            }

            for (ramses_internal::RenderTargetHandle i(0); i < scene.getRenderTargetCount(); ++i)
            {
                EXPECT_FALSE(scene.isRenderTargetAllocated(i));
            }

            for (ramses_internal::TextureSamplerHandle i(0); i < scene.getTextureSamplerCount(); ++i)
            {
                EXPECT_FALSE(scene.isTextureSamplerAllocated(i));
            }
        }
    };

    template <typename ResourceType>
    class ClientOwnershipTest : public LocalTestClientWithScene, public testing::Test{};

    TYPED_TEST_SUITE(SceneOwnershipTest, SceneObjectTypes);
    TYPED_TEST_SUITE(ClientOwnershipTest, ClientObjectTypes);

    TYPED_TEST(SceneOwnershipTest, sceneObjectsAreOfTypeSceneObject)
    {
        auto obj = &this->template createObject<TypeParam>("objectName");
        ASSERT_TRUE(nullptr != obj);
        EXPECT_TRUE(obj->isOfType(ERamsesObjectType::SceneObject));
        EXPECT_TRUE(obj->isOfType(ERamsesObjectType::ClientObject));
    }

    TYPED_TEST(SceneOwnershipTest, sceneObjectsHaveReferenceToTheirScene)
    {
        auto obj = &this->template createObject<TypeParam>("objectName");
        ASSERT_TRUE(nullptr != obj);
        EXPECT_EQ(&this->m_scene.m_impl, &obj->m_impl.getSceneImpl());
        EXPECT_EQ(&this->m_scene.m_impl.getIScene(), &obj->m_impl.getIScene());
        EXPECT_EQ(&this->client.m_impl, &obj->m_impl.getClientImpl());
    }

    TYPED_TEST(SceneOwnershipTest, sceneContainsCreatedObject)
    {
        const RamsesObject* obj = &this->template createObject<TypeParam>("objectName");
        ASSERT_TRUE(nullptr != obj);
        const RamsesObject* sn = this->m_scene.findObjectByName("objectName");
        ASSERT_TRUE(nullptr != sn);
        EXPECT_EQ(obj, sn);
        EXPECT_EQ(obj, this->m_constRefToScene.findObjectByName("objectName"));
    }

    // TODO Violin improve this test
    // The test is very ugly, because it checks that "a scene does not contain ANY LL object after
    // a specific HL object was destroyed. Instead, it should be testing that a scene does not
    // contain the LL components of the HL object which is being deleted
    TYPED_TEST(SceneOwnershipTest, sceneDoesNotContainDestroyedObject)
    {
        auto obj = &this->template createObject<TypeParam>("objectName");
        ASSERT_TRUE(nullptr != obj);
        EXPECT_EQ(StatusOK, this->m_scene.destroy(*obj));
        const RamsesObject* sn = this->m_scene.findObjectByName("objectName");
        ASSERT_TRUE(nullptr == sn);

        this->expectNoFrameworkObjectsAllocated();
    }

    TYPED_TEST(SceneOwnershipTest, creatingAndDestroyingObjectsUpdatesStatisticCounter)
    {
        this->m_scene.m_impl.getStatisticCollection().nextTimeInterval(); //object number is updated by nextTimeInterval()
        uint32_t initialNumber = this->m_scene.m_impl.getStatisticCollection().statObjectsCount.getCounterValue();
        EXPECT_EQ(0u, this->m_scene.m_impl.getStatisticCollection().statObjectsCreated.getCounterValue());
        EXPECT_EQ(0u, this->m_scene.m_impl.getStatisticCollection().statObjectsDestroyed.getCounterValue());

        auto obj = &this->template createObject<TypeParam>("objectName");
        uint32_t numberCreated = this->m_scene.m_impl.getStatisticCollection().statObjectsCreated.getCounterValue();
        EXPECT_LE(1u, numberCreated); //some types create multiple scene objects (e.g. RenderTarget)
        EXPECT_EQ(0u, this->m_scene.m_impl.getStatisticCollection().statObjectsDestroyed.getCounterValue());

        this->m_scene.m_impl.getStatisticCollection().nextTimeInterval();
        EXPECT_EQ(initialNumber + numberCreated, this->m_scene.m_impl.getStatisticCollection().statObjectsCount.getCounterValue());

        this->m_scene.destroy(*obj);

        EXPECT_LE(1u, this->m_scene.m_impl.getStatisticCollection().statObjectsDestroyed.getCounterValue());

        this->m_scene.m_impl.getStatisticCollection().nextTimeInterval();
        EXPECT_GT(initialNumber + numberCreated, this->m_scene.m_impl.getStatisticCollection().statObjectsCount.getCounterValue());
        EXPECT_LE(initialNumber, this->m_scene.m_impl.getStatisticCollection().statObjectsCount.getCounterValue());
    }

    TYPED_TEST(ClientOwnershipTest, clientContainsCreatedObject)
    {
        const RamsesObject* obj = &this->template createObject<TypeParam>("objectName");
        ASSERT_TRUE(nullptr != obj);
        const RamsesObject* sn = this->client.findSceneByName("objectName");
        ASSERT_TRUE(nullptr != sn);
        EXPECT_EQ(obj, sn);
    }

    TYPED_TEST(ClientOwnershipTest, clientDoesNotContainDestroyedObject)
    {
        RamsesObject* obj = &this->template createObject<TypeParam>("objectName");
        ASSERT_TRUE(nullptr != obj);
        this->client.destroy(static_cast<Scene&>(*obj));
        const RamsesObject* sn = this->client.findSceneByName("objectName");
        ASSERT_TRUE(nullptr == sn);
    }

    TYPED_TEST(SceneOwnershipTest, sceneObjectNameChanged)
    {
        RamsesObject* obj = &this->template createObject<TypeParam>("objectName");
        ASSERT_TRUE(nullptr != obj);
        obj->setName("otherObjectName");
        RamsesObject* sn = this->m_scene.findObjectByName("otherObjectName");
        ASSERT_TRUE(nullptr != sn);
        EXPECT_EQ(obj, sn);
    }

    TYPED_TEST(ClientOwnershipTest, clientObjectsAreOfTypeClientObject)
    {
        const ClientObject* obj = &this->template createObject<TypeParam>("objectName");
        ASSERT_TRUE(nullptr != obj);
        EXPECT_TRUE(obj->isOfType(ERamsesObjectType::ClientObject));
    }

    TYPED_TEST(ClientOwnershipTest, clientObjectsHaveReferenceToTheirClient)
    {
        const ClientObject* obj = &this->template createObject<TypeParam>("objectName");
        ASSERT_TRUE(nullptr != obj);
        EXPECT_EQ(&this->client.m_impl, &obj->m_impl.getClientImpl());
    }

    TYPED_TEST(ClientOwnershipTest, clientObjectNameChanged)
    {
        RamsesObject* obj = &this->template createObject<TypeParam>("objectName");
        ASSERT_TRUE(nullptr != obj);
        obj->setName("otherObjectName");
        RamsesObject* sn = this->client.findSceneByName("otherObjectName");
        ASSERT_TRUE(nullptr != sn);
        EXPECT_EQ(obj, sn);
    }

    TYPED_TEST(SceneOwnershipTest, sceneNotContainsDestroyedObject)
    {
        RamsesObject& obj = this->template createObject<TypeParam>("objectName");
        TypeParam& objTyped = static_cast<TypeParam&>(obj);

        this->m_scene.destroy(objTyped);
        const RamsesObject* sn = this->m_scene.findObjectByName("objectName");
        EXPECT_EQ(nullptr, sn);
    }

    TYPED_TEST(ClientOwnershipTest, clientDoesNotFindDestroyedObject)
    {
        RamsesObject* obj = &this->template createObject<TypeParam>("objectName");
        TypeParam& objTyped = static_cast<TypeParam&>(*obj);
        ASSERT_TRUE(obj);

        this->client.destroy(objTyped);
        RamsesObject* sn = this->client.findSceneByName("objectName");
        ASSERT_FALSE(sn);
    }
}
