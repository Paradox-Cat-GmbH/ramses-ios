//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/MeshNode.h"

// internal
#include "RenderGroupImpl.h"

namespace ramses
{
    RenderGroup::RenderGroup(std::unique_ptr<RenderGroupImpl> impl)
        : SceneObject{ std::move(impl) }
        , m_impl{ static_cast<RenderGroupImpl&>(SceneObject::m_impl) }
    {
    }

    status_t RenderGroup::addMeshNode(const MeshNode& mesh, int32_t orderWithinGroup)
    {
        const status_t status = m_impl.addMeshNode(mesh.m_impl, orderWithinGroup);
        LOG_HL_CLIENT_API2(status, LOG_API_RAMSESOBJECT_STRING(mesh), orderWithinGroup);
        return status;
    }

    status_t RenderGroup::removeMeshNode(const MeshNode& mesh)
    {
        const status_t status = m_impl.remove(mesh.m_impl);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(mesh));
        return status;
    }

    bool RenderGroup::containsMeshNode(const MeshNode& mesh) const
    {
        return m_impl.contains(mesh.m_impl);
    }

    status_t RenderGroup::getMeshNodeOrder(const MeshNode& mesh, int32_t& orderWithinGroup) const
    {
        return m_impl.getMeshNodeOrder(mesh.m_impl, orderWithinGroup);
    }

    status_t RenderGroup::addRenderGroup(const RenderGroup& renderGroup, int32_t orderWithinGroup)
    {
        const status_t status = m_impl.addRenderGroup(renderGroup.m_impl, orderWithinGroup);
        LOG_HL_CLIENT_API2(status, LOG_API_RAMSESOBJECT_STRING(renderGroup), orderWithinGroup);
        return status;
    }

    status_t RenderGroup::removeRenderGroup(const RenderGroup& renderGroup)
    {
        const status_t status = m_impl.remove(renderGroup.m_impl);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(renderGroup));
        return status;
    }

    bool RenderGroup::containsRenderGroup(const RenderGroup& renderGroup) const
    {
        return m_impl.contains(renderGroup.m_impl);
    }

    status_t RenderGroup::getRenderGroupOrder(const RenderGroup& renderGroup, int32_t& orderWithinGroup) const
    {
        return m_impl.getRenderGroupOrder(renderGroup.m_impl, orderWithinGroup);
    }

    status_t RenderGroup::removeAllRenderables()
    {
        const status_t status = m_impl.removeAllRenderables();
        LOG_HL_CLIENT_API_NOARG(status);
        return status;
    }

    ramses::status_t RenderGroup::removeAllRenderGroups()
    {
        const status_t status = m_impl.removeAllRenderGroups();
        LOG_HL_CLIENT_API_NOARG(status);
        return status;
    }
}
