#include <common.h>

#define MAX_IRINGBUF 16

// 只需要PC和指令，读写指针合为一体
typedef struct {
  word_t pc;
  uint32_t inst;
} ITraceNode;

ITraceNode iringbuf[MAX_IRINGBUF];
int ptr = 0;
bool full = false;

void store_inst(word_t pc, uint32_t inst) {
  iringbuf[ptr].pc = pc;
  iringbuf[ptr].inst = inst;
  ptr = (ptr + 1) % MAX_IRINGBUF;
  full = full || (ptr == 0);
}

void display_inst() {
  // empty buffer
  if(!full || !ptr) return;

  // 也许可以采用两种输出方法
  // 一种是直接顺序打印出buf
  // 另一种是从最旧打印到最新，不过这样貌似就没有加箭头指示的必要了
  int i = 0;

  void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
  char inst_buf[128];
  char *p;

  // 满了就打印到MAX_IRINGBUF，没满就打印到ptr
  while (i < MAX_IRINGBUF || (!full && i <= ptr)) {
    p = inst_buf;
    // 让他输出得更好看一些
    // p += sprintf(inst_buf, "%s" FMT_WORD ": %08x ", (i == ptr)?" --> ":"     ", iringbuf[i].pc, iringbuf[i].inst);
    p += sprintf(inst_buf, "%s" FMT_WORD ": ", (i == ptr) ? " --> " : "     ", iringbuf[i].pc);

    uint32_t inst = iringbuf[i].inst;
    for (int j = 3; j >= 0; j--) {
      uint8_t byte = (inst >> (j * 8)) & 0xFF;
      p += sprintf(p, "%02x ", byte);
    }
    p += sprintf(p, "  ");
    disassemble(p, inst_buf + sizeof(inst_buf) - p, iringbuf[i].pc, (uint8_t *)&iringbuf[i].inst, 4);

    puts(inst_buf);
    i++;
  }
  
}