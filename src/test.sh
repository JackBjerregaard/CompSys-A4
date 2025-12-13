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

# Test simulate (fib).
./sim ../predictor-benchmarks/fib.elf -- 7 | sed '$d' > simulate.txt
./riscv-sim ../predictor-benchmarks/fib.elf -- 7 | sed '$d' | sed '$d' > riscv-simulate.txt

if diff -w simulate.txt riscv-simulate.txt > /dev/null; then
  echo "Passed simulation (fib)"
  PASSED=$((PASSED + 1))
else
  echo "Failed simulation (fib)"
  FAILED=$((FAILED + 1))
fi
rm simulate.txt riscv-simulate.txt

# Test simulate log (fib).
./sim ../predictor-benchmarks/fib.elf -l log -- 7 > /dev/null
sed '$d' log > log.tmp && mv log.tmp log 
./riscv-sim ../predictor-benchmarks/fib.elf -l riscv-log -- 7 > /dev/null
sed '$d' riscv-log | sed '$d' > riscv-log.tmp && mv riscv-log.tmp riscv-log

if diff -w log riscv-log > /dev/null; then
  echo "Passed simulation with log (fib)"
  PASSED=$((PASSED + 1))
else
  echo "Failed simulation with log (fib)"
  FAILED=$((FAILED + 1))
fi
rm log riscv-log

# Test disassemble (fib)
./sim ../predictor-benchmarks/fib.elf -d -- 7 > disassemble.txt
./riscv-sim ../predictor-benchmarks/fib.elf -d -- 7 > riscv-disassemble.txt
if diff -w disassemble.txt riscv-disassemble.txt > /dev/null; then
  echo "Passed disassemble (fib)"
  PASSED=$((PASSED + 1))
else
  echo "Failed disassemble (fib)"
  FAILED=$((FAILED + 1))
fi
rm disassemble.txt riscv-disassemble.txt

# Test simulate (erat)
./sim ../predictor-benchmarks/erat.elf | sed '$d' > simulate.txt
./riscv-sim ../predictor-benchmarks/erat.elf | sed '$d' | sed '$d' > riscv-simulate.txt

if diff -w simulate.txt riscv-simulate.txt > /dev/null; then
  echo "Passed simulation (erat)"
  PASSED=$((PASSED + 1))
else
  echo "Failed simulation (erat)"
  FAILED=$((FAILED + 1))
fi
rm simulate.txt riscv-simulate.txt

# Test simulate log (erat)
./sim ../predictor-benchmarks/erat.elf -l log > /dev/null
sed '$d' log > log.tmp && mv log.tmp log
./riscv-sim ../predictor-benchmarks/erat.elf -l riscv-log > /dev/null
sed '$d' riscv-log | sed '$d' > riscv-log.tmp && mv riscv-log.tmp riscv-log

if diff -w log riscv-log > /dev/null; then
  echo "Passed simulation with log (erat)"
  PASSED=$((PASSED + 1))
else
  echo "Failed simulation with log (erat)"
  FAILED=$((FAILED + 1))
fi
rm log riscv-log

# Test disassemble (erat)
./sim ../predictor-benchmarks/erat.elf -d > disassemble.txt
./riscv-sim ../predictor-benchmarks/erat.elf -d > riscv-disassemble.txt
if diff -w disassemble.txt riscv-disassemble.txt > /dev/null; then
  echo "Passed disassemble (erat)"
  PASSED=$((PASSED + 1))
else
  echo "Failed disassemble (erat)"
  FAILED=$((FAILED + 1))
fi
rm disassemble.txt riscv-disassemble.txt

echo "${PASSED} PASSED, ${FAILED} FAILED"

exit 0
