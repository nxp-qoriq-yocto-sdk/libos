#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <libos/bitops.h>
#include <libos/console.h>
#include <libos/chardev.h>

static chardev_t *console;
static uint32_t console_lock;
int crashing;

void console_init(chardev_t *cd)
{
	console = cd;
}

static int __putchar(int c)
{
	uint8_t ch = c;

	if (console) {
		if (c == '\n')
			console->ops->tx(console, (uint8_t *)"\r", 1, CHARDEV_BLOCKING);

		console->ops->tx(console, &ch, 1, CHARDEV_BLOCKING);
	}

	return c;
}

int putchar(int c)
{
	spin_lock(&console_lock);
	int ret = __putchar(c);
	spin_unlock(&console_lock);
	return ret;
}

static void __puts_len(const char *s, size_t len)
{
	while (*s && len--)
		__putchar(*s++);
}

void puts_len(const char *s, size_t len)
{
	spin_lock(&console_lock);
	__puts_len(s, len);
	spin_unlock(&console_lock);
}

int puts(const char *s)
{
	spin_lock(&console_lock);

	if (console) {
		__puts_len(s, INT_MAX);
		console->ops->tx(console, (uint8_t *)"\r\n", 2, CHARDEV_BLOCKING);
	}

	spin_unlock(&console_lock);
	return 0;
}

size_t printf(const char *str, ...)
{
	enum {
		buffer_size = 4096,
	};

	static char buffer[buffer_size];

	spin_lock(&console_lock);

	va_list args;
	va_start(args, str);
	size_t ret = vsnprintf(buffer, buffer_size, str, args);
	va_end(args);
	
	if (ret > buffer_size)
		ret = buffer_size;
	
	__puts_len(buffer, ret);
	spin_unlock(&console_lock);
	return ret;
}
