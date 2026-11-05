#include "texture.h"

#include "main_window.h"
#include "stb/stb_image.h"

texture load_texture_from_memory(const unsigned char *data, const size_t size) {
    texture result;

    auto dev = main_window::get_d3d11_device();
    if (!dev || !data || size == 0)
        return result;

    int channels = 0;
    unsigned char* imageData = stbi_load_from_memory(
        data, static_cast<int>(size), &result.width, &result.height, &channels, 4);

    if (imageData == nullptr)
        return result;

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = result.width;
    desc.Height = result.height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA subResource = {};
    subResource.pSysMem = imageData;
    subResource.SysMemPitch = result.width * 4;

    ID3D11Texture2D* tex = nullptr;
    if (SUCCEEDED(dev->CreateTexture2D(&desc, &subResource, &tex)) && tex)
    {
        if (FAILED(dev->CreateShaderResourceView(tex, nullptr, &result.src)))
            result.src = nullptr;
        tex->Release();
    }

    stbi_image_free(imageData);

    if (!result.src)
        result.width = result.height = 0;

    return result;
}
