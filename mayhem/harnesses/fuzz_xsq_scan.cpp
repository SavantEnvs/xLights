// .xsq / .xml sequence scanner (src-core/utils/XsqFileScanner.cpp): sniffs a file the user
// opens/imports for <xsequence and extracts <mediaFile>.
#include <cstdint>
#include "fuzz_common.h"
#include "utils/XsqFileScanner.h"

extern "C" int LLVMFuzzerInitialize(int*, char***) { fuzz_init_scratch(); return 0; }

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    std::string p = fuzz_write("input.xsq", data, size);
    XsqFileInfo info = ScanXsqFile(p);
    (void)info.mediaFile.size();
    return 0;
}
