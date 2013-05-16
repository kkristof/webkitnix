/*
 * Copyright (C) 2004, 2005, 2006 Apple Computer, Inc.  All rights reserved.
 * Copyright (C) 2007-2008 Torch Mobile, Inc.
 * Copyright (C) 2012 Company 100 Inc.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#ifndef NativeImagePtr_h
#define NativeImagePtr_h

#if PLATFORM(WX)
class wxBitmap;
class wxGraphicsBitmap;
#elif USE(CG)
typedef struct CGImage* CGImageRef;
#elif PLATFORM(QT)
#include "NativeImageQt.h"
#include <qglobal.h>
QT_BEGIN_NAMESPACE
class QPixmap;
QT_END_NAMESPACE
#elif USE(CAIRO)
#include "RefPtrCairo.h"
#elif USE(SKIA)
#include "NativeImageSkia.h"
namespace WebCore {
class NativeImageSkia;
}
#elif USE(GL2D)
#include "NativeImageGL2D.h"
#elif OS(WINCE)
#include "SharedBitmap.h"
#endif

namespace WTF {
class MemoryObjectInfo;
}

namespace WebCore {

// FIXME: NativeImagePtr and PassNativeImagePtr should be smart
// pointers (see SVGImage::nativeImageForCurrentFrame()).
#if USE(CG)
typedef CGImageRef NativeImagePtr;
#elif PLATFORM(QT)
typedef QPixmap* NativeImagePtr;
#elif PLATFORM(OPENVG)
class TiledImageOpenVG;
typedef TiledImageOpenVG* NativeImagePtr;
#elif PLATFORM(WX)
#if USE(WXGC)
typedef wxGraphicsBitmap* NativeImagePtr;
#else
typedef wxBitmap* NativeImagePtr;
#endif
#elif USE(CAIRO)
typedef RefPtr<cairo_surface_t> NativeImagePtr;
typedef PassRefPtr<cairo_surface_t> PassNativeImagePtr;
#elif USE(GL2D)
typedef WebCore::NativeImageGL2D* NativeImagePtr;
#elif USE(SKIA)
typedef RefPtr<NativeImageSkia> NativeImagePtr;
typedef PassRefPtr<NativeImageSkia> PassNativeImagePtr;
void reportMemoryUsage(const NativeImageSkia*, WTF::MemoryObjectInfo*);
#elif OS(WINCE)
typedef RefPtr<SharedBitmap> NativeImagePtr;
#elif PLATFORM(BLACKBERRY)
typedef void* NativeImagePtr;
#endif

#if !USE(SKIA) && !USE(CAIRO)
typedef NativeImagePtr PassNativeImagePtr;
#endif

}

#endif
