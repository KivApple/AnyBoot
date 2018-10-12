#include "stdfunctions.h"

void *memset(void *dst, int c, size_t count) {
	char *dstBytes = (char*) dst;
	while (count--) {
		*(dstBytes++) = c;
	}
	return dst;
}

void *memcpy(void *dst, const void *src, size_t count) {
	const char *srcBytes = (const char*) src;
	char *dstBytes = (char*) dst;
	while (count--) {
		*(dstBytes++) = *(srcBytes++);
	}
	return dst;
}

void *memchr(const void *m, int c, size_t count) {
	const char *bytes = (const char*) m;
	while (count--) {
		if (*bytes == c) {
			return (void*) bytes;
		}
		bytes++;
	}
	return NULL;
}

int memcmp(const void *m1, const void *m2, size_t count) {
	const char *bytes1 = (const char*) m1;
	const char *bytes2 = (const char*) m2;
	while (count--) {
		int diff = *(bytes1++) - *(bytes2++);
		if (diff != 0) {
			return diff;
		}
	}
	return 0;
}

size_t strlen(const char *s) {
	size_t len = 0;
	while (s[len++]);
	return len - 1;
}

char *strcpy(char *dst, const char *src) {
	memcpy(dst, src, strlen(src) + 1);
	return dst;
}

char *strncpy(char *dst, const char *src, size_t count) {
	size_t len = strlen(src);
	if (len >= count) len = count - 1;
	memcpy(dst, src, len);
	dst[len] = '\0';
	return dst;
}

char *strchr(const char *s, int c) {
	return memchr(s, c, strlen(s) + 1);
}

int strcmp(const char *s1, const char *s2) {
	do {
		int diff = *s1 - *s2;
		if (diff != 0) {
			return diff;
		}
	} while (*s1);
	return 0;
}

int strncmp(const char *s1, const char *s2, size_t count) {
	do {
		if (count-- == 0) break;
		int diff = *s1 - *s2;
		if (diff != 0) {
			return diff;
		}
	} while (*s1);
	return 0;
}

char *strdup(const char *s) {
	size_t len = strlen(s) + 1;
	char *new_s = malloc(len);
	if (new_s) {
		memcpy(new_s, s, len);
	}
	return new_s;
}

void panic(const char *message) {
	puts(message);
	reboot();
}

typedef struct PrintfState {
	char *buffer;
	size_t buffer_size;
	size_t offset;
} PrintfState;

static void vsnprintf_putc(PrintfState *state, char ch) {
	if (state->offset < state->buffer_size) {
		state->buffer[state->offset] = ch;
	}
	state->offset++;
}

static void vsnprintf_puts(PrintfState *state, const char *s, size_t padding, char pad_char) {
	if (!s) {
		s = "(null)";
	}
	size_t len = strlen(s);
	if (len < padding) {
		padding -= len;
	} else {
		padding = 0;
	}
	while (padding--) {
		vsnprintf_putc(state, pad_char);
	}
	while (*s) {
		vsnprintf_putc(state, *(s++));
	}
}

static void vsnprintf_putn(PrintfState *state, unsigned long long n, unsigned char radix, bool minus, size_t padding, char pad_char) {
	static const char digits[] = "0123456789ABCDEF";
	char buffer[64 + 2];
	size_t offset = sizeof(buffer);
	buffer[--offset] = '\0';
	do {
		buffer[--offset] = digits[n % radix];
		n /= radix;
	} while (n > 0);
	if (minus) {
		buffer[--offset] = '-';
	}
	vsnprintf_puts(state, buffer + offset, padding, pad_char);
}

static void vsnprintf_putu(PrintfState *state, unsigned long long n, unsigned char radix, size_t padding, char pad_char) {
	vsnprintf_putn(state, n, radix, false, padding, pad_char);
}

static void vsnprintf_puti(PrintfState *state, long long n, unsigned char radix, size_t padding, char pad_char) {
	bool minus = n < 0;
	vsnprintf_putn(state, minus ? -n : n, radix, minus, padding, pad_char);
}

