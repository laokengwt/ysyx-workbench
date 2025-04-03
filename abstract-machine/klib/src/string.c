#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  const char *temp = s;
  while (*temp++);
  return --temp - s;
}

char *strcpy(char *dst, const char *src) {
  char *temp = dst;
  while ((*temp++ = *src++));
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  char *temp = dst;
  while (n-- && (*temp++ = *src++));
  while (n-- > 0) *temp = '\0';
  return dst;
}

char *strcat(char *dst, const char *src) {
  char *temp = dst;
  while (*temp) temp++;
  while ((*temp++ = *src++));
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  while (*s1 && (*s1 == *s2)) {
    s1++;
    s2++;
  }
  return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  while (n-- && *s1 && (*s1 == *s2)) {
    s1++;
    s2++;
  }
  // check if n is ended;
  return n == (size_t)-1 ? 0 : *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

void *memset(void *s, int c, size_t n) {
  unsigned char *temp = s;
  while (n--) *temp++ = (unsigned char)c;
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  unsigned char *d = dst;
  const unsigned char *s = src;
  // handle the situation that dst and src are overlaped;
  if (d < s) {
    while (n--) *d++ = *s++;
  } else {
    d += n;
    s += n;
    while (n--) *--d = *--s;
  }
  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  unsigned char *d = out;
  const unsigned char *s = in;
  while (n--) *d++ = *s++;
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const unsigned char *temp1 = s1;
  const unsigned char *temp2 = s2;
  while (n--) {
    if (*temp1 != *temp2) return *temp1 - *temp2;
    temp1++;
    temp2++;
  }
  return 0;
}

#endif
