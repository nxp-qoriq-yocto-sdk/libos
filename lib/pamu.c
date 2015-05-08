/*
 * Copyright (C) 2008-2011 Freescale Semiconductor, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <libos/pamu.h>
#include <libos/alloc.h>
#include <libos/printlog.h>
#include <libos/io.h>
#include <libos/bitops.h>
#include <libos/errors.h>

#include <limits.h>

/* expose PAMU tables & bypass reg to client app */
static paace_t *ppaact;
static paace_t *spaact;
static ome_t *omt;
static uint32_t *pamubypenreg_vaddr;
static uint32_t pamu_lock;
static size_t pamu_reg_space_size;
static unsigned long pamu_reg_space_vaddr;
static unsigned long pamu_fspi;
static unsigned int max_subwindow_count;


static unsigned int map_addrspace_size_to_wse(phys_addr_t addrspace_size)
{
	assert(!(addrspace_size & (addrspace_size - 1)));

	/* window size is 2^(WSE+1) bytes */
	return count_lsb_zeroes(addrspace_size >> PAMU_PAGE_SHIFT) + PAMU_PAGE_SHIFT - 1;
}
static unsigned int map_subwindow_cnt_to_wce(uint32_t subwindow_cnt)
{
       /* window count is 2^(WCE+1) bytes */
       return count_lsb_zeroes_32(subwindow_cnt) - 1;
}


/**  Sets validation bit of PACCE
 *
 * @parm[in] liodn PAACT index for desired PAACE
 *
 * @return Returns 0 upon success else error code < 0 returned
 */
int32_t pamu_enable_liodn(uint32_t liodn)
{
	paace_t *ppaace;

	ppaace = pamu_get_ppaace(liodn);
	if (!ppaace)
		return ERR_NOTFOUND;

	if (!get_bf(ppaace->addr_bitfields, PPAACE_AF_WSE)) {
		printlog(LOGTYPE_PAMU, LOGLEVEL_ERROR,
		     "%s: liodn %d not configured\n", __func__, liodn);
		return ERR_INVALID;
	}

	/* Ensure that all other stores to the ppaace complete first */
	sync();

	ppaace->addr_bitfields |= PAACE_V_VALID;
	sync();

	return 0;
}

/** Clears validation bit of PACCE
 *
 * @parm[in]  liodn PAACT index for desired PAACE
 *
 * @return Returns 0 upon success else error code < 0 returned
 */
int32_t pamu_disable_liodn(uint32_t liodn)
{
	paace_t *ppaace;

	ppaace = pamu_get_ppaace(liodn);
	if (!ppaace)
		return ERR_NOTFOUND;

	set_bf(ppaace->addr_bitfields, PAACE_AF_V, PAACE_V_INVALID);
	sync();

	return 0;
}

/** Initializes PAMU registers, tables, and bypass register base addresses.
 *
 * @param[in] pamu_reg_vaddr   virtual base address of PAMU mapped register space
 * @param[in] reg_space_size   size of the allocated pamu register space
 * @param[in] pamubypenr_vaddr pointer to virtual address of PAMU Bypass Enable Reg
 * @param[in] pamu_tbl_vbase   pointer to virtual base address of PAMU tables
 * @param[in] pamu_tbl_size    size of the allocated PAMU table space
 * @param[in] hw_ready         true if PAMU HW was already enabled, tables set,
 *			       and PAMUBYPENR set
 *
 * @return Returns 0 upon success else error code < 0 returned
 */
int32_t pamu_hw_init(void *pamu_reg_vaddr, size_t reg_space_size,
		     void *pamubypenr_vaddr, void *pamu_tbl_vbase,
		     size_t pamu_tbl_size, int hw_ready)
{
	void *ptr;
	pamu_mmap_regs_t *pamu_regs;
	uintptr_t pamu_offset = 0;
	uint32_t pamubypenr;
	phys_addr_t phys;
	unsigned long pamu_reg_off;
	unsigned long pamu_reg_base = (unsigned long) pamu_reg_vaddr;
	uint32_t reg_val;

	printlog(LOGTYPE_MISC, LOGLEVEL_DEBUG, "Starting PAMU HW init\n");

	pamu_reg_space_vaddr = pamu_reg_base;
	pamu_reg_space_size = reg_space_size;
	pamubypenreg_vaddr = pamubypenr_vaddr;
	ptr = pamu_tbl_vbase;
	ppaact = ptr;
	ptr += align(PPAACT_SIZE + 1, PAMU_TABLE_ALIGNMENT);
	spaact = ptr;
	ptr += align(SPAACT_SIZE + 1, PAMU_TABLE_ALIGNMENT);
	omt = ptr;

	if ((ptr + OMT_SIZE) > (pamu_tbl_vbase + pamu_tbl_size)) {
		printlog(LOGTYPE_PAMU, LOGLEVEL_ERROR,
			 "PAMU tables don't fit in the allocted memory\n");
		return ERR_RANGE;
	}


	if (!ppaact) {
		printlog(LOGTYPE_PAMU, LOGLEVEL_ERROR,
			"PAMU table space not allocated\n");
		return ERR_INVALID;
	}

	if (pamu_reg_base > (pamu_reg_space_vaddr + pamu_reg_space_size)) {
		printlog(LOGTYPE_PAMU, LOGLEVEL_ERROR,
			"PAMU Register Space has been exceeded\n");
		return ERR_RANGE;
	}

	/* Check the version of the PAMU */
	reg_val = in32((uint32_t *)(pamu_reg_vaddr + PAMU_PC3));
	max_subwindow_count = 1 << (1 + PAMU_PC3_MWCE(reg_val));

	if (hw_ready)
		return 0;

	for (pamu_reg_off = 0; pamu_reg_off < pamu_reg_space_size; pamu_reg_off += PAMU_OFFSET) {
		pamu_offset = pamu_reg_base + pamu_reg_off;

		/* if this PAMU is already enabled just disable it back and
		 * enable it later. This should be safe as the boot-loader
		 * shouldn't be leaving in-flight dma transactions.
		 */
		if (in32((uint32_t *)(pamu_offset + PAMU_PC)) & PAMU_PC_PE) {
			out32((uint32_t *)(pamu_offset + PAMU_PC),
			      in32((uint32_t *)(pamu_offset + PAMU_PC)) & ~PAMU_PC_PE);

			printlog(LOGTYPE_PAMU, LOGLEVEL_DEBUG,
				"PAMU %lu already configured\n",
				(PAMU_IDX((pamu_offset - pamu_reg_space_vaddr))+1));
		}

		pamu_regs = (pamu_mmap_regs_t *) (pamu_offset + PAMU_MMAP_REGS_BASE);

		/* Workaround for erratum A-005982
		 *
		 * Writing either the primary PAACT base address high or low
		 * registers (PAMUx_PPBAH and PAMUx_PPBAL) or the secondary
		 * PAACT base address high or low registers (PAMUx_SPBAH and
		 * PAMUx_SPBAL) will not invalidate the contents of the primary
		 * PAACT cache as intended if the OMT cache is disabled
		 * (PAMUx_PC[OCE] = 0).
		 *
		 * To workaround, do a dummy OMT cache enable just before
		 * setting the PPAACT & SPAACT base registers.
		 */
		out32((uint32_t *)(pamu_offset + PAMU_PC), PAMU_PC_OCE);

		/* set up pointers to corenet control blocks */

		phys = virt_to_phys(ppaact);
		out32(&pamu_regs->ppbah, phys >> 32);
		out32(&pamu_regs->ppbal, (uint32_t)phys);
		phys = virt_to_phys(ppaact + PAACE_NUMBER_ENTRIES);
		out32(&pamu_regs->pplah, phys >> 32);
		out32(&pamu_regs->pplal, (uint32_t)phys);

		phys = virt_to_phys(spaact);
		out32(&pamu_regs->spbah, phys >> 32);
		out32(&pamu_regs->spbal, (uint32_t)phys);
		phys = virt_to_phys(spaact + SPAACE_NUMBER_ENTRIES);
		out32(&pamu_regs->splah, phys >> 32);
		out32(&pamu_regs->splal, (uint32_t)phys);

		phys = virt_to_phys(omt);
		out32(&pamu_regs->obah, phys >> 32);
		out32(&pamu_regs->obal, (uint32_t)phys);
		phys = virt_to_phys(omt + OME_NUMBER_ENTRIES);
		out32(&pamu_regs->olah, phys >> 32);
		out32(&pamu_regs->olal, (uint32_t)phys);

		/* Disable PAMU bypass for this PAMU */
		pamubypenr = in32(pamubypenreg_vaddr);
		pamubypenr &= ~(PAMU_BYP_BIT(pamu_offset - pamu_reg_space_vaddr));
		out32(pamubypenreg_vaddr, pamubypenr);

	}

	sync();
	return 0;
}

