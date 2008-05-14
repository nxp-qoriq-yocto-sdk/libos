#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <libos/bitops.h>
#include <libos/console.h>
#include <libos/chardev.h>
#include <libos/readline.h>

static chardev_t *console;
#ifdef CONFIG_LIBOS_QUEUE
DECLARE_QUEUE(early_console, 4096);
static queue_t *qconsole = &early_console;
#endif
static uint32_t console_lock;
int crashing;

#ifdef CONFIG_LIBOS_READLINE
readline_t *rl_console;
#endif

uint8_t loglevels[NUM_LOGTYPES] = 
	{ [0 ... NUM_LOGTYPES - 1] = CONFIG_LIBOS_DEFAULT_LOGLEVEL };

void console_init(chardev_t *cd)
{
#ifdef CONFIG_LIBOS_QUEUE
	if (qconsole == &early_console) {
		assert(early_console.head == 0);
		
		cd->ops->tx(cd, early_console.buf,
		            early_console.tail, CHARDEV_BLOCKING);

		qconsole = NULL;
	}
#endif

	console = cd;
}

#ifdef CONFIG_LIBOS_QUEUE
void qconsole_init(queue_t *q)
{
	if (qconsole == &early_console) {
		assert(early_console.head == 0);
		queue_write(q, early_console.buf, early_console.tail);
		queue_notify_consumer(q);
	}

	qconsole = q;
}
#endif

static int __putchar(int c)
{
	uint8_t ch = c;

	if (console) {
		if (c == '\n')
			console->ops->tx(console, (uint8_t *)"\r", 1, CHARDEV_BLOCKING);

		console->ops->tx(console, &ch, 1, CHARDEV_BLOCKING);
	}

#ifdef CONFIG_LIBOS_QUEUE
	if (qconsole) {
		if (c == '\n')
			queue_writechar(qconsole, '\r');

		queue_writechar(qconsole, ch);
	}
#endif

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

#ifdef CONFIG_LIBOS_QUEUE
	if (qconsole)
		queue_notify_consumer(qconsole);
#endif

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

#ifdef CONFIG_LIBOS_QUEUE
	if (qconsole)
		queue_notify_consumer(qconsole);
#endif

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

	__puts_len(s, INT_MAX);
	__puts_len("\r\n", 2);

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
