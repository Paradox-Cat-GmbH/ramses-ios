//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "RendererCommands/LogRendererInfo.h"
#include "RendererLib/RendererCommandBuffer.h"

namespace ramses_internal
{
    LogRendererInfo::LogRendererInfo(RendererCommandBuffer& rendererCommandBuffer)
        : m_rendererCommandBuffer(rendererCommandBuffer)
    {
        description = "print renderer information";
        registerKeyword("rinfo");
        registerKeyword("printRendererInfo");

        getArgument<0>().setDefaultValue("all");
        getArgument<1>().setDefaultValue(false);
        getArgument<2>().setDefaultValue(NodeHandle::Invalid().asMemoryHandle());

        getArgument<0>().setDescription("topic (display|scene|res|queue|links|ec|events|all)");
        getArgument<1>().setDescription("verbose mode");
        getArgument<2>().setDescription("node Id filter");

        getArgument<1>().registerKeyword("v");
        getArgument<2>().registerKeyword("f");
    }

    bool LogRendererInfo::execute(std::string& topic, bool& verbose, MemoryHandle& nodeHandleFilter) const
    {
        const ERendererLogTopic etopic = GetRendererTopic(topic);
        if (etopic == ERendererLogTopic::COUNT)
            return false;

        m_rendererCommandBuffer.enqueueCommand(ramses_internal::RendererCommand::LogInfo{ etopic, verbose, NodeHandle(nodeHandleFilter) });
        return true;
    }

    ERendererLogTopic LogRendererInfo::GetRendererTopic(const std::string& topicName)
    {
        if (topicName == "display")
            return ERendererLogTopic::Displays;
        if (topicName == "scene")
            return ERendererLogTopic::SceneStates;
        if (topicName == "res")
            return ERendererLogTopic::Resources;
        if (topicName == "queue")
            return ERendererLogTopic::RenderQueue;
        if (topicName == "links")
            return ERendererLogTopic::Links;
        if (topicName == "ec")
            return ERendererLogTopic::EmbeddedCompositor;
        if (topicName == "events")
            return ERendererLogTopic::EventQueue;
        if (topicName == "all")
            return ERendererLogTopic::All;

        return ERendererLogTopic::COUNT;
    }
}
