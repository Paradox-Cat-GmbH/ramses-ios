//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "ClientTestUtils.h"
#include "ramses-client-api/Node.h"
#include "SceneAPI/IScene.h"
#include "Scene/SceneActionCollection.h"


namespace ramses
{
    using namespace testing;
    class ADistributedScene : public LocalTestClientWithScene, public ::testing::Test
    {
    public:
        ADistributedScene()
        {
            framework.connect();
        }

        ~ADistributedScene() override
        {
        }

        void publishScene()
        {
            const ramses_internal::IScene& iscene = m_scene.m_impl.getIScene();
            ramses_internal::SceneInfo info(iscene.getSceneId(), iscene.getName());
            EXPECT_CALL(sceneActionsCollector, handleNewSceneAvailable(info, _));
            EXPECT_CALL(sceneActionsCollector, handleInitializeScene(info, _));
            EXPECT_EQ(StatusOK, m_scene.publish());
        }

        void unpublishScene()
        {
            EXPECT_CALL(sceneActionsCollector, handleSceneBecameUnavailable(ramses_internal::SceneId(m_scene.m_impl.getSceneId().getValue()), _));
            EXPECT_EQ(StatusOK, m_scene.unpublish());
        }

        void doSceneOperations()
        {
            // do random m_scene stuff
            Node* node = m_scene.createNode("node");
            Node* nodeTrans = m_scene.createNode("nodetrans");
            nodeTrans->setParent(*node);
        }

        void expectActionListsEqual(const ramses_internal::SceneActionCollection& list1, const ramses_internal::SceneActionCollection& list2) const
        {
            EXPECT_EQ(list1.numberOfActions(), list2.numberOfActions());
            for (uint32_t i = 0u; i < list1.numberOfActions(); ++i)
            {
                EXPECT_EQ(list1[i].type(), list2[i].type());
            }
        }

        void expectSceneOperationsSent()
        {
            EXPECT_CALL(sceneActionsCollector, handleSceneUpdate_rvr(ramses_internal::SceneId(m_scene.m_impl.getSceneId().getValue()), _, _));
        }

        void expectSceneUnpublication()
        {
            EXPECT_CALL(sceneActionsCollector, handleSceneBecameUnavailable(ramses_internal::SceneId(m_scene.m_impl.getSceneId().getValue()), _));
        }

    protected:
    };

    TEST_F(ADistributedScene, flushProducesSingleActionList)
    {
        publishScene();
        expectSceneOperationsSent();
        EXPECT_EQ(StatusOK, m_scene.flush());
        sceneActionsCollector.resetCollecting();

        doSceneOperations();
        expectSceneOperationsSent();
        EXPECT_EQ(StatusOK, m_scene.flush());
        ramses_internal::SceneActionCollection actionsBeforeFirstFlush(sceneActionsCollector.getCopyOfCollectedActions());
        EXPECT_EQ(1u, sceneActionsCollector.getNumReceivedActionLists());
        sceneActionsCollector.resetCollecting();

        doSceneOperations();
        expectSceneOperationsSent();
        EXPECT_EQ(StatusOK, m_scene.flush());
        ramses_internal::SceneActionCollection actionsBeforeSecondFlush(sceneActionsCollector.getCopyOfCollectedActions());
        EXPECT_EQ(1u, sceneActionsCollector.getNumReceivedActionLists());
        sceneActionsCollector.resetCollecting();

        expectActionListsEqual(actionsBeforeFirstFlush, actionsBeforeSecondFlush);
        expectSceneUnpublication();
    }

    TEST_F(ADistributedScene, publishCausesSceneToBeSentToImplicitSubscriberInLocalCase)
    {
        expectSceneOperationsSent();
        m_scene.flush();
        sceneActionsCollector.resetCollecting();
        doSceneOperations();
        publishScene();
        EXPECT_EQ(1u, sceneActionsCollector.getNumReceivedActionLists());
        expectSceneUnpublication();
    }

    TEST_F(ADistributedScene, unpublishingSceneResetsPendingActions)
    {
        publishScene();
        expectSceneOperationsSent();
        EXPECT_EQ(StatusOK, m_scene.flush());
        sceneActionsCollector.resetCollecting();

        doSceneOperations();
        unpublishScene();

        EXPECT_EQ(0u, sceneActionsCollector.getNumReceivedActionLists());
        sceneActionsCollector.resetCollecting();

        // no new messages/calls, everything has been already flushed
        EXPECT_EQ(StatusOK, m_scene.flush());
        EXPECT_EQ(0u, sceneActionsCollector.getNumReceivedActionLists());
        EXPECT_EQ(0u, sceneActionsCollector.getNumberOfActions());
    }

    TEST_F(ADistributedScene, destroyingSceneCausesUnpublish)
    {
        const ramses_internal::SceneId sceneId(33u);
        Scene* otherScene = client.createScene(ramses::sceneId_t(sceneId.getValue()));
        ASSERT_TRUE(otherScene != nullptr);
        const ramses_internal::IScene& otherIScene = otherScene->m_impl.getIScene();

        ramses_internal::SceneInfo sceneInfo(sceneId, otherIScene.getName());
        EXPECT_CALL(sceneActionsCollector, handleNewSceneAvailable(sceneInfo, _));
        EXPECT_EQ(StatusOK, otherScene->publish());

        EXPECT_CALL(sceneActionsCollector, handleSceneBecameUnavailable(sceneId, _));
        client.destroy(*otherScene);
    }

    TEST_F(ADistributedScene, confidence_emptyFlushDoesNotCollectAnySceneActions)
    {
        // First do immediate mode, no commits
        publishScene();
        doSceneOperations();
        expectSceneOperationsSent();
        m_scene.flush();
        sceneActionsCollector.resetCollecting();
        m_scene.flush();
        EXPECT_EQ(0u, sceneActionsCollector.getNumReceivedActionLists());
        expectSceneUnpublication();
    }
}
