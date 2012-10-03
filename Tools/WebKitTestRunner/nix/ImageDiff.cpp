/*
 * Copyright (C) 2009 Zan Dobersek <zandobersek@gmail.com>
 * Copyright (C) 2010 Igalia S.L.
 * Copyright (C) 2012 INdT - Instituto Nokia de Tecnologia
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

extern "C" {
#include <png.h>
}

using namespace std;

static double tolerance = 0;

class PixelBuf {
public:
    PixelBuf() : data(0) {}
    ~PixelBuf();

    void copyConfiguration(const PixelBuf&);

    bool hasAlpha() const { return colorType & PNG_COLOR_MASK_ALPHA; }

    int height;
    int width;
    int depth;
    int channels;
    int colorType;
    int interlaceType;
    int compressionType;
    int filterMethod;
    int stride;
    unsigned char *data;
};

PixelBuf::~PixelBuf()
{
    delete [] this->data;
}

void PixelBuf::copyConfiguration(const PixelBuf& other)
{
    width = other.width;
    height = other.height;
    channels = other.channels;
    depth = other.depth;
    stride = other.stride;
    colorType = other.colorType;
    interlaceType = other.interlaceType;
    compressionType = other.compressionType;
    filterMethod = other.filterMethod;
}

PixelBuf* differenceImageFromDifferenceBuffer(unsigned char* buffer, const PixelBuf* referenceImage)
{
    PixelBuf* image = new PixelBuf;
    if (!image)
        return 0;

    image->copyConfiguration(*referenceImage);

    image->data = new unsigned char[image->width * image->height * image->depth * image->channels / 8];

    unsigned char* diffPixels = image->data;

    for (int x = 0; x < image->width; x++) {
        for (int y = 0; y < image->height; y++) {
            unsigned char* diffPixel = diffPixels + (y * image->stride) + (x * image->channels);
            diffPixel[0] = diffPixel[1] = diffPixel[2] = *buffer++;
            if (referenceImage->hasAlpha())
                diffPixel[3] = 255;
        }
    }

    return image;
}

// Code below mostly from GTK's ImageDiff, changing from GtkPixelBuf to our PixelBuf.
float calculateDifference(const PixelBuf *baselineImage, const PixelBuf *actualImage, PixelBuf **differenceImage)
{
    int width = actualImage->width;
    int height = actualImage->height;
    int numberOfChannels = actualImage->channels;

    if ((width != baselineImage->width)
        || (height != baselineImage->height)
        || (numberOfChannels != baselineImage->channels)
        || (actualImage->hasAlpha() != baselineImage->hasAlpha())) { // FIXME Gtk also tests alpha
        fprintf(stderr, "Error, test and reference images have different properties.\n");
        return 100;
    }

    unsigned char* diffBuffer = new unsigned char[width*height];
    float count = 0;
    float sum = 0;
    float maxDistance = 0;
    int actualRowStride = actualImage->stride;
    int baseRowStride = baselineImage->stride;

    unsigned char* actualPixels = actualImage->data;
    unsigned char* basePixels = baselineImage->data;
    unsigned char* currentDiffPixel = diffBuffer;

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            unsigned char* actualPixel = actualPixels + (y * actualRowStride) + (x * numberOfChannels);
            unsigned char* basePixel = basePixels + (y * baseRowStride) + (x * numberOfChannels);

            float red = (actualPixel[0] - basePixel[0]) / max<float>(255 - basePixel[0], basePixel[0]);
            float green = (actualPixel[1] - basePixel[1]) / max<float>(255 - basePixel[1], basePixel[1]);
            float blue = (actualPixel[2] - basePixel[2]) / max<float>(255 - basePixel[2], basePixel[2]);
            float alpha = 0;
            if (actualImage->hasAlpha())
                alpha = (actualPixel[3] - basePixel[3]) / max<float>(255 - basePixel[3], basePixel[3]);
            float distance = sqrtf(red * red + green * green + blue * blue + alpha * alpha) / 2;
            *currentDiffPixel++ = (unsigned char)(distance * 255);

            if (distance >= 1 / 255) {
                count += 1;
                sum += distance;
                maxDistance = max<float>(maxDistance, distance);
            }
        }
    }

    // Compute the difference as a percentage combining both the number of
    // different pixels and their difference amount i.e. the average distance
    // over the entire image
    float difference = 0;
    if (count > 0)
        difference = 100 * sum / (height * width);
    if (difference <= tolerance)
        difference = 0;
    else {
        difference = roundf(difference * 100) / 100;
        difference = max(difference, 0.01f); // round to 2 decimal places
         *differenceImage = differenceImageFromDifferenceBuffer(diffBuffer, actualImage);
    }

    delete [] diffBuffer;
    return difference;
}

static int bytesToBeWritten = 0;

void userWriteData(png_structp pngPtr, png_bytep data, png_size_t length)
{
    png_voidp a = png_get_io_ptr(pngPtr);
    bytesToBeWritten += fwrite(data, 1, length, (FILE*)a);
}

int printImage(const PixelBuf* image)
{
    png_structp pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

    if (!pngPtr)
        return -1;

    png_infop pngInfo = png_create_info_struct(pngPtr);

    if (!pngInfo) {
        png_destroy_write_struct(&pngPtr, 0);
        return -1;
    }

    if (setjmp(png_jmpbuf(pngPtr))) {
        png_destroy_write_struct(&pngPtr, &pngInfo);
        return -1;
    }

    // We need to know the size of the png file *before* writing it to stdout due to the
    // Content-Length line, so use tmpfile as a buffer.
    FILE* bufferStream = tmpfile();

    png_set_write_fn(pngPtr, (png_voidp)bufferStream, userWriteData, 0);

    png_set_IHDR(pngPtr, pngInfo, image->width, image->height, image->depth, image->colorType, image->interlaceType,
                 image->compressionType, image->filterMethod);

    png_write_info(pngPtr, pngInfo);

    // Just like reading, write each row separately.
    png_bytep row = 0;
    for (int y = 0; y < image->height; y++) {
        png_uint_32 q = y*image->stride;
        row = (png_bytep) image->data + q;
        png_write_row(pngPtr, row);
    }

    png_write_end(pngPtr, 0);
    png_destroy_write_struct(&pngPtr, &pngInfo);

    // Actually output something.
    rewind(bufferStream);
    printf("Content-Length: %d\n", bytesToBeWritten);

    unsigned char buf[2048];
    while (bytesToBeWritten > 0) {
        int bytesRead = fread(buf, 1, 2048, bufferStream);
        int bytesActuallyWritten = fwrite(buf, 1, bytesRead, stdout);
        bytesToBeWritten -= bytesActuallyWritten;
    }

    fclose(bufferStream);
    bytesToBeWritten = 0;

    return 0;
}


void printImageDifferences(PixelBuf* baselineImage, PixelBuf* actualImage, float tolerance)
{
    PixelBuf* differenceImage = 0;
    float difference = calculateDifference(baselineImage, actualImage, &differenceImage);
    if (difference > tolerance) {
        if (differenceImage) {
            if (printImage(differenceImage)) {
                fprintf(stderr, "Error writing PNG data.\n");
                delete differenceImage;
                return;
            }
            delete differenceImage;
        }

        printf("diff: %01.2f%% failed\n", difference);

    } else
        printf("diff: %01.2f%% passed\n", difference);
}

// Actually imageSize is ignored and we trust libpng will load only the needed bytes.
// If somehow the incoming data is corrupt the comparison will fail as the next image load will fail anyway.
PixelBuf* readPixbufFromStdin(long imageSize)
{
    // PNG Initialization stuff
    png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

    if (!pngPtr)
        return 0;

    png_infop pngInfo = png_create_info_struct(pngPtr);

    if (!pngInfo) {
        png_destroy_read_struct(&pngPtr, 0, 0);
        return 0;
    }

    png_infop pngEnd = png_create_info_struct(pngPtr);

    if (!pngEnd) {
        png_destroy_read_struct(&pngPtr, &pngInfo, 0);
        return 0;
    }

    if (setjmp(png_jmpbuf(pngPtr))) {
        png_destroy_read_struct(&pngPtr, &pngInfo, &pngEnd);
        return 0;
    }

    png_init_io(pngPtr, stdin);

    png_read_info(pngPtr, pngInfo);

    PixelBuf* image = new PixelBuf();
    image->width = png_get_image_width(pngPtr, pngInfo);
    image->height = png_get_image_height(pngPtr, pngInfo);
    image->depth = png_get_bit_depth(pngPtr, pngInfo);
    image->channels = png_get_channels(pngPtr, pngInfo);
    image->colorType = png_get_color_type(pngPtr, pngInfo);
    image->interlaceType = png_get_interlace_type(pngPtr, pngInfo);
    image->compressionType = png_get_compression_type(pngPtr, pngInfo);
    image->filterMethod = png_get_filter_type(pngPtr, pngInfo);

    image->stride = image->width * image->depth * image->channels / 8;
    image->data = new unsigned char[image->height * image->stride];

    // png_read_image requires an array of row pointers to store the image data.
    png_bytep* rowPointers = new png_bytep[image->height];

    for (size_t i = 0; i < image->height; i++) {
        png_uint_32 q = (i) * image->stride;
        rowPointers[i] = (png_bytep) image->data + q;
    }

    png_read_image(pngPtr, rowPointers);

    delete [] (png_bytep)rowPointers;
    png_destroy_read_struct(&pngPtr, &pngInfo, &pngEnd);
    return image;
}

int main(int argc, char* argv[])
{
    PixelBuf* actualImage = 0;
    PixelBuf* baselineImage = 0;
    char buffer[2048];
    float tolerance = 0;

    for (int i = 1; i < argc-1; ++i) {
        if (!strcmp("--tolerance", argv[i]) || !strcmp("-t", argv[i])) {
            sscanf(argv[i+1], "%f", &tolerance);
            break;
        }
    }

    while (fgets(buffer, sizeof(buffer), stdin)) {
        // Convert the first newline into a NUL character so that strtok doesn't produce it.
        char* newLineCharacter = strchr(buffer, '\n');
        if (newLineCharacter)
            *newLineCharacter = '\0';

        if (!strncmp("Content-Length: ", buffer, 16)) {
            char* token = 0;
            token = strtok(buffer, " ");
            token = strtok(0, " ");
            if (!token) {
                printf("Error, image size must be specified..\n");
                return 1;
            }

            long imageSize = strtol(token, 0, 10);
            if (imageSize > 0 && !actualImage) {
                if (!(actualImage = readPixbufFromStdin(imageSize))) {
                    printf("Error, could not read actual image.\n");
                    return 1;
                }
            } else if (imageSize > 0 && !baselineImage) {
                if (!(baselineImage = readPixbufFromStdin(imageSize))) {
                    printf("Error, could not read baseline image.\n");
                    return 1;
                }
            } else {
                printf("Error, image size must be specified..\n");
                return 1;
            }
        }

        if (actualImage && baselineImage) {
            printImageDifferences(baselineImage, actualImage, tolerance);
            delete actualImage;
            delete baselineImage;
            actualImage = 0;
            baselineImage = 0;
        }

        fflush(stdout);
    }

    return 0;
}
