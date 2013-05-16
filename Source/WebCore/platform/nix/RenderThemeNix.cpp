/*
 * Copyright (C) 2012-2013 Nokia Corporation and/or its subsidiary(-ies).
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
#include "RenderThemeNix.h"

#include "PaintInfo.h"
// #include "PlatformContextCairo.h"
#include "public/WebCanvas.h"
#include "public/WebThemeEngine.h"
#include "public/WebRect.h"
#include "public/Platform.h"

namespace WebCore {

static const unsigned defaultButtonBackgroundColor = 0xffdddddd;

static void setSizeIfAuto(RenderStyle* style, const IntSize& size)
{
    if (style->width().isIntrinsicOrAuto())
        style->setWidth(Length(size.width(), Fixed));
    if (style->height().isAuto())
        style->setHeight(Length(size.height(), Fixed));
}


PassRefPtr<RenderTheme> RenderTheme::themeForPage(Page*)
{
    return RenderThemeNix::create();
}

PassRefPtr<RenderTheme> RenderThemeNix::create()
{
    return adoptRef(new RenderThemeNix);
}

RenderThemeNix::RenderThemeNix()
    : RenderTheme()
{

}

RenderThemeNix::~RenderThemeNix()
{

}

void RenderThemeNix::systemFont(int, FontDescription&) const
{
}

static WebKit::WebThemeEngine::State getWebThemeState(const RenderTheme* theme, const RenderObject* o)
{
    if (!theme->isEnabled(o))
        return WebKit::WebThemeEngine::StateDisabled;
    if (theme->isPressed(o))
        return WebKit::WebThemeEngine::StatePressed;
    if (theme->isHovered(o))
        return WebKit::WebThemeEngine::StateHover;

    return WebKit::WebThemeEngine::StateNormal;
}

bool RenderThemeNix::paintButton(RenderObject* o, const PaintInfo& i, const IntRect& rect)
{
#if 0
    WebKit::WebThemeEngine::ButtonExtraParams extraParams;
    extraParams.isDefault = isDefault(o);
    extraParams.hasBorder = true;
    extraParams.backgroundColor = defaultButtonBackgroundColor;
    if (o->hasBackground())
        extraParams.backgroundColor = o->style()->visitedDependentColor(CSSPropertyBackgroundColor).rgb();

    WebKit::WebCanvas* canvas = i.context->platformContext()->cr();
    WebKit::WebThemeEngine* themeEngine = WebKit::Platform::current()->themeEngine();
    themeEngine->paintButton(canvas, getWebThemeState(this, o), WebKit::WebRect(rect), extraParams);
#endif
    return false;
}

bool RenderThemeNix::paintTextField(RenderObject* o, const PaintInfo& i, const IntRect& rect)
{
#if 0
    // WebThemeEngine does not handle border rounded corner and background image
    // so return true to draw CSS border and background.
    if (o->style()->hasBorderRadius() || o->style()->hasBackgroundImage())
        return true;

    WebKit::WebCanvas* canvas = i.context->platformContext()->cr();
    WebKit::WebThemeEngine* themeEngine = WebKit::Platform::current()->themeEngine();
    themeEngine->paintTextField(canvas, getWebThemeState(this, o), WebKit::WebRect(rect));
#endif
    return false;
}

bool RenderThemeNix::paintTextArea(RenderObject* o, const PaintInfo& i, const IntRect& rect)
{
    return paintTextField(o, i, rect);
}

bool RenderThemeNix::paintCheckbox(RenderObject* o, const PaintInfo& i, const IntRect& rect)
{
#if 0
    WebKit::WebThemeEngine::ButtonExtraParams extraParams;
    extraParams.checked = isChecked(o);
    extraParams.indeterminate = isIndeterminate(o);

    WebKit::WebCanvas* canvas = i.context->platformContext()->cr();
    WebKit::WebThemeEngine* themeEngine = WebKit::Platform::current()->themeEngine();
    themeEngine->paintCheckbox(canvas, getWebThemeState(this, o), WebKit::WebRect(rect), extraParams);
#endif
    return false;
}

void RenderThemeNix::setCheckboxSize(RenderStyle* style) const
{
    // If the width and height are both specified, then we have nothing to do.
    if (!style->width().isIntrinsicOrAuto() && !style->height().isAuto())
        return;

    IntSize size = WebKit::Platform::current()->themeEngine()->getCheckboxSize();
    setSizeIfAuto(style, size);
}

bool RenderThemeNix::paintRadio(RenderObject* o, const PaintInfo& i, const IntRect& rect)
{
#if 0
    WebKit::WebThemeEngine::ButtonExtraParams extraParams;
    extraParams.checked = isChecked(o);
    extraParams.indeterminate = isIndeterminate(o);

    WebKit::WebCanvas* canvas = i.context->platformContext()->cr();
    WebKit::WebThemeEngine* themeEngine = WebKit::Platform::current()->themeEngine();
    themeEngine->paintRadio(canvas, getWebThemeState(this, o), WebKit::WebRect(rect), extraParams);
#endif
    return false;
}

void RenderThemeNix::setRadioSize(RenderStyle* style) const
{
    // If the width and height are both specified, then we have nothing to do.
    if (!style->width().isIntrinsicOrAuto() && !style->height().isAuto())
        return;

    IntSize size = WebKit::Platform::current()->themeEngine()->getRadioSize();
    setSizeIfAuto(style, size);
}

bool RenderThemeNix::paintMenuList(RenderObject* o, const PaintInfo& i, const IntRect& rect)
{
#if 0
    WebKit::WebCanvas* canvas = i.context->platformContext()->cr();
    WebKit::WebThemeEngine* themeEngine = WebKit::Platform::current()->themeEngine();
    themeEngine->paintMenuList(canvas, getWebThemeState(this, o), WebKit::WebRect(rect));
#endif
    return false;
}

void RenderThemeNix::adjustMenuListStyle(StyleResolver*, RenderStyle* style, Element*) const
{
    style->resetBorder();
    style->setWhiteSpace(PRE);

    WebKit::WebThemeEngine* themeEngine = WebKit::Platform::current()->themeEngine();
    int paddingTop = 0;
    int paddingLeft = 0;
    int paddingBottom = 0;
    int paddingRight = 0;
    themeEngine->getMenuListPadding(paddingTop, paddingLeft, paddingBottom, paddingRight);
    style->setPaddingTop(Length(paddingTop, Fixed));
    style->setPaddingRight(Length(paddingRight, Fixed));
    style->setPaddingBottom(Length(paddingBottom, Fixed));
    style->setPaddingLeft(Length(paddingLeft, Fixed));
}

}
