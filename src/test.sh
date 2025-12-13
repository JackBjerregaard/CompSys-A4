#!/bin/bash

if [ ! -f "./riscv-sim" ]; then
  echo "Getting CompSys simulator..."
  ARCH=$(uname -m)
  
  if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    echo "linux"
    curl -L -o riscv-sim https://github.com/diku-compSys/compSys-e2025/raw/refs/heads/main/tools/riscv-sim/sim-linux
  elif [[ "$OSTYPE" == "darwin"* ]]; then
    if [[ "$ARCH" == "arm64" ]]; then
      curl -L -o riscv-sim https://github.com/diku-compSys/compSys-e2025/raw/refs/heads/main/tools/riscv-sim/sim-mac
    elif [[ "$ARCH" == "x86_64" ]]; then
      curl -L -o riscv-sim https://github.com/diku-compSys/compSys-e2025/raw/refs/heads/main/tools/riscv-sim/sim-mac-x86
    fi
  fi
  chmod +x riscv-sim
fi

make

FAILED=0
PASSED=0

# P.S. removing summary lines.
# Changed 'head -n -1' to "sed '$d'" (delete last line) for Mac compatibility.

# Test simulate.
./sim ../predictor-benchmarks/fib.elf -- 7 | sed '$d' > simulate.txt
./riscv-sim ../predictor-benchmarks/fib.elf -- 7 | sed '$d' | sed '$d' > riscv-simulate.txt

if diff -w simulate.txt riscv-simulate.txt > /dev/null; then
  echo "Passed simulation"
  PASSED=$((PASSED + 1))
else
  echo "Failed simulation"
  FAILED=$((FAILED + 1))
fi
rm simulate.txt riscv-simulate.txt

# Test simulate log.
./sim ../predictor-benchmarks/fib.elf -l log -- 7
sed '$d' log > log.tmp && mv log.tmp log 
./riscv-sim ../predictor-benchmarks/fib.elf -l riscv-log -- 7
sed '$d' riscv-log | sed '$d' > riscv-log.tmp && mv riscv-log.tmp riscv-log

if diff -w log riscv-log > /dev/null; then
  echo "Passed simulation with log"
  PASSED=$((PASSED + 1))
else
  echo "Failed simulation with log"
  FAILED=$((FAILED + 1))
fi
rm log riscv-log

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

# Test simulate.
./sim ../predictor-benchmarks/erat.elf -- 7 | sed '$d' > simulate.txt
./riscv-sim ../predictor-benchmarks/erat.elf -- 7 | sed '$d' | sed '$d' > riscv-simulate.txt

if diff -w simulate.txt riscv-simulate.txt > /dev/null; then
  echo "Passed simulation"
  PASSED=$((PASSED + 1))
else
  echo "Failed simulation"
  FAILED=$((FAILED + 1))
fi
rm simulate.txt riscv-simulate.txt

# Test simulate log.
./sim ../predictor-benchmarks/erat.elf -l log -- 7
sed '$d' log > log.tmp && mv log.tmp log
./riscv-sim ../predictor-benchmarks/erat.elf -l riscv-log -- 7
sed '$d' riscv-log | sed '$d' > riscv-log.tmp && mv riscv-log.tmp riscv-log

if diff -w log riscv-log > /dev/null; then
  echo "Passed simulation with log"
  PASSED=$((PASSED + 1))
else
  echo "Failed simulation with log"
  FAILED=$((FAILED + 1))
fi
rm log riscv-log

# Test disassemble
./sim ../predictor-benchmarks/erat.elf -d > disassemble.txt
./riscv-sim ../predictor-benchmarks/erat.elf -d > riscv-disassemble.txt
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
