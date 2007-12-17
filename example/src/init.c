
#include <libos/libos.h>
#include <libos/percpu.h>
#include <libos/fsl-booke-tlb.h>
#include <libos/trapframe.h>
#include <libos/uart.h>

extern uint8_t init_stack_top;

cpu_t cpu0 = {
        .kstack = &init_stack_top - FRAMELEN,
        .client = 0,
};


struct console_calls console = {
	.putc = uart_putc
};


static void tlb1_init(void);
static void  core_init(void);

/* hardcoded hack for now */
#define CCSRBAR_PA              0xfe000000
#define CCSRBAR_VA              0x01000000
#define CCSRBAR_SIZE            TLB_TSIZE_16M
#define UART_OFFSET 0x11c500

void init(unsigned long devtree_ptr)
{

	core_init();

	uart_init(CCSRBAR_VA + UART_OFFSET);

	console_init();

}


static void core_init(void)
{

    /* set up a TLB entry for CCSR space */
    tlb1_init();

}

/*
 *    after tlb1_init:
 *        TLB1[0]  = CCSR
 *        TLB1[15] = OS image 16M
 */

extern int print_ok;  /* set to indicate printf can work now */

static void tlb1_init(void)
{
        tlb1_set_entry(0, CCSRBAR_VA, CCSRBAR_PA, CCSRBAR_SIZE, TLB_MAS2_IO,
                       TLB_MAS3_KERN, 0, 0, 0);

	print_ok = 1;
}

void start(unsigned long devtree_ptr)
{
	init(devtree_ptr);

	printf("Hello World\n");
}
