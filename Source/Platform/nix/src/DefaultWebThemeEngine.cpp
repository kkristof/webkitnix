#include "config.h"

#include "DefaultWebThemeEngine.h"
#include "public/WebRect.h"
#include "public/WebSize.h"
#include <cairo/cairo.h>

namespace WebKit {

static void notImplemented(WebCanvas* canvas, const WebRect& rect, bool blue = false)
{
    cairo_save(canvas);
    cairo_set_source_rgb(canvas, int(!blue), 0, int(blue));
    cairo_rectangle(canvas, rect.x, rect.y, rect.width, rect.height);
    cairo_fill(canvas);
    cairo_restore(canvas);
}

void DefaultWebThemeEngine::paintButton(WebCanvas* canvas, State state, const WebRect& rect, const ButtonExtraParams&) const
{
    notImplemented(canvas, rect);
}

void DefaultWebThemeEngine::paintTextField(WebCanvas* canvas, State state, const WebRect& rect) const
{
    notImplemented(canvas, rect);
}

void DefaultWebThemeEngine::paintTextArea(WebCanvas* canvas, State state, const WebRect& rect) const
{
    notImplemented(canvas, rect);
}

WebSize DefaultWebThemeEngine::getCheckboxSize() const
{
    return WebSize(13, 13);
}

void DefaultWebThemeEngine::paintCheckbox(WebCanvas* canvas, State state, const WebRect& rect, const ButtonExtraParams& param) const
{
    notImplemented(canvas, rect, param.checked);
}

WebSize DefaultWebThemeEngine::getRadioSize() const
{
    return WebSize(13, 13);
}

void DefaultWebThemeEngine::paintRadio(WebCanvas* canvas, State state, const WebRect& rect, const ButtonExtraParams& param) const
{
    notImplemented(canvas, rect, param.checked);
}

void DefaultWebThemeEngine::paintMenuList(WebCanvas* canvas, State, const WebRect& rect) const
{
    notImplemented(canvas, rect);
}

}
