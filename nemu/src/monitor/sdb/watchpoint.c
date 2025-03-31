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

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  char expr[128];
  uint32_t last_value;
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp() {
  if (free_ == NULL) {
    printf("No free watchpoints available.\n");
    return NULL;
  }

  // 从空闲池中取出一个监视点
  WP *wp = free_;
  free_ = free_->next;

  // 将监视点添加到使用中的链表
  wp->next = head;
  head = wp;

  return wp;
}

void free_wp(WP *wp) {
  if (wp == NULL) {
    return;
  }

  // handle the situation that wp is the head node or just a node in it
  if (head == wp) {
    // wp is the head
    head = head->next;
  } else {
    // wp is one of the node in the list
    // get the prev_node of wp
    WP *prev = head;
    while (prev != NULL && prev->next != wp) {
      prev = prev->next;
    }
    if (prev == NULL) {
      // wp is not in the list, so prev is null
      return;
    }
    prev->next = wp->next; // kick the wp away from the list
  }
  // put the wp into the free list
  wp->next = free_;
  free_ = wp;
}

// 以下函数均为外部访问监视点池的接口

// 设置监视点
void set_watchpoint(char *expr_str) {
  // 分配一个监视点结构体
  WP *wp = new_wp();
  if (wp == NULL) {
    printf("No free watchpoints available.\n");
    return;
  }

  // 记录表达式
  strncpy(wp->expr, expr_str, sizeof(wp->expr) - 1);
  wp->expr[sizeof(wp->expr) - 1] = '\0';  // 确保字符串以 '\0' 结尾

  // 对表达式求值
  bool success;
  int32_t value = expr(wp->expr, &success);  // 调用 expr 函数
  if (!success) {
    printf("Invalid expression: %s\n", wp->expr);
    free_wp(wp);  // 释放监视点结构体
    return;
  }

  // 初始化监视点的值
  wp->last_value = value;

  // 提示用户监视点设置成功
  printf("Watchpoint %d: %s (initial value = %d)\n", wp->NO, wp->expr, wp->last_value);
}

void scan_watchpoints() {
  for (WP *wp = head; wp != NULL; wp = wp->next) {
    bool success;
    int32_t current_value = expr(wp->expr, &success);
    if (!success) {
      printf("Error evaluating expression: %s\n", wp->expr);
      continue;
    }

    if (current_value != wp->last_value) {
      printf("Watchpoint %d (%s) triggered:\n", wp->NO, wp->expr);
      printf("Old value = %d\n", wp->last_value);
      printf("New value = %d\n", current_value);

      if (nemu_state.state == NEMU_QUIT) return;
      else nemu_state.state = NEMU_STOP;

      // 更新上一次的值
      wp->last_value = current_value;
    }
  }
}

void info_watchpoints() {
  if (head == NULL) {
    printf("No watchpoints set.\n");
    return;
  }

  printf("Num     Expression         Last Value\n");
  for (WP *wp = head; wp != NULL; wp = wp->next) {
    printf("%-8d%-20s%d\n", wp->NO, wp->expr, wp->last_value);
  }
}

void delete_watchpoint(int NO) {
  WP *wp = head;
  WP *prev = NULL;

  // 查找要删除的监视点
  while (wp != NULL && wp->NO != NO) {
    prev = wp;
    wp = wp->next;
  }

  if (wp == NULL) {
    printf("Watchpoint %d not found.\n", NO);
    return;
  }

  // 从使用中的链表中移除监视点
  if (prev == NULL) {
    // 要删除的是头节点
    head = wp->next;
  } else {
    // 要删除的是中间或尾节点
    prev->next = wp->next;
  }

  // 调用 free_wp() 将监视点放回空闲链表
  free_wp(wp);

  printf("Deleted watchpoint %d\n", NO);
}