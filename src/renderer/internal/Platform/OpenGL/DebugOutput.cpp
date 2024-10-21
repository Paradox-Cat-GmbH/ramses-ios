//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/OpenGL/DebugOutput.h"

#include "internal/Core/Utils/LogMacros.h"
#include <array>
#include <cassert>

namespace ramses::internal
{
    static void debugCallback(GLenum /*source*/,
                                       GLenum type,
                                       GLuint /*id*/,
                                       GLenum /*severity*/,
                                       GLsizei /*length*/,
                                       const GLchar* message,
                                       const void* userParam)
    {
        assert(userParam);
        
        LOG_WARN(CONTEXT_RENDERER, "OpenGL warning: {}", message);
    }

    void DebugOutput::enable()
    {
        if (!IsAvailable())
        {
            LOG_INFO(CONTEXT_RENDERER, "Could not find OpenGL debug output extension");
            return;
        }
/*
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

        // pass flag per userParam so we can set it in case of an error
        glDebugMessageCallback(debugCallback, &m_errorOccured);

        // enable all debug messages...
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

        // ... except redundant state change warnings on nvidia cards
        const std::array<const GLuint, 1> messageIds{{8}};
        glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_PERFORMANCE, GL_DONT_CARE, static_cast<GLsizei>(messageIds.size()), messageIds.data(), GL_FALSE);*/
    }

    bool DebugOutput::IsAvailable()
    {
        return false;//(glDebugMessageCallback != nullptr && glDebugMessageControl != nullptr);
    }

    bool DebugOutput::checkAndResetError() const
    {
        if (m_errorOccured)
        {
            m_errorOccured = false;
            return true;
        }
        return false;
    }
}
