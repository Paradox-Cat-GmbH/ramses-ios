//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "gmock/gmock.h"
#include "TaskFramework/EnqueueOnlyOneAtATimeQueue.h"
#include "TaskFramework/TaskForwardingQueue.h"
#include "MockTaskQueue.h"
#include "MockITask.h"
#include "TaskFramework/ITask.h"
#include "Collections/Vector.h"

#include <atomic>
#include <thread>
#include <chrono>

namespace ramses_internal
{
    using namespace testing;

    template <typename ObjectType>
    class TaskForwardingQueuesTest : public testing::Test
    {
    public:
    };

    class DirectTaskExecutor : public ITaskQueue
    {
    public:
        bool enqueue(ITask& Task) override
        {
            Task.execute();
            return true;
        }

        void disableAcceptingTasksAfterExecutingCurrentQueue() override
        {
        }
    };

    using TestedQueues = ::testing::Types<EnqueueOnlyOneAtATimeQueue, TaskForwardingQueue>;
    TYPED_TEST_SUITE(TaskForwardingQueuesTest, TestedQueues);

    TYPED_TEST(TaskForwardingQueuesTest, EnqueueCorrrectNumberOfTasksAtATime)
    {
        MockTaskQueue successor;
        MockITask Task1;
        MockITask Task2;
        TypeParam queue(successor);

        EXPECT_CALL(successor, enqueue(_)).Times(std::is_same<TypeParam, EnqueueOnlyOneAtATimeQueue>::value ? 1 : 2);

        queue.enqueue(Task1);
        queue.enqueue(Task2);

        EXPECT_CALL(Task1, destructor());
        EXPECT_CALL(Task2, destructor());
    }


    TYPED_TEST(TaskForwardingQueuesTest, EnqueueSecondTaskAfterFirstFinished)
    {
        DirectTaskExecutor successor;
        MockITask Task1;
        MockITask Task2;
        TypeParam queue(successor);

        {
            InSequence seq;
            EXPECT_CALL(Task1, execute());
            EXPECT_CALL(Task2, execute());

            queue.enqueue(Task1);
            queue.enqueue(Task2);
        }

        EXPECT_CALL(Task1, destructor());
        EXPECT_CALL(Task2, destructor());
    }

    TYPED_TEST(TaskForwardingQueuesTest, EnqueueTaskAfterQueueIsEmpty)
    {
        MockTaskQueue successor;
        NiceMock<MockITask> Task1;
        NiceMock<MockITask> Task2;
        TypeParam queue(successor);
        TaskHelper helper;

        InSequence seq;

        EXPECT_CALL(successor, enqueue(_)).Times(1).WillOnce(Invoke(&helper, &TaskHelper::enqueue));
        queue.enqueue(Task1);
        helper.execute();

        EXPECT_CALL(successor, enqueue(_));
        queue.enqueue(Task2);

        EXPECT_CALL(Task2, destructor());
        EXPECT_CALL(Task1, destructor());
    }

    TEST(TaskForwardingQueueTest, disableUnlocksOnlyAfterTasksAreFinished)
    {
        MockTaskQueue successor;
        NiceMock<MockITask> task1;
        NiceMock<MockITask> task2;
        TaskForwardingQueue queue(successor);
        std::vector<ITask*> mockQ;
        std::atomic<bool> unlocked(false);

        EXPECT_CALL(successor, enqueue(_)).Times(2).WillRepeatedly([&](ITask& task)
        {
            task.addRef();
            mockQ.push_back(&task);
            return true;
        });

        queue.enqueue(task1);
        queue.enqueue(task2);

        std::thread t([&]()
        {
            queue.disableAcceptingTasksAfterExecutingCurrentQueue();
            unlocked = true;
        });

        EXPECT_FALSE(unlocked);
        mockQ[0]->execute();
        mockQ[0]->release();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        EXPECT_FALSE(unlocked);
        mockQ[1]->execute();
        mockQ[1]->release();
        t.join();
        EXPECT_TRUE(unlocked);
        EXPECT_CALL(task2, destructor());
        EXPECT_CALL(task1, destructor());
    }
}
