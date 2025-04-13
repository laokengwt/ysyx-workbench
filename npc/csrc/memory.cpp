// memory.cpp
#include "memory.h"
#include <string.h>

// 存储器定义
#define MEM_SIZE 256  // 256字 = 1KB
static uint32_t mem[MEM_SIZE];  // static限制作用域在当前文件
const uint32_t MEM_BASE = 0x80000000;

// 内部地址转换函数
static uint32_t addr_to_index(uint32_t addr) {
    return (addr - MEM_BASE) >> 2;
}

// 初始化存储器（放置一条addi x2, x1, 1指令）
void init_memory(void) {
    memset(mem, 0, sizeof(mem));  // 清空存储器
    mem[0] = 0x00f00093;  // addi x1, x0, 15
    mem[1] = 0x00100113;  // addi x2, x0, 1
    mem[2] = 0x00108193;  // addi x3, x1, 1
    mem[3] = 0x00100073;  // addi x3, x1, 1
}

// 存储器读取函数
uint32_t pmem_read(uint32_t addr) {
    uint32_t index = addr_to_index(addr);
    return (index < MEM_SIZE) ? mem[index] : 0;
}
