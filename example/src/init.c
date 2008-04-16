
#include <libos/libos.h>
#include <libos/percpu.h>
#include <libos/fsl-booke-tlb.h>
#include <libos/trapframe.h>
#include <libos/uart.h>
#include <libos/ns16550.h>
#include <libfdt.h>
#include <libos/io.h>

extern uint8_t init_stack_top;

cpu_t cpu0 = {
	.kstack = &init_stack_top - FRAMELEN,
	.client = 0,
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
	chardev_t *cd;

	core_init();

	unsigned long heap = (unsigned long)0x11000000; // FIXME-- hardcoded location for heap
	heap = (heap + 15) & ~15;

	alloc_init(heap, heap + (0x100000-1));  // FIXME: hardcoded 1MB heap
	console_init(ns16550_init((uint8_t *)CCSRBAR_VA + UART_OFFSET, 0, 0, 16));
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

static void tlb1_init(void)
{
	tlb1_set_entry(0, CCSRBAR_VA, CCSRBAR_PA, CCSRBAR_SIZE, TLB_MAS2_IO,
	               TLB_MAS3_KERN, 0, 0, 0);
}

void start(unsigned long devtree_ptr)
{
	init(devtree_ptr);

	printf("Hello World\n");
}

void secondary_init(void)
{
	/*  stub for now */
}
