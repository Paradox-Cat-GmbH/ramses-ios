#  -------------------------------------------------------------------------
#  Copyright (C) 2020 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

SET(Platfom_EGL_Dependencies    Context_EGL
                                Device_GL
                                ramses-renderer-lib)

# It should be possible to have Platform_EGL without Device_EGL_Extension if gbm and libdrm
# are not supported on some platforms
if (TARGET Device_EGL_Extension)
    SET(Platfom_EGL_Dependencies ${Platfom_EGL_Dependencies} Device_EGL_Extension)
endif()

createModule(
    NAME                    Platform_EGL
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF
    INCLUDE_PATHS           include
    SRC_FILES               include/Platform_EGL/*.h
                            src/*.cpp
    DEPENDENCIES            ${Platfom_EGL_Dependencies}
)
