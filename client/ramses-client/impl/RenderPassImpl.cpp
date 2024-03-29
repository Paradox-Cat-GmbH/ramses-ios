//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RenderPassImpl.h"

#include "ramses-client-api/Camera.h"
#include "ramses-client-api/RenderTarget.h"

#include "SerializationContext.h"
#include "CameraNodeImpl.h"
#include "RenderTargetImpl.h"
#include "RenderGroupImpl.h"
#include "RamsesObjectTypeUtils.h"

#include "Scene/ClientScene.h"

#include <string>

namespace ramses
{
    RenderPassImpl::RenderPassImpl(SceneImpl& scene, std::string_view renderpassName)
        : SceneObjectImpl(scene, ERamsesObjectType::RenderPass, renderpassName)
        , m_cameraImpl(nullptr)
        , m_renderTargetImpl(nullptr)
    {
    }

    RenderPassImpl::~RenderPassImpl()
    {
    }

    status_t RenderPassImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(SceneObjectImpl::serialize(outStream, serializationContext));

        outStream << m_renderPassHandle;

        if (m_cameraImpl != nullptr)
        {
            outStream << serializationContext.getIDForObject(m_cameraImpl);
        }
        else
        {
            outStream << SerializationContext::GetObjectIDNull();
        }

        if (m_renderTargetImpl != nullptr)
        {
            outStream << serializationContext.getIDForObject(m_renderTargetImpl);
        }
        else
        {
            outStream << SerializationContext::GetObjectIDNull();
        }

        outStream << static_cast<uint32_t>(m_renderGroups.size());
        for (const auto groupImpl : m_renderGroups)
        {
            assert(nullptr != groupImpl);
            outStream << serializationContext.getIDForObject(groupImpl);
        }

