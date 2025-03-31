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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <memory/vaddr.h>

enum {
  TK_NOTYPE = 256,
  TK_EQ,
  TK_NEQ,
  TK_AND,
  TK_NUM,
  TK_HEX,
  TK_REG,
  TK_NEG,
  TK_DEREF
  /* TODO: Add more token types */
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {     

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"0x[0-9a-fA-F]+", TK_HEX}, // hex
  {"[0-9]+", TK_NUM},   // number
  {"\\$[a-zA-Z0-9_]+", TK_REG}, // reg
  {"\\+", '+'},         // plus
  {"-", '-'},           // subtract
  {"\\*", '*'},         // multiply or dereference
  {"/", '/'},           // divide
  {"\\(", '('},         // left bracket
  {"\\)", ')'},         // right bracket
  {"==", TK_EQ},        // equal
  {"!=", TK_NEQ},           // not equal
  {"&&", TK_AND},           // logical AND
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

// 对tokens数组扩容
// static Token tokens[32] __attribute__((used)) = {};
static Token tokens[655] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        // Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            // i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE:
            break;

          case TK_EQ:
          case TK_NEQ:
          case TK_AND:
          case TK_NUM:
          case TK_HEX:
          case TK_REG:
          case '+':
          case '-':
          case '*':
          case '/':
          case '(':
          case ')':
            tokens[nr_token].type = rules[i].token_type;
            // printf("%d\n", tokens[nr_token].type);
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token++;
            break;
        
          default:
            printf("Unknown token type");
            return false;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  /* Second pass: identify special cases (negative and dereference) */
  for (i = 0; i < nr_token; i++) {
    /* Handle negative sign */
    if (tokens[i].type == '-' && 
        (i == 0 || 
         tokens[i-1].type == '+' ||
         tokens[i-1].type == '-' ||
         tokens[i-1].type == '*' ||
         tokens[i-1].type == '/' ||
         tokens[i-1].type == '(' ||
         tokens[i-1].type == TK_EQ ||
         tokens[i-1].type == TK_NEQ ||
         tokens[i-1].type == TK_AND)) {
      tokens[i].type = TK_NEG;
    }
    
    /* Handle dereference */
    if (tokens[i].type == '*' && 
        (i == 0 || 
         tokens[i-1].type == '+' ||
         tokens[i-1].type == '-' ||
         tokens[i-1].type == '*' ||
         tokens[i-1].type == '/' ||
         tokens[i-1].type == '(' ||
         tokens[i-1].type == TK_EQ ||
         tokens[i-1].type == TK_NEQ ||
         tokens[i-1].type == TK_AND ||
         tokens[i-1].type == TK_NEG)) {
      tokens[i].type = TK_DEREF;
    }
  }

  return true;
}

bool check_parentheses (int p, int q) {
  if (tokens[p].type=='(' && tokens[q].type==')') {
    int depth = 0; // dee
    for (int i = p; i <= q; i++) {
      if (tokens[i].type=='(') depth++;
      else if (tokens[i].type==')') depth--;

      // the leftest parenthese is matched while the pointer isn't at the end
      // process the expression using the main operator
      if (depth == 0) return i==q;
    }
  }
  return false;
}

int get_main_op(int p, int q) {
  int main_op_pos = -1;
  int min_priority = INT8_MAX;
  int depth = 0;

  for (int i = p; i <= q; i++) {
    if (tokens[i].type == '(') {
      depth++;
    } else if (tokens[i].type == ')') {
      depth--;
    }

    if (depth == 0) {
      int current_priority = INT8_MAX;
      
      if (tokens[i].type == TK_AND) {
        current_priority = 0; // Lowest priority
      }
      else if (tokens[i].type == TK_EQ || tokens[i].type == TK_NEQ) {
        current_priority = 1;
      }
      else if (tokens[i].type == '+' || tokens[i].type == '-') {
        current_priority = 2;
      }
      else if (tokens[i].type == '*' || tokens[i].type == '/') {
        current_priority = 3;
      }
      else {
        continue; // Not an operator we care about
      }

      // For equal priority, we want the rightmost operator (left associativity)
      if (current_priority <= min_priority) {
        min_priority = current_priority;
        main_op_pos = i;
      }
    }
    // printf("%d\n", min_priority);
  }
  
  return main_op_pos;
}

// Make it a signed calculation
// get all the "uint32_t" changed to "int32_t"
// to avoid the situation that 12/-4=0 !!!
int32_t eval(int p, int q, bool *legal) {
  *legal = true;
  if (p > q) {
    /* Bad expression */
    *legal = false;
    return 0;
  }
  else if (p == q) {
    /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
     */
    if (tokens[p].type == TK_NUM) {
      int32_t result = strtol(tokens[p].str, NULL, 10);
      return result;
    }
    else if (tokens[p].type == TK_HEX) {
      int32_t result = strtol(tokens[p].str, NULL, 16);
      return result;
    }
    else if (tokens[p].type == TK_REG) {
      // Handle register access
      bool success = false;
      int32_t val = isa_reg_str2val(tokens[p].str + 1, &success); // Skip '$'
      if (!success) {
        *legal = false;
        return 0;
      }
      return val;
    }
    else {
      *legal = false;
      return 0;
    }
  }
  else if (check_parentheses(p, q) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    return eval(p + 1, q - 1, legal);
  }
  else {
    // handle negative signs(maybe there will be more than one negative sign)
    while (p <= q && tokens[p].type == TK_NEG) {
      int32_t val = eval(p + 1, q, legal);
      if (!*legal) return 0;
      return -(int32_t)val;
    }

    while (p <= q && tokens[p].type == TK_DEREF) {
      int32_t addr = eval(p + 1, q, legal);
      if (!*legal) return 0;
      // 检查地址是否对齐
      if (addr & 0x3) {
          *legal = false;
          return 0;
      }
      int32_t val = vaddr_read(addr, 4);
      return val;
    }

    // find the position of operator
    int op = get_main_op(p, q);
    // printf("%d\n", op);
    if (op < 0) {
      *legal = false;
      return 0;
    }
    
    int32_t val1 = eval(p, op - 1, legal);
    if (!*legal) return 0;
    int32_t val2 = eval(op + 1, q, legal);
    if (!*legal) return 0;

    switch (tokens[op].type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': if (val2 == 0) {
        *legal = false;
        return 0;
      }
      return val1 / val2;
      case TK_EQ: return val1 == val2 ? 1 : 0;
      case TK_NEQ: return val1 != val2 ? 1 : 0;
      case TK_AND: return (val1 && val2) ? 1 : 0;
      default: assert(0);
    }
  }
}

int32_t expr(char *expr, bool *success) {
  if (!make_token(expr)) {
    *success = false;
    return 0;
  }

  return eval(0, nr_token - 1, success);
}