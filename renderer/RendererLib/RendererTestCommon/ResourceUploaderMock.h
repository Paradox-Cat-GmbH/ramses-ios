//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCEUPLOADERMOCK_H
#define RAMSES_RESOURCEUPLOADERMOCK_H

#include "renderer_common_gmock_header.h"
#include "gmock/gmock.h"
#include "RendererLib/IResourceUploader.h"
#include "RendererAPI/IRenderBackend.h"
#include "RendererLib/ResourceDescriptor.h"

namespace ramses_internal
{
    class ResourceUploaderMock : public IResourceUploader
    {
    public:
        ResourceUploaderMock();

        MOCK_METHOD(std::optional<DeviceResourceHandle> , uploadResource, (IRenderBackend&, const ResourceDescriptor&, uint32_t&), (override));
        MOCK_METHOD(void, unloadResource, (IRenderBackend&, EResourceType, ResourceContentHash, DeviceResourceHandle), (override));
        MOCK_METHOD(void, storeShaderInBinaryShaderCache, (IRenderBackend&, DeviceResourceHandle, const ResourceContentHash&, SceneId), (override));

        static const DeviceResourceHandle FakeResourceDeviceHandle;
    };
}

#endif
