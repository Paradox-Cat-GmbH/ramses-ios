//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERAPI_IRENDERERRESOURCECACHE_H
#define RAMSES_RENDERERAPI_IRENDERERRESOURCECACHE_H

#include "Types.h"
#include "ramses-framework-api/APIExport.h"

namespace ramses
{
    /**
    * @brief An interface used to implement a caching mechanism for resources used on the RamsesRenderer.
    *        Important: Please note that the resource ids in this context do not match client resource ids.
    * @ingroup RendererAPI
    */
    class RAMSES_API IRendererResourceCache
    {
    public:

        /**
        * @brief Destructor of IRendererResourceCache
        */
        virtual ~IRendererResourceCache() = default;

        /**
         * @brief Called by RamsesRenderer to ask for a resource with the given id.
         *
         * @param resourceId Id for the resource.
         * @param[out] size The size of the found resource in bytes. This value is only relevant if
         *                  the resource exists in the cache.
         * @return true if a resource with the given resource id exists in the cache.
         */
        virtual bool hasResource(rendererResourceId_t resourceId, uint32_t& size) const = 0;

        /**
        * @brief Called by RamsesRenderer to get the resource data associated with a given resource id.
        *        This method will be called immediately after hasResource(...), if the resource exists in the cache.
        *
        * @param resourceId Id for the resource.
        * @param buffer A pre-allocated buffer which the resource data will be copied into.
        * @param bufferSize The size of the pre-allocated buffer in bytes. It should be at least the size of the
        *             requested resource (returned by hasResource(...)).
        * @return true if the resource was copied successfully into the buffer.
        */
        virtual bool getResourceData(rendererResourceId_t resourceId, uint8_t* buffer, uint32_t bufferSize) const = 0;

        /**
        * @brief Called by RamsesRenderer when a resource was not in the cache and is now available from
        *        other source. The cache is asked if it wants to store a given resource or not. This
        *        avoids the overhead of preparing the resource data in case it is not to be cached.
        *
        * @param resourceId Id for the resource.
        * @param resourceDataSize The size of the resource in bytes.
        * @param cacheFlag The cache flag associated with the resource (set on client side).
        * @param sceneId The id of the first scene which requested the resource. In case of multiple scenes
        *                 using the same resource, only the first scene id is guaranteed to be reported.
        * @return true if the cache wants to store the resource.
        */
        [[nodiscard]] virtual bool shouldResourceBeCached(rendererResourceId_t resourceId, uint32_t resourceDataSize, resourceCacheFlag_t cacheFlag, sceneId_t sceneId) const = 0;

        /**
        * @brief Called by RamsesRenderer with the final resource for storing. This is called
        *        immediately after shouldResourceBeCached(...), if it was requested to be cached.
        *
        * @param resourceId Id for the resource.
        * @param resourceData The resource data which will be copied into the cache.
        * @param resourceDataSize The size of the resource in bytes.
        * @param cacheFlag The cache flag associated with the resource (set on client side).
        * @param sceneId The id of the first scene which requested the resource. In case of multiple scenes
        *                 using the same resource, only the first scene id is guaranteed to be reported.
        */
        virtual void storeResource(rendererResourceId_t resourceId, const uint8_t* resourceData, uint32_t resourceDataSize, resourceCacheFlag_t cacheFlag, sceneId_t sceneId) = 0;
    };
}

#endif
