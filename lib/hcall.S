/*
 *
 *
 *
 */

#define ARG9 0x8
#define ARG10 0xC

.text

.globl hcall_ret0
hcall_ret0:

	lwz	%r11,ARG9(%r1)  /* the token is on the caller's stack */

	sc	1

	/* r3 has status */

	blr

.globl hcall_ret1
hcall_ret1:

	lwz	%r11,ARG9(%r1)  /* the token is on the caller's stack */

	sc	1

	/* r3 has status, r4-r10 any return values */

	lwz	%r12,ARG10(%r1)  /* r12 = return arg array */
	stw	%r4,(0*4)(%r12)

	blr

.globl hcall_ret2
hcall_ret2:

	lwz	%r11,ARG9(%r1)  /* the token is on the caller's stack */

	sc	1

	/* r3 has status, r4-r10 any return values */

	lwz	%r12,ARG10(%r1)  /* r12 = return arg array */
	stw	%r4,(0*4)(%r12)
	stw	%r5,(1*4)(%r12)

	blr

.globl hcall_ret3
hcall_ret3:

	lwz	%r11,ARG9(%r1)  /* the token is on the caller's stack */

	sc	1

	/* r3 has status, r4-r10 any return values */

	lwz	%r12,ARG10(%r1)  /* r12 = return arg array */
	stw	%r4,(0*4)(%r12)
	stw	%r5,(1*4)(%r12)
	stw	%r6,(2*4)(%r12)

	blr

.globl hcall_ret5
hcall_ret5:

	lwz	%r11,ARG9(%r1)  /* the token is on the caller's stack */

	sc	1

	/* r3 has status, r4-r10 any return values */

	lwz	%r12,ARG10(%r1)  /* r12 = return arg array */
	stw	%r4,(0*4)(%r12)
	stw	%r5,(1*4)(%r12)
	stw	%r6,(2*4)(%r12)
	stw	%r7,(3*4)(%r12)
	stw	%r8,(4*4)(%r12)

	blr
