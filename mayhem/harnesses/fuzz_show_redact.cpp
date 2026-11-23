// Show-folder XML redactor (src-core/utils/ShowRedactor.cpp): parses xlights_rgbeffects.xml /
// xlights_networks.xml (pugixml) and rewrites paths/addresses. First byte picks which file kind.
#include <cstdint>
#include "fuzz_common.h"
#include "utils/ShowRedactor.h"

extern "C" int LLVMFuzzerInitialize(int*, char***) { fuzz_init_scratch(); return 0; }

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size < 1) return -1;
    bool networks = data[0] & 1;
    std::string in = fuzz_write(networks ? "xlights_networks.xml" : "xlights_rgbeffects.xml", data + 1, size - 1);
    std::string out = g_scratch + "/redacted.xml";
    ShowRedactor::Stats st;
    ShowRedactor::RedactFileToFile(in, out, networks, &st);
    return 0;
}
