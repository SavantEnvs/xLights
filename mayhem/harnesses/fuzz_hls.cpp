// HLS .hlsIdata import (src-core/import_export/HLSFile.cpp): universes -> channels -> per-frame
// hex intensity rows, RGB-triple collapsing and CCR strand-name probing.
#include <cstdint>
#include "fuzz_common.h"
#include "import_export/HLSFile.h"

extern "C" int LLVMFuzzerInitialize(int*, char***) { fuzz_init_scratch(); return 0; }

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    pugi::xml_document doc;
    if (!doc.load_buffer(data, size)) return 0;
    HLSFile h(doc);
    for (const auto& n : h.GetChannelNames()) { (void)h.GetChannelColor(n); (void)h.IsCCRName(n); }
    for (const auto& c : h.GetCCRNames()) (void)h.FindStrandName(c, 1);
    return 0;
}
