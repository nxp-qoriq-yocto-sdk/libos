#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <libos/bitops.h>
#include <libos/console.h>
#include <libos/chardev.h>
#include <libos/readline.h>

static chardev_t *console;
static uint32_t console_lock;
int crashing;

#ifdef CONFIG_LIBOS_READLINE
readline_t *rl_console;
#endif

uint8_t loglevels[NUM_LOGTYPES];

void console_init(chardev_t *cd)
{
	memset(loglevels, CONFIG_LIBOS_DEFAULT_LOGLEVEL, NUM_LOGTYPES);
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
	register_t saved = spin_lock_critsave(&console_lock);

#ifdef CONFIG_LIBOS_READLINE
	if (rl_console && !crashing)
		readline_suspend(rl_console);
#endif

	int ret = __putchar(c);

#ifdef CONFIG_LIBOS_READLINE
	if (rl_console && !crashing && c == '\n')
		readline_resume(rl_console);
#endif

	spin_unlock_critsave(&console_lock, saved);
	return ret;
}

static void __puts_len(const char *s, size_t len)
{
	int last = '\n';

#ifdef CONFIG_LIBOS_READLINE
	if (rl_console && !crashing)
		readline_suspend(rl_console);
#endif

	while (*s && len--) {
		last = *s;
		__putchar(*s++);
	}

#ifdef CONFIG_LIBOS_READLINE
	if (rl_console && !crashing && last == '\n')
		readline_resume(rl_console);
#endif
}

void puts_len(const char *s, size_t len)
{
	register_t saved = spin_lock_critsave(&console_lock);
	__puts_len(s, len);
	spin_unlock_critsave(&console_lock, saved);
}

int puts(const char *s)
{
	register_t saved = spin_lock_critsave(&console_lock);

	if (console) {
		__puts_len(s, INT_MAX);
		__puts_len("\r\n", 2);
	}

	spin_unlock_critsave(&console_lock, saved);
	return 0;
}

size_t vprintf(const char *str, va_list args)
{
	enum {
		buffer_size = 4096,
	};

	static char buffer[buffer_size];
	register_t saved = spin_lock_critsave(&console_lock);

	size_t ret = vsnprintf(buffer, buffer_size, str, args);
	if (ret > buffer_size)
		ret = buffer_size;
	
	__puts_len(buffer, ret);

	spin_unlock_critsave(&console_lock, saved);
	return ret;
}

size_t printf(const char *str, ...)
{
	size_t ret;
	va_list args;

	va_start(args, str);
	ret = vprintf(str, args);
	va_end(args);

	return ret;
}
