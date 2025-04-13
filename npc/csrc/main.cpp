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

extern "C" void sim_finish() {
    printf("EBREAK encountered, stopping via DPI-C.\n");
    exit(0);  // 强制终止仿真
}

int main (int argc, char** argv){
  reset(1);
  init_memory();
    
  // 测试阶段
  while (1) {
    top.inst = pmem_read(top.pc);
    top.eval();
    single_cycle();
  }
  
  top.final();
  return 0;
}

