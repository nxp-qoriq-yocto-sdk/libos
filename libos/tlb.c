
#include "tlb.h"
#include "spr.h"



void tlb1_write_entry(unsigned int idx, uint32_t mas1,
                 uint32_t mas2, uint32_t mas3, uint32_t mas7)
{
        uint32_t mas0;

        //debugf("tlb1_write_entry: s\n");

        /* Select entry */
        mas0 = MAS0_TLBSEL(1) | MAS0_ESEL(idx);
        //debugf("tlb1_write_entry: mas0 = 0x%08x\n", mas0);

        mtspr(SPR_MAS0, mas0);
        __asm volatile("isync");
        mtspr(SPR_MAS1, mas1);
        __asm volatile("isync");
        mtspr(SPR_MAS2, mas2);
        __asm volatile("isync");
        mtspr(SPR_MAS3, mas3);
        __asm volatile("isync");
        mtspr(SPR_MAS7, mas7);
        __asm volatile("isync; tlbwe; isync; msync");

        //debugf("tlb1_write_entry: e\n");;
}

/*
 * Return the largest uint value log such that 2^log <= num.
 */
static unsigned int
ilog2(unsigned int num)
{
        int lz;

        __asm ("cntlzw %0, %1" : "=r" (lz) : "r" (num));
        return (31 - lz);
}


/*
 * Convert TLB TSIZE value to mapped region size.
 */
static uint32_t
tsize2size(unsigned int tsize)
{
        /*
         * size = 4^tsize KB
         * size = 4^tsize * 2^10 = 2^(2 * tsize - 10)
         */

        return (1 << (2 * tsize)) * 1024;
}


/*
 * Convert region size (must be power of 4) to TLB TSIZE value.
 */
unsigned int
size2tsize(uint32_t size)
{
        /*
         * tsize = log2(size) / 2 - 5
         */

        return (ilog2(size) / 2 - 5);
}

