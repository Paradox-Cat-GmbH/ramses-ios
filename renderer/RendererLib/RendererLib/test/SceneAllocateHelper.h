//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEALLOCATEHELPER_H
#define RAMSES_SCENEALLOCATEHELPER_H

#include "SceneAPI/SceneTypes.h"
#include "SceneAPI/ECameraProjectionType.h"
#include "SceneAPI/EDataSlotType.h"
#include "SceneAPI/DataSlot.h"
#include "SceneAPI/TextureEnums.h"
#include "SceneAPI/DataFieldInfo.h"
#include "SceneAPI/MipMapSize.h"
#include "SceneAPI/SceneId.h"

namespace ramses_internal
{
    class IScene;
    enum class EDataBufferType : uint8_t;
    struct TextureSampler;
    struct RenderBuffer;

    class SceneAllocateHelper
    {
    public:
        explicit SceneAllocateHelper(IScene& scene);

        RenderableHandle            allocateRenderable(NodeHandle nodeHandle, RenderableHandle handle = RenderableHandle::Invalid());
        RenderStateHandle           allocateRenderState(RenderStateHandle handle = RenderStateHandle::Invalid());
        CameraHandle                allocateCamera(ECameraProjectionType type, NodeHandle nodeHandle, DataInstanceHandle dataInstance, CameraHandle handle = CameraHandle::Invalid());
        NodeHandle                  allocateNode(uint32_t childrenCount = 0u, NodeHandle handle = NodeHandle::Invalid());
        TransformHandle             allocateTransform(NodeHandle nodeHandle, TransformHandle handle = TransformHandle::Invalid());
        DataLayoutHandle            allocateDataLayout(const DataFieldInfoVector& dataFields, const ResourceContentHash& effectHash, DataLayoutHandle handle = DataLayoutHandle::Invalid());
        DataInstanceHandle          allocateDataInstance(DataLayoutHandle finishedLayoutHandle, DataInstanceHandle handle = DataInstanceHandle::Invalid());
        TextureSamplerHandle        allocateTextureSampler(const TextureSampler& sampler, TextureSamplerHandle handle = TextureSamplerHandle::Invalid());
        RenderGroupHandle           allocateRenderGroup(RenderGroupHandle handle = RenderGroupHandle::Invalid());
        RenderPassHandle            allocateRenderPass(uint32_t renderGroupCount = 0u, RenderPassHandle handle = RenderPassHandle::Invalid());
        BlitPassHandle              allocateBlitPass(RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle handle = BlitPassHandle::Invalid());
        RenderTargetHandle          allocateRenderTarget(RenderTargetHandle handle = RenderTargetHandle::Invalid());
        RenderBufferHandle          allocateRenderBuffer(const RenderBuffer& renderBuffer, RenderBufferHandle handle = RenderBufferHandle::Invalid());
        DataSlotHandle              allocateDataSlot(const DataSlot& dataSlot, DataSlotHandle handle = DataSlotHandle::Invalid());
        DataBufferHandle            allocateDataBuffer(EDataBufferType dataBufferType, EDataType dataType, uint32_t maximumSizeInBytes, DataBufferHandle handle = DataBufferHandle::Invalid());
        TextureBufferHandle         allocateTextureBuffer(ETextureFormat textureFormat, const MipMapDimensions& mipMapDimensions, TextureBufferHandle handle = TextureBufferHandle::Invalid());
        PickableObjectHandle        allocatePickableObject(DataBufferHandle geometryHandle, NodeHandle nodeHandle, PickableObjectId id, PickableObjectHandle pickableHandle = PickableObjectHandle::Invalid());
        SceneReferenceHandle        allocateSceneReference(SceneId sceneId, SceneReferenceHandle handle = {});

    private:
        template <typename HANDLE>
        HANDLE preallocateHandle(HANDLE handle);

        IScene& m_scene;
    };
}

#endif
