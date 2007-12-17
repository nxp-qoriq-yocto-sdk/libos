#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <libos/bitops.h>
#include <libos/console.h>

extern struct console_calls console;

void console_init(void)
{

}

int putchar(int c)
{
	if (c == '\n')
		console.putc('\r');

	console.putc(c);
	return c;
}

void puts_len(const char *s, int len)
{
	while (*s && len--)
		putchar(*s++);
}

int puts(const char *s)
{
	puts_len(s, INT_MAX);
	console.putc('\r');
	console.putc('\n');
	return 0;
}

size_t printf(const char *str, ...)
{
	enum {
		buffer_size = 4096,
	};

	static char buffer[buffer_size];
	static uint32_t lock;

//	spin_lock(&lock);

	va_list args;
	va_start(args, str);
	size_t ret = vsnprintf(buffer, buffer_size, str, args);
	va_end(args);
	
	if (ret > buffer_size)
		ret = buffer_size;
	
	puts_len(buffer, ret);
//	spin_unlock(&lock);
	return ret;
}