size_t vsnprintf(char *buffer, size_t buffer_size, const char *fmt, va_list arg) {
	PrintfState state;
	state.buffer = buffer;
	state.buffer_size = buffer_size;
	state.offset = 0;
	while (*fmt) {
		if (*fmt == '%') {
			const char *saved_fmt = fmt++;
			char pad_char = ' ';
			if (*fmt == '0') {
				pad_char = '0';
				fmt++;
			}
			size_t padding = 0;
			while (*fmt >= '0' && *fmt <= '9') {
				padding = padding * 10 + *fmt - '0';
				fmt++;
			}
			size_t long_flag = 0;
			while (*fmt == 'l') {
				long_flag++;
				fmt++;
			}
			if (*fmt == 'z') {
				long_flag = sizeof(int) >= sizeof(size_t) ? 0 : (sizeof(long) >= sizeof(size_t) ? 1 : 2);
				fmt++;
			}
			switch (*(fmt++)) {
				case '%':
					vsnprintf_puts(&state, "%", padding, pad_char);
					continue;
				case 'c': {
					char buffer[2] = { va_arg(arg, int), '\0' };
					vsnprintf_puts(&state, buffer, padding, pad_char);
					continue;
				}
				case 's':
					vsnprintf_puts(&state, va_arg(arg, const char*), padding, pad_char);
					continue;
				case 'i':
					if (long_flag == 0) {
						vsnprintf_puti(&state, va_arg(arg, int), 10, padding, pad_char);
					} else if (long_flag == 1) {
						vsnprintf_puti(&state, va_arg(arg, long), 10, padding, pad_char);
					} else {
						vsnprintf_puti(&state, va_arg(arg, long long), 10, padding, pad_char);
					}
					continue;
				case 'u':
					if (long_flag == 0) {
						vsnprintf_putu(&state, va_arg(arg, unsigned int), 10, padding, pad_char);
					} else if (long_flag == 1) {
						vsnprintf_putu(&state, va_arg(arg, unsigned long), 10, padding, pad_char);
					} else {
						vsnprintf_putu(&state, va_arg(arg, unsigned long long), 10, padding, pad_char);
					}
					continue;
				case 'x':
					if (long_flag == 0) {
						vsnprintf_putu(&state, va_arg(arg, unsigned int), 16, padding, pad_char);
					} else if (long_flag == 1) {
						vsnprintf_putu(&state, va_arg(arg, unsigned long), 16, padding, pad_char);
					} else {
						vsnprintf_putu(&state, va_arg(arg, unsigned long long), 16, padding, pad_char);
					}
					continue;
				case 'b':
					if (long_flag == 0) {
						vsnprintf_putu(&state, va_arg(arg, unsigned int), 2, padding, pad_char);
					} else if (long_flag == 1) {
						vsnprintf_putu(&state, va_arg(arg, unsigned long), 2, padding, pad_char);
					} else {
						vsnprintf_putu(&state, va_arg(arg, unsigned long long), 2, padding, pad_char);
					}
					continue;
				case 'p':
					vsnprintf_puts(&state, "0x", padding > sizeof(size_t) * 2 ? padding - sizeof(size_t) * 2 : 0, pad_char);
					vsnprintf_putu(&state, va_arg(arg, size_t), 16, sizeof(size_t) * 2, '0');
					continue;
				default:
					vsnprintf_putc(&state, '%');
			}
			fmt = saved_fmt;
		}
		vsnprintf_putc(&state, *(fmt++));
	}
	if (buffer_size > 0) {
		buffer[state.offset < buffer_size ? state.offset : buffer_size - 1] = '\0';
	}
	return state.offset;
}

size_t snprintf(char *buffer, size_t buffer_size, const char *fmt, ...) {
	va_list arg;
	va_start(arg, fmt);
	size_t result = vsnprintf(buffer, buffer_size, fmt, arg);
	va_end(arg);
	return result;
}

size_t vprintf(const char *fmt, va_list arg) {
	va_list arg_copy;
	va_copy(arg_copy, arg);
	size_t len = vsnprintf(NULL, 0, fmt, arg_copy);
	va_end(arg_copy);
	if (len < 1024) {
		char buffer[1024];
		vsnprintf(buffer, sizeof(buffer), fmt, arg);
		puts(buffer);
	} else {
		char *buffer = malloc(len + 1);
		if (buffer) {
			vsnprintf(buffer, len + 1, fmt, arg);
			puts(buffer);
			free(buffer);
		} else {
			printf("WARNING: printf() failed to allocate %zu bytes\n", len + 1);
		}
	}
	return len;
}

size_t printf(const char *fmt, ...) {
	va_list arg;
	va_start(arg, fmt);
	size_t result = vprintf(fmt, arg);
	va_end(arg);
	return result;
}

char *strdup_vprintf(const char *fmt, va_list arg) {
	va_list arg_copy;
	va_copy(arg_copy, arg);
	size_t len = vsnprintf(NULL, 0, fmt, arg_copy);
	va_end(arg_copy);
	char *result = malloc(len + 1);
	if (result) {
		vsnprintf(result, len + 1, fmt, arg);
	}
	return result;
}

char *strdup_printf(const char *fmt, ...) {
	va_list arg;
	va_start(arg, fmt);
	char *result = strdup_vprintf(fmt, arg);
	va_end(arg);
	return result;
}
