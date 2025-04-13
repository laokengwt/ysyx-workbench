// memory.h
#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

// 初始化存储器
void init_memory(void);

// 存储器读取函数
uint32_t pmem_read(uint32_t addr);

#endif // MEMORY_H
