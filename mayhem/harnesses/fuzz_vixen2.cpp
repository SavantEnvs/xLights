// Vixen 2 .vix import (src-core/import_export/Vixen2File.cpp): XML document with channel list /
// profile reference and base64-encoded per-channel event values.
#include <cstdint>
#include "fuzz_common.h"
#include "import_export/Vixen2File.h"

extern "C" int LLVMFuzzerInitialize(int*, char***) { fuzz_init_scratch(); return 0; }

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    pugi::xml_document doc;
    if (!doc.load_buffer(data, size)) return 0;
    Vixen2File v(doc, g_scratch, g_scratch);
    if (!v.IsValid()) return 0;
    for (const auto& n : v.GetChannelNames()) (void)v.GetChannelColor(n);
    (void)v.GetFrames();
    return 0;
}
