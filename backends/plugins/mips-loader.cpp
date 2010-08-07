/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#if defined(DYNAMIC_MODULES) && defined(MIPS_TARGET)

#include "mips-loader.h"

#define __DEBUG_PLUGINS__

#ifdef __DEBUG_PLUGINS__
#define DBG(x,...) printf(x, ## __VA_ARGS__)
#else
#define DBG(x,...)
#endif

#define seterror(x,...) printf(x, ## __VA_ARGS__)

/**
 * Follow the instruction of a relocation section.
 *
 * @param DLFile 	 SeekableReadStream of File
 * @param offset 	 Offset into the File
 * @param size   	 Size of relocation section
 * @param relSegment Base address of relocated segment in memory (memory offset)
 *
 */
bool MIPSDLObject::relocate(Common::SeekableReadStream* DLFile, unsigned long offset, unsigned long size, void *relSegment) {
	Elf32_Rel *rel = NULL;	// relocation entry

	// Allocate memory for relocation table
	if (!(rel = (Elf32_Rel *)malloc(size))) {
		seterror("Out of memory.");
		return false;
	}

	// Read in our relocation table
	if (DLFile->seek(offset, SEEK_SET) < 0 ||
	        DLFile->read(rel, size) != (ssize_t)size) {
		seterror("Relocation table load failed.");
		free(rel);
		return false;
	}

	// Treat each relocation entry. Loop over all of them
	int cnt = size / sizeof(*rel);

	DBG("Loaded relocation table. %d entries. base address=%p\n", cnt, relSegment);

	bool seenHi16 = false;	// For treating HI/LO16 commands
	int firstHi16 = -1;		// Mark the point of the first hi16 seen
	Elf32_Addr ahl = 0;		// Calculated addend
	int a = 0;				// Addend: taken from the target

	unsigned int *lastTarget = 0;	// For processing hi16 when lo16 arrives
	unsigned int relocation = 0;
	int debugRelocs[10] = {0};		// For debugging
	int extendedHi16 = 0;			// Count extended hi16 treatments
	Elf32_Addr lastHiSymVal = 0;
	bool hi16InShorts = false;

#define DEBUG_NUM 2

	// Loop over relocation entries
	for (int i = 0; i < cnt; i++) {
		// Get the symbol this relocation entry is referring to
		Elf32_Sym *sym = (Elf32_Sym *)(_symtab) + (REL_INDEX(rel[i].r_info));

		// Get the target instruction in the code
		unsigned int *target = (unsigned int *)((char *)relSegment + rel[i].r_offset);

		unsigned int origTarget = *target;	// Save for debugging

		// Act differently based on the type of relocation
		switch (REL_TYPE(rel[i].r_info)) {

		case R_MIPS_HI16:						// Absolute addressing.
			if (sym->st_shndx < SHN_LOPROC &&		// Only shift for plugin section (ie. has a real section index)
			        firstHi16 < 0) {				// Only process first in block of HI16s
				firstHi16 = i;						// Keep the first Hi16 we saw
				seenHi16 = true;
				ahl = (*target & 0xffff) << 16;		// Take lower 16 bits shifted up

				lastHiSymVal = sym->st_value;
				hi16InShorts = (ShortsMan.inGeneralSegment((char *)sym->st_value)); // Fix for problem with switching btw segments
				if (debugRelocs[0]++ < DEBUG_NUM)	// Print only a set number
					DBG("R_MIPS_HI16: i=%d, offset=%x, ahl = %x, target = %x\n",
					    i, rel[i].r_offset, ahl, *target);
			}
			break;

		case R_MIPS_LO16:						// Absolute addressing. Needs a HI16 to come before it
			if (sym->st_shndx < SHN_LOPROC) {		// Only shift for plugin section. (ie. has a real section index)
				if (!seenHi16) {					// We MUST have seen HI16 first
					seterror("R_MIPS_LO16 w/o preceding R_MIPS_HI16 at relocation %d!\n", i);
					free(rel);
					return false;
				}

				// Fix: bug in gcc makes LO16s connect to wrong HI16s sometimes (shorts and regular segment)
				// Note that we can check the entire shorts segment because the executable's shorts don't belong to this plugin section
				//	and will be screened out above
				bool lo16InShorts = ShortsMan.inGeneralSegment((char *)sym->st_value);

				// Correct the bug by getting the proper value in ahl (taken from the current symbol)
				if ((hi16InShorts && !lo16InShorts) || (!hi16InShorts && lo16InShorts)) {
					ahl -= (lastHiSymVal & 0xffff0000);		// We assume gcc meant the same offset
					ahl += (sym->st_value & 0xffff0000);
				}

				ahl &= 0xffff0000;				// Clean lower 16 bits for repeated LO16s
				a = *target & 0xffff;			// Take lower 16 bits of the target
				a = (a << 16) >> 16;			// Sign extend them
				ahl += a;						// Add lower 16 bits. AHL is now complete

				// Fix: we can have LO16 access to the short segment sometimes
				if (lo16InShorts) {
					relocation = ahl + _shortsSegment->getOffset();		// Add in the short segment offset
				} else	// It's in the regular segment
					relocation = ahl + (Elf32_Addr)_segment;			// Add in the new offset for the segment

				if (firstHi16 >= 0) {					// We haven't treated the HI16s yet so do it now
					for (int j = firstHi16; j < i; j++) {
						if (REL_TYPE(rel[j].r_info) != R_MIPS_HI16) continue;	// Skip over non-Hi16s

						lastTarget = (unsigned int *)((char *)relSegment + rel[j].r_offset);	// get hi16 target
						*lastTarget &= 0xffff0000;		// Clear the lower 16 bits of the last target
						*lastTarget |= (relocation >> 16) & 0xffff;	// Take the upper 16 bits of the relocation
						if (relocation & 0x8000)(*lastTarget)++;	// Subtle: we need to add 1 to the HI16 in this case
					}
					firstHi16 = -1;						// Reset so we'll know we treated it
				} else {
					extendedHi16++;
				}

				*target &= 0xffff0000;						// Clear the lower 16 bits of current target
				*target |= relocation & 0xffff;				// Take the lower 16 bits of the relocation

				if (debugRelocs[1]++ < DEBUG_NUM)
					DBG("R_MIPS_LO16: i=%d, offset=%x, a=%x, ahl = %x, lastTarget = %x, origt = %x, target = %x\n",
					    i, rel[i].r_offset, a, ahl, *lastTarget, origTarget, *target);
				if (lo16InShorts && debugRelocs[2]++ < DEBUG_NUM)
					DBG("R_MIPS_LO16s: i=%d, offset=%x, a=%x, ahl = %x, lastTarget = %x, origt = %x, target = %x\n",
					    i, rel[i].r_offset, a, ahl, *lastTarget, origTarget, *target);
			}
			break;

		case R_MIPS_26:									// Absolute addressing (for jumps and branches only)
			if (sym->st_shndx < SHN_LOPROC) {			// Only relocate for main segment
				a = *target & 0x03ffffff;				// Get 26 bits' worth of the addend
				a = (a << 6) >> 6; 							// Sign extend a
				relocation = ((a << 2) + (Elf32_Addr)_segment) >> 2;	// a already points to the target. Subtract our offset
				*target &= 0xfc000000;					// Clean lower 26 target bits
				*target |= (relocation & 0x03ffffff);

				if (debugRelocs[3]++ < DEBUG_NUM)
					DBG("R_MIPS_26: i=%d, offset=%x, symbol=%d, stinfo=%x, a=%x, origTarget=%x, target=%x\n",
					    i, rel[i].r_offset, REL_INDEX(rel[i].r_info), sym->st_info, a, origTarget, *target);
			} else {
				if (debugRelocs[4]++ < DEBUG_NUM)
					DBG("R_MIPS_26: i=%d, offset=%x, symbol=%d, stinfo=%x, a=%x, origTarget=%x, target=%x\n",
					    i, rel[i].r_offset, REL_INDEX(rel[i].r_info), sym->st_info, a, origTarget, *target);
			}
			break;

		case R_MIPS_GPREL16:							// GP Relative addressing
			if (_shortsSegment->getOffset() != 0 && 	// Only relocate if we shift the shorts section
			        ShortsMan.inGeneralSegment((char *)sym->st_value)) {	// Only relocate things in the plugin hole
				a = *target & 0xffff;				    // Get 16 bits' worth of the addend
				a = (a << 16) >> 16;						// Sign extend it

				relocation = a + _shortsSegment->getOffset();

				*target &= 0xffff0000;					// Clear the lower 16 bits of the target
				*target |= relocation & 0xffff;

				if (debugRelocs[5]++ < DEBUG_NUM)
					DBG("R_MIPS_GPREL16: i=%d, a=%x, gpVal=%x, origTarget=%x, target=%x, offset=%x\n",
					    i, a, _gpVal, origTarget, *target, _shortsSegment->getOffset());
			}

			break;

		case R_MIPS_32:									// Absolute addressing
			if (sym->st_shndx < SHN_LOPROC) {			// Only shift for plugin section.
				a = *target;							// Get full 32 bits of addend

				if (ShortsMan.inGeneralSegment((char *)sym->st_value)) // Check if we're in the shorts segment
					relocation = a + _shortsSegment->getOffset();	   // Shift by shorts offset
				else												   // We're in the main section
					relocation = a + (Elf32_Addr)_segment;			   // Shift by main offset
				*target = relocation;

				if (debugRelocs[6]++ < DEBUG_NUM)
					DBG("R_MIPS_32: i=%d, a=%x, origTarget=%x, target=%x\n", i, a, origTarget, *target);
			}
			break;

		default:
			seterror("Unknown relocation type %x at relocation %d.\n", REL_TYPE(rel[i].r_info), i);
			free(rel);
			return false;
		}
	}

	DBG("Done with relocation. extendedHi16=%d\n\n", extendedHi16);

	free(rel);
	return true;
}