        return StatusOK;
    }

    status_t RenderPassImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(SceneObjectImpl::deserialize(inStream, serializationContext));

        inStream >> m_renderPassHandle;

        serializationContext.ReadDependentPointerAndStoreAsID(inStream, m_cameraImpl);
        serializationContext.ReadDependentPointerAndStoreAsID(inStream, m_renderTargetImpl);

        uint32_t numberOfGroups = 0;
        inStream >> numberOfGroups;
        assert(m_renderGroups.empty());
        m_renderGroups.reserve(numberOfGroups);

        for (uint32_t i = 0; i < numberOfGroups; ++i)
        {
            RenderGroupImpl* groupImpl = nullptr;
            serializationContext.ReadDependentPointerAndStoreAsID(inStream, groupImpl);
            m_renderGroups.push_back(groupImpl);
        }

        serializationContext.addForDependencyResolve(this);

        return StatusOK;
    }

    status_t RenderPassImpl::resolveDeserializationDependencies(DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(SceneObjectImpl::resolveDeserializationDependencies(serializationContext));

        serializationContext.resolveDependencyIDImplAndStoreAsPointer(m_cameraImpl);
        serializationContext.resolveDependencyIDImplAndStoreAsPointer(m_renderTargetImpl);

        for (size_t i = 0; i < m_renderGroups.size(); ++i)
        {
            RenderGroupImpl* group = const_cast<RenderGroupImpl*>(m_renderGroups[i]);
            serializationContext.resolveDependencyIDImplAndStoreAsPointer(group);
            m_renderGroups[i] = group;
        }

        return StatusOK;
    }

    status_t RenderPassImpl::validate() const
    {
        status_t status = SceneObjectImpl::validate();

        if (nullptr == m_cameraImpl)
            status = addValidationMessage(EValidationSeverity::Warning, "renderpass does not have a camera set");

        if (0 == m_renderGroups.size())
            status = addValidationMessage(EValidationSeverity::Warning, "renderpass does not contain any rendergroups");
        else
        {
            for (const auto& renderGroup : m_renderGroups)
                status = std::max(status, addValidationOfDependentObject(*renderGroup));
        }

        if (nullptr != m_renderTargetImpl)
            status = std::max(status, addValidationOfDependentObject(*m_renderTargetImpl));
        else if (getClearFlags() != ramses_internal::EClearFlags_None)
            status = std::max(status, addValidationMessage(EValidationSeverity::Warning, "renderpass has clear flags enabled whithout any rendertarget, clear flags will have no effect"));

        return status;
    }

    void RenderPassImpl::initializeFrameworkData()
    {
        m_renderPassHandle = getIScene().allocateRenderPass();
    }

    void RenderPassImpl::deinitializeFrameworkData()
    {
        assert(m_renderPassHandle.isValid());
        getIScene().releaseRenderPass(m_renderPassHandle);
        m_renderPassHandle = ramses_internal::RenderPassHandle::Invalid();
    }

    status_t RenderPassImpl::setCamera(const CameraNodeImpl& cameraImpl)
    {
        if (!isFromTheSameSceneAs(cameraImpl))
            return addErrorEntry("RenderPass::setCamera failed - camera is not from the same scene as this RenderPass");

        const status_t cameraValidity = cameraImpl.validate();
        if (StatusOK == cameraValidity)
        {
            m_cameraImpl = &cameraImpl;
            getIScene().setRenderPassCamera(m_renderPassHandle, cameraImpl.getCameraHandle());
        }
        else
        {
            std::string str = "RenderPass::setCamera failed - camera is not valid, maybe camera was not initialized:\n";
            str += cameraImpl.getValidationReport(EValidationSeverity::Warning);
            return addErrorEntry(str);
        }

        return cameraValidity;
    }

    const Camera* RenderPassImpl::getCamera() const
    {
        if (m_cameraImpl != nullptr)
        {
            return &RamsesObjectTypeUtils::ConvertTo<Camera>(m_cameraImpl->getRamsesObject());
        }

        return nullptr;
    }

    Camera* RenderPassImpl::getCamera()
    {
        // non-const version of getCamera cast to its const version to avoid duplicating code
        return const_cast<Camera*>((const_cast<const RenderPassImpl&>(*this)).getCamera());
    }

    status_t RenderPassImpl::setClearColor(const glm::vec4& clearColor)
    {
        getIScene().setRenderPassClearColor(m_renderPassHandle, clearColor);
        return StatusOK;
    }

    const glm::vec4& RenderPassImpl::getClearColor() const
    {
        return getIScene().getRenderPass(m_renderPassHandle).clearColor;
    }

    status_t RenderPassImpl::setClearFlags(uint32_t clearFlags)
    {
        static_assert(
            static_cast<uint32_t>(ramses::EClearFlags_None) == static_cast<uint32_t>(ramses_internal::EClearFlags_None) &&
            static_cast<uint32_t>(ramses::EClearFlags_Color) == static_cast<uint32_t>(ramses_internal::EClearFlags_Color) &&
            static_cast<uint32_t>(ramses::EClearFlags_Depth) == static_cast<uint32_t>(ramses_internal::EClearFlags_Depth) &&
            static_cast<uint32_t>(ramses::EClearFlags_Stencil) == static_cast<uint32_t>(ramses_internal::EClearFlags_Stencil) &&
            static_cast<uint32_t>(ramses::EClearFlags_All) == static_cast<uint32_t>(ramses_internal::EClearFlags_All), "Type conversion mismatch");

        getIScene().setRenderPassClearFlag(m_renderPassHandle, clearFlags);
        return StatusOK;
    }

    uint32_t RenderPassImpl::getClearFlags() const
    {
        return getIScene().getRenderPass(m_renderPassHandle).clearFlags;
    }

    status_t RenderPassImpl::addRenderGroup(const RenderGroupImpl& renderGroup, int32_t orderWithinPass)
    {
        if (!isFromTheSameSceneAs(renderGroup))
        {
            return addErrorEntry("RenderPass::addRenderGroup failed - renderGroup is not from the same scene as this RenderPass");
        }

        if (!ramses_internal::contains_c(m_renderGroups, &renderGroup))
        {
            const ramses_internal::RenderGroupHandle renderGroupHandle = renderGroup.getRenderGroupHandle();
            getIScene().addRenderGroupToRenderPass(m_renderPassHandle, renderGroupHandle, orderWithinPass);
            m_renderGroups.push_back(&renderGroup);
        }

        return StatusOK;
    }

    status_t RenderPassImpl::remove(const RenderGroupImpl& renderGroup)
    {
        RenderGroupVector::iterator iter = ramses_internal::find_c(m_renderGroups, &renderGroup);
        if (iter == m_renderGroups.end())
        {
            return addErrorEntry("RenderPass::removeRenderGroup failed - could not remove RenderGroup from Renderpass because it was not contained");
        }

        removeInternal(iter);

        return StatusOK;
    }

    void RenderPassImpl::removeIfContained(const RenderGroupImpl& renderGroup)
    {
        RenderGroupVector::iterator iter = ramses_internal::find_c(m_renderGroups, &renderGroup);
        if (iter != m_renderGroups.end())
        {
            removeInternal(iter);
        }
    }

    bool RenderPassImpl::contains(const RenderGroupImpl& renderGroup) const
    {
        return ramses_internal::contains_c(m_renderGroups, &renderGroup);
    }

    ramses::status_t RenderPassImpl::getRenderGroupOrder(const RenderGroupImpl& renderGroup, int32_t& orderWithinPass) const
    {
        if (!contains(renderGroup))
        {
            return addErrorEntry("RenderPass::getRenderGroupOrder failed - render group not contained in RenderPass");
        }

        const ramses_internal::RenderGroupHandle renderGroupHandle = renderGroup.getRenderGroupHandle();
        const ramses_internal::RenderPass& internalRP = getIScene().getRenderPass(m_renderPassHandle);
        for (const auto& rgEntry : internalRP.renderGroups)
        {
            if (rgEntry.renderGroup == renderGroupHandle)
            {
                orderWithinPass = rgEntry.order;
                return StatusOK;
            }
        }

        assert(false);
        return addErrorEntry("RenderPass::getRenderGroupOrder failed - fatal, render group not found in internal render pass");
    }

    const RenderGroupVector& RenderPassImpl::getAllRenderGroups() const
    {
        return m_renderGroups;
    }

    status_t RenderPassImpl::removeAllRenderGroups()
    {
        while (!m_renderGroups.empty())
        {
            CHECK_RETURN_ERR(remove(*m_renderGroups.front()));
        }

        return StatusOK;
    }

    ramses_internal::RenderPassHandle RenderPassImpl::getRenderPassHandle() const
    {
        return m_renderPassHandle;
    }

    status_t RenderPassImpl::setRenderTarget(RenderTargetImpl* renderTargetImpl)
    {
        if (renderTargetImpl == m_renderTargetImpl)
            return StatusOK;

        ramses_internal::RenderTargetHandle rtHandle(ramses_internal::RenderTargetHandle::Invalid());
        if (nullptr != renderTargetImpl)
        {
            if (nullptr == m_cameraImpl)
                return addErrorEntry("RenderPass::setRenderTarget failed - must explicitly assign a custom camera (perspective or orthographic) before rendering to render terget.");

            if (!isFromTheSameSceneAs(*renderTargetImpl))
                return addErrorEntry("RenderPass::setRenderTarget failed - renderTarget is not from the same scene as this RenderPass.");

            rtHandle = renderTargetImpl->getRenderTargetHandle();
        }

        m_renderTargetImpl = renderTargetImpl;
        getIScene().setRenderPassRenderTarget(m_renderPassHandle, rtHandle);

        return StatusOK;
    }

    const RenderTarget* RenderPassImpl::getRenderTarget() const
    {
        if (m_renderTargetImpl != nullptr)
        {
            return &RamsesObjectTypeUtils::ConvertTo<RenderTarget>(m_renderTargetImpl->getRamsesObject());
        }

        return nullptr;
    }

    status_t RenderPassImpl::setRenderOrder(int32_t renderOrder)
    {
        getIScene().setRenderPassRenderOrder(m_renderPassHandle, renderOrder);
        return StatusOK;
    }

    int32_t RenderPassImpl::getRenderOrder() const
    {
        return getIScene().getRenderPass(m_renderPassHandle).renderOrder;
    }

    status_t RenderPassImpl::setEnabled(bool isEnabeld)
    {
        getIScene().setRenderPassEnabled(m_renderPassHandle, isEnabeld);
        return StatusOK;
    }

    bool RenderPassImpl::isEnabled() const
    {
        return getIScene().getRenderPass(m_renderPassHandle).isEnabled;
    }

    void RenderPassImpl::removeInternal(RenderGroupVector::iterator iter)
    {
        const ramses_internal::RenderGroupHandle renderGroupHandle = (*iter)->getRenderGroupHandle();
        getIScene().removeRenderGroupFromRenderPass(m_renderPassHandle, renderGroupHandle);
        m_renderGroups.erase(iter);
    }

    status_t RenderPassImpl::setRenderOnce(bool enable)
    {
        getIScene().setRenderPassRenderOnce(m_renderPassHandle, enable);
        return StatusOK;
    }

    bool RenderPassImpl::isRenderOnce() const
    {
        return getIScene().getRenderPass(m_renderPassHandle).isRenderOnce;
    }

    status_t RenderPassImpl::retriggerRenderOnce()
    {
        if (!isRenderOnce())
        {
            return addErrorEntry("RenderPass::retriggerRenderOnce - cannot retrigger rendering of render pass that does not have render once flag set");
        }

        getIScene().retriggerRenderPassRenderOnce(m_renderPassHandle);
        return StatusOK;
    }
}
