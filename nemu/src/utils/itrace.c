#include <common.h>
#include <elf.h>

#define MAX_IRINGBUF 16

// iringbuf

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

// mtrace

void display_pread(paddr_t addr, int len) {
  printf("pread at " FMT_PADDR " len = %d\n", addr, len);
}

void display_pwrite(paddr_t addr, int len, word_t data) {
  printf("pwrite at " FMT_PADDR " len = %d, data = " FMT_WORD "\n", addr, len, data);
}

// ftrace
// Symbol Entry
typedef struct {
  paddr_t addr;     // function address
  uint32_t size;    // function size
  char *name;       // function name
} SymbolEntry;

// Symbol Table
typedef struct {
  SymbolEntry *entries;   // dynamic array for entries
  uint32_t count;         // current symbol entries count
  uint32_t capacity;      // capacity of current array
} SymbolTable;

// global symbol table
SymbolTable global_symtab;

static void init_symbol_table() {
  global_symtab.capacity = 16;
  global_symtab.count = 0;
  global_symtab.entries = malloc(global_symtab.capacity * sizeof(SymbolEntry));
  Assert(global_symtab.entries, "malloc failed");
} 

static void add_symbol(paddr_t addr, uint32_t size, const char *name) {
  // check if the array is overflow
  if (global_symtab.count >= global_symtab.capacity) {
    global_symtab.capacity *= 2;
    global_symtab.entries = realloc(global_symtab.entries, global_symtab.capacity * sizeof(SymbolEntry));
    Assert(global_symtab.entries, "realloc failed");
  }

  // store the symbol
  global_symtab.entries[global_symtab.count].addr = addr;
  global_symtab.entries[global_symtab.count].size = size;
  global_symtab.entries[global_symtab.count].name = strdup(name);
  global_symtab.count++;
}

// 查找指定类型的节头表项
static Elf32_Shdr find_section(FILE *fp, Elf32_Ehdr ehdr, Elf32_Word type) {
  Elf32_Shdr shdr;
  fseek(fp, ehdr.e_shoff, SEEK_SET);
  while (1) {
      if (fread(&shdr, sizeof(Elf32_Shdr), 1, fp) <= 0) {
          fprintf(stderr, "Failed to find section with type %d\n", type);
          fclose(fp);
          exit(EXIT_FAILURE);
      }
      if (shdr.sh_type == type) {
          return shdr;
      }
  }
}

void parse_elf(char *elf_file) {
  init_symbol_table();

  FILE *fp = fopen(elf_file, "rb");
  Assert(fp, "Can not open '%s'", elf_file);

  // get the ELF Header
  Elf32_Ehdr elf_header;
  if (fread(&elf_header, sizeof(elf_header), 1, fp) <= 0) {
    fclose(fp);
    exit(EXIT_FAILURE);
  }
  
  // check the Magic Number
  if (memcmp(elf_header.e_ident, ELFMAG, SELFMAG) != 0) {
    fprintf(stderr, "Not an ELF file!\n");
    fclose(fp);
    exit(EXIT_FAILURE);
  }

  // get the .symtab and .strtab
  Elf32_Shdr strtab_header = find_section(fp, elf_header, SHT_STRTAB);
  Elf32_Shdr symtab_header = find_section(fp, elf_header, SHT_SYMTAB);

  // read the .symtab
  int num_symbols = symtab_header.sh_size / symtab_header.sh_entsize;
  Elf32_Sym *syms = malloc(symtab_header.sh_size);
  fseek(fp, symtab_header.sh_offset, SEEK_SET);
  if (fread(syms, symtab_header.sh_size, 1, fp) <= 0) {
    fclose(fp);
    exit(EXIT_FAILURE);
  }

  // read the .strtab
  char *strtab_data = malloc(strtab_header.sh_size);
  fseek(fp, strtab_header.sh_offset, SEEK_SET); 
  if (fread(strtab_data, strtab_header.sh_size, 1, fp) <= 0) {
    fclose(fp);
    exit(EXIT_FAILURE);
  }

  // get the FUNC symbol and store it
  for (int i = 0; i < num_symbols; i++) {
    if (ELF32_ST_TYPE(syms[i].st_info) == STT_FUNC) {
      char *name = strtab_data + syms[i].st_name;
      add_symbol(syms[i].st_value, syms[i].st_size, name);
    }
  }

  // free the pointer
  free(syms);
  free(strtab_data);
  fclose(fp);
}

// test function
void print_all_symbols() {
  printf("Total symbols: %d\n", global_symtab.count);
  for (int i = 0; i < global_symtab.count; i++) {
      printf("0x" FMT_PADDR " - 0x" FMT_PADDR ": %s\n",
             global_symtab.entries[i].addr,
             global_symtab.entries[i].addr + global_symtab.entries[i].size,
             global_symtab.entries[i].name);
  }
}

const char *find_symbol_by_addr(unsigned long addr) {
  for (int i = 0; i < global_symtab.count; i++) {
      if (addr >= global_symtab.entries[i].addr &&
          addr < global_symtab.entries[i].addr + global_symtab.entries[i].size) {
          return global_symtab.entries[i].name;
      }
  }
  return NULL;
}

int call_depth = 0; // current call depth

void trace_func_call(unsigned long pc, unsigned long target_addr) {
  const char *callee_name = find_symbol_by_addr(target_addr);

  // 打印调用信息（缩进表示层级）
  printf("0x%lx: ", pc);
  for (int i = 0; i < call_depth; i++) printf("  ");
  printf("call [%s@0x%lx]\n", callee_name ? callee_name : "???", target_addr);

  call_depth++;
}

void trace_func_ret(unsigned long pc) {
  const char *func_name = find_symbol_by_addr(pc);

  call_depth--;  // 减少深度后再打印

  // 打印返回信息
  printf("0x%lx: ", pc);
  for (int i = 0; i < call_depth; i++) printf("  ");
  printf("ret  [%s]\n", func_name ? func_name : "???");
}
