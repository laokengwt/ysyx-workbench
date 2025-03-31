/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <memory/vaddr.h>
#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_si(char *args);

static int cmd_info(char *args);

static int cmd_x(char *args);

static int cmd_p(char *args);

static int cmd_test(char *args);

static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Single step execution", cmd_si },
  { "info", "Print program state", cmd_info },
  { "x", "Scan the memory", cmd_x },
  { "p", "Evaluate expression", cmd_p },
  { "test", "Test for evaluating expression", cmd_test },

  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_si(char *args) {
  int n = 1;
  if (args != NULL) {
    char *endptr;
    n = strtol(args, &endptr, 10);
    if (*endptr != '\0') {
      printf("Invalid step number: %s\n", args);
      return 0;
    }
    if (n <= 0) {
      printf("Step number must be positive\n");
      return 0;
    }
  }
  cpu_exec(n);
  return 0;
}

static int cmd_info(char *args) {
  if (args == NULL) {
    printf("Usage: info r - Print register values\n");
    return 0;
  }

  char *arg = strtok(args, " ");
  if (strcmp(arg, "r") == 0) {
    isa_reg_display();
  } else {
    printf("Unknown info subcommand '%s'\n", arg);
  }
  return 0;
}

static int cmd_x(char *args) {
  // 默认参数
  int count = 1;
  vaddr_t addr = 0;

  // 首先尝试解析第一个参数是否为地址
  char *arg = strtok(args, " ");
  if (arg) {
      // 检查是否是十六进制地址
      if (strncmp(arg, "0x", 2) == 0) {
          addr = strtoul(arg, NULL, 16);
      } 
      // 否则尝试解析为计数
      else {
          count = atoi(arg);
          if (count <= 0) {
              printf("Invalid count: %s\n", arg);
              return 0;
          }
          
          // 解析地址参数
          arg = strtok(NULL, " ");
          if (!arg || strncmp(arg, "0x", 2) != 0) {
              printf("Missing or invalid address (must be 0x...)\n");
              return 0;
          }
          addr = strtoul(arg, NULL, 16);
      }
  } else {
      printf("Usage: x [N] 0xADDR\n");
      return 0;
  }

  // 读取并显示内存
  for (int i = 0; i < count; i++) {
      vaddr_t current_addr = addr + i * 4;
      word_t value = vaddr_read(current_addr, 4);
      printf("0x%08x: 0x%08x\n", current_addr, value);
  }

  return 0;
}

static int cmd_p(char *args) {
  static int p_count = 1;  // 用于记录p命令的序号
  
  if (args == NULL || *args == '\0') {
    printf("Usage: p EXPR\n");
    return 0;
  }

  bool success = true;
  word_t result = expr(args, &success);
  
  if (success) {
    printf("$%d = 0x%08x %d\n", p_count++, result, result);
  } else {
    printf("Failed to evaluate expression: %s\n", args);
  }

  return 0;
}

static int cmd_test(char *args){
  int right_ans = 0;
  FILE *input_file = fopen("/home/zhou/ysyx-learning/ysyx-workbench/nemu/tools/gen-expr/input", "r");
    if (input_file == NULL) {
        perror("Error opening input file");
        return 1;
    }
 
    char record[1024];
    unsigned real_val;
    char buf[1024];
 
    // 循环读取每一条记录
    for (int i = 0; i < 100; i++) {
        // 读取一行记录
        if (fgets(record, sizeof(record), input_file) == NULL) {
            perror("Error reading input file");
            break;
        }
 
        // 分割记录，获取数字和表达式
        char *token = strtok(record, " ");
        if (token == NULL) {
            printf("Invalid record format\n");
            continue;
        }
        real_val = atoi(token); // 将数字部分转换为整数
 
        // 处理表达式部分，可能跨越多行
        strcpy(buf, ""); // 清空buf
        while ((token = strtok(NULL, "\n")) != NULL) {
            strcat(buf, token);
            strcat(buf, " "); // 拼接换行后的部分，注意添加空格以分隔多行内容
        }

        bool flag = false;
        unsigned res = expr(buf,&flag); 
        // 输出结果
        printf("Computed Value: %u,Real Value: %u, Expression: %s\n", res,real_val, buf);

        if(res == real_val)right_ans ++;
    }
    printf("test 963 expressions,the accuracy is %d/100\n",right_ans);
    fclose(input_file);
    return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
