/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PDFPluginTextAnnotation_h
#define PDFPluginTextAnnotation_h

#if ENABLE(PDFKIT_PLUGIN)

#include "PDFPluginAnnotation.h"

namespace WebCore {
class Element;
}

OBJC_CLASS PDFAnnotationTextWidget;

namespace WebKit {

class PDFPluginTextAnnotation : public PDFPluginAnnotation {
public:
    static PassRefPtr<PDFPluginTextAnnotation> create(PDFAnnotation *, PDFLayerController *, PDFPlugin*);
    virtual ~PDFPluginTextAnnotation();

    virtual void updateGeometry() OVERRIDE;
    virtual void commit() OVERRIDE;

private:
    PDFPluginTextAnnotation(PDFAnnotation *annotation, PDFLayerController *pdfLayerController, PDFPlugin* plugin)
        : PDFPluginAnnotation(annotation, pdfLayerController, plugin)
        , m_eventListener(PDFPluginTextAnnotationEventListener::create(this))
    {
    }

    class PDFPluginTextAnnotationEventListener : public WebCore::EventListener {
    public:
        static PassRefPtr<PDFPluginTextAnnotationEventListener> create(PDFPluginTextAnnotation* annotation)
        {
            return adoptRef(new PDFPluginTextAnnotationEventListener(annotation));
        }

        virtual bool operator==(const EventListener& listener) OVERRIDE { return this == &listener; }

        void setTextAnnotation(PDFPluginTextAnnotation* annotation) { m_annotation = annotation; }

    private:
        PDFPluginTextAnnotationEventListener(PDFPluginTextAnnotation* annotation)
            : WebCore::EventListener(WebCore::EventListener::CPPEventListenerType)
            , m_annotation(annotation)
        {
        }

        virtual void handleEvent(WebCore::ScriptExecutionContext*, WebCore::Event*) OVERRIDE;

        PDFPluginTextAnnotation* m_annotation;
    };

    virtual PassRefPtr<WebCore::Element> createAnnotationElement() OVERRIDE;

    PDFAnnotationTextWidget * textAnnotation() { return static_cast<PDFAnnotationTextWidget *>(annotation()); }

    RefPtr<PDFPluginTextAnnotationEventListener> m_eventListener;
};

} // namespace WebKit

#endif // ENABLE(PDFKIT_PLUGIN)

#endif // PDFPluginTextAnnotation_h
