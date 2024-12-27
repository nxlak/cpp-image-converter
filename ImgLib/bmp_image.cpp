#include "bmp_image.h"
#include "pack_defines.h"

#include <fstream>
#include <vector>
#include <cstdint>
#include <cassert>

using namespace std;

namespace img_lib {

PACKED_STRUCT_BEGIN BitmapFileHeader {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
}
PACKED_STRUCT_END

PACKED_STRUCT_BEGIN BitmapInfoHeader {
    uint32_t biSize;
    int32_t  biWidth;
    int32_t  biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
}
PACKED_STRUCT_END

static int GetBMPStride(int width) {
    return (width * 3 + 3) & ~3;
}

bool SaveBMP(const Path& file, const Image& image) {
    ofstream ofs(file, ios::binary);
    if (!ofs) {
        return false;
    }

    int width = image.GetWidth();
    int height = image.GetHeight();
    int stride = GetBMPStride(width);
    uint32_t pixel_data_size = stride * height;

    BitmapFileHeader file_header;
    file_header.bfType = 0x4D42;
    file_header.bfSize = static_cast<uint32_t>(sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader) + pixel_data_size);
    file_header.bfReserved1 = 0;
    file_header.bfReserved2 = 0;
    file_header.bfOffBits = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader);

    BitmapInfoHeader info_header;
    info_header.biSize = sizeof(BitmapInfoHeader);
    info_header.biWidth = width;
    info_header.biHeight = height;
    info_header.biPlanes = 1;
    info_header.biBitCount = 24;
    info_header.biCompression = 0;
    info_header.biSizeImage = pixel_data_size;
    info_header.biXPelsPerMeter = 11811;
    info_header.biYPelsPerMeter = 11811;
    info_header.biClrUsed = 0;
    info_header.biClrImportant = 0x1000000;

    ofs.write(reinterpret_cast<const char*>(&file_header), sizeof(file_header));
    ofs.write(reinterpret_cast<const char*>(&info_header), sizeof(info_header));

    if (!ofs) {
        return false;
    }

    vector<char> row_buffer(stride, 0);

    for (int y = height - 1; y >= 0; --y) {
        const Color* line = image.GetLine(y);
        for (int x = 0; x < width; ++x) {
            row_buffer[x * 3 + 0] = static_cast<char>(static_cast<uint8_t>(line[x].b));
            row_buffer[x * 3 + 1] = static_cast<char>(static_cast<uint8_t>(line[x].g));
            row_buffer[x * 3 + 2] = static_cast<char>(static_cast<uint8_t>(line[x].r));
        }

        ofs.write(row_buffer.data(), stride);
        if (!ofs) {
            return false;
        }
    }

    return ofs.good();
}

Image LoadBMP(const Path& file) {
    ifstream ifs(file, ios::binary);
    if (!ifs) {
        return {};
    }

    BitmapFileHeader file_header;
    if (!ifs.read(reinterpret_cast<char*>(&file_header), sizeof(file_header))) {
        return {};
    }

    if (file_header.bfType != 0x4D42) {
        return {};
    }

    BitmapInfoHeader info_header;
    if (!ifs.read(reinterpret_cast<char*>(&info_header), sizeof(info_header))) {
        return {};
    }

    if (info_header.biSize != sizeof(BitmapInfoHeader) ||
        info_header.biBitCount != 24 ||
        info_header.biCompression != 0 ||
        info_header.biWidth <= 0 ||
        info_header.biHeight <= 0) {
        return {};
    }

    int width = info_header.biWidth;
    int height = info_header.biHeight;
    int stride = GetBMPStride(width);

    if (!ifs.seekg(file_header.bfOffBits, ios::beg)) {
        return {};
    }

    vector<char> row_buffer(stride, 0);
    Image image(width, height, Color::Black());

    for (int y = height - 1; y >= 0; --y) {
        if (!ifs.read(row_buffer.data(), stride)) {
            return {};
        }

        Color* line = image.GetLine(y);
        for (int x = 0; x < width; ++x) {
            unsigned char blue  = static_cast<unsigned char>(row_buffer[x * 3 + 0]);
            unsigned char green = static_cast<unsigned char>(row_buffer[x * 3 + 1]);
            unsigned char red   = static_cast<unsigned char>(row_buffer[x * 3 + 2]);

            line[x].b = static_cast<byte>(blue);
            line[x].g = static_cast<byte>(green);
            line[x].r = static_cast<byte>(red);
            line[x].a = byte{255};
        }
    }

    return image;
}

}  // namespace img_lib