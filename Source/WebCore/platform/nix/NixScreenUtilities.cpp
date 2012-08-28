/*
 * Copyright (C) 2012 INdT - Instituto Nokia de Tecnologia
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
#include "EflScreenUtilities.h"

#include <wtf/HashMap.h>
#include <wtf/text/StringHash.h>

// FIXME: REMOVE THIS FILE AS SOON AS WEBKIT IS FIXED
namespace WebCore {

void applyFallbackCursor(Ecore_Evas* ecoreEvas, const char* cursorString)
{
    return;
}

bool isUsingEcoreX(const Evas* evas)
{
    return false;
}

} // namespace WebCore
