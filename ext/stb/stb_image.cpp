/*
* configure stb_image about
 * the image we will support
 */
#define STB_IMAGE_IMPLEMENTATION

#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#define STBI_NO_GIF
#define STBI_NO_PIC

#include "stb_image.h"

#ifdef __cplusplus
extern "C" {
#endif

    /*
     * exported function wrapper from the library
     */

    unsigned char *lottie_image_load(char const *filename, int *x,
                                     int *y, int *comp, int req_comp)
    {
        return stbi_load(filename, x, y, comp, req_comp);
    }

    unsigned char *lottie_image_load_from_data(const char *imageData,
                                               int len, int *x, int *y,
                                               int *comp, int req_comp)
    {
        const unsigned char *data = (const unsigned char *)imageData;
        return stbi_load_from_memory(data, len, x, y, comp, req_comp);
    }

    void lottie_image_free(unsigned char *data)
    {
        stbi_image_free(data);
    }

#ifdef __cplusplus
}
#endif