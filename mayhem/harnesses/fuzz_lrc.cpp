// LRC synced-lyrics parser (src-core/lyrics/LRCParser.cpp): lyric files / LRCLIB responses the
// lyric import turns into timing tracks.
#include <cstdint>
#include <string>
#include "fuzz_common.h"
#include "lyrics/LRCParser.h"

extern "C" int LLVMFuzzerInitialize(int*, char***) { spdlog::set_level(spdlog::level::off); return 0; }

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    std::string s(reinterpret_cast<const char*>(data), size);
    auto lines = lrc::ParseLRC(s);
    for (auto& l : lines) {
        std::string t = l.second;
        lrc::SanitizePhraseText(t);
    }
    std::string whole = s;
    lrc::SanitizePhraseText(whole);
    return 0;
}
