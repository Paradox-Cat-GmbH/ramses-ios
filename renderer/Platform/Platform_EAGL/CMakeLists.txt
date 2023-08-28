#  -------------------------------------------------------------------------
#  Copyright (C) 2020 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

IF(CMAKE_SYSTEM_NAME MATCHES "iOS")
    SET(Platfom_EAGL_Dependencies   Context_EAGL
                                    Device_GL
                                    ramses-renderer-lib)

    ACME_MODULE(

        #==========================================================================
        # general module information
        #==========================================================================
        NAME                    Platform_EAGL
        TYPE                    STATIC_LIBRARY
        ENABLE_INSTALL          OFF

        #==========================================================================
        # files of this module
        #==========================================================================
        FILES_PRIVATE_HEADER    include/Platform_EAGL/*.h
        FILES_SOURCE            src/*.cpp

        #==========================================================================
        # dependencies
        #==========================================================================
        DEPENDENCIES            ${Platfom_EAGL_Dependencies}
    )
ENDIF()