/** Initializes PAMU interrupts
 *
 * @param[in] pamu_reg_vaddr   virtual address of PAMU(x) registers
 * @param[in] pamu_enable_ints bitmap of different error conditions that can be enabled.
 * @param[in] threshold        Threshold value for the number of ECC single-bit error
 *			       that are detected before reporting an error condition.
 */
void pamu_enable_interrupts(void *pamu_reg_vaddr, uint8_t pamu_enable_ints,
			    uint32_t threshold)
{
	uintptr_t pamu_offset;
	uint32_t pics_val = 0;
	uint32_t ecc_val = 0;
	uint32_t *pc;
	unsigned long pamu_reg_off;
	unsigned long pamu_reg_base = (unsigned long) pamu_reg_vaddr;

	for (pamu_reg_off = 0; pamu_reg_off < pamu_reg_space_size; pamu_reg_off += PAMU_OFFSET) {
		pamu_offset = pamu_reg_base + pamu_reg_off;

		/*
		 * set PAMU enable bit,
		 * plus allow ppaact and spaact to be cached
		 * & enable PAMU access violation interrupts.
		 */
		pc = (uint32_t *) (pamu_offset + PAMU_PC);

		if (pamu_enable_ints & (1 << pamu_int_av))
			pics_val |= PAMU_ACCESS_VIOLATION_ENABLE;

		if (pamu_enable_ints & (1 << pamu_int_operation))
			pics_val |= PAMU_OPERATION_ERROR_INT_ENABLE;

		if (pamu_enable_ints & (1 << pamu_int_singlebit)) {
			ecc_val |= PAMU_SB_ECC_ERR;
			out32((uint32_t *)(pamu_offset + PAMU_EECTL),
			     threshold << PAMU_EECTL_THR_SHIFT);
		}

		if (pamu_enable_ints & (1 << pamu_int_multibit))
			ecc_val |= PAMU_MB_ECC_ERR;

		out32((uint32_t *)(pamu_offset + PAMU_PICS), pics_val);
		out32((uint32_t *)(pamu_offset + PAMU_EEINTEN), ecc_val);
		out32((uint32_t *)(pamu_offset + PAMU_EEDIS), ~ecc_val & PAMU_ECC_ERR_MASK);
		out32(pc, PAMU_PC_PE | PAMU_PC_OCE | PAMU_PC_SPCC | PAMU_PC_PPCC);
	}
}

/** Sets up PPAACE entry for specified liodn
 *
 * @param[in] liodn      Logical IO device number
 * @param[in] win_addr   starting address of DSA window
 * @param[in] win-size   size of DSA window
 * @param[in] omi        Operation mapping index -- if ~omi == 0 then omi not defined
 * @param[in] rpn        real (true physical) page number
 * @param[in] stashid    cache stash id for associated cpu -- if ~stashid == 0 then
 *		         stashid not defined
 * @param[in] snoopid    snoop id for hardware coherency -- if ~snoopid == 0 then
 *		         snoopid not defined
 * @param[in] subwin_cnt number of sub-windows
 *
 * @return Returns 0 upon success else error code < 0 returned
 */
