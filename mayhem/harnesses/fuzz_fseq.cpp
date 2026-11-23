// FSEQ sequence reader (src-core/render/FSEQFile.cpp): V1/V2/ESEQ headers, variable headers,
// sparse ranges, zstd/zlib block tables and frame decode — the path xLights, xSchedule and FPP
// use to play a .fseq file a user opens or downloads.
#include <cstdint>
#include <memory>
#include <vector>
#include "fuzz_common.h"
#include "render/FSEQFile.h"

extern "C" int LLVMFuzzerInitialize(int*, char***) { fuzz_init_scratch(); return 0; }

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    std::string p = fuzz_write("input.fseq", data, size);
    std::unique_ptr<FSEQFile> f(FSEQFile::openFSEQFile(p));
    if (!f) return 0;
    (void)f->getMediaFilename();
    for (auto& vh : f->getVariableHeaders()) (void)vh.data.size();
    uint64_t chans = f->getChannelCount();
    uint32_t maxCh = f->getMaxChannel();
    // Cap what we read so a header claiming GBs of channels can't turn one input into an OOM.
    uint32_t want = (uint32_t)std::min<uint64_t>(std::max<uint64_t>(chans, maxCh), 1u << 20);
    f->prepareRead({{0, want ? want : 1}}, 0);
    std::vector<uint8_t> buf(want + 16);
    uint64_t frames = std::min<uint64_t>(f->getNumFrames(), 64);
    for (uint32_t i = 0; i < frames; i++) {
        std::unique_ptr<FSEQFile::FrameData> fd(f->getFrame(i));
        if (!fd) break;
        fd->readFrame(buf.data(), want);
    }
    return 0;
}
