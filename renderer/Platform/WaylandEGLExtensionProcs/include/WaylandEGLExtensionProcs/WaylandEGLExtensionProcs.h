//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WAYLANDEGLEXTENSIONPROCS_H
#define RAMSES_WAYLANDEGLEXTENSIONPROCS_H

#include "Utils/Warnings.h"
#include "Collections/HashSet.h"

WARNINGS_PUSH
WARNING_DISABLE_LINUX(-Wdeprecated-declarations)
#include "wayland-egl.h"
WARNINGS_POP

#include "GLES2/gl2.h"
#include "GLES2/gl2ext.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"
#ifdef RAMSES_HAS_EGLMESAEXT
#include "EGL/eglmesaext.h"
#endif

#include <string>

namespace ramses_internal
{
    class WaylandEGLExtensionProcs
    {
    public:
        explicit WaylandEGLExtensionProcs(wl_display* waylandWindowDisplay);
        explicit WaylandEGLExtensionProcs(EGLDisplay eglDisplay);

        EGLImageKHR eglCreateImageKHR(EGLContext context, EGLenum target, EGLClientBuffer buffer, const EGLint* attributeList) const;
        EGLBoolean eglDestroyImageKHR(EGLImageKHR image) const;
        void glEGLImageTargetTexture2DOES(GLenum target, GLeglImageOES image) const;
        EGLBoolean eglBindWaylandDisplayWL(wl_display* waylandDisplay) const;
        EGLBoolean eglUnbindWaylandDisplayWL(wl_display* waylandDisplay) const;

        [[nodiscard]] bool areExtensionsSupported()const;
        [[nodiscard]] bool areDmabufExtensionsSupported()const;

    private:

        void Init();

        static bool CheckExtensionAvailable(const ramses_internal::HashSet<std::string>& eglExtensions, const std::string& extensionName);

        const EGLDisplay m_eglDisplay;

        PFNEGLCREATEIMAGEKHRPROC m_eglCreateImageKHR;
        PFNEGLDESTROYIMAGEKHRPROC m_eglDestroyImageKHR;
        PFNGLEGLIMAGETARGETTEXTURE2DOESPROC m_glEGLImageTargetTexture2DOES;
        PFNEGLBINDWAYLANDDISPLAYWL m_eglBindWaylandDisplayWL;
        PFNEGLUNBINDWAYLANDDISPLAYWL m_eglUnbindWaylandDisplayWL;

        bool m_extensionsSupported;
        bool m_dmabufExtensionsSupported;
    };
}

#endif
