#!/bin/bash

if [ ! -f "./riscv-sim" ]; then
  echo "Getting CompSys simulator..."
  if [[ "$OSTYPE" == "linux-gnu" ]]; then
    echo "linux"
    wget -O riscv-sim https://github.com/diku-compSys/compSys-e2025/raw/refs/heads/main/tools/riscv-sim/sim-linux
  elif [[ "$OSTYPE" == "darwin*" ]]; then 
    if [[ "$ARCH" == "arm64" ]]; then
      wget -O riscv-sim https://github.com/diku-compSys/compSys-e2025/raw/refs/heads/main/tools/riscv-sim/sim-mac
    elif [[ "$ARCH" == "x86_64" ]]; then
      wget -O riscv-sim https://github.com/diku-compSys/compPSys-e2025/raw/refs/heads/main/tools/riscv-sim/sim-mac-x86
    fi
  fi
  chmod +x riscv-sim
fi

make

FAILED=0
PASSED=0

# P.S. removing summary lines of both outputs, as execution time differs

# Test simulate.
./sim ../predictor-benchmarks/fib.elf -- 7 | head -n -1 > simulate.txt 
./riscv-sim ../predictor-benchmarks/fib.elf -- 7 | head -n -2 > riscv-simulate.txt
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
head log -n -1 > log.tmp
mv log.tmp log
./riscv-sim ../predictor-benchmarks/fib.elf -l riscv-log -- 7 
head riscv-log -n -2 > riscv-log.tmp
mv riscv-log.tmp riscv-log
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

echo "${PASSED} PASSED, ${FAILED} FAILED"

exit 0
