#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

SET( EGL_FOUND FALSE )

IF (TARGET_OS MATCHES "Windows")

    # Windows does not suppot EGL natively

ELSEIF(TARGET_OS MATCHES "Linux" OR TARGET_OS MATCHES "Android")
    FIND_PATH(EGL_INCLUDE_DIRS EGL/egl.h
        /usr/include
    )

    FIND_LIBRARY(EGL_LIBRARY
        NAMES EGL
        PATHS
    )

    SET( EGL_FOUND "NO" )
    IF(EGL_LIBRARY)
        SET( EGL_LIBRARIES ${EGL_LIBRARY} )
        SET( EGL_FOUND TRUE )
        #message(STATUS "Found EGL libs: ${EGL_LIBRARIES}")
        #message(STATUS "Found EGL includes: ${EGL_INCLUDE_DIRS}")
    ENDIF(EGL_LIBRARY)

    MARK_AS_ADVANCED(
        EGL_INCLUDE_DIRS
        EGL_LIBRARIES
        EGL_LIBRARY
        EGL_FOUND
    )
ENDIF()
