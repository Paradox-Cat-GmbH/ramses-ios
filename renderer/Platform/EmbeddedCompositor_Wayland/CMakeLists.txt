#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

createModule(
    NAME                    EmbeddedCompositor_Wayland
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          OFF
    INCLUDE_PATHS           include
    SRC_FILES               include/EmbeddedCompositor_Wayland/*.h
                            src/*.cpp
    DEPENDENCIES            ramses-renderer-lib
                            wayland-server
                            wayland-ivi-extension
                            wayland-zwp-linux-dmabuf-v1-extension
                            libdrm
                            WaylandUtilities
                            Context_EGL
                            WaylandEGLExtensionProcs
)

if (${ramses-sdk_BUILD_TESTS})
    createModule(
        NAME                    EmbeddedCompositor_Wayland_Test
        TYPE                    BINARY
        ENABLE_INSTALL          ON
        SRC_FILES            ../EmbeddedCompositor_Wayland/test/*.cpp
                                ../EmbeddedCompositor_Wayland/test/*.h
        DEPENDENCIES            EmbeddedCompositor_Wayland
                                wayland-client
                                Device_GL
                                RendererTestCommon
                                WaylandTestUtils
                                WaylandUtilities
    )

    makeTestFromTarget(
        TARGET EmbeddedCompositor_Wayland_Test
        SUFFIX RNDSANDWICHTEST_SWRAST
        SKIPPABLE)
endif()
