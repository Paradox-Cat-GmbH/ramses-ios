//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "Platform_MacOS_EGL/OSWindow.h"

#include <iostream>
#include <sstream>

OSWindow::OSWindow() : mX(0), mY(0), mWidth(0), mHeight(0) {}

OSWindow::~OSWindow() {}

int OSWindow::getX() const
{
    return mX;
}

int OSWindow::getY() const
{
    return mY;
}
int OSWindow::getWidth() const
{
    return mWidth;
}

int OSWindow::getHeight() const
{
    return mHeight;
}

bool OSWindow::popEvent(Event *event)
{
    if (mEvents.size() > 0 && event)
    {
        *event = mEvents.front();
        mEvents.pop_front();
        return true;
    }
    else
    {
        return false;
    }
}

void OSWindow::pushEvent(Event event)
{
    switch (event.Type)
    {
        case Event::EVENT_MOVED:
            mX = event.Move.X;
            mY = event.Move.Y;
            break;
        case Event::EVENT_RESIZED:
            mWidth  = event.Size.Width;
            mHeight = event.Size.Height;
            break;
        default:
            break;
    }

    mEvents.push_back(event);

#if DEBUG_EVENTS
    PrintEvent(event);
#endif
}

// static
void OSWindow::Delete(OSWindow **window)
{
    delete *window;
    *window = nullptr;
}
