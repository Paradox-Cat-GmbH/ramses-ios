//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEACTIONAPPLIER_H
#define RAMSES_SCENEACTIONAPPLIER_H

#include "SceneActionCollection.h"
#include "SceneAPI/SceneVersionTag.h"
#include "Collections/Vector.h"
#include "SceneReferencing/SceneReferenceAction.h"
#include <memory>

namespace ramses_internal
{
    class IScene;
    struct ResourceChanges;
    struct SceneSizeInformation;
    class IResource;
    struct FlushTimeInformation;

    class SceneActionApplier
    {
    public:
        using ResourceVector = std::vector<std::unique_ptr<IResource>>;

        static void ApplyActionsOnScene(IScene& scene, const SceneActionCollection& actions);

    private:
        static void GetSceneSizeInformation(SceneActionCollection::SceneActionReader& action, SceneSizeInformation& sizeInfo);
        static void ApplySingleActionOnScene(IScene& scene, SceneActionCollection::SceneActionReader& action);
    };
}

#endif
