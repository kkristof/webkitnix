#ifndef DefaultWebThemeEngine_h
#define DefaultWebThemeEngine_h

#include "public/WebThemeEngine.h"

namespace WebKit {

class DefaultWebThemeEngine : public WebThemeEngine {
public:
    virtual void paintButton(WebCanvas*, State state, const WebRect&, const ButtonExtraParams&) const;
    virtual void paintTextField(WebCanvas*, State, const WebRect&) const;
    virtual void paintTextArea(WebCanvas*, State, const WebRect&) const;
    virtual WebSize getCheckboxSize() const;
    virtual void paintCheckbox(WebCanvas*, State, const WebRect&, const ButtonExtraParams&) const;
    virtual WebSize getRadioSize() const;
    virtual void paintRadio(WebCanvas*, State, const WebRect&, const ButtonExtraParams&) const;
    virtual void paintMenuList(WebCanvas *, State, const WebRect &) const;
};

}
#endif
