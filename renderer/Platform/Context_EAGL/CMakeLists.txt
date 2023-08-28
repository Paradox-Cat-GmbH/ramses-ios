#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

IF(CMAKE_SYSTEM_NAME MATCHES "iOS")
    ACME_MODULE(

        #==========================================================================
        # general module information
        #==========================================================================
        NAME                    Context_EAGL
        TYPE                    STATIC_LIBRARY
        ENABLE_INSTALL          OFF

        #==========================================================================
        # files of this module
        #==========================================================================
        FILES_PRIVATE_HEADER    include/Context_EAGL/*.h
        FILES_SOURCE            src/*.mm

        #==========================================================================
        # dependencies
        #==========================================================================
        DEPENDENCIES            OpenGL
                                ramses-renderer-lib
    )
ENDIF()