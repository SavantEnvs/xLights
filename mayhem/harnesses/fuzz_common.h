// Shared helpers for the xLights harnesses: a per-process scratch dir under $TMPDIR (file-reading
// APIs need a real path) and quiet logging. The only state kept across inputs is the scratch dir
// itself (each input overwrites its file) and spdlog's level, which no input can change.
#pragma once
#include <spdlog/spdlog.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <unistd.h>

static std::string g_scratch;

static void fuzz_init_scratch() {
    spdlog::set_level(spdlog::level::off);
    const char* t = getenv("TMPDIR");
    std::string tmpl = std::string(t && *t ? t : "/tmp") + "/xlfuzz.XXXXXX";
    if (!mkdtemp(tmpl.data())) { perror("mkdtemp"); abort(); }
    g_scratch = tmpl;
}

static std::string fuzz_write(const std::string& name, const uint8_t* data, size_t size) {
    std::string p = g_scratch + "/" + name;
    FILE* f = fopen(p.c_str(), "wb");
    if (!f) { perror("fopen"); abort(); }
    if (size) fwrite(data, 1, size, f);
    fclose(f);
    return p;
}
