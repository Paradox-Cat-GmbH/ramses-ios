//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCEINFO_H
#define RAMSES_RESOURCEINFO_H

#include "Resource/ResourceTypes.h"
#include "SceneAPI/ResourceContentHash.h"
#include "IResource.h"

namespace ramses_internal
{
    struct ResourceInfo
    {
        EResourceType type;
        ResourceContentHash hash;
        uint32_t decompressedSize;
        uint32_t compressedSize;

        ResourceInfo(const EResourceType type_, const ResourceContentHash& hash_, const uint32_t decompressedSize_, const uint32_t compressedSize_)
            : type(type_)
            , hash(hash_)
            , decompressedSize(decompressedSize_)
            , compressedSize(compressedSize_)
        {
        }

        ResourceInfo()
            : type(EResourceType_Invalid)
            , decompressedSize(0)
            , compressedSize(0)
        {
        }

        explicit ResourceInfo(const IResource* resourceToGetDataFrom)
            : type(resourceToGetDataFrom->getTypeID())
            , hash(resourceToGetDataFrom->getHash())
            , decompressedSize(resourceToGetDataFrom->getDecompressedDataSize())
            , compressedSize(resourceToGetDataFrom->getCompressedDataSize())
        {
        }

        bool operator==(const ResourceInfo& other) const
        {
            return (type == other.type)
                && (hash == other.hash)
                && (decompressedSize == other.decompressedSize)
                && (compressedSize == other.compressedSize);
        }

        bool operator!=(const ResourceInfo& other) const
        {
            return !operator==(other);
        }
    };

    using ResourceInfoVector = std::vector<ResourceInfo>;
}

#endif
