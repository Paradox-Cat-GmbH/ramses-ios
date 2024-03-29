//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

//API
#include "ramses-client-api/PickableObject.h"
#include "ramses-client-api/Camera.h"

//internal
#include "PickableObjectImpl.h"
#include "RamsesFrameworkTypesImpl.h"

namespace ramses
{
    PickableObject::PickableObject(std::unique_ptr<PickableObjectImpl> impl)
        : Node{ std::move(impl) }
        , m_impl{ static_cast<PickableObjectImpl&>(Node::m_impl) }
    {
    }

    const ArrayBuffer& PickableObject::getGeometryBuffer() const
    {
        return m_impl.getGeometryBuffer();
    }

    const Camera* PickableObject::getCamera() const
    {
        return m_impl.getCamera();
    }

    status_t PickableObject::setCamera(const Camera& camera)
    {
        const status_t status = m_impl.setCamera(camera.m_impl);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(camera));
        return status;
    }

    pickableObjectId_t PickableObject::getPickableObjectId() const
    {
        return m_impl.getPickableObjectId();
    }

    status_t PickableObject::setPickableObjectId(pickableObjectId_t id)
    {
        const status_t status = m_impl.setPickableObjectId(id);
        LOG_HL_CLIENT_API1(status, id);
        return status;
    }

    status_t PickableObject::setEnabled(bool enable)
    {
        const status_t status = m_impl.setEnabled(enable);
        LOG_HL_CLIENT_API1(status, enable);
        return status;
    }

    bool PickableObject::isEnabled() const
    {
        return m_impl.isEnabled();
    }
}