int32_t pamu_config_ppaace(uint32_t liodn, phys_addr_t win_addr,
	                   phys_addr_t win_size, uint32_t omi, unsigned long rpn,
			   uint32_t snoopid, uint32_t stashid,
			   uint32_t subwin_cnt)
{
	paace_t *ppaace;
	register_t saved;
	unsigned long fspi;

	if ((win_size & (win_size - 1)) || win_size < PAMU_PAGE_SIZE) {
		printlog(LOGTYPE_PAMU, LOGLEVEL_ERROR,
			"%s: window size too small or not a power of two\n", __func__);
		return ERR_BADTREE;
	}

	if (win_addr & (win_size - 1)) {
		printlog(LOGTYPE_PAMU, LOGLEVEL_ERROR,
			"%s: window address is not aligned with window size\n", __func__);
		return ERR_BADTREE;
	}

	saved = spin_lock_intsave(&pamu_lock);
	ppaace = pamu_get_ppaace(liodn);
	if (!ppaace) {
		spin_unlock_intsave(&pamu_lock, saved);
		return ERR_NOTFOUND;
	}

	if (get_bf(ppaace->addr_bitfields, PPAACE_AF_WSE)) {
		spin_unlock_intsave(&pamu_lock, saved);
		return ERR_BUSY;
	}

	/* window size is 2^(WSE+1) bytes */
	set_bf(ppaace->addr_bitfields, PPAACE_AF_WSE,
           map_addrspace_size_to_wse(win_size));
	spin_unlock_intsave(&pamu_lock, saved);

	pamu_setup_default_xfer_to_host_ppaace(ppaace);

	ppaace->wbah = win_addr >> (PAMU_PAGE_SHIFT + 20);
	set_bf(ppaace->addr_bitfields, PPAACE_AF_WBAL,
	       (win_addr >> PAMU_PAGE_SHIFT));

	/* set up operation mapping if it's configured */
	if (omi < OME_NUMBER_ENTRIES) {
		set_bf(ppaace->impl_attr, PAACE_IA_OTM, PAACE_OTM_INDEXED);
		ppaace->op_encode.index_ot.omi = omi;
	} else if (~omi != 0) {
		printlog(LOGTYPE_PAMU, LOGLEVEL_ERROR,
			"%s: bad operation mapping index: %d\n", __func__, omi);
		return ERR_BADTREE;
	}

	/* configure stash id */
	if (~stashid != 0)
		set_bf(ppaace->impl_attr, PAACE_IA_CID, stashid);

	/* configure snoop id */
	if (~snoopid != 0)
		ppaace->domain_attr.to_host.snpid = snoopid;

	if (subwin_cnt) {
		/* The first entry is in the primary PAACE instead */
		fspi = pamu_get_fspi_and_allocate(subwin_cnt - 1);
		if (fspi == ULONG_MAX) {
			printlog(LOGTYPE_PAMU, LOGLEVEL_ERROR,
				"%s: spaace indexes exhausted\n", __func__);
			return ERR_INVALID;
		}

		/* window count is 2^(WCE+1) bytes */
		set_bf(ppaace->impl_attr, PAACE_IA_WCE,
		       map_subwindow_cnt_to_wce(subwin_cnt));
		set_bf(ppaace->addr_bitfields, PPAACE_AF_MW, 0x1);
		ppaace->fspi = fspi;
	} else {
		set_bf(ppaace->impl_attr, PAACE_IA_ATM, PAACE_ATM_WINDOW_XLATE);
		ppaace->twbah = rpn >> 20;
		set_bf(ppaace->win_bitfields, PAACE_WIN_TWBAL, rpn);
		set_bf(ppaace->addr_bitfields, PAACE_AF_AP, PAACE_AP_PERMS_ALL);
	}
	sync();

	return 0;
}

