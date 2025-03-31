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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

static int buf_len; // 当前 buf 的长度

static uint32_t choose(uint32_t n) {
    return rand() % n;
}

static void gen_num() {
    uint32_t num = choose(100) + 1;
    char num_str[12];
    int len = snprintf(num_str, sizeof(num_str), "%u", num);

    strcat(buf, num_str);
    buf_len += len;
}

static void gen(char c) {
    char char_str[2] = {c, '\0'};
    strcat(buf, char_str);
    buf_len += 1;
}

static void gen_rand_op() {
    char ops[] = {'+', '-', '*', '/'};
    uint32_t index = choose(4);
    char op_str[2] = {ops[index], '\0'};
    strcat(buf, op_str);
    buf_len += 1;
}

static void gen_rand_expr() {
    if (buf_len >= 100) {
        gen_num();
        return;
    }

    switch (choose(3)) {
        case 0: gen_num(); break;
        case 1: gen('('); gen_rand_expr(); gen(')'); break;
        default: gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;
    }
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    buf[0] = '\0'; // 初始化数组，避免生成的时候从上一个表达式的结尾开始生起
    buf_len = 0;
    gen_rand_expr();

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc -Werror /tmp/.code.c -o /tmp/.expr 2> /dev/null");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
