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
#include "GraphicsContext.h"

#include "AffineTransform.h"
#include "Color.h"
#include "FloatConversion.h"
#include "Font.h"
#include "ImageBuffer.h"
#include "NotImplemented.h"
#include "Path.h"
#include "Pattern.h"
#include "PlatformContextGL2D.h"
#include "TransformationMatrix.h"

#include <wtf/MathExtras.h>

namespace WebCore {

void GraphicsContext::platformInit(PlatformGraphicsContext* context)
{
    m_data = reinterpret_cast<GraphicsContextPlatformPrivate*>(context);

    setPaintingDisabled(!context);

    if (!context)
        return;
}

PlatformContextGL2D* GraphicsContext::platformContext() const
{
    return reinterpret_cast<PlatformGraphicsContext*>(m_data);
}

void GraphicsContext::platformDestroy()
{
//    while (!m_data->layers.isEmpty())
//      endTransparencyLayer();

    platformContext()->deref();
}

AffineTransform GraphicsContext::getCTM(IncludeDeviceScale) const
{
    return AffineTransform();
}

void GraphicsContext::savePlatformState()
{
    platformContext()->save();
}

void GraphicsContext::restorePlatformState()
{
    platformContext()->restore();
}

// Draws a filled rectangle with a stroked border.
// This is only used to draw borders (real fill is done via fillRect), and
// thus it must not cast any shadow.
void GraphicsContext::drawRect(const IntRect& rect)
{
}

// This is only used to draw borders.
// Must not cast any shadow.
void GraphicsContext::drawLine(const IntPoint& point1, const IntPoint& point2)
{
}

// This method is only used to draw the little circles used in lists.
void GraphicsContext::drawEllipse(const IntRect& rect)
{
}

void GraphicsContext::drawConvexPolygon(size_t npoints, const FloatPoint* points, bool shouldAntialias)
{
}

void GraphicsContext::clipConvexPolygon(size_t numPoints, const FloatPoint* points, bool antialiased)
{
}

void GraphicsContext::fillPath(const Path& path)
{
}

void GraphicsContext::strokePath(const Path& path)
{
}

void GraphicsContext::fillRect(const FloatRect& rect)
{
    fillRect(rect, fillColor(), ColorSpaceDeviceRGB);
}

void GraphicsContext::fillRect(const FloatRect& rect, const Color& color, ColorSpace colorSpace)
{
    UNUSED_PARAM(colorSpace);
    platformContext()->fillRect(rect, color);
}

void GraphicsContext::fillRoundedRect(const IntRect& rect, const IntSize& topLeft, const IntSize& topRight, const IntSize& bottomLeft, const IntSize& bottomRight, const Color& color, ColorSpace colorSpace)
{
}

void GraphicsContext::clip(const FloatRect& rect)
{
    platformContext()->clip(rect);
}

void GraphicsContext::clipPath(const Path& path, WindRule clipRule)
{
}

void GraphicsContext::setPlatformCompositeOperation(CompositeOperator op, BlendMode blendOp)
{
}

void GraphicsContext::clip(const Path& path, WindRule windRule)
{
}

void GraphicsContext::canvasClip(const Path& path, WindRule windRule)
{
    clip(path, windRule);
}


void GraphicsContext::drawFocusRing(const Path& path, int /* width */, int offset, const Color& color)
{
    // FIXME: Use 'offset' for something? http://webkit.org/b/49909

}

/**
 * Focus ring handling for form controls is not handled here. Qt style in
 * RenderTheme handles drawing focus on widgets which 
 * need it. It is still handled here for links.
 */
void GraphicsContext::drawFocusRing(const Vector<IntRect>& rects, int width, int offset, const Color& color)
{
}

void GraphicsContext::drawLineForText(const FloatPoint& origin, float width, bool)
{
}


/*
 *   NOTE: This code is completely based upon the one from
 *   Source/WebCore/platform/graphics/cairo/DrawErrorUnderline.{h|cpp}
 *
 *   Draws an error underline that looks like one of:
 *
 *               H       E                H
 *      /\      /\      /\        /\      /\               -
 *    A/  \    /  \    /  \     A/  \    /  \              |
 *     \   \  /    \  /   /D     \   \  /    \             |
 *      \   \/  C   \/   /        \   \/   C  \            | height = heightSquares * square
 *       \      /\  F   /          \  F   /\   \           |
 *        \    /  \    /            \    /  \   \G         |
 *         \  /    \  /              \  /    \  /          |
 *          \/      \/                \/      \/           -
 *          B                         B
 *          |---|
 *        unitWidth = (heightSquares - 1) * square
 *
 *  The x, y, width, height passed in give the desired bounding box;
 *  x/width are adjusted to make the underline a integer number of units wide.
*/
/*static void drawErrorUnderline(QPainter *painter, qreal x, qreal y, qreal width, qreal height)
{
}*/


void GraphicsContext::drawLineForDocumentMarker(const FloatPoint& origin, float width, DocumentMarkerLineStyle style)
{
}

FloatRect GraphicsContext::roundToDevicePixels(const FloatRect& frect, RoundingMode)
{
    // It is not enough just to round to pixels in device space. The rotation part of the
    // affine transform matrix to device space can mess with this conversion if we have a
    // rotating image like the hands of the world clock widget. We just need the scale, so
    // we get the affine transform matrix and extract the scale.

    return FloatRect(frect);
}

void GraphicsContext::setPlatformShadow(const FloatSize& size, float blur, const Color& color, ColorSpace colorSpace)
{
}

void GraphicsContext::clearPlatformShadow()
{
}

void GraphicsContext::beginPlatformTransparencyLayer(float opacity)
{
}

void GraphicsContext::endPlatformTransparencyLayer()
{
}

bool GraphicsContext::supportsTransparencyLayers()
{
// it was true!
    return false;
}

void GraphicsContext::clearRect(const FloatRect& rect)
{
}

void GraphicsContext::strokeRect(const FloatRect& rect, float lineWidth)
{
}

void GraphicsContext::setLineCap(LineCap lc)
{
}

void GraphicsContext::setLineDash(const DashArray& dashes, float dashOffset)
{
}

void GraphicsContext::setLineJoin(LineJoin lj)
{
}

void GraphicsContext::setMiterLimit(float limit)
{
}

void GraphicsContext::setAlpha(float opacity)
{
}

void GraphicsContext::clipOut(const Path& path)
{
}

void GraphicsContext::translate(float x, float y)
{
    platformContext()->translate(x, y);
}

void GraphicsContext::rotate(float radians)
{
}

void GraphicsContext::scale(const FloatSize& s)
{
}

void GraphicsContext::clipOut(const IntRect& rect)
{
}

void GraphicsContext::concatCTM(const AffineTransform& transform)
{
}

void GraphicsContext::setCTM(const AffineTransform& transform)
{
}

#if ENABLE(3D_RENDERING)
TransformationMatrix GraphicsContext::get3DTransform() const
{
    if (paintingDisabled())
        return TransformationMatrix();
    return TransformationMatrix();
    //return platformContext()->combinedTransform();
}

void GraphicsContext::concat3DTransform(const TransformationMatrix& transform)
{
}

void GraphicsContext::set3DTransform(const TransformationMatrix& transform)
{
}
#endif

void GraphicsContext::setURLForRect(const KURL&, const IntRect&)
{
}

void GraphicsContext::setPlatformStrokeColor(const Color& color, ColorSpace colorSpace)
{
}

void GraphicsContext::setPlatformStrokeThickness(float thickness)
{
}

void GraphicsContext::setPlatformFillColor(const Color& color, ColorSpace colorSpace)
{
}

void GraphicsContext::setPlatformShouldAntialias(bool enable)
{
}

void GraphicsContext::setImageInterpolationQuality(InterpolationQuality quality)
{
}

InterpolationQuality GraphicsContext::imageInterpolationQuality() const
{
    return InterpolationDefault;
}

} //namespace Webcore

// vim: ts=4 sw=4 et
