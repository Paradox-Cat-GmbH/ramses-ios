//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTURE2DBUFFERIMPL_H
#define RAMSES_TEXTURE2DBUFFERIMPL_H

#include "SceneObjectImpl.h"
#include "SceneAPI/Handles.h"
#include "SceneAPI/MipMapSize.h"
#include "ramses-framework-api/EDataType.h"
#include "ramses-client-api/TextureEnums.h"

#include <string_view>

namespace ramses
{
    class Texture2DBufferImpl : public SceneObjectImpl
    {
    public:
        Texture2DBufferImpl(SceneImpl& scene, std::string_view textureBufferName);
        ~Texture2DBufferImpl() override;

        void             initializeFrameworkData(const ramses_internal::MipMapDimensions& mipDimensions, ETextureFormat textureFormat);
        void     deinitializeFrameworkData() override;
        status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;
        status_t validate() const override;

        status_t setData(const ramses_internal::Byte* data, size_t mipLevel, uint32_t offsetX, uint32_t offsetY, uint32_t width, uint32_t height);
        size_t getMipLevelCount() const;
        ETextureFormat getTexelFormat() const;
        status_t getMipLevelData(size_t mipLevel, char* buffer, size_t bufferSize) const;
        status_t getMipLevelSize(size_t mipLevel, uint32_t& widthOut, uint32_t& heightOut) const;
        size_t getMipLevelDataSizeInBytes(size_t mipLevel) const;

        ramses_internal::TextureBufferHandle getTextureBufferHandle() const;

    private:
        ramses_internal::TextureBufferHandle m_textureBufferHandle;
    };
}

#endif
