#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

void reboot(void);
void puts(const char *string);

void main(void);

void *memset(void *dst, int c, size_t count);
void *memcpy(void *dst, const void *src, size_t count);
void *memchr(const void *m, int c, size_t count);
int memcmp(const void *m1, const void *m2, size_t count);

size_t strlen(const char *s);
char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, size_t count);
char *strchr(const char *s, int c);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t count);
char *strdup(const char *s);

void *malloc(size_t count);
void free(void *ptr);

void panic(const char *message);

size_t vsnprintf(char *buffer, size_t buffer_size, const char *fmt, va_list arg);
size_t snprintf(char *buffer, size_t buffer_size, const char *fmt, ...);

size_t vprintf(const char *fmt, va_list arg);
size_t printf(const char *fmt, ...);

char *strdup_vprintf(const char *fmt, va_list arg);
char *strdup_printf(const char *fmt, ...);
