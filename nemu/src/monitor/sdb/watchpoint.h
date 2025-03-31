// src/monitor/sdb/watchpoint.h

#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include <stdint.h>

void set_watchpoint(char *expr_str);
void scan_watchpoints();
void info_watchpoints();
void delete_watchpoint(int NO);


#endif // __WATCHPOINT_H__