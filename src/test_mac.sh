#!/bin/bash

set -euo pipefail

ARCH="$(uname -m)"
OS="$(uname -s)"

download() {
  local url="$1"
  local output="$2"
  if command -v wget >/dev/null 2>&1; then
    wget -O "${output}" "${url}"
  elif command -v curl >/dev/null 2>&1; then
    curl -L -o "${output}" "${url}"
  else
    echo "Neither wget nor curl is available for downloading files." >&2
    exit 1
  fi
}

if [ ! -f "./riscv-sim" ]; then
  echo "Getting CompSys simulator..."
  if [[ "${OS}" == "Linux" ]]; then
    download https://github.com/diku-compSys/compSys-e2025/raw/refs/heads/main/tools/riscv-sim/sim-linux riscv-sim
  elif [[ "${OS}" == "Darwin" ]]; then
    if [[ "${ARCH}" == "arm64" ]]; then
      download https://github.com/diku-compSys/compSys-e2025/raw/refs/heads/main/tools/riscv-sim/sim-mac riscv-sim
    elif [[ "${ARCH}" == "x86_64" ]]; then
      download https://github.com/diku-compSys/compSys-e2025/raw/refs/heads/main/tools/riscv-sim/sim-mac-x86 riscv-sim
    else
      echo "Unsupported macOS architecture: ${ARCH}" >&2
      exit 1
    fi
  else
    echo "Unsupported OS: ${OS}" >&2
    exit 1
  fi
  chmod +x riscv-sim
fi

make

FAILED=0
PASSED=0

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

# Remove summary lines of both outputs, as execution time differs

# Test simulate.
./sim ../predictor-benchmarks/fib.elf -- 7 > simulate_raw.txt
trim_last_lines simulate_raw.txt 1 simulate.txt
./riscv-sim ../predictor-benchmarks/fib.elf -- 7 > riscv-simulate_raw.txt
trim_last_lines riscv-simulate_raw.txt 2 riscv-simulate.txt
if diff -w simulate.txt riscv-simulate.txt > /dev/null; then
  echo "Passed simulation"
  PASSED=$((PASSED + 1))
else
  echo "Failed simulation"
  FAILED=$((FAILED + 1))
fi
rm simulate_raw.txt riscv-simulate_raw.txt simulate.txt riscv-simulate.txt

# Test simulate log.
./sim ../predictor-benchmarks/fib.elf -l log_raw -- 7
trim_last_lines log_raw 1 log
./riscv-sim ../predictor-benchmarks/fib.elf -l riscv-log-raw -- 7
trim_last_lines riscv-log-raw 2 riscv-log
if diff -w log riscv-log > /dev/null; then
  echo "Passed simulation with log"
  PASSED=$((PASSED + 1))
else
  echo "Failed simulation with log"
  FAILED=$((FAILED + 1))
fi
rm log_raw riscv-log-raw log riscv-log

# Test disassemble
./sim ../predictor-benchmarks/fib.elf -d > disassemble.txt
./riscv-sim ../predictor-benchmarks/fib.elf -d > riscv-disassemble.txt
if diff -w disassemble.txt riscv-disassemble.txt > /dev/null; then
  echo "Passed disassemble"
  PASSED=$((PASSED + 1))
else
  echo "Failed disassemble"
  FAILED=$((FAILED + 1))
fi
rm disassemble.txt riscv-disassemble.txt

echo "${PASSED} PASSED, ${FAILED} FAILED"

exit 0
