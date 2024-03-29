//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCETABLEOFCONTENTS_H
#define RAMSES_RESOURCETABLEOFCONTENTS_H

#include "SceneAPI/ResourceContentHash.h"
#include "Collections/HashMap.h"
#include "Collections/IInputStream.h"
#include "Collections/IOutputStream.h"
#include "ResourcePersistation.h"
#include "Utils/BinaryFileOutputStream.h"
#include "Utils/BinaryFileInputStream.h"
#include "Resource/ResourceInfo.h"

namespace ramses_internal
{
    struct ResourceFileEntry
    {
        uint32_t offsetInBytes = 0;
        uint32_t sizeInBytes = 0;
        ResourceInfo resourceInfo;
    };

    using TableOfContentsMap = HashMap<ResourceContentHash, ResourceFileEntry>;

    class ResourceTableOfContents
    {
    public:
        [[nodiscard]] bool containsResource(const ResourceContentHash& hash) const;
        void registerContents(const ResourceInfo& info, uint32_t offsetInBytes, uint32_t sizeInBytes);
        [[nodiscard]] const ResourceFileEntry& getEntryForHash(const ResourceContentHash& hash) const;
        [[nodiscard]] const TableOfContentsMap& getFileContents() const;
        bool readTOCPosAndTOCFromStream(IInputStream& instream);
        void writeTOCToStream(IOutputStream& outstream);

    private:
        TableOfContentsMap m_fileContents;
    };
}

#endif
