/*
 * Copyright (C) 2013 Nokia Corporation and/or its subsidiary(-ies).
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#include "DefaultWebThemeEngine.h"
#include "public/WebRect.h"
#include "public/WebSize.h"
#include <algorithm>
#include <cmath>
#include <cairo/cairo.h>

const double BG_COLOR1 = 0xF6/256.0;
const double BG_COLOR2 = 0xDE/256.0;
const double BORDER_COLOR = 0xA4/256.0;
const double BORDER_ONHOVER_COLOR = 0x7A/256.0;
const double CHECK_COLOR = 0x66/256.0;
const double TEXTFIELD_DARK_BORDER_COLOR = 0x9A/256.0;
const double TEXTFIELD_LIGHT_BORDER_COLOR = 0xEE/256.0;

const int MENULIST_BORDER = 5;
const int MENULIST_ARROW_SIZE = 6;

namespace WebKit {

static void gradientFill(cairo_t* cairo, double yStart, double yLength, bool inverted = false)
{
#if 0
    double gradStartColor = BG_COLOR1;
    double gradEndColor = BG_COLOR2;
    if (inverted)
        std::swap(gradStartColor, gradEndColor);

    cairo_pattern_t* gradient = cairo_pattern_create_linear(0, yStart, 0, yStart + yLength);
    cairo_pattern_add_color_stop_rgb(gradient, 0, gradStartColor, gradStartColor, gradStartColor);
    cairo_pattern_add_color_stop_rgb(gradient, 1, gradEndColor, gradEndColor, gradEndColor);
    cairo_set_source(cairo, gradient);
    cairo_fill(cairo);
    cairo_pattern_destroy(gradient);
#endif
}

static void setupBorder(cairo_t * cairo, WebThemeEngine::State state)
{
#if 0
    double borderColor = state == WebThemeEngine::StateHover ? BORDER_ONHOVER_COLOR : BORDER_COLOR;
    cairo_set_source_rgb(cairo, borderColor, borderColor, borderColor);
    cairo_set_line_width(cairo, 1);
#endif
}

void DefaultWebThemeEngine::paintButton(WebCanvas* canvas, State state, const WebRect& rect, const ButtonExtraParams&) const
{
#if 0
    cairo_save(canvas);
    setupBorder(canvas, state);
    // Cairo uses a coordinate system not based on pixel coordinates, so
    // we need to add  0.5 to x and y coord or the line will stay between
    // two pixels instead of in the middle of a pixel.
    cairo_rectangle(canvas, rect.x + 0.5, rect.y + 0.5, rect.width, rect.height);
    cairo_stroke_preserve(canvas);

    gradientFill(canvas, rect.y, rect.height, state == StatePressed);
    cairo_restore(canvas);
#endif
}

void DefaultWebThemeEngine::paintTextField(WebCanvas* canvas, State, const WebRect& rect) const
{
#if 0
    cairo_save(canvas);

    const double lineWidth = 2;
    const double correction = lineWidth / 2.0;

    cairo_set_line_width(canvas, lineWidth);
    cairo_set_source_rgb(canvas, TEXTFIELD_DARK_BORDER_COLOR, TEXTFIELD_DARK_BORDER_COLOR, TEXTFIELD_DARK_BORDER_COLOR);
    cairo_move_to(canvas, rect.x + correction, rect.y + correction + rect.height);
    cairo_rel_line_to(canvas, 0, -rect.height);
    cairo_rel_line_to(canvas, rect.width, 0);
    cairo_stroke(canvas);

    cairo_set_source_rgb(canvas, TEXTFIELD_LIGHT_BORDER_COLOR, TEXTFIELD_LIGHT_BORDER_COLOR, TEXTFIELD_LIGHT_BORDER_COLOR);
    cairo_move_to(canvas, rect.x + correction + rect.width, rect.y + correction);
    cairo_rel_line_to(canvas, 0, rect.height);
    cairo_rel_line_to(canvas, -rect.width, 0);
    cairo_stroke(canvas);

    cairo_restore(canvas);
#endif
}

void DefaultWebThemeEngine::paintTextArea(WebCanvas* canvas, State state, const WebRect& rect) const
{
    paintTextField(canvas, state, rect);
}

WebSize DefaultWebThemeEngine::getCheckboxSize() const
{
    return WebSize(13, 13);
}

void DefaultWebThemeEngine::paintCheckbox(WebCanvas* canvas, State state, const WebRect& rect, const ButtonExtraParams& param) const
{
#if 0
    cairo_save(canvas);
    setupBorder(canvas, state);
    cairo_rectangle(canvas, rect.x + 0.5, rect.y + 0.5, rect.width, rect.height);
    cairo_stroke_preserve(canvas);

    gradientFill(canvas, rect.y, rect.height, state == StatePressed);

    if (param.checked) {
        const double border = 3;
        cairo_set_line_width(canvas, 2);
        cairo_set_source_rgb(canvas, CHECK_COLOR, CHECK_COLOR, CHECK_COLOR);
        cairo_move_to(canvas, rect.x + 0.5 + border, rect.y + 0.5 + rect.height - border);
        cairo_rel_line_to(canvas, rect.width - border * 2, -rect.height + border * 2);
        cairo_move_to(canvas, rect.x + 0.5 + border, rect.y + 0.5 + border);
        cairo_rel_line_to(canvas, rect.width - border * 2, rect.height - border * 2);
        cairo_stroke(canvas);
    }

    cairo_restore(canvas);
#endif
}

WebSize DefaultWebThemeEngine::getRadioSize() const
{
    return WebSize(13, 13);
}

void DefaultWebThemeEngine::paintRadio(WebCanvas* canvas, State state, const WebRect& rect, const ButtonExtraParams& param) const
{
#if 0
    cairo_save(canvas);
    setupBorder(canvas, state);
    cairo_arc(canvas, rect.x + rect.width/2.0, rect.y + rect.height/2.0, rect.width/2.0, 0, 2 * M_PI);
    cairo_stroke_preserve(canvas);

    gradientFill(canvas, rect.y, rect.height);

    if (param.checked) {
        cairo_set_source_rgb(canvas, CHECK_COLOR, CHECK_COLOR, CHECK_COLOR);
        cairo_arc(canvas, rect.x + rect.width/2.0, rect.y + rect.height/2.0, rect.width/4.0, 0, 2 * M_PI);
        cairo_fill(canvas);
    }
    cairo_restore(canvas);
#endif
}

void DefaultWebThemeEngine::getMenuListPadding(int& paddingTop, int& paddingLeft, int& paddingBottom, int& paddingRight) const
{
    paddingTop = MENULIST_BORDER;
    paddingLeft = MENULIST_BORDER;
    paddingBottom = MENULIST_BORDER;
    paddingRight = 2 * MENULIST_BORDER + MENULIST_ARROW_SIZE;
}

void DefaultWebThemeEngine::paintMenuList(WebCanvas* canvas, State state, const WebRect& rect) const
{
#if 0
    cairo_save(canvas);
    setupBorder(canvas, state);
    cairo_rectangle(canvas, rect.x + 0.5, rect.y + 0.5, rect.width, rect.height);
    cairo_stroke_preserve(canvas);

    gradientFill(canvas, rect.y, rect.height, state == StatePressed);

    cairo_move_to(canvas, rect.x + rect.width - MENULIST_ARROW_SIZE - MENULIST_BORDER, rect.y + 1 + rect.height/2 - MENULIST_ARROW_SIZE/2);
    cairo_set_source_rgb(canvas, CHECK_COLOR, CHECK_COLOR, CHECK_COLOR);
    cairo_rel_line_to(canvas, MENULIST_ARROW_SIZE, 0);
    cairo_rel_line_to(canvas, -MENULIST_ARROW_SIZE/2, MENULIST_ARROW_SIZE);
    cairo_close_path(canvas);
    cairo_fill(canvas);

    cairo_restore(canvas);
#endif
}

}
