//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// OSWindow:
//   Operating system window integration base class.

#ifndef UTIL_OSWINDOW_H_
#define UTIL_OSWINDOW_H_

#include <stdint.h>
#include <functional>
#include <list>
#include <string>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "Event.h"

class OSWindow
{
  public:
    static OSWindow *New();
    static void Delete(OSWindow **osWindow);

    virtual bool initialize(const std::string &name, int width, int height) = 0;
    virtual void destroy()                                                  = 0;

    int getX() const;
    int getY() const;
    int getWidth() const;
    int getHeight() const;

    // Re-initializes the native window. This is used on platforms which do not
    // have a reusable EGLNativeWindowType in order to recreate it, and is
    // needed by the test suite because it re-uses the same OSWindow for
    // multiple EGLSurfaces.
    virtual void resetNativeWindow() = 0;

    virtual EGLNativeWindowType getNativeWindow() const   = 0;
    virtual EGLNativeDisplayType getNativeDisplay() const = 0;

    virtual void messageLoop() = 0;
    virtual void messageLoopOnce() = 0;

    bool popEvent(Event *event);
    virtual void pushEvent(Event event);

    virtual bool resize(int width, int height)  = 0;
    virtual bool setPosition(int x, int y)      = 0;
    virtual void setVisible(bool isVisible)     = 0;

    typedef std::function<int()> LoopDelegate;
    typedef LoopDelegate LoopStartDelegate;
    virtual bool hasOwnLoop() const { return false; }
    virtual int runOwnLoop(LoopStartDelegate initDelegate, LoopDelegate loopDelegate) { return 0; }

  protected:
    OSWindow();
    virtual ~OSWindow();
    friend void FreeOSWindow(OSWindow *window);

    int mX;
    int mY;
    int mWidth;
    int mHeight;

    std::list<Event> mEvents;
};

#endif  // UTIL_OSWINDOW_H_
