//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_UTILS_LOGHELPER_H
#define RAMSES_UTILS_LOGHELPER_H

#include "Utils/LogLevel.h"
#include "PlatformAbstraction/PlatformTypes.h"

#include <string>
#include <string_view>

namespace ramses_internal
{
    namespace LogHelper
    {
        using ContextFilter = std::pair<ELogLevel, std::string>;

        bool StringToLogLevel(std::string_view str, ELogLevel& logLevel);
        ELogLevel GetLoglevelFromInt(int32_t logLevelInt);

        std::vector<ContextFilter> ParseContextFilters(const std::string& filterCommand);
    }
}

#endif
