//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ECAMERAPROJECTIONTYPE_H
#define RAMSES_ECAMERAPROJECTIONTYPE_H

#include "Utils/LoggingUtils.h"
#include "PlatformAbstraction/PlatformTypes.h"

namespace ramses_internal
{
    enum class ECameraProjectionType
    {
        Perspective = 0,
        Orthographic
    };

    const std::array ECameraProjectionTypeNames = {
        "Perspective",
        "Orthographic"
    };
}

MAKE_ENUM_CLASS_PRINTABLE_NO_EXTRA_LAST(ramses_internal::ECameraProjectionType,
                                        "ECameraProjectionType",
                                        ramses_internal::ECameraProjectionTypeNames,
                                        ramses_internal::ECameraProjectionType::Orthographic);

#endif
