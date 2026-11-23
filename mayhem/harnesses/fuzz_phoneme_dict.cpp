// Phoneme dictionary loader (src-core/lyrics/PhonemeDictionary.cpp): standard/extended/user
// dictionaries and the phoneme mapping file from the show/user folder, then word breakdown.
// The input is used as the standard dictionary; the bytes after the first "\n%%\n" (if any) are the
// phoneme mapping file (otherwise the whole input is used for both).
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include "fuzz_common.h"
#include "lyrics/PhonemeDictionary.h"

extern "C" int LLVMFuzzerInitialize(int*, char***) {
    fuzz_init_scratch();
    fuzz_write("extended_dictionary", nullptr, 0);   // read-only across inputs
    fuzz_write("user_dictionary", nullptr, 0);
    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    std::string s(reinterpret_cast<const char*>(data), size);
    std::string dict = s, map = s;
    size_t sep = s.find("\n%%\n");
    if (sep != std::string::npos) { dict = s.substr(0, sep + 1); map = s.substr(sep + 4); }
    fuzz_write("standard_dictionary", (const uint8_t*)dict.data(), dict.size());
    fuzz_write("phoneme_mapping", (const uint8_t*)map.data(), map.size());
    PhonemeDictionary d;
    d.LoadDictionaries({g_scratch});
    std::vector<std::string> out;
    // Break down the first few words of the dictionary itself plus fixed words.
    std::string text = dict.substr(0, 256);
    PhonemeDictionary::InsertSpacesAfterPunctuation(text);
    size_t pos = 0; int n = 0;
    while (pos < text.size() && n < 16) {
        size_t e = text.find_first_of(" \t\r\n", pos);
        std::string w = text.substr(pos, e == std::string::npos ? std::string::npos : e - pos);
        if (!w.empty()) { out.clear(); d.BreakdownWord(w, out); n++; }
        if (e == std::string::npos) break;
        pos = e + 1;
    }
    out.clear(); d.BreakdownWord("hello", out);
    return 0;
}
