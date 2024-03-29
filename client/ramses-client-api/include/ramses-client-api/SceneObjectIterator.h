//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEOBJECTITERATOR_H
#define RAMSES_SCENEOBJECTITERATOR_H

#include "ramses-client-api/RamsesObject.h"

namespace ramses
{
    class ObjectIteratorImpl;
    class Scene;

    /**
    * @ingroup CoreAPI
    * @brief The SceneObjectIterator traverses objects in a Scene.
    *
    * It provides a way to traverse all objects owned by a given scene.
    */
    class SceneObjectIterator
    {
    public:
        /**
        * @brief A SceneObjectIterator can iterate through objects of given type within a scene.
        *
        * @param[in] scene Scene whose objects to iterate through
        * @param[in] objectType Optional type of objects to iterate through.
        **/
        RAMSES_API explicit SceneObjectIterator(const Scene& scene, ERamsesObjectType objectType = ERamsesObjectType::RamsesObject);

        /**
        * @brief Destructor
        **/
        RAMSES_API ~SceneObjectIterator();

        /**
        * @brief Iterate through all objects of given type
        * @return next object, null if no more objects available
        *
        * Iterator is invalid and may no longer be used if any objects are added or removed.
        **/
        RAMSES_API RamsesObject* getNext();

    private:
        std::unique_ptr<ObjectIteratorImpl> m_impl;
    };
}

#endif
