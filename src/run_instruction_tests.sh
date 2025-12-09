#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_DIR="${SCRIPT_DIR}/tests"
BUILD_DIR="${TEST_DIR}/build"
SIM_BIN="${SCRIPT_DIR}/sim"
REF_SIM_BIN="${SCRIPT_DIR}/riscv-sim"

trim_last_lines() {
  local file="$1"
  local count="$2"
  local output="$3"
  python3 - "$file" "$count" "$output" <<'PY'
import sys
in_path, count_str, out_path = sys.argv[1:]
count = int(count_str)
with open(in_path, 'r', encoding='utf-8') as src:
    lines = src.readlines()
if count > 0:
    lines = lines[:-count] if count <= len(lines) else []
with open(out_path, 'w', encoding='utf-8') as dst:
    dst.writelines(lines)
PY
}

if [ ! -x "${SIM_BIN}" ]; then
  echo "Simulator 'sim' not found. Run 'make' in ${SCRIPT_DIR} first." >&2
  exit 1
fi

if [ ! -x "${REF_SIM_BIN}" ]; then
  echo "Reference simulator 'riscv-sim' not found. Run src/test.sh once to download it." >&2
  exit 1
fi

if ! command -v riscv64-unknown-elf-gcc >/dev/null 2>&1; then
  echo "Cross-compiler 'riscv64-unknown-elf-gcc' not found in PATH." >&2
  exit 1
fi

mkdir -p "${BUILD_DIR}"

PASSED=0
FAILED=0

for SRC in "${TEST_DIR}"/*.s; do
  BASE="$(basename "${SRC}" .s)"
  ELF="${BUILD_DIR}/${BASE}.elf"
  SIM_LOG="${BUILD_DIR}/${BASE}.sim.log"
  SIM_TRIM="${BUILD_DIR}/${BASE}.sim.trim.log"
  REF_LOG="${BUILD_DIR}/${BASE}.ref.log"
  REF_TRIM="${BUILD_DIR}/${BASE}.ref.trim.log"

  echo "== ${BASE}"

  riscv64-unknown-elf-gcc \
    -march=rv32im -mabi=ilp32 \
    -nostdlib -nostartfiles \
    -Ttext=0x1000 \
    "${SRC}" -o "${ELF}"

  "${SIM_BIN}" "${ELF}" -l "${SIM_LOG}"
  trim_last_lines "${SIM_LOG}" 1 "${SIM_TRIM}"

  "${REF_SIM_BIN}" "${ELF}" -l "${REF_LOG}"
  trim_last_lines "${REF_LOG}" 2 "${REF_TRIM}"

  if diff -w "${SIM_TRIM}" "${REF_TRIM}" >/dev/null; then
    echo "   PASS"
    PASSED=$((PASSED + 1))
  else
    echo "   FAIL"
    FAILED=$((FAILED + 1))
  fi

  rm -f "${SIM_LOG}" "${REF_LOG}" "${SIM_TRIM}" "${REF_TRIM}"
done

echo "${PASSED} passed, ${FAILED} failed"

exit ${FAILED}
