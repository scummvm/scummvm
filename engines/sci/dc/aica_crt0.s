#
# Copyright 2000, 2001, 2002
#         Dan Potter. All rights reserved.
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of Cryptic Allusion nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#

.text
.globl	arm_main
.globl	jps

# Meaningless but makes the linker shut up
.globl	reset
reset:

# Exception vectors
	b	start
	b	undef
	b	softint
	b	pref_abort
	b	data_abort
	b	rsrvd
	b	irq


# FIQ code adapted from the Marcus AICA example
fiq:
	# Save regs
	#stmdb	sp!, {r0-r14}

	# Grab interrupt type (store as parameter)
	ldr	r8,intreq
	ldr	r9,[r8]
	and	r9,r9,#7

	# Timer interupt?
	cmp	r9,#2
	beq	fiq_timer

	# Bus request?
	cmp	r9,#5
	beq	fiq_busreq

	# Dunno -- ack and skip
	b	fiq_done

fiq_busreq:
	# Type 5 is bus request. Wait until the INTBusRequest register
	# goes back from 0x100.
	ldr	r8,busreq_control
fiq_busreq_loop:
	# This could probably be done more efficiently, but I'm
	# no ARM assembly expert...
	ldr	r9,[r8]
	and	r9,r9,#0x100
	cmp	r9,#0
	bne	fiq_busreq_loop

	b	fiq_done

fiq_timer:
	# Type 2 is timer interrupt. Increment timer variable.
	# Update the next line to AICA_MEM_CLOCK if you change AICA_CMD_IFACE
	mov	r8,#0x21000
	ldr	r9,[r8]
	add	r9,r9,#1
	str	r9,[r8]
	
	# Request a new timer interrupt. We'll calculate the number
	# put in here based on the "jps" (jiffies per second). 
	ldr	r8, timer_control
	mov	r9,#256-(44100/4410)
#	ldr	r9,jps
	str	r9,[r8,#0x10]
	mov	r9,#0x40
	str	r9,[r8,#0x24]
#	b	fiq_done
	
	# Return from interrupt
fiq_done:

	# Clear interrupt
	ldr	r8,intclr
	mov	r9,#1
	str	r9,[r8]
	str	r9,[r8]
	str	r9,[r8]
	str	r9,[r8]

	# Restore regs and return
	#ldmdb	sp!, {r0-r14}
	subs	pc,r14,#4

intreq:
	.long	0x00802d00
intclr:
	.long	0x00802d04
timer_control:
	.long	0x00802880
busreq_control:
	.long	0x00802808
jps:
	# 1000 jiffies per second
	.long	256-(44100/1000)


start:
	# Setup a basic stack, disable IRQ, enable FIQ
	mov	sp,#0xb000
	mrs	r10,CPSR
	orr	r10,r10,#0x80
	bic	r10,r10,#0x40
	msr	CPSR_all,r10

	# Call the main for the SPU
	bl	arm_main

	# Loop infinitely if we get here
here:	b	here


# Handlers we don't bother to catch
undef:
softint:
	mov	pc,r14
pref_abort:
data_abort:
irq:
rsrvd:
	sub	pc,r14,#4
