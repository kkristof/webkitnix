/*
 Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies)
 Copyright (C) 2012 Company 100, Inc.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Library General Public
 License as published by the Free Software Foundation; either
 version 2 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Library General Public License for more details.

 You should have received a copy of the GNU Library General Public License
 along with this library; see the file COPYING.LIB.  If not, write to
 the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include "WebCoordinatedSurface.h"

#if USE(COORDINATED_GRAPHICS)
#include "CoordinatedGraphicsArgumentCoders.h"
#include "GraphicsContext.h"
#include "WebCoreArgumentCoders.h"
#include <WebCore/GraphicsSurfaceToken.h>

#if USE(TEXTURE_MAPPER)
#include "TextureMapperGL.h"
#endif

#if USE(GL2D)
#include "PlatformContextGL2D.h"
#endif

// #include <stdio.h>
// #include <time.h>
// static void printFps()
// {
//     static int init = 1;
//     static clock_t start[3];
//     static int fps[2];
//     static double sum;
//     static clock_t printStart;
//     clock_t currentTime = clock();
// 
//     if (init) {
//         start[0] = start[1] = start[2] = printStart = currentTime;
//         fps[0] = fps[1] = fps[2] = 0;
//         sum = 0 /* fps[0] + fps[1] */;
//         init = 0;
//     }
// 
//     sum++;
// 
//     if (start[2] + (2 * CLOCKS_PER_SEC) < currentTime) {
//         sum -= fps[0] + fps[1];
//         fps[0] = fps[1];
//         fps[1] = sum;
//         start[0] = start[1];
//         start[1] = start[2];
// 
//         sum += fps[0];
//         start[2] += 2 * CLOCKS_PER_SEC;
//     }
// 
//     if (printStart + (CLOCKS_PER_SEC / 4) < currentTime)
//     {
//         printf("FPS: %.2lf   \r", sum / ((double)(currentTime - start[0]) / (double)CLOCKS_PER_SEC));
//         fflush(stdout);
//         printStart += CLOCKS_PER_SEC / 4;
//     }
// }

using namespace WebCore;

