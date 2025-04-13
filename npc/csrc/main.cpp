#include <verilated.h>
#include "Vtop.h"
#include "memory.h"

static Vtop top;

static void single_cycle() {
  top.clk = 0; top.eval();
  top.clk = 1; top.eval();
}

void reset(int n) {
  top.rst = 1;
  while (n -- > 0) single_cycle();
  top.rst = 0;
}

int main (int argc, char** argv){
  reset(1);
  init_memory();
    
  // 测试阶段
  for (int cycle = 0; cycle < 3; cycle++) {
    top.inst = pmem_read(top.pc);
    top.eval();
    single_cycle();
  }
  
  top.final();
  return 0;
}

