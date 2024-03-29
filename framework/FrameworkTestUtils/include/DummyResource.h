//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DUMMYRESOURCE_H
#define RAMSES_DUMMYRESOURCE_H

#include "Resource/ResourceBase.h"

#include <string_view>

namespace ramses_internal
{
    class DummyResource : public ResourceBase
    {
    public:
        DummyResource(const ResourceContentHash& hash, EResourceType typeId, ResourceCacheFlag cacheFlag = ResourceCacheFlag(123u), std::string_view name = {})
            : ResourceBase(typeId, cacheFlag, name)
            , m_hash(hash)
        {
        }

        const ResourceContentHash& getHash() const override
        {
            return m_hash;
        }

        uint32_t getCompressedDataSize() const override
        {
            return 0;
        }

        uint32_t getDecompressedDataSize() const override
        {
            return 42;
        }

        void serializeResourceMetadataToStream(IOutputStream& /*output*/) const override
        {
        }

    private:
        ResourceContentHash m_hash;
    };
}

#endif
