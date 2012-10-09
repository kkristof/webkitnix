/*
 *  Copyright (C) 2008 INdT - Instituto Nokia de Tecnologia
 *  Copyright (C) 2009-2010 ProFUSION embedded systems
 *  Copyright (C) 2009-2010 Samsung Electronics
 *  Copyright (C) 2012 INdT - Instituto Nokia de Tecnologia
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"
#include "ClipboardNix.h"

#include "DataTransferItemList.h"
#include "Editor.h"
#include "FileList.h"
#include "NotImplemented.h"
#include <wtf/text/StringHash.h>

namespace WebCore {
PassRefPtr<Clipboard> Editor::newGeneralClipboard(ClipboardAccessPolicy policy, Frame*)
{
    return ClipboardNix::create(policy, Clipboard::CopyAndPaste);
}

PassRefPtr<Clipboard> Clipboard::create(ClipboardAccessPolicy, DragData*, Frame*)
{
    return 0;
}

ClipboardNix::ClipboardNix(ClipboardAccessPolicy policy, ClipboardType clipboardType)
    : Clipboard(policy, clipboardType)
{
    notImplemented();
}

ClipboardNix::~ClipboardNix()
{
    notImplemented();
}

void ClipboardNix::clearData(const String&)
{
    notImplemented();
}

void ClipboardNix::writePlainText(const WTF::String&)
{
    notImplemented();
}

void ClipboardNix::clearAllData()
{
    notImplemented();
}

String ClipboardNix::getData(const String&) const
{
    notImplemented();
    return String();
}

bool ClipboardNix::setData(const String&, const String&)
{
    notImplemented();
    return false;
}

ListHashSet<String> ClipboardNix::types() const
{
    notImplemented();
    return ListHashSet<String>();
}

PassRefPtr<FileList> ClipboardNix::files() const
{
    notImplemented();
    return 0;
}

IntPoint ClipboardNix::dragLocation() const
{
    notImplemented();
    return IntPoint(0, 0);
}

CachedImage* ClipboardNix::dragImage() const
{
    notImplemented();
    return 0;
}

void ClipboardNix::setDragImage(CachedImage*, const IntPoint&)
{
    notImplemented();
}

Node* ClipboardNix::dragImageElement()
{
    notImplemented();
    return 0;
}

void ClipboardNix::setDragImageElement(Node*, const IntPoint&)
{
    notImplemented();
}

DragImageRef ClipboardNix::createDragImage(IntPoint&) const
{
    notImplemented();
    return 0;
}

void ClipboardNix::declareAndWriteDragImage(Element*, const KURL&, const String&, Frame*)
{
    notImplemented();
}

void ClipboardNix::writeURL(const KURL&, const String&, Frame*)
{
    notImplemented();
}

void ClipboardNix::writeRange(Range*, Frame*)
{
    notImplemented();
}

bool ClipboardNix::hasData()
{
    notImplemented();
    return false;
}

#if ENABLE(DATA_TRANSFER_ITEMS)
PassRefPtr<DataTransferItemList> ClipboardNix::items()
{
    notImplemented();
    return 0;
}
#endif


}
