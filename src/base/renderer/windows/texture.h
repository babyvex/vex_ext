#pragma once

#include <cstddef>
#include <d3d11.h>
#include <dxgiformat.h>

struct texture {
    ID3D11ShaderResourceView* src = nullptr;
    int height = 0, width = 0;

    void release()
    {
        if (src)
        {
            src->Release();
            src = nullptr;
        }
        width = height = 0;
    }
};

texture load_texture_from_memory(const unsigned char* data, const size_t size);
