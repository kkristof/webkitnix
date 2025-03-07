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

#ifndef LayerTreeContext_h
#define LayerTreeContext_h

#include <stdint.h>

namespace CoreIPC {
    class ArgumentDecoder;
    class ArgumentEncoder;
}

namespace WebKit {

enum LayerHostingMode {
    LayerHostingModeDefault,
#if HAVE(LAYER_HOSTING_IN_WINDOW_SERVER)
    LayerHostingModeInWindowServer
#endif
};

class LayerTreeContext {
public:
    LayerTreeContext();
    ~LayerTreeContext();

    void encode(CoreIPC::ArgumentEncoder&) const;
    static bool decode(CoreIPC::ArgumentDecoder&, LayerTreeContext&);

    bool isEmpty() const;

#if PLATFORM(MAC)
    uint32_t contextID;
#elif PLATFORM(QT) || PLATFORM(EFL) || PLATFORM(NIX)
    uint32_t coordinatedLayerID;
#elif PLATFORM(GTK)
    uint64_t windowHandle;
#endif
};

bool operator==(const LayerTreeContext&, const LayerTreeContext&);

inline bool operator!=(const LayerTreeContext& a, const LayerTreeContext& b)
{
    return !(a == b);
}

};

#endif // LayerTreeContext_h