/** Sets up SPAACE entry for specified subwindow
 *
 * @param[in] liodn       Logical IO device number
 * @param[in] subwin_cnt  number of sub-windows associated with dma-window
 * @param[in] subwin_addr starting address of subwindow
 * @param[in] subwin_size size of subwindow
 * @param[in] omi         Operation mapping index
 * @param[in] rpn         real (true physical) page number
 * @param[in] snoopid     snoop id for hardware coherency -- if ~snoopid == 0 then
 *			  snoopid not defined
 * @param[in] stashid     cache stash id for associated cpu
 *
 * @return Returns 0 upon success else error code < 0 returned
 */
int32_t pamu_config_spaace(uint32_t liodn, uint32_t subwin_cnt,
			   phys_addr_t subwin_addr, phys_addr_t subwin_size,
			   uint32_t omi, unsigned long rpn, uint32_t snoopid,
			   uint32_t stashid)
{
	paace_t *paace;
	unsigned long fspi;

	/* setup sub-windows */
	if (subwin_cnt) {
		paace = pamu_get_ppaace(liodn);
		if (subwin_addr > 0 && paace) {
			fspi = paace->fspi;
			paace = pamu_get_spaace(fspi, subwin_addr - 1);

			pamu_setup_default_xfer_to_host_spaace(paace);
			set_bf(paace->addr_bitfields, SPAACE_AF_LIODN, liodn);
		}

		if (!paace)
			return ERR_NOTFOUND;

		if (subwin_size & (subwin_size - 1) || subwin_size < PAMU_PAGE_SIZE) {
			printlog(LOGTYPE_PAMU, LOGLEVEL_ERROR,
				"%s: subwindow size out of range, or not a power of 2\n", __func__);
			return ERR_BADTREE;
		}

		if (rpn == ULONG_MAX) {
			printlog(LOGTYPE_PAMU, LOGLEVEL_ERROR,
				"%s: real page number out of range\n", __func__);
			return ERR_NOTRANS;
		}

		/* window size is 2^(WSE+1) bytes */
		set_bf(paace->win_bitfields, PAACE_WIN_SWSE,
		       map_addrspace_size_to_wse(subwin_size));

		set_bf(paace->impl_attr, PAACE_IA_ATM, PAACE_ATM_WINDOW_XLATE);
		paace->twbah = rpn >> 20;
		set_bf(paace->win_bitfields, PAACE_WIN_TWBAL, rpn);
		set_bf(paace->addr_bitfields, PAACE_AF_AP, PAACE_AP_PERMS_ALL);

		/* configure snoop id */
		if (~snoopid != 0)
			paace->domain_attr.to_host.snpid = snoopid;

		if (~omi != 0) {
			set_bf(paace->impl_attr, PAACE_IA_OTM, PAACE_OTM_INDEXED);
			paace->op_encode.index_ot.omi = omi;
			if (~stashid != 0)
				set_bf(paace->impl_attr, PAACE_IA_CID, stashid);
		}

		lwsync();
		paace->addr_bitfields |= PAACE_V_VALID;
	}

	sync();
	return 0;
}

/** Reconfigures primary PAMU table for those cases in which there
 *  are no subwindows.
 *
 * @param[in]  liodn        Logical IO device number
 * @param[out] subwin_cnt   number of sub-windows
 * @param[out] window_addr  starting address of DSA window
 * @param[out] window_size  size of DSA window
 * @param[in]  rpn          real (true physical) page number
 *
 * @return Returns 0 upon success else error code < 0 returned
 */
int32_t pamu_reconfig_liodn(uint32_t liodn, unsigned long rpn)
{
	paace_t *ppaace;
	unsigned long curr_rpn;

	ppaace = pamu_get_ppaace(liodn);
	if (!ppaace)
		return ERR_NOTFOUND;

	curr_rpn = ppaace->twbah << 20 | get_bf(ppaace->win_bitfields, PAACE_WIN_TWBAL);
	if (curr_rpn != rpn) {
		set_bf(ppaace->addr_bitfields, PAACE_AF_AP, PAACE_AP_PERMS_DENIED);
		lwsync();
		set_bf(ppaace->impl_attr, PAACE_IA_ATM, PAACE_ATM_WINDOW_XLATE);
		ppaace->twbah = rpn >> 20;
		set_bf(ppaace->win_bitfields, PAACE_WIN_TWBAL, rpn);
		lwsync();
		set_bf(ppaace->addr_bitfields, PAACE_AF_AP, PAACE_AP_PERMS_ALL);
	}

	sync();
	return 0;
}