bool MIPSDLObject::relocateRels(Common::SeekableReadStream* DLFile, Elf32_Ehdr *ehdr, Elf32_Shdr *shdr) {

	// Loop over sections, finding relocation sections
	for (int i = 0; i < ehdr->e_shnum; i++) {

		Elf32_Shdr *curShdr = &(shdr[i]);
		//Elf32_Shdr *linkShdr = &(shdr[curShdr->sh_info]);

		if (curShdr->sh_type == SHT_REL && 						// Check for a relocation section
		        curShdr->sh_entsize == sizeof(Elf32_Rel) &&		    // Check for proper relocation size
		        (int)curShdr->sh_link == _symtab_sect &&			// Check that the sh_link connects to our symbol table
		        curShdr->sh_info < ehdr->e_shnum &&					// Check that the relocated section exists
		        (shdr[curShdr->sh_info].sh_flags & SHF_ALLOC)) {  	// Check if relocated section resides in memory
			if (!ShortsMan.inGeneralSegment((char *)shdr[curShdr->sh_info].sh_addr)) {			// regular segment
				if (!relocate(DLFile, curShdr->sh_offset, curShdr->sh_size, _segment)) {
					return false;
				}
			} else { 	// In Shorts segment
				if (!relocate(DLFile, curShdr->sh_offset, curShdr->sh_size, (void *)_shortsSegment->getOffset())) {
					return false;
				}
			}

		}
	}

	return true;
}

#endif /* defined(DYNAMIC_MODULES) && defined(MIPS_TARGET) */
