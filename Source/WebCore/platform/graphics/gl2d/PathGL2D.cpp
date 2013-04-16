/*
 * Copyright (C) 2013 University of Szeged
 * Copyright (C) 2013 Kristof Kosztyo
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY UNIVERSITY OF SZEGED ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL UNIVERSITY OF SZEGED OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "Path.h"

#include "FloatRect.h"
#include "FloatPoint.h"
#include "NotImplemented.h"

namespace WebCore{

Path::Path()
    : m_path(0)
{
}

Path::Path(const Path& other)
{

}

Path::~Path()
{

}

Path& Path::operator=(const Path& other)
{
    notImplemented();
}

bool Path::isEmpty() const
{
    notImplemented();
}

bool Path::contains(const FloatPoint& point, WindRule rule) const
{
    notImplemented();
}

bool Path::hasCurrentPoint() const
{
    notImplemented();
}

FloatPoint Path::currentPoint() const
{
    notImplemented();
}

bool Path::strokeContains(StrokeStyleApplier* applier, const FloatPoint& point) const
{

}

FloatRect Path::boundingRect() const
{
    notImplemented();
}

FloatRect Path::strokeBoundingRect(StrokeStyleApplier* applier) const
{
    notImplemented();
}

void Path::moveTo(const FloatPoint& point)
{

}

void Path::addRect(const FloatRect& rect)
{

}

void Path::addLineTo(const FloatPoint& point)
{

}

void Path::addQuadCurveTo(const FloatPoint& controlPoint, const FloatPoint& point)
{

}

void Path::addBezierCurveTo(const FloatPoint& p1, const FloatPoint& p2, const FloatPoint& ep)
{

}

void Path::addEllipse(const FloatRect& rect)
{

}

void Path::addArc(const FloatPoint& p, float r, float sa, float ea, bool anticlockwise)
{

}

void Path::addArcTo(const FloatPoint& p1, const FloatPoint& p2, float radius)
{

}

void Path::apply(void* info, PathApplierFunction function) const
{

}

void Path::closeSubpath()
{

}

void Path::clear()
{

}

void Path::transform(const AffineTransform& xform)
{
    notImplemented();
}

void Path::translate(const FloatSize& p)
{

}


}
