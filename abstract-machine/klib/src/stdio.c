#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

static void itoa(int num, char *str) {
  int i = 0;
  int is_negative = 0;

  if (num < 0) {
      is_negative = 1;
      num = -num;
  }

  // 处理0的特殊情况
  if (num == 0) {
      str[i++] = '0';
  } else {
      // 生成数字字符串（逆序）
      while (num > 0) {
          str[i++] = (num % 10) + '0';
          num /= 10;
      }
  }

  // 添加负号
  if (is_negative) {
      str[i++] = '-';
  }

  str[i] = '\0';

  // 反转字符串
  for (int j = 0; j < i / 2; j++) {
      char tmp = str[j];
      str[j] = str[i - j - 1];
      str[i - j - 1] = tmp;
  }
}

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  if (out == NULL || fmt == NULL) return -1;

    va_list ap;
    va_start(ap, fmt);

    char *start = out;
    char num_buf[32]; // 临时存储数字字符串

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
                case 'd': {
                    int num = va_arg(ap, int);
                    itoa(num, num_buf);
                    strcpy(out, num_buf);
                    out += strlen(num_buf);
                    break;
                }
                case 's': {
                    char *s = va_arg(ap, char *);
                    if (!s) s = "(null)";
                    strcpy(out, s);
                    out += strlen(s);
                    break;
                }
                case 'c': {
                    *out++ = (char)va_arg(ap, int);
                    break;
                }
                case '%': {
                    *out++ = '%';
                    break;
                }
                default: {
                    va_end(ap);
                    return -1; // 不支持的格式
                }
            }
            fmt++;
        } else {
            *out++ = *fmt++;
        }
    }

    *out = '\0';
    va_end(ap);
    return (int)(out - start);
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
