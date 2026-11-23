// Image decode (src-core/utils/xlImage.cpp over stb_image): still-image load used by the Pictures
// effect / embedded media, and the animated-GIF frame compositor (disposal, background, delays).
// No pre-filtering: every input reaches xlImage exactly as a user-supplied file would. A GIF whose
// logical-screen header declares a huge canvas makes LoadAnimatedGIFFromMemory allocate that canvas
// unchecked (xlImage.cpp) — that is an app finding, not something the harness hides.
#include <cstdint>
#include "fuzz_common.h"
#include "utils/xlImage.h"

extern "C" int LLVMFuzzerInitialize(int*, char***) { fuzz_init_scratch(); return 0; }

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    xlImage img;
    if (img.LoadFromMemory(data, size) && img.IsOk()) {
        (void)img.GetAlpha(img.GetWidth() - 1, img.GetHeight() - 1);
    }
    AnimatedImageData a = LoadAnimatedGIFFromMemory(data, size);
    (void)a.frames.size();
    return 0;
}
