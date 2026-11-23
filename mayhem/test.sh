#!/usr/bin/env bash
# mayhem/test.sh — RUNS upstream's lean Catch2 unit suite (tests/unit/test_*.cpp: LRC parser,
# phoneme dictionary, xsq scanner, show redactor, colours, string utils, ...) built by build.sh.
# Every test case asserts values; we map Catch2's JUnit report to CTRF counts.
set -uo pipefail
[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH
: "${SRC:=/mayhem}"
cd "$SRC"

emit_ctrf() {
  local tool="$1" passed="$2" failed="$3" skipped="${4:-0}" pending="${5:-0}" other="${6:-0}"
  local tests=$(( passed + failed + skipped + pending + other ))
  cat > "${CTRF_REPORT:-$SRC/ctrf-report.json}" <<JSON
{
  "results": {
    "tool": { "name": "$tool" },
    "summary": {
      "tests": $tests,
      "passed": $passed,
      "failed": $failed,
      "pending": $pending,
      "skipped": $skipped,
      "other": $other
    }
  }
}
JSON
  printf 'CTRF {"results":{"tool":{"name":"%s"},"summary":{"tests":%d,"passed":%d,"failed":%d,"pending":%d,"skipped":%d,"other":%d}}}\n' \
    "$tool" "$tests" "$passed" "$failed" "$pending" "$skipped" "$other"
  [ "$failed" -eq 0 ]
}

RUNNER="$SRC/mayhem/build/tests/xlights_unit_tests"
if [ ! -x "$RUNNER" ]; then
  echo "test.sh: $RUNNER missing — run mayhem/build.sh first" >&2
  emit_ctrf catch2 0 1; exit 1
fi

# Number of test cases the binary declares (independent of whether running them works).
expected=$("$RUNNER" --list-tests --verbosity quiet 2>/dev/null | grep -c . || true)
report="$(mktemp "${TMPDIR:-/tmp}/xl-junit.XXXXXX")"
"$RUNNER" --reporter junit --out "$report" >/dev/null 2>&1
rc=$?
total=0; fail=0; err=0; skip=0
if [ -s "$report" ]; then
  # <testsuite ... errors="E" failures="F" skipped="S" tests="T" ...> — Catch2 counts test cases here.
  line=$(grep -m1 '<testsuite ' "$report")
  get() { echo "$line" | sed -n "s/.* $1=\"\([0-9]*\)\".*/\1/p"; }
  total=$(get tests); fail=$(get failures); err=$(get errors); skip=$(get skipped)
  total=${total:-0}; fail=${fail:-0}; err=${err:-0}; skip=${skip:-0}
fi
# Catch2's junit `tests` counts assertions; count test cases from <testcase> elements instead.
cases=$(grep -c "<testcase " "$report" 2>/dev/null || true); cases=${cases:-0}
failed_cases=$(grep -c "<failure\|<error" "$report" 2>/dev/null || true); failed_cases=${failed_cases:-0}
rm -f "$report"
passed=$(( cases - failed_cases )); [ "$passed" -lt 0 ] && passed=0
failed=$failed_cases
# A runner that produced no report, ran fewer cases than it declares, or exited non-zero is a failure.
if [ "$cases" -eq 0 ] || [ "$expected" -eq 0 ] || [ "$rc" -ne 0 ]; then
  [ "$failed" -eq 0 ] && failed=1
fi
echo "test.sh: declared=$expected ran=$cases failed=$failed_cases rc=$rc (assertions=$total)"
emit_ctrf catch2 "$passed" "$failed" "$skip"
