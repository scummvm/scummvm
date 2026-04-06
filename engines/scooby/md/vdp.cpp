// VDP code taken from the genemu project
// https://github.com/rasky/genemu

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "common/endian.h"
#include "common/debug.h"
#include "vdp.h"
#include "mem.h"
//#include "cpu.h"
//extern "C" {
//    #include "m68k/m68k.h"
//}

namespace Scooby {

//extern int framecounter;
#define VERSION_PAL false

#define SCREEN_WIDTH 320

#define REG0_HVLATCH          BIT(regs[0], 1)
#define REG0_LINE_INTERRUPT   BIT(regs[0], 4)
#define REG1_PAL              BIT(regs[1], 3)
#define REG1_DMA_ENABLED      BIT(regs[1], 4)
#define REG1_VBLANK_INTERRUPT BIT(regs[1], 5)
#define REG1_DISP_ENABLED     BIT(regs[1], 6)
#define REG2_NAMETABLE_A      (BITS(regs[2], 3, 3) << 13)
#define REG3_NAMETABLE_W      (BITS(regs[3], 1, 5) << 11)
#define REG4_NAMETABLE_B      (BITS(regs[4], 0, 3) << 13)
#define REG5_SAT_ADDRESS      ((regs[5] & (mode_h40 ? 0x7E : 0x7F)) << 9)
#define REG5_SAT_SIZE         (mode_h40 ? (1<<10) : (1<<9))
#define REG10_LINE_COUNTER    BITS(regs[10], 0, 8)
#define REG12_MODE_H40        BIT(regs[12], 0)
#define REG15_DMA_INCREMENT   regs[15]
#define REG19_DMA_LENGTH      (regs[19] | (regs[20] << 8))
#define REG21_DMA_SRCADDR_LOW (regs[21] | (regs[22] << 8))
#define REG23_DMA_SRCADDR_HIGH ((regs[23] & 0x7F) << 16)
#define REG23_DMA_TYPE        BITS(regs[23], 6, 2)

//class VDP VDP;

int VDP::screen_offset() { return (SCREEN_WIDTH - screen_width()) / 2; }
int VDP::screen_width() { return BIT(regs[12], 0) ? 40*8 : 32*8; }

// Return 9-bit accurate hcounter
int VDP::hcounter(void) {
	int mclk = 0; //TODO m68k_cycles_run() * M68K_FREQ_DIVISOR;
	int pixclk;

	// Accurate 9-bit hcounter emulation, from timing posted here:
	// http://gendev.spritesmind.net/forum/viewtopic.php?p=17683#17683
	if (REG12_MODE_H40) {
		pixclk = mclk * 420 / VDP_CYCLES_PER_LINE;
		pixclk += 0xD;
		if (pixclk >= 0x16D)
			pixclk += 0x1C9 - 0x16D;
	} else {
		pixclk = mclk * 342 / VDP_CYCLES_PER_LINE;
		pixclk += 0xB;
		if (pixclk >= 0x128)
			pixclk += 0x1D2 - 0x128;
	}

	return pixclk & 0x1FF;
}

int VDP::vcounter(void) {
	int vc = _vcounter;

	if (VERSION_PAL && mode_pal && vc >= 0x10B)
		vc += 0x1D2 - 0x10B;
	else if (VERSION_PAL && !mode_pal && vc >= 0x103)
		vc += 0x1CA - 0x103;
	else if (!VERSION_PAL && vc >= 0xEB)
		vc += 0x1E5 - 0xEB;
	assert(vc < 0x200);
	return vc;
}

bool VDP::vblank(void) {
	int vc = vcounter();

	if (!REG1_DISP_ENABLED)
		return true;

	if (mode_pal)
		return (vc >= 0xF0 && vc < 0x1FF);
	else
		return (vc >= 0xE0 && vc < 0x1FF);
}

void VDP::register_w(int reg, uint8_t value) {
	// Mode4 is not emulated yet. Anyway, access to registers > 0xA is blocked.
	if (!BIT(regs[0x1], 2) && reg > 0xA) return;

	regs[reg] = value;
	debug("VDP reg:%02d <- %02x\n", reg, value);

	// Writing a register clear the first command word
	// (see sonic3d intro wrong colors, and vdpfifotesting)
	code_reg &= ~0x3;
	address_reg &= ~0x3FFF;

	// Here we handle only cases where the register write
	// has an immediate effect on VDP.
	switch (reg) {
	case 0:
		if (REG0_HVLATCH && !hvcounter_latched) {
			hvcounter_latch = hvcounter_r16();
			hvcounter_latched = true;
		} else if (!REG0_HVLATCH && hvcounter_latched)
			hvcounter_latched = false;
		break;
	}

}


void VDP::push_fifo(uint16_t value) {
	fifo[3] = fifo[2];
	fifo[2] = fifo[1];
	fifo[1] = fifo[0];
	fifo[0] = value;
}

void VDP::VRAM_W(uint16_t address, uint8_t value) {
	VRAM[address] = value;

	// Update internal SAT cache if it was modified
	// This cache is needed for Castlevania Bloodlines (level 6-2)
	if (address >= REG5_SAT_ADDRESS && address_reg < REG5_SAT_ADDRESS + REG5_SAT_SIZE)
		SAT_CACHE[address - REG5_SAT_ADDRESS] = value;
}


void VDP::data_port_w16(uint16_t value) {
	command_word_pending = false;

	push_fifo(value);

	switch (code_reg & 0xF) {
	case 0x1:
//        mem_log("VDP", "Direct VRAM write: addr:%x increment:%d value:%04x A0=%x vc:%x hc:%x\n",
//                address_reg, REG15_DMA_INCREMENT, value, m68k_get_reg(NULL, M68K_REG_A0), vcounter(), hcounter());
		VRAM_W((address_reg) & 0xFFFF, value >> 8);
		VRAM_W((address_reg ^ 1) & 0xFFFF, value & 0xFF);
		address_reg += REG15_DMA_INCREMENT;
		break;
	case 0x3:
//        mem_log("VDP", "Direct CRAM write: addr:%x increment:%d value:%04x vc:%x hc:%x\n",
//                address_reg, REG15_DMA_INCREMENT, value, vcounter(), hcounter());
		CRAM[(address_reg >> 1) & 0x3F] = value;
		address_reg += REG15_DMA_INCREMENT;
		break;
	case 0x5:
//        mem_log("VDP", "Direct VSRAM write: addr:%x increment:%d value:%04x vc:%x hc:%x\n",
//                address_reg, REG15_DMA_INCREMENT, value, vcounter(), hcounter());
		VSRAM[(address_reg >> 1) & 0x3F] = value;
		address_reg += REG15_DMA_INCREMENT;
		break;

	case 0x0:
	case 0x4:
	case 0x8:
		// Write operation after setting up read: ignored (ex: ecco2, aladdin)
		break;

	case 0x9:
		// invalid, ignore (vdpfifotesting)
		break;

	default:
		debug("VDP invalid data port write16: code:%02x\n", code_reg);
		assert(!"data port w not handled");
	}

	// When a DMA fill is pending, the value sent to the
	// data port is the actual fill value and triggers the fill
	if (dma_fill_pending) {
		dma_fill_pending = false;
		dma_fill(value);
		return;
	}
}

uint16_t VDP::data_port_r16(void) {
	enum {
		CRAM_BITMASK = 0x0EEE, VSRAM_BITMASK = 0x07FF, VRAM8_BITMASK = 0x00FF
	};
	uint16_t value;

	command_word_pending = false;
	debug("VDP data port r16: code:%x, addr:%x\n", code_reg, address_reg);

	switch (code_reg & 0xF) {
	case 0x0:
		// No byteswapping here, see vdpfifotesting
		value = VRAM[(address_reg) & 0xFFFE] << 8;
		value |= VRAM[(address_reg | 1) & 0xFFFF];
		address_reg += REG15_DMA_INCREMENT;
		address_reg &= 0xFFFF;
		return value;

	case 0x4:
		if (((address_reg >> 1) & 0x3F) >= 0x28)
			value = VSRAM[0];
		else
			value = VSRAM[(address_reg >> 1) & 0x3F];
		value = (value & VSRAM_BITMASK) | (fifo[3] & ~VSRAM_BITMASK);
		address_reg += REG15_DMA_INCREMENT;
		address_reg &= 0x7F;
		return value;

	case 0x8:
		value = CRAM[(address_reg >> 1) & 0x3F];
		value = (value & CRAM_BITMASK) | (fifo[3] & ~CRAM_BITMASK);
		address_reg += REG15_DMA_INCREMENT;
		address_reg &= 0x7F;
		return value;

	case 0xC: // undocumented 8-bit VRAM access, see vdpfifotesting
		value = VRAM[(address_reg ^ 1) & 0xFFFF];
		value = (value & VRAM8_BITMASK) | (fifo[3] & ~VRAM8_BITMASK);
		address_reg += REG15_DMA_INCREMENT;
		address_reg &= 0xFFFF;
		return value;

	default:
//        fprintf(stdout, "[VDP][PC=%06x](%04d) invalid data port write16: code:%02x\n", m68k_get_reg(NULL, M68K_REG_PC), framecounter, code_reg);
		assert(!"data port r not handled");
		return 0xFF;
	}


}


void VDP::control_port_w(uint16_t value) {
	if (command_word_pending) {
		// second half of the command word
		code_reg &= ~0x3C;
		code_reg |= (value >> 2) & 0x3C;
		address_reg &= 0x3FFF;
		address_reg |= value << 14;
		command_word_pending = false;
		debug("[VDP] command word 2nd: code:%02x addr:%04x\n", code_reg, address_reg);
		if (code_reg & (1 << 5))
			dma_trigger();
		return;
	}

	if ((value >> 14) == 2) {
		register_w((value >> 8) & 0x1F, value & 0xFF);
		return;
	}

	// Anything else is treated as first half of the "command word"
	// We directly update the code reg and address reg
	code_reg &= ~0x3;
	code_reg |= value >> 14;
	address_reg &= ~0x3FFF;
	address_reg |= value & 0x3FFF;
	command_word_pending = true;
	debug("[VDP] command word 1st: code:%02x addr:%04x\n", code_reg, address_reg);
}

uint16_t VDP::status_register_r(void) {
#define STATUS_FIFO_EMPTY      (1<<9)
#define STATUS_FIFO_FULL       (1<<8)
#define STATUS_VIRQPENDING     (1<<7)
#define STATUS_SPRITEOVERFLOW  (1<<6)
#define STATUS_SPRITECOLLISION (1<<5)
#define STATUS_ODDFRAME        (1<<4)
#define STATUS_VBLANK          (1<<3)
#define STATUS_HBLANK          (1<<2)
#define STATUS_DMAPROGRESS     (1<<1)
#define STATUS_PAL             (1<<0)

	uint16_t status = status_reg;
	int hc = hcounter();
	int vc = vcounter();

	// TODO: FIFO not emulated
	status |= STATUS_FIFO_EMPTY;

	// VBLANK bit
	if (vblank())
		status |= STATUS_VBLANK;

	// HBLANK bit (see Nemesis doc, as linked in hcounter())
	if (REG12_MODE_H40) {
		if (hc < 0xA || hc >= 0x166)
			status |= STATUS_HBLANK;
	} else {
		if (hc < 0x9 || hc >= 0x126)
			status |= STATUS_HBLANK;
	}

	if (sprite_overflow)
		status |= STATUS_SPRITEOVERFLOW;
	if (sprite_collision)
		status |= STATUS_SPRITECOLLISION;

	if (mode_pal)
		status |= STATUS_PAL;

	// reading the status clears the pending flag for command words
	command_word_pending = false;

	return status;
}

uint16_t VDP::hvcounter_r16(void) {
	if (hvcounter_latched)
		return hvcounter_latch;

	int hc = hcounter();
	int vc = vcounter();
	assert(vc < 512);
	assert(hc < 512);

	return ((vc & 0xFF) << 8) | (hc >> 1);
}

void VDP::scanline(uint8_t *screen) {
	mode_h40 = REG12_MODE_H40;
	mode_pal = REG1_PAL;

	debug(3,"VDP render scanline %d\n", _vcounter);
	render_scanline(screen, _vcounter);

	// On these lines, the line counter interrupt is reloaded
	if (_vcounter == 0 || _vcounter >= (mode_pal ? 0xF1 : 0xE1)) {
		if (REG0_LINE_INTERRUPT)
			debug(3,"VDP HINTERRUPT counter reloaded: (vcounter: %d, new counter: %d)\n", _vcounter, REG10_LINE_COUNTER);
		line_counter_interrupt = REG10_LINE_COUNTER;
	}

	if (--line_counter_interrupt < 0) {
		if (REG0_LINE_INTERRUPT && (_vcounter <= (mode_pal ? 0xF0 : 0xE0))) {
			debug(3,"VDP HINTERRUPT (_vcounter: %d, new counter: %d)\n", _vcounter, REG10_LINE_COUNTER);
			hint_pending = true;
			if (!(status_reg & STATUS_VIRQPENDING))
				;
//                CPU_M68K.irq(4);
			//TODO fire off hint here.
		}

		line_counter_interrupt = REG10_LINE_COUNTER;
	}

	_vcounter++;
	if (_vcounter == (VERSION_PAL ? 313 : 262)) {
		_vcounter = 0;
		sprite_overflow = 0;
		sprite_collision = false;
	}

	if (_vcounter == (mode_pal ? 0xF0 : 0xE0))   // vblank begin
	{
		if (REG1_VBLANK_INTERRUPT) {
			status_reg |= STATUS_VIRQPENDING;
//            CPU_M68K.irq(6);
			//TODO vint here
		}
//        CPU_Z80.set_irq_line(true);
	}
	if (_vcounter == (mode_pal ? 0xF1 : 0xE1)) {
		// The Z80 IRQ line stays asserted for one line
//        CPU_Z80.set_irq_line(false);
	}
}

unsigned int VDP::num_scanlines(void) {
	return VERSION_PAL ? 313 : 262;
}


/**************************************************************
 * DMA
 **************************************************************/

void VDP::dma_trigger() {
	// Check master DMA enable, otherwise skip
	if (!REG1_DMA_ENABLED)
		return;

	switch (REG23_DMA_TYPE) {
	case 0:
	case 1:
		dma_m68k();
		break;

	case 2:
		// VRAM fill will trigger on next data port write
		dma_fill_pending = true;
		break;

	case 3:
		dma_copy();
		break;
	}
}

void VDP::dma_fill(uint16_t value) {
	/* FIXME: should be done in parallel and non blocking */
	int length = REG19_DMA_LENGTH;

	// This address is not required for fills,
	// but it's still updated by the DMA engine.
	uint16_t src_addr_low = REG21_DMA_SRCADDR_LOW;

	if (length == 0)
		length = 0xFFFF;

	debug("VDP (V=%x,H=%x) DMA %s fill: dst:%04x, length:%d, increment:%d, value=%02x\n",
		  vcounter(), hcounter(),
		  (code_reg & 0xF) == 1 ? "VRAM" : ((code_reg & 0xF) == 3 ? "CRAM" : "VSRAM"),
		  address_reg, length, REG15_DMA_INCREMENT, value >> 8);

	switch (code_reg & 0xF) {
	case 0x1:
		do {
			VRAM_W((address_reg ^ 1) & 0xFFFF, value >> 8);
			address_reg += REG15_DMA_INCREMENT;
			src_addr_low++;
		} while (--length);
		break;
	case 0x3:  // undocumented and buggy, see vdpfifotesting
		do {
			CRAM[(address_reg >> 1) & 0x3F] = fifo[3];
			address_reg += REG15_DMA_INCREMENT;
			src_addr_low++;
		} while (--length);
		break;
	case 0x5:  // undocumented and buggy, see vdpfifotesting:
		do {
			VSRAM[(address_reg >> 1) & 0x3F] = fifo[3];
			address_reg += REG15_DMA_INCREMENT;
			src_addr_low++;
		} while (--length);
		break;
	default:
		debug("VDP invalid code_reg:%x during DMA fill\n", code_reg);
	}

	// Clear DMA length at the end of transfer
	regs[19] = regs[20] = 0;

	// Update DMA source address after end of transfer
	regs[21] = src_addr_low & 0xFF;
	regs[22] = src_addr_low >> 8;
}

void VDP::dma_m68k() {
	int length = REG19_DMA_LENGTH;
	uint16_t src_addr_low = REG21_DMA_SRCADDR_LOW;
	uint32_t src_addr_high = REG23_DMA_SRCADDR_HIGH;

	// Special case for length = 0 (ex: sonic3d)
	if (length == 0)
		length = 0xFFFF;

//    mem_log("VDP", "(V=%x,H=%x) DMA M68k->%s copy: src:%04x, dst:%04x, length:%d, increment:%d\n",
//        vcounter(), hcounter(),
//        (code_reg&0xF)==1 ? "VRAM" : ( (code_reg&0xF)==3 ? "CRAM" : "VSRAM"),
//        (src_addr_high | src_addr_low) << 1, address_reg, length, REG15_DMA_INCREMENT);

	do {
		unsigned int value = 0; //TODO m68k_read_memory_16((src_addr_high | src_addr_low) << 1);
		push_fifo(value);

		switch (code_reg & 0xF) {
		case 0x1:
			VRAM_W((address_reg) & 0xFFFF, value >> 8);
			VRAM_W((address_reg ^ 1) & 0xFFFF, value & 0xFF);
			break;
		case 0x3:
			CRAM[(address_reg >> 1) & 0x3F] = value;
			break;
		case 0x5:
			VSRAM[(address_reg >> 1) & 0x3F] = value;
			break;
		default:
			debug("VDP invalid code_reg:%x during DMA fill\n", code_reg);
			break;
		}

		address_reg += REG15_DMA_INCREMENT;
		src_addr_low += 1;

	} while (--length);

	// Update DMA source address after end of transfer
	regs[21] = src_addr_low & 0xFF;
	regs[22] = src_addr_low >> 8;

	// Clear DMA length at the end of transfer
	regs[19] = regs[20] = 0;
}

void VDP::dma_copy() {
	int length = REG19_DMA_LENGTH;
	uint16_t src_addr_low = REG21_DMA_SRCADDR_LOW;

	assert(length != 0);
	debug("VDP DMA copy: src:%04x dst:%04x len:%x\n", src_addr_low, address_reg & 0xFFFF, length);

	do {
		uint16_t value = VRAM[src_addr_low ^ 1];
		VRAM_W((address_reg ^ 1) & 0xFFFF, value);

		address_reg += REG15_DMA_INCREMENT;
		src_addr_low++;
	} while (--length);

	// Update DMA source address after end of transfer
	regs[21] = src_addr_low & 0xFF;
	regs[22] = src_addr_low >> 8;

	// Clear DMA length at the end of transfer
	regs[19] = regs[20] = 0;

}

int VDP::get_nametable_A() { return REG2_NAMETABLE_A; }

int VDP::get_nametable_W() { return REG3_NAMETABLE_W; }

int VDP::get_nametable_B() { return REG4_NAMETABLE_B; }

int m68k_int_ack(int irq) {
	return 0; //TODO VDP.irq_acked(irq);
}

void VDP::reset() {
	command_word_pending = false;
	address_reg = 0;
	code_reg = 0;
	_vcounter = 0;
	status_reg = 0x3C00;
	line_counter_interrupt = 0;
	hvcounter_latched = false;

	memset(VRAM, 0, sizeof(VRAM));
	memset(CRAM, 0, sizeof(CRAM));
	memset(VSRAM, 0, sizeof(VSRAM));
	memset(SAT_CACHE, 0, sizeof(SAT_CACHE));

//    TODO m68k_set_int_ack_callback(m68k_int_ack);
}

//void vdp_mem_w8(unsigned int address, unsigned int value)
//{
//    switch (address & 0x1F) {
//        case 0x11:
//        case 0x13:
//        case 0x15:
//        case 0x17:
//            mem_log("SN76489", "write: %02x\n", value);
//            return;
//
//        default:
//            vdp_mem_w16(address & ~1, (value << 8) | value);
//            return;
//            //fprintf(stdout, "[VDP][PC=%06x](%04d) unhandled write8 IO:%02x val:%04x\n", m68k_get_reg(NULL, M68K_REG_PC), framecounter, address&0x1F, value);
//    }
//}

//void vdp_mem_w16(unsigned int address, unsigned int value)
//{
//    switch (address & 0x1F) {
//        case 0x0:
//        case 0x2: data_port_w16(value); return;
//
//        case 0x4:
//        case 0x6: control_port_w(value); return;
//
//        // unused register, see vdpfifotesting
//        case 0x18:
//            return;
//        // debug register
//        case 0x1C:
//            return;
//
//        default:
//            fprintf(stdout, "[VDP][PC=%06x](%04d) unhandled write16 IO:%02x val:%04x\n", m68k_get_reg(NULL, M68K_REG_PC), framecounter, address&0x1F, value);
//            assert(!"unhandled vdp_mem_w16");
//    }
//
//}
//
//unsigned int vdp_mem_r8(unsigned int address)
//{
//    unsigned int ret = vdp_mem_r16(address & ~1);
//    if (address & 1)
//        return ret & 0xFF;
//    return ret >> 8;
//}
//
//unsigned int vdp_mem_r16(unsigned int address)
//{
//    unsigned int ret;
//
//    switch (address & 0x1F) {
//        case 0x0:
//        case 0x2: return data_port_r16();
//
//        case 0x4:
//        case 0x6: return status_register_r();
//
//        case 0x8:
//        case 0xA:
//        case 0xC:
//        case 0xE: return hvcounter_r16();
//
//        // unused register, see vdpfifotesting
//        case 0x18: return 0xFFFF;
//
//        // debug register
//        case 0x1C: return 0xFFFF;
//
//        default:
//            fprintf(stdout, "[VDP][PC=%06x](%04d) unhandled read16 IO:%02x\n", m68k_get_reg(NULL, M68K_REG_PC), framecounter, address&0x1F);
//            assert(!"unhandled vdp_mem_r16");
//            return 0xFF;
//    }
//}

int VDP::irq_acked(int irq) {
	if (irq == 6) {
		status_reg &= ~STATUS_VIRQPENDING;
		return hint_pending ? 4 : 0;
	} else if (irq == 4) {
		hint_pending = false;
		return 0;
	}
	assert(0);
}

//GFX methods
#define DRAW_ALWAYS             0    // draw all the pixels
#define DRAW_NOT_ON_SPRITE      1    // draw only if the buffer doesn't contain a pixel from a sprite
#define DRAW_MAX_PRIORITY       2    // draw only if priority >= pixel in the buffer

#define COLOR_3B_TO_8B(c)  (((c) << 5) | ((c) << 2) | ((c) >> 1))
#define CRAM_R(c)          COLOR_3B_TO_8B(BITS((c), 1, 3))
#define CRAM_G(c)          COLOR_3B_TO_8B(BITS((c), 5, 3))
#define CRAM_B(c)          COLOR_3B_TO_8B(BITS((c), 9, 3))

#define MODE_SHI           BITS(regs[12], 3, 1)

#define SHADOW_COLOR(r,g,b) \
    do { r >>= 1; g >>= 1; b >>= 1; } while (0)
#define HIGHLIGHT_COLOR(r,g,b) \
    do { SHADOW_COLOR(r,g,b); r |= 0x80; g |= 0x80; b |= 0x80; } while(0)

// While we draw the planes, we use bit 0x80 on each pixel to save the
// high-priority flag, so that we can later prioritize.
#define PIXATTR_HIPRI        0x80

// After mixing code, we use free bits 0x80 and 0x40 to indicate the
// shadow/highlight effect to apply on each pixel. Notice that we use
// 0x80 to indicate normal drawing and 0x00 to indicate shadowing,
// which does match exactly the semantic of PIXATTR_HIPRI. This simplifies
// mixing code quite a bit.
#define SHI_NORMAL(x)        ((x) | 0x80)
#define SHI_HIGHLIGHT(x)     ((x) | 0x40)
#define SHI_SHADOW(x)        ((x) & 0x3F)

#define SHI_IS_SHADOW(x)     (!((x) & 0x80))
#define SHI_IS_HIGHLIGHT(x)  ((x) & 0x40)

template <bool fliph>
bool VDP::draw_pattern(uint8_t *screen, uint8_t *pattern, uint8_t attrs)
{
	bool overdraw = false;

	if (fliph)
		pattern += 3;

	for (int x = 0; x < 4; ++x)
	{
		uint8_t pix = *pattern;
		uint8_t pix1 = !fliph ? pix>>4 : pix&0xF;
		uint8_t pix2 = !fliph ? pix&0xF : pix>>4;

		// Never overwrite already-written bytes. This is only
		// useful for sprites; within each plane, we never overdraw anyway.
		if ((screen[0] & 0xF) == 0)
			screen[0] = attrs | pix1;
		else
			overdraw |= pix1 & 0xF;
		if ((screen[1] & 0xF) == 0)
			screen[1] = attrs | pix2;
		else
			overdraw |= pix2 & 0xF;

		if (fliph)
			pattern--;
		else
			pattern++;
		screen += 2;
	}

	return overdraw;
}

template <bool check_overdraw>
bool VDP::draw_pattern(uint8_t *screen, uint16_t name, int paty)
{
	int pat_idx = BITS(name, 0, 11);
	int pat_fliph = BITS(name, 11, 1);
	int pat_flipv = BITS(name, 12, 1);
	int pat_palette = BITS(name, 13, 2);
	int pat_pri = BITS(name, 15, 1);
	uint8_t *pattern = VRAM + pat_idx * 32;
	uint8_t attrs = (pat_palette << 4) | (pat_pri ? PIXATTR_HIPRI : 0);
	bool overdraw = false;

	if (!pat_flipv)
		pattern += paty*4;
	else
		pattern += (7-paty)*4;

	if (!pat_fliph)
		overdraw |= draw_pattern<false>(screen, pattern, attrs);
	else
		overdraw |= draw_pattern<true>(screen, pattern, attrs);

	if (!check_overdraw) overdraw=false;  // this is enough to let the optimizer take everything out
	return overdraw;
}

void VDP::draw_plane_w(uint8_t *screen, int y)
{
	int addr_w = get_nametable_W();
	int row = y >> 3;
	int paty = y & 7;
	uint16_t ntwidth = (screen_width() == 320 ? 64 : 32);
	uint8_t *nt = VRAM + addr_w + row*2*ntwidth;

	for (int i = 0; i < screen_width() / 8; ++i)
	{
		draw_pattern<false>(screen, FETCH16(nt), paty);
		nt += 2;
		screen += 8;
	}
}

void VDP::draw_plane_ab(uint8_t *screen, int line, int ntaddr, uint16_t scrollx, uint16_t *vsram)
{
	uint8_t *end = screen + screen_width();
	uint16_t ntwidth = BITS(regs[16], 0, 2);
	uint16_t ntheight = BITS(regs[16], 4, 2);
	uint16_t ntw_mask, nth_mask;
	int numcell;
	bool column_scrolling = BIT(regs[11], 2);

	if (column_scrolling && line==0)
		debug("SCROLL column scrolling\n");

	assert(ntwidth != 2);  // invalid setting
	assert(ntheight != 2); // invalid setting

	ntwidth  = (ntwidth + 1) * 32;
	ntheight = (ntheight + 1) * 32;
	ntw_mask = ntwidth - 1;
	nth_mask = ntheight - 1;

	// Invert horizontal scrolling (because it goes right, but we need to offset of the first screen pixel)
	scrollx = -scrollx;
	uint8_t col = (scrollx >> 3) & ntw_mask;
	uint8_t patx = scrollx & 7;

	numcell = 0;
	screen -= patx;
	while (screen < end)
	{
		// Calculate vertical scrolling for the current line
		uint16_t scrolly = (*vsram & 0x3FF) + line;
		uint8_t row = (scrolly >> 3) & nth_mask;
		uint8_t paty = scrolly & 7;
		uint8_t *nt = VRAM + ntaddr + row*(2*ntwidth);

		draw_pattern<false>(screen, FETCH16(nt + col*2), paty);

		col = (col + 1) & ntw_mask;
		screen += 8;
		numcell++;

		// If per-column scrolling is active, increment VSRAM pointer
		if (column_scrolling && (numcell & 1) == 0)
			vsram += 2;
	}
}

void VDP::draw_sprites(uint8_t *screen, int line)
{
	// Plane/sprite disable, show only backdrop
	if (!BIT(regs[1], 6)) // TODO || keystate[SDL_SCANCODE_S])
		return;

	uint8_t mask = mode_h40 ? 0x7E : 0x7F;
	uint8_t *start_table = VRAM + ((regs[5] & mask) << 9);

	// This is both the size of the table as seen by the VDP
	// *and* the maximum number of sprites that are processed
	// (important in case of infinite loops in links).
	const int SPRITE_TABLE_SIZE     = (screen_width() == 320) ?  80 :  64;
	const int MAX_SPRITES_PER_LINE  = (screen_width() == 320) ?  20 :  16;
	const int MAX_PIXELS_PER_LINE   = (screen_width() == 320) ? 320 : 256;

#if 0
	if (line == 220)
    {
        int sidx = 0;
        for (int i = 0; i < 64; ++i)
        {
            uint8_t *table = start_table + sidx*8;
            int sy = ((table[0] & 0x3) << 8) | table[1];
            int sh = BITS(table[2], 0, 2) + 1;
            uint16_t name = (table[4] << 8) | table[5];
            int flipv = BITS(name, 12, 1);
            int fliph = BITS(name, 11, 1);
            int sw = BITS(table[2], 2, 2) + 1;
            int sx = ((table[6] & 0x3) << 8) | table[7];
            int link = BITS(table[3], 0, 7);
            int pat_idx = BITS(name, 0, 11);

            mem_log("SPRITE", "%d (sx:%d, sy:%d sz:%d,%d, name:%04x, link:%02x, VRAM:%04x)\n",
                    sidx, sx, sy, sw*8, sh*8, name, link, pat_idx*32);

            if (link == 0) break;
            sidx = link;
        }
    }
#endif

	bool masking = false, one_sprite_nonzero = false, overdraw = false;
	int sidx = 0, num_sprites = 0, num_pixels = 0;
	for (int i = 0; i < SPRITE_TABLE_SIZE && sidx < SPRITE_TABLE_SIZE; ++i)
	{
		uint8_t *table = start_table + sidx*8;
		uint8_t *cache = SAT_CACHE + sidx*8;
		int sy = ((cache[0] & 0x3) << 8) | cache[1];
		int sh = BITS(cache[2], 0, 2) + 1;
		int link = BITS(cache[3], 0, 7);
		uint16_t name = (table[4] << 8) | table[5];
		int flipv = BITS(name, 12, 1);
		int fliph = BITS(name, 11, 1);
		int sw = BITS(table[2], 2, 2) + 1;
		int sx = ((table[6] & 0x3) << 8) | table[7];

		sy -= 128;
		if (line >= sy && line < sy+sh*8)
		{
			// Sprite masking: a sprite on column 0 masks
			// any lower-priority sprite, but with the following conditions
			//   * it only works from the second visible sprite on each line
			//   * if the previous line had a sprite pixel overflow, it
			//     works even on the first sprite
			// Notice that we need to continue parsing the table after masking
			// to see if we reach a pixel overflow (because it would affect masking
			// on next line).
			if (sx == 0)
			{
				if (one_sprite_nonzero || sprite_overflow == line-1)
					masking = true;
			}
			else
				one_sprite_nonzero = true;

			int row = (line - sy) >> 3;
			int paty = (line - sy) & 7;
			if (flipv)
				row = sh - row - 1;

			sx -= 128;
			if (sx > -sw*8 && sx < screen_width() && !masking)
			{
				name += row;
				if (fliph)
					name += sh*(sw-1);
				for (int p=0;p<sw && num_pixels < MAX_PIXELS_PER_LINE;p++)
				{
					overdraw |= draw_pattern<true>(screen + sx + p*8, name, paty);
					if (!fliph)
						name += sh;
					else
						name -= sh;
					num_pixels += 8;
				}
			}
			else
				num_pixels += sw*8;

			if (num_pixels >= MAX_PIXELS_PER_LINE)
			{
				sprite_overflow = line;
				break;
			}
			if (++num_sprites >= MAX_SPRITES_PER_LINE)
				break;
		}

		if (link == 0) break;
		sidx = link;
	}

	if (overdraw)
		sprite_collision = true;
}

uint8_t *VDP::get_hscroll_vram(int line)
{
	int table_offset = regs[13] & 0x3F;
	int mode = regs[11] & 3;
	uint8_t *table = VRAM + (table_offset << 10);
	int idx;

	switch (mode)
	{
	case 0: // Full screen scrolling
		idx = 0;
		break;
	case 1: // First 8 lines
		idx = (line & 7);
		break;
	case 2: // Every row
		idx = (line & ~7);
		break;
	case 3: // Every line
		idx = line;
		break;
	}

	return table + idx*4;
}

void VDP::draw_plane_a(uint8_t *screen, int line)
{
	//TODO if (keystate[SDL_SCANCODE_A]) return;
	uint16_t hsa = FETCH16(get_hscroll_vram(line) + 0) & 0x3FF;
	draw_plane_ab(screen, line, get_nametable_A(), hsa, VSRAM);
}

void VDP::draw_plane_b(uint8_t *screen, int line)
{
//  TODO  if (keystate[SDL_SCANCODE_B]) return;
	uint16_t hsb = FETCH16(get_hscroll_vram(line) + 2) & 0x3FF;
	draw_plane_ab(screen, line, get_nametable_B(), hsb, VSRAM+1);
}

uint8_t VDP::mix(uint8_t back, uint8_t b, uint8_t a, uint8_t s)
{
	uint8_t tile = back;

	if (b & 0xF)
		tile = b;
	if ((a & 0xF) && (tile & 0x80) <= (a & 0x80))
		tile = a;
	if ((s & 0xF) && (tile & 0x80) <= (s & 0x80))
	{
		if (MODE_SHI)
			switch (s & 0x3F) {
			case 0x0E:
			case 0x1E:
			case 0x2E: return SHI_NORMAL(s);         // draw sprite, normal
			case 0x3E: return SHI_HIGHLIGHT(tile);   // draw tile, force highlight
			case 0x3F: return SHI_SHADOW(tile);      // draw tile, force shadow
			}
		return s;
	}

	if (MODE_SHI)
		tile |= (b|a) & 0x80;
	return tile;
}

inline bool VDP::in_window(int x, int y)
{
//  TODO  if (keystate[SDL_SCANCODE_W]) return false;

	int winv = (regs[18] & 0x1F) * 8;
	bool winvdown = BIT(regs[18], 7);

	if (winvdown && y >= winv) return true;
	if (!winvdown && y < winv) return true;

	int winh = (regs[17] & 0x1F) * 16;
	bool winhright = BIT(regs[17], 7);

	if (winhright && x >= winh) return true;
	if (!winhright && x < winh) return true;

	return false;
}

void VDP::render_scanline(uint8_t *screen, int line)
{
	// Overflow is the maximum size we can draw outside to avoid
	// wasting time and code in clipping. The maximum object is a 4x4 sprite,
	// so 32 pixels (on both side) is enough.
	enum { PIX_OVERFLOW = 32 };
	uint8_t buffer[4][SCREEN_WIDTH + PIX_OVERFLOW*2];

	if (BITS(regs[12], 1, 2) != 0)
		assert(!"interlace mode");

	if (line >= (mode_pal ? 240 : 224))
		return;

#if 0
	if (line == 0) {
        int winh = regs[17] & 0x1F;
        int winhright = regs[17] >> 7;
        int winv = regs[18] & 0x1F;
        int winvdown = regs[18] >> 7;
        int addr_a = get_nametable_A();
        int addr_b = get_nametable_B();
        int addr_w = get_nametable_W();
        mem_log("GFX", "A(addr:%04x) B(addr:%04x) W(addr:%04x) SPR(addr:%04x)\n", addr_a, addr_b, addr_w, ((regs[5] & 0x7F) << 9));
        mem_log("GFX", "W(h:%d, right:%d, v:%d, down:%d\n)", winh, winhright, winv, winvdown);
        mem_log("GFX", "SCROLL: %04x %04x\n", VSRAM[0], VSRAM[1]);

        FILE *f;
        f=fopen("vram.dmp", "wb");
        fwrite(VRAM, 1, 65536, f);
        fclose(f);
        f=fopen("cram.dmp", "wb");
        fwrite(CRAM, 1, 64*2, f);
        fclose(f);
        f=fopen("vsram.dmp", "wb");
        fwrite(VSRAM, 1, 64*2, f);
        fclose(f);
    }
#endif


	// Display enable
	memset(screen, 0, SCREEN_WIDTH*4);
	if (BIT(regs[0], 0))
		return;

	// Gfx enable
	bool enable_planes = BIT(regs[1], 6);

	memset(buffer, 0, sizeof(buffer));

	uint8_t back = BITS(regs[7], 0, 6);
	uint8_t *pb = &buffer[0][PIX_OVERFLOW];
	uint8_t *pa = &buffer[1][PIX_OVERFLOW];
	uint8_t *pw = &buffer[2][PIX_OVERFLOW];
	uint8_t *ps = &buffer[3][PIX_OVERFLOW];

	draw_plane_b(pb+screen_offset(), line);
	draw_plane_a(pa+screen_offset(), line);
	draw_plane_w(pw+screen_offset(), line);
	draw_sprites(ps+screen_offset(), line);

	for (int i=0; i<SCREEN_WIDTH; ++i)
	{
		int x = i - screen_offset();
		uint8_t pix = back;

		if (x >= 0 && x < screen_width())
		{
			if (enable_planes)
			{
				uint8_t *aw = in_window(x, line) ? pw : pa;
				pix = mix(back, pb[i], aw[i], ps[i]);
			}
		}


		uint8_t index = pix & 0x3F;
		uint16_t rgb = CRAM[index];

		uint8_t r = CRAM_R(rgb);
		uint8_t g = CRAM_G(rgb);
		uint8_t b = CRAM_B(rgb);

		if (MODE_SHI) //TODO  && !keystate[SDL_SCANCODE_H])
		{
			if (SHI_IS_HIGHLIGHT(pix))
				HIGHLIGHT_COLOR(r,g,b);
			else if (SHI_IS_SHADOW(pix))
				SHADOW_COLOR(r,g,b);
		}

		*screen++ = r;
		*screen++ = g;
		*screen++ = b;
		*screen++ = 0;
	}
}

VDP::VDP() {
	reset();
}

void VDP::writeVRAM(uint16 destAddress, byte *srcBuf, uint16 length) {
	assert(destAddress + length <= sizeof(VRAM));
	memcpy(VRAM + destAddress, srcBuf, length);
}

void VDP::writeCRAM(uint16 destAddress, byte *srcBuf, uint16 length) {
	assert(destAddress + length <= sizeof(CRAM));
	memcpy(CRAM + destAddress, srcBuf, length);
}

void VDP::writeVSRAM(uint16 destAddress, byte *srcBuf, uint16 length) {
	assert(destAddress + length <= sizeof(VSRAM));
	memcpy(VSRAM + destAddress, srcBuf, length);
}

void VDP::writePalette(uint16 paletteIndex, uint16 colour) {
	assert(paletteIndex < 64);
	CRAM[paletteIndex] = colour;
}

void VDP::zeroVRAM(uint16 destAddress, uint16 lengthInWords) {
	assert(destAddress + lengthInWords * 2 <= sizeof(VRAM));
	memset(VRAM + destAddress, 0, lengthInWords * 2);
}

uint16 VDP::readPaletteRecord(uint16 paletteIndex) {
	assert(paletteIndex < 64);
	return CRAM[paletteIndex];
}

int VDP::getNameTableAddress(VDP::PlaneType planeType) {
	switch(planeType) {
	case PlaneA : return get_nametable_A();
	case PlaneB : return get_nametable_B();
	case Window : return get_nametable_W();
	default : break;
	}
	return 0;
}

int VDP::getPlaneWidth(VDP::PlaneType planeType) {
	if (planeType == PlaneA || planeType == PlaneB) {
		return 	(BITS(regs[16], 0, 2) + 1) * 32;
	}
	return (screen_width() == 320 ? 128 : 64);
}

void VDP::writeVRAMWord(uint16 destAddress, uint16 word) {
	assert(destAddress + 2 <= sizeof(VRAM));
	WRITE_BE_INT16(VRAM + destAddress, word);
}

} //End of namespace Scooby