/** Reconfigures SPAACE entries (subwindows) associated with specified PPAACE.
 *
 * @param[in] liodn  Logical IO device number
 * @param[in] ppaace Pointer to PAACE tables
 * @param[in] subwin sub-window number
 * @param[in] rpn    real (true physical) page number
 *
 * @return Returns 0 upon success else error code < 0 returned
 */
int32_t pamu_reconfig_subwin(uint32_t liodn,  uint32_t subwin, unsigned long rpn)
{
	paace_t *paace;
	unsigned long curr_rpn, fspi;

	paace = pamu_get_ppaace(liodn);
	if (!paace)
		return ERR_NOTFOUND;

	if (subwin != 0) {
		fspi  = paace->fspi;
		paace = pamu_get_spaace(fspi, subwin - 1);
		if (!paace)
			return ERR_NOTFOUND;
	}

	curr_rpn = paace->twbah << 20 | get_bf(paace->win_bitfields, PAACE_WIN_TWBAL);
	if (curr_rpn == rpn)
		return 0;                   /* continue */

	set_bf(paace->addr_bitfields, PAACE_AF_AP, PAACE_AP_PERMS_DENIED);
	lwsync();
	set_bf(paace->impl_attr, PAACE_IA_ATM, PAACE_ATM_WINDOW_XLATE);
	paace->twbah = rpn >> 20;
	set_bf(paace->win_bitfields, PAACE_WIN_TWBAL, rpn);
	lwsync();
	set_bf(paace->addr_bitfields, PAACE_AF_AP, PAACE_AP_PERMS_ALL);

	sync();
	return 0;
}

paace_t *pamu_get_ppaace(uint32_t liodn)
{
	if (!ppaact || liodn >= PAACE_NUMBER_ENTRIES) {
		printlog(LOGTYPE_PAMU, LOGLEVEL_ERROR,
			 "%s: PPAACT doesn't exist\n", __func__);
		return NULL;
	}

	return &ppaact[liodn];
}

ome_t *pamu_get_ome(uint8_t omi)
{
	if (omt)
		return &omt[omi];

	printlog(LOGTYPE_PAMU, LOGLEVEL_ERROR,
		 "%s: OMT doesn't exist\n", __func__);
	return NULL;
}

paace_t *pamu_get_spaace(uint32_t fspi_index, uint32_t wnum)
{
	return &spaact[fspi_index + wnum];
}

unsigned long pamu_get_fspi_and_allocate(uint32_t subwindow_cnt)
{
	unsigned long tmp;

	do {
		tmp = pamu_fspi;
		/*
		 * This check should be MP-safe as atomic compare_and_swap()
		 * will ensure that we re-iterate here if "fspi" gets updated.
		 */
		if ((tmp + subwindow_cnt) > SPAACE_NUMBER_ENTRIES)
			return ULONG_MAX;
	} while (!compare_and_swap(&pamu_fspi, tmp, tmp + subwindow_cnt));

	return tmp;
}

void pamu_setup_default_xfer_to_host_ppaace(paace_t *ppaace)
{
	set_bf(ppaace->addr_bitfields, PAACE_AF_PT, PAACE_PT_PRIMARY);

	set_bf(ppaace->domain_attr.to_host.coherency_required, PAACE_DA_HOST_CR,
	       PAACE_M_COHERENCE_REQ);
}

void pamu_setup_default_xfer_to_host_spaace(paace_t *spaace)
{
	set_bf(spaace->addr_bitfields, PAACE_AF_PT, PAACE_PT_SECONDARY);
	set_bf(spaace->domain_attr.to_host.coherency_required, PAACE_DA_HOST_CR,
	       PAACE_M_COHERENCE_REQ);
}

unsigned int pamu_get_max_subwindow_count(void)
{
	return max_subwindow_count;
}