namespace WebKit {

WebCoordinatedSurface::Handle::Handle()
{
}

void WebCoordinatedSurface::Handle::encode(CoreIPC::ArgumentEncoder& encoder) const
{
    encoder << m_size << m_flags;
#if USE(GRAPHICS_SURFACE)
    encoder << m_graphicsSurfaceToken;
    if (m_graphicsSurfaceToken.isValid())
        return;
#endif
    encoder << m_bitmapHandle;
}

bool WebCoordinatedSurface::Handle::decode(CoreIPC::ArgumentDecoder& decoder, Handle& handle)
{
    if (!decoder.decode(handle.m_size))
        return false;
    if (!decoder.decode(handle.m_flags))
        return false;
#if USE(GRAPHICS_SURFACE)
    if (!decoder.decode(handle.m_graphicsSurfaceToken))
        return false;
    if (handle.m_graphicsSurfaceToken.isValid())
        return true;
#endif
    if (!decoder.decode(handle.m_bitmapHandle))
        return false;

    return true;
}

PassRefPtr<WebCoordinatedSurface> WebCoordinatedSurface::create(const IntSize& size, CoordinatedSurface::Flags flags)
{
    RefPtr<WebCoordinatedSurface> surface;
#if USE(GRAPHICS_SURFACE)
    surface = createWithSurface(size, flags);
#endif

    if (!surface)
        surface = create(size, flags, ShareableBitmap::createShareable(size, (flags & SupportsAlpha) ? ShareableBitmap::SupportsAlpha : ShareableBitmap::NoFlags));

    return surface.release();
}

#if USE(GRAPHICS_SURFACE)
PassRefPtr<WebCoordinatedSurface> WebCoordinatedSurface::createWithSurface(const IntSize& size, CoordinatedSurface::Flags flags)
{
    GraphicsSurface::Flags surfaceFlags =
        GraphicsSurface::SupportsSoftwareWrite
        | GraphicsSurface::SupportsCopyToTexture
        | GraphicsSurface::SupportsSharing;

    if (flags & SupportsAlpha)
        surfaceFlags |= GraphicsSurface::SupportsAlpha;

    // This might return null, if the system is unable to provide a new graphics surface.
    // In that case, this function would return null and allow falling back to ShareableBitmap.
    RefPtr<GraphicsSurface> surface = GraphicsSurface::create(size, surfaceFlags);
    if (!surface)
        return 0;

    ASSERT(surface);
    return adoptRef(new WebCoordinatedSurface(size, flags, surface.release()));
}
#endif

PassOwnPtr<WebCore::GraphicsContext> WebCoordinatedSurface::createGraphicsContext(const IntRect& rect)
{
#if USE(GRAPHICS_SURFACE)
    if (isBackedByGraphicsSurface())
        return m_graphicsSurface->beginPaint(rect, 0 /* Write without retaining pixels*/);
#endif

    ASSERT(m_bitmap);
    OwnPtr<GraphicsContext> graphicsContext = m_bitmap->createGraphicsContext();
//     printFps();
#if USE(GL2D)
    graphicsContext->platformContext()->setUpdateRect(rect);
#endif
    graphicsContext->clip(rect);
    graphicsContext->translate(rect.x(), rect.y());
    return graphicsContext.release();
}

PassRefPtr<WebCoordinatedSurface> WebCoordinatedSurface::create(const IntSize& size, CoordinatedSurface::Flags flags, PassRefPtr<ShareableBitmap> bitmap)
{
    return adoptRef(new WebCoordinatedSurface(size, flags, bitmap));
}

WebCoordinatedSurface::WebCoordinatedSurface(const IntSize& size, CoordinatedSurface::Flags flags, PassRefPtr<ShareableBitmap> bitmap)
    : m_size(size)
    , m_flags(flags)
    , m_bitmap(bitmap)
{
}

#if USE(GRAPHICS_SURFACE)
WebCoordinatedSurface::WebCoordinatedSurface(const WebCore::IntSize& size, CoordinatedSurface::Flags flags, PassRefPtr<WebCore::GraphicsSurface> surface)
    : m_size(size)
    , m_flags(flags)
    , m_graphicsSurface(surface)
{
}

PassRefPtr<WebCoordinatedSurface> WebCoordinatedSurface::create(const IntSize& size, CoordinatedSurface::Flags flags, PassRefPtr<GraphicsSurface> surface)
{
    return adoptRef(new WebCoordinatedSurface(size, flags, surface));
}
#endif

WebCoordinatedSurface::~WebCoordinatedSurface()
{
}

PassRefPtr<WebCoordinatedSurface> WebCoordinatedSurface::create(const Handle& handle)
{
#if USE(GRAPHICS_SURFACE)
    if (handle.graphicsSurfaceToken().isValid()) {
        GraphicsSurface::Flags surfaceFlags = 0;
        if (handle.m_flags & SupportsAlpha)
            surfaceFlags |= GraphicsSurface::SupportsAlpha;
        RefPtr<GraphicsSurface> surface = GraphicsSurface::create(handle.m_size, surfaceFlags, handle.m_graphicsSurfaceToken);
        if (surface)
            return adoptRef(new WebCoordinatedSurface(handle.m_size, handle.m_flags, PassRefPtr<GraphicsSurface>(surface)));
    }
#endif

    RefPtr<ShareableBitmap> bitmap = ShareableBitmap::create(handle.m_bitmapHandle);
    if (!bitmap)
        return 0;

    return create(handle.m_size, handle.m_flags, bitmap.release());
}

bool WebCoordinatedSurface::createHandle(Handle& handle)
{
    handle.m_size = m_size;
    handle.m_flags = m_flags;

#if USE(GRAPHICS_SURFACE)
    handle.m_graphicsSurfaceToken = m_graphicsSurface ? m_graphicsSurface->exportToken() : GraphicsSurfaceToken();
    if (handle.m_graphicsSurfaceToken.isValid())
        return true;
#endif
    if (!m_bitmap->createHandle(handle.m_bitmapHandle))
        return false;

    return true;
}

#if USE(TEXTURE_MAPPER)
void WebCoordinatedSurface::copyToTexture(PassRefPtr<WebCore::BitmapTexture> passTexture, const IntRect& target, const IntPoint& sourceOffset)
{
    RefPtr<BitmapTexture> texture(passTexture);

#if USE(GRAPHICS_SURFACE)
    if (isBackedByGraphicsSurface()) {
        RefPtr<BitmapTextureGL> textureGL = toBitmapTextureGL(texture.get());
        if (textureGL) {
            uint32_t textureID = textureGL->id();
            uint32_t textureTarget = textureGL->textureTarget();
            m_graphicsSurface->copyToGLTexture(textureTarget, textureID, target, sourceOffset);
            return;
        }

        RefPtr<Image> image = m_graphicsSurface->createReadOnlyImage(IntRect(sourceOffset, target.size()));
        texture->updateContents(image.get(), target, IntPoint::zero(), BitmapTexture::UpdateCanModifyOriginalImageData);
        return;
    }
#endif

    ASSERT(m_bitmap);
#if USE(GL2D)
    texture->updateContents(m_bitmap->rawImageData(), target, sourceOffset, m_bitmap->size().width() * 4, BitmapTexture::UpdateCanModifyOriginalImageData);
#else
    RefPtr<Image> image = m_bitmap->createImage();
    texture->updateContents(image.get(), target, sourceOffset, BitmapTexture::UpdateCanModifyOriginalImageData);
#endif
}
#endif // USE(TEXTURE_MAPPER)

} // namespace WebKit
#endif // USE(COORDINATED_GRAPHICS)
