//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Context_EAGL/Context_EAGL.h"
#include "Platform_Base/RenderTargetGpuResource.h"
#include "Utils/ThreadLocalLogForced.h"
#include <GLKit/GLKit.h>

#include <OpenGLES/ES3/gl.h>

namespace ramses_internal
{
    Context_EAGL::Context_EAGL(CAEAGLLayer* drawable, Context_EAGL * sharedContext) :
        m_sharedContext(sharedContext ? sharedContext->m_context  : nullptr), m_drawable(drawable)
    {
    }

    Bool Context_EAGL::init()
    {
        if(m_sharedContext) {
            m_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3 sharegroup: [m_sharedContext sharegroup]];
        } else {
            m_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
            
            [EAGLContext setCurrentContext: m_context];
          
            glGenFramebuffers(1, &_defaultFrameBuffer);
            auto error = glGetError();

            glGenRenderbuffers(1, &_colorRenderbuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, _defaultFrameBuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderbuffer);
            
            [m_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:m_drawable];
            
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _colorRenderbuffer);

            GLint backingWidth;
            GLint backingHeight;
            glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backingWidth);
            glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backingHeight);
                        
            glGenRenderbuffers(1, &_depthRenderbuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderbuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, backingWidth, backingHeight);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _depthRenderbuffer);
            

            if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
                LOG_ERROR(CONTEXT_RENDERER, "Context_EAGL::init(): failed to make complete framebuffer object");
                return false;
            }
            
            m_framebufferResource = m_resources.registerResource(std::make_unique<RenderTargetGPUResource>(_defaultFrameBuffer));
        }
        
        
        LOG_INFO_P(CONTEXT_RENDERER, "Context_EAGL::init(): EAGL context creation succeeded");
        return true;
    }
    

    DeviceResourceHandle Context_EAGL::getFramebufferResource() const { return m_framebufferResource;}

    Context_EAGL::~Context_EAGL()
    {
        LOG_INFO(CONTEXT_RENDERER, "Context_EAGL destroy");
        
        m_context = nullptr;
        m_sharedContext = nullptr;
        m_drawable = nullptr;
        
        LOG_INFO(CONTEXT_RENDERER, "Context_EAGL::~Context_EAGL done.");
    }

    Bool Context_EAGL::swapBuffers()
    {
        LOG_TRACE(CONTEXT_RENDERER, "Context_EAGL swap buffers");
            
        glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderbuffer);
        [m_context presentRenderbuffer:GL_RENDERBUFFER];
        glBindFramebuffer(GL_FRAMEBUFFER, _defaultFrameBuffer);
        
        return true;
    }

    Bool Context_EAGL::enable()
    {
        LOG_TRACE(CONTEXT_RENDERER, "Context_EAGL enable");
        [EAGLContext setCurrentContext: m_context];
        
        return true;
    }

    Bool Context_EAGL::disable()
    {
        LOG_TRACE(CONTEXT_RENDERER, "Context_EAGL disable");
        [EAGLContext setCurrentContext: nullptr];
        
        return true;
    }

    void* Context_EAGL::getProcAddress(const char* name) const
    {
        return nullptr;
    }

}


