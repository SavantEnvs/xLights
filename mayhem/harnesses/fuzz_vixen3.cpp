// Vixen 3 sequence import (src-core/import_export/Vixen3.cpp): reads a .tim sequence XML (plus an
// SystemConfig.xml beside it), extracts timing marks, models-with-effects and per-model effect settings.
#include <cstdint>
#include "fuzz_common.h"
#include "import_export/Vixen3.h"
#include <cstring>
#include <string_view>

// Input = <sequence .tim bytes> [ "<!--SYSCONFIG-->" <SystemConfig.xml bytes> ]
static const std::string_view kSep = "<!--SYSCONFIG-->";

extern "C" int LLVMFuzzerInitialize(int*, char***) { fuzz_init_scratch(); return 0; }

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    std::string_view in(reinterpret_cast<const char*>(data), size);
    size_t cut = in.find(kSep);
    std::string sys = g_scratch + "/SystemConfig.xml";
    if (cut == std::string_view::npos) {
        unlink(sys.c_str());
        cut = size;
    } else {
        size_t off = cut + kSep.size();
        fuzz_write("SystemConfig.xml", data + off, size - off);
    }
    std::string p = fuzz_write("input.tim", data, cut);
    Vixen3 v(p);
    for (const auto& t : v.GetTimings()) {
        (void)v.GetTimingType(t);
        for (const auto& m : v.GetTimings(t)) (void)Vixen3::ConvertTiming(m.start, 50);
    }
    for (const auto& m : v.GetModelsWithEffects()) {
        for (const auto& e : v.GetEffects(m.first)) {
            (void)e.GetPalette();
            (void)e.GetSettings();
            (void)e.GetXLightsType();
        }
    }
    return 0;
}
