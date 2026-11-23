# syntax=docker/dockerfile:1.7
#
# Commit image for xLights (savantenvs/xLights). Builds the wx-free src-core parsers that read
# untrusted files (FSEQ sequence data, LRC lyrics, phoneme dictionaries, .xsq sequences, show-folder
# XML) into libFuzzer harnesses, plus upstream's lean Catch2 unit suite (tests/unit) as the oracle.
#
#   DOCKER_BUILDKIT=1 docker build -f mayhem/Dockerfile --build-arg COMMIT=$(git rev-parse HEAD) -t xlights .
FROM ghcr.io/savantenvs/base:20261023

ARG COMMIT
ARG REPO_URL
ARG SOURCE_DATE_EPOCH
ARG SANITIZER_FLAGS
ARG COVERAGE_FLAGS=
ARG MAYHEM_JOBS=

USER root
RUN [ -n "$SOURCE_DATE_EPOCH" ] && repro-sources-list.sh || true ; \
    apt-get update && apt-get install -yq --no-install-recommends \
        libzstd-dev zlib1g-dev cmake ninja-build ca-certificates curl \
 && rm -rf /var/lib/apt/lists/*

# Upstream pulls these in as git submodules (dependencies/spdlog, dependencies/pugixml) and the unit
# suite FetchContent-clones Catch2. They are fetched once here, pinned to the exact submodule commits
# (+ sha256), so build.sh resolves everything offline (SPEC §6.2 item 19). spdlog (with its bundled
# fmt) and pugixml are used as source/headers; Catch2 is built out of tree into a static prefix.
ARG SPDLOG_REV=45b67eee668507813743f84f1b1b9ab674ea221e
ARG SPDLOG_SHA256=9f7daebea74c5c5a17fb46d89becb1b94676c58370c654d36898e3c06c75fd2e
ARG PUGIXML_REV=3070fb6320848a8acdcc205415440a2f2cdb3537
ARG PUGIXML_SHA256=b632d06bffd005ff00dd75bab092e6369695d568f625c73fbead4be83c703501
ARG CATCH2_REV=2b60af89e23d28eefc081bc930831ee9d45ea58b
ARG CATCH2_SHA256=553e6c94498c42edaecc9c8459f1a320a7e4cf49155635cf7123212bb9f09cf2
RUN set -eux; fetch() { d="/opt/toolchains/deps/$1"; mkdir -p "$d/src"; \
        curl -fsSL -o /tmp/dep.tgz "https://github.com/$2/archive/$3.tar.gz"; \
        echo "$4  /tmp/dep.tgz" | sha256sum -c -; \
        tar -xzf /tmp/dep.tgz --strip-components=1 -C "$d/src"; rm -f /tmp/dep.tgz; }; \
    fetch spdlog-${SPDLOG_REV} gabime/spdlog ${SPDLOG_REV} ${SPDLOG_SHA256}; \
    fetch pugixml-${PUGIXML_REV} zeux/pugixml ${PUGIXML_REV} ${PUGIXML_SHA256}; \
    fetch catch2-${CATCH2_REV} catchorg/Catch2 ${CATCH2_REV} ${CATCH2_SHA256}; \
    d=/opt/toolchains/deps/catch2-${CATCH2_REV}; \
    cmake -S "$d/src" -B /tmp/catch2-build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo \
        -DCMAKE_CXX_STANDARD=20 -DCMAKE_CXX_FLAGS="-O2 -g -gdwarf-3" -DCMAKE_INSTALL_PREFIX="$d" \
        -DCATCH_INSTALL_DOCS=OFF -DCATCH_INSTALL_EXTRAS=ON; \
    cmake --build /tmp/catch2-build -j"$(nproc)"; cmake --install /tmp/catch2-build; \
    rm -rf /tmp/catch2-build; chmod -R a+rX /opt/toolchains/deps
ENV XL_SPDLOG_DIR=/opt/toolchains/deps/spdlog-45b67eee668507813743f84f1b1b9ab674ea221e/src \
    XL_PUGIXML_DIR=/opt/toolchains/deps/pugixml-3070fb6320848a8acdcc205415440a2f2cdb3537/src \
    XL_CATCH2_DIR=/opt/toolchains/deps/catch2-2b60af89e23d28eefc081bc930831ee9d45ea58b

USER mayhem
WORKDIR /mayhem
COPY --chown=2000:2000 . /mayhem

RUN if [ -x /opt/mayhem/ccache/prefill ]; then /opt/mayhem/ccache/prefill bash mayhem/build.sh; \
    else bash mayhem/build.sh; fi

RUN if [ -f mayhem/test.sh ]; then bash mayhem/test.sh; else echo "no mayhem/test.sh"; fi

LABEL org.opencontainers.image.source="${REPO_URL}" \
      org.opencontainers.image.revision="${COMMIT}" \
      rlenv.tier="commit" \
      rlenv.source_date_epoch="${SOURCE_DATE_EPOCH}" \
      rlenv.user="mayhem"

USER mayhem
WORKDIR /mayhem
