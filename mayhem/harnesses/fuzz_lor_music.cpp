// Light-O-Rama .lms/.las sequence import (src-core/import_export/LORMusic.cpp): channel /
// rgbChannel / CCR indexing, colour decoding, timing grids and timing marks.
#include <cstdint>
#include "fuzz_common.h"
#include "import_export/LORMusic.h"

extern "C" int LLVMFuzzerInitialize(int*, char***) { fuzz_init_scratch(); return 0; }

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    pugi::xml_document doc;
    if (!doc.load_buffer(data, size)) return 0;
    LORMusic m(doc, 20);
    for (const auto& n : m.GetChannelNames()) (void)m.GetChannelColor(n);
    for (const auto& c : m.GetCCRNames()) { (void)m.IsCCRName(c); (void)m.ResolveCCRNodeName(c, 0); }
    for (const auto& t : m.GetTimingTracks()) (void)m.GetTimings(t, 0);
    for (auto ch : doc.select_nodes("//channel")) {
        (void)LORMusic::LongChannelName(ch.node());
        (void)LORMusic::MatchesChannelName(ch.node(), "x");
    }
    return 0;
}
