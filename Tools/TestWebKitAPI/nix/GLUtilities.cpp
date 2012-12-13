#include "config.h"

#include "GLUtilities.h"
#include <iostream>
#include <png.h>

using namespace std;
using namespace TestWebKitAPI::Util;

namespace TestWebKitAPI {
namespace Util {

void GLOffscreenBuffer::dumpToPng(const char* fileName)
{
    FILE* fp = fopen(fileName, "w+");
    if (!fp) {
        cerr << "Error writing " << fileName << endl;
        return;
    }

    png_structp pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    png_infop pngInfo = png_create_info_struct(pngPtr);

    if (!pngPtr || !pngInfo) {
        png_destroy_write_struct(&pngPtr, &pngInfo);
        cerr << "Error creating PNG write strucs\n";
        return;
    }

    png_init_io(pngPtr, fp);
    png_set_IHDR(pngPtr, pngInfo, m_width, m_height, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(pngPtr, pngInfo);

    const size_t imageDataSize = m_width * m_height * 4;
    unsigned char* imageData = new unsigned char[imageDataSize];
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE, imageData);

    // write each row separately.
    png_bytep row = 0;
    for (int y = 0; y < m_height; ++y) {
        // glReadPixels returns the image up side down, so we write last rows first.
        png_uint_32 q = imageDataSize - (y + 1) * m_width * 4;
        row = (png_bytep) imageData + q;
        png_write_row(pngPtr, row);
    }
    delete[] imageData;

    png_write_end(pngPtr, 0);
    png_destroy_write_struct(&pngPtr, &pngInfo);

    fclose(fp);
}

RGBAPixel GLOffscreenBuffer::readPixelAtPoint(unsigned x, unsigned y)
{
    RGBAPixel result = { 0, 0, 0, 0 };
    // Note: the two first arguments indicate the lower left corner of the rectangle being sampled
    // and glReadPixels considers the origin (0, 0) is at bottom-left of the buffer. For example,
    // to get the top-left pixel (a 1x1 rectangle to be sampled), we need to use (0, height - 1).
    glReadPixels(x, m_height - y - 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, reinterpret_cast<unsigned char*>(&result));
    return result;
}

} // namespace Util
} // namespace TestWebKitAPI
