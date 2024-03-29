//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "SceneActionUtils.h"
#include <algorithm>

namespace ramses_internal
{
    uint32_t SceneActionCollectionUtils::CountNumberOfActionsOfType(const SceneActionCollection& actions, ESceneActionId type)
    {
        auto p = [&type](const SceneActionCollection::SceneActionReader& reader)-> bool
            {
                return reader.type() == type;
            };

        return static_cast<uint32_t>(std::count_if(actions.begin(),actions.end(), p));
    }

    uint32_t SceneActionCollectionUtils::CountNumberOfActionsOfType(const SceneActionCollection& actions, const SceneActionIdVector& types)
    {
        auto p = [&types](const SceneActionCollection::SceneActionReader& action)-> bool
            {
                return contains_c(types, action.type());
            };

        return static_cast<uint32_t>(std::count_if(actions.begin(),actions.end(), p));
    }
}
