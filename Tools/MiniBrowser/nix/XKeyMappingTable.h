/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef XKeyMappingTable_h
#define XKeyMappingTable_h

#include "NixEvents.h"

using namespace Nix;

static const unsigned XKeySymMappingTable[] = {

    // misc keys

    XK_Escape,                  KeyEvent::Key_Escape,
    XK_Tab,                     KeyEvent::Key_Tab,
    XK_ISO_Left_Tab,            KeyEvent::Key_Backtab,
    XK_BackSpace,               KeyEvent::Key_Backspace,
    XK_Return,                  KeyEvent::Key_Return,
    XK_Insert,                  KeyEvent::Key_Insert,
    XK_Delete,                  KeyEvent::Key_Delete,
    XK_Clear,                   KeyEvent::Key_Delete,
    XK_Pause,                   KeyEvent::Key_Pause,
    XK_Print,                   KeyEvent::Key_Print,
    0x1005FF60,                 KeyEvent::Key_SysReq,         // hardcoded Sun SysReq
    0x1007ff00,                 KeyEvent::Key_SysReq,         // hardcoded X386 SysReq

    // cursor movement

    XK_Home,                    KeyEvent::Key_Home,
    XK_End,                     KeyEvent::Key_End,
    XK_Left,                    KeyEvent::Key_Left,
    XK_Up,                      KeyEvent::Key_Up,
    XK_Right,                   KeyEvent::Key_Right,
    XK_Down,                    KeyEvent::Key_Down,
    XK_Prior,                   KeyEvent::Key_PageUp,
    XK_Next,                    KeyEvent::Key_PageDown,

    // modifiers

    XK_Shift_L,                 KeyEvent::Key_Shift,
    XK_Shift_R,                 KeyEvent::Key_Shift,
    XK_Shift_Lock,              KeyEvent::Key_Shift,
    XK_Control_L,               KeyEvent::Key_Control,
    XK_Control_R,               KeyEvent::Key_Control,
    XK_Meta_L,                  KeyEvent::Key_Meta,
    XK_Meta_R,                  KeyEvent::Key_Meta,
    XK_Alt_L,                   KeyEvent::Key_Alt,
    XK_Alt_R,                   KeyEvent::Key_Alt,
    XK_Caps_Lock,               KeyEvent::Key_CapsLock,
    XK_Num_Lock,                KeyEvent::Key_NumLock,
    XK_Scroll_Lock,             KeyEvent::Key_ScrollLock,
    XK_Super_L,                 KeyEvent::Key_Super_L,
    XK_Super_R,                 KeyEvent::Key_Super_R,
    XK_Menu,                    KeyEvent::Key_Menu,
    XK_Hyper_L,                 KeyEvent::Key_Hyper_L,
    XK_Hyper_R,                 KeyEvent::Key_Hyper_R,
    XK_Help,                    KeyEvent::Key_Help,
    0x1000FF74,                 KeyEvent::Key_Backtab,        // hardcoded HP backtab
    0x1005FF10,                 KeyEvent::Key_F11,            // hardcoded Sun F36 (labeled F11)
    0x1005FF11,                 KeyEvent::Key_F12,            // hardcoded Sun F37 (labeled F12)

    // numeric and function keypad keys

    XK_KP_Space,                KeyEvent::Key_Space,
    XK_KP_Tab,                  KeyEvent::Key_Tab,
    XK_KP_Enter,                KeyEvent::Key_Enter,
    XK_KP_Home,                 KeyEvent::Key_Home,
    XK_KP_Left,                 KeyEvent::Key_Left,
    XK_KP_Up,                   KeyEvent::Key_Up,
    XK_KP_Right,                KeyEvent::Key_Right,
    XK_KP_Down,                 KeyEvent::Key_Down,
    XK_KP_Prior,                KeyEvent::Key_PageUp,
    XK_KP_Next,                 KeyEvent::Key_PageDown,
    XK_KP_End,                  KeyEvent::Key_End,
    XK_KP_Begin,                KeyEvent::Key_Clear,
    XK_KP_Insert,               KeyEvent::Key_Insert,
    XK_KP_Delete,               KeyEvent::Key_Delete,
    XK_KP_Equal,                KeyEvent::Key_Equal,
    XK_KP_Multiply,             KeyEvent::Key_Asterisk,
    XK_KP_Add,                  KeyEvent::Key_Plus,
    XK_KP_Separator,            KeyEvent::Key_Comma,
    XK_KP_Subtract,             KeyEvent::Key_Minus,
    XK_KP_Decimal,              KeyEvent::Key_Period,
    XK_KP_Divide,               KeyEvent::Key_Slash,

    // International input method support keys

    // International & multi-key character composition
    XK_ISO_Level3_Shift,        KeyEvent::Key_AltGr,
    XK_Multi_key,		KeyEvent::Key_Multi_key,
    XK_Codeinput,		KeyEvent::Key_Codeinput,
    XK_SingleCandidate,		KeyEvent::Key_SingleCandidate,
    XK_MultipleCandidate,	KeyEvent::Key_MultipleCandidate,
    XK_PreviousCandidate,	KeyEvent::Key_PreviousCandidate,

    // Misc Functions
    XK_Mode_switch,		KeyEvent::Key_Mode_switch,
    XK_script_switch,		KeyEvent::Key_Mode_switch,

    // dead keys
    XK_dead_grave,              KeyEvent::Key_Dead_Grave,
    XK_dead_acute,              KeyEvent::Key_Dead_Acute,
    XK_dead_circumflex,         KeyEvent::Key_Dead_Circumflex,
    XK_dead_tilde,              KeyEvent::Key_Dead_Tilde,
    XK_dead_macron,             KeyEvent::Key_Dead_Macron,
    XK_dead_breve,              KeyEvent::Key_Dead_Breve,
    XK_dead_abovedot,           KeyEvent::Key_Dead_Abovedot,
    XK_dead_diaeresis,          KeyEvent::Key_Dead_Diaeresis,
    XK_dead_abovering,          KeyEvent::Key_Dead_Abovering,
    XK_dead_doubleacute,        KeyEvent::Key_Dead_Doubleacute,
    XK_dead_caron,              KeyEvent::Key_Dead_Caron,
    XK_dead_cedilla,            KeyEvent::Key_Dead_Cedilla,
    XK_dead_ogonek,             KeyEvent::Key_Dead_Ogonek,
    XK_dead_iota,               KeyEvent::Key_Dead_Iota,
    XK_dead_voiced_sound,       KeyEvent::Key_Dead_Voiced_Sound,
    XK_dead_semivoiced_sound,   KeyEvent::Key_Dead_Semivoiced_Sound,
    XK_dead_belowdot,           KeyEvent::Key_Dead_Belowdot,
    XK_dead_hook,               KeyEvent::Key_Dead_Hook,
    XK_dead_horn,               KeyEvent::Key_Dead_Horn,

    0,                          0
};

#endif
