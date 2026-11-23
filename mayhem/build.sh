#!/usr/bin/env bash
# mayhem/build.sh — builds the xLights src-core parser harnesses (ASan+UBSan, libFuzzer +
# standalone) and upstream's lean unit suite (normal flags) for mayhem/test.sh.
# Re-runnable and offline: spdlog/pugixml/Catch2 come from /opt/toolchains/deps (see Dockerfile).
set -euo pipefail
[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH
: "${SANITIZER_FLAGS=-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer}"
: "${DEBUG_FLAGS:=-g -gdwarf-3}"
: "${CC:=clang}" ; : "${CXX:=clang++}" ; : "${LIB_FUZZING_ENGINE:=-fsanitize=fuzzer}"
: "${MAYHEM_JOBS:=$(nproc)}"
: "${COVERAGE_FLAGS=}"
: "${SRC:=/mayhem}"
: "${STANDALONE_FUZZ_MAIN:=/opt/mayhem/StandaloneFuzzTargetMain.c}"
: "${XL_SPDLOG_DIR:=/opt/toolchains/deps/spdlog-45b67eee668507813743f84f1b1b9ab674ea221e/src}"
: "${XL_PUGIXML_DIR:=/opt/toolchains/deps/pugixml-3070fb6320848a8acdcc205415440a2f2cdb3537/src}"
: "${XL_CATCH2_DIR:=/opt/toolchains/deps/catch2-2b60af89e23d28eefc081bc930831ee9d45ea58b}"
export XL_SPDLOG_DIR XL_PUGIXML_DIR XL_CATCH2_DIR
cd "$SRC"

# Build dirs live under mayhem/build/ (gitignored by upstream's `build` rule).
OBJ="$SRC/mayhem/build/fuzz"
mkdir -p "$OBJ"

INC=(-I"$SRC/src-core" -I"$SRC/src-core/utils" -I"$SRC/src-core/render" -I"$SRC/src-core/lyrics"
     -I"$SRC/include" -I"$SRC/dependencies" -I"$XL_SPDLOG_DIR/include" -I"$XL_PUGIXML_DIR/src")
CXXFLAGS=(-std=gnu++20 -O1 $SANITIZER_FLAGS $DEBUG_FLAGS -fsanitize=fuzzer-no-link -DLINUX
          -ffunction-sections -fdata-sections "${INC[@]}")
# The import readers' effect-synthesis paths (MapChannelEffects -> EffectLayer) are not called by
# any harness; --gc-sections drops them so the readers link without the render/effect engine.
LDGC=(-fuse-ld=lld -Wl,--gc-sections)

# 1) The project code under test, instrumented. Same leaf files as tests/unit/core_sources.cmake,
#    plus the FSEQ reader (src-core/render/FSEQFile.cpp, as fseq_convert builds it) and the
#    third-party sequence import readers (Vixen 3/2, HLS, LOR .lms/.las), and the image decoder
#    (xlImage over the in-tree stb headers).
CORE_SRCS=(
  src-core/render/FSEQFile.cpp
  src-core/utils/Color.cpp
  src-core/utils/string_utils.cpp
  src-core/utils/AppCallbacks.cpp
  src-core/utils/NodeUtils.cpp
  src-core/utils/XsqFileScanner.cpp
  src-core/utils/ShowRedactor.cpp
  src-core/lyrics/LRCParser.cpp
  src-core/lyrics/PhonemeDictionary.cpp
  src-core/utils/UtilFunctions.cpp
  src-core/import_export/ImportDataRow.cpp
  src-core/import_export/Vixen3.cpp
  src-core/import_export/Vixen2File.cpp
  src-core/import_export/HLSFile.cpp
  src-core/import_export/LORMusic.cpp
  src-core/utils/xlImage.cpp
)
pids=()
objs=()
for s in "${CORE_SRCS[@]}"; do
  o="$OBJ/$(echo "$s" | tr '/' '_').o"; objs+=("$o")
  $CXX "${CXXFLAGS[@]}" -c "$SRC/$s" -o "$o" & pids+=($!)
done
o="$OBJ/pugixml.o"; objs+=("$o")
$CXX "${CXXFLAGS[@]}" -c "$XL_PUGIXML_DIR/src/pugixml.cpp" -o "$o" & pids+=($!)
$CXX "${CXXFLAGS[@]}" -c "$SRC/mayhem/lsan_off.cc" -o "$OBJ/lsan_off.o" & pids+=($!)
$CC $SANITIZER_FLAGS $DEBUG_FLAGS -c "$STANDALONE_FUZZ_MAIN" -o "$OBJ/standalone_main.o" & pids+=($!)
for p in "${pids[@]}"; do wait "$p"; done

# 2) Harnesses: libFuzzer binary + run-once standalone reproducer each.
LIBS=(-lzstd -lz -lpthread)
TARGETS=(fuzz_fseq fuzz_lrc fuzz_phoneme_dict fuzz_xsq_scan fuzz_show_redact
         fuzz_vixen3 fuzz_vixen2 fuzz_hls fuzz_lor_music fuzz_image)
pids=()
for t in "${TARGETS[@]}"; do
  (
    $CXX "${CXXFLAGS[@]}" -c "$SRC/mayhem/harnesses/$t.cpp" -o "$OBJ/$t.o"
    $CXX $SANITIZER_FLAGS $DEBUG_FLAGS "${LDGC[@]}" $LIB_FUZZING_ENGINE "$OBJ/$t.o" "${objs[@]}" "$OBJ/lsan_off.o" "${LIBS[@]}" -o "$SRC/$t"
    $CXX $SANITIZER_FLAGS $DEBUG_FLAGS "${LDGC[@]}" "$OBJ/$t.o" "$OBJ/standalone_main.o" "${objs[@]}" "$OBJ/lsan_off.o" "${LIBS[@]}" -o "$SRC/$t-standalone"
  ) & pids+=($!)
done
for p in "${pids[@]}"; do wait "$p"; done

# 3) Upstream's lean unit suite (tests/unit, Catch2) with NORMAL flags, for mayhem/test.sh.
(
  unset CFLAGS CXXFLAGS LDFLAGS
  cmake -S "$SRC/mayhem/unit" -B "$SRC/mayhem/build/tests" -G Ninja -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_FLAGS="$COVERAGE_FLAGS" -DCMAKE_EXE_LINKER_FLAGS="$COVERAGE_FLAGS"
  cmake --build "$SRC/mayhem/build/tests" -j"$MAYHEM_JOBS"
)
echo "build.sh: built ${TARGETS[*]} and mayhem/build/tests/xlights_unit_tests"
