/*
 * Copyright (C) 2013-2013 Nokia Corporation and/or its subsidiary(-ies).
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
#ifndef WebFFTFrame_h
#define WebFFTFrame_h

namespace WebKit {

class WebAudioBus;

class WebFFTFrame {
public:
    virtual ~WebFFTFrame() { }

    virtual void doFFT(const float* data) { }
    virtual void doInverseFFT(float* data) { }
    virtual void multiply(const WebFFTFrame& frame) { }

    virtual unsigned frequencyDomainSampleCount() const { return 0; }
    // After multiplication and transform operations, the data is scaled
    // to take in account the scale used internally in WebKit, originally
    // from Mac's vecLib.
    // After multiplication: Planar data is scaled by 0.5.
    // After direct transform: Planar data is scaled by 2.0.
    // After inverse transform: Time domain data is scaled by 1.0/(2* FFT size).
    virtual float* realData() const { return 0; }
    virtual float* imagData() const { return 0; }
};

} // namespace WebKit

#endif
