/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "glk/scott/decompress_z80.h"
#include "common/algorithm.h"
#include "common/array.h"
#include "common/textconsole.h"

namespace Glk {
namespace Scott {

/* Sizes of some of the arrays in the snap structure */
const int SNAPSHOT_RAM_PAGES = 16;
//const int SNAPSHOT_SLT_PAGES = 256;
//const int SNAPSHOT_ZXATASP_PAGES = 32;
//const int SNAPSHOT_ZXCF_PAGES = 64;
//const int SNAPSHOT_DOCK_EXROM_PAGES = 8;
//const int SNAPSHOT_JOYSTICKS = 7;
//const int SNAPSHOT_DIVIDE_PAGES = 4;

void *libspectrumReallocN(void *ptr, size_t nmemb, size_t size) {
	if (nmemb > SIZE_MAX / size)
		error("Can't reallocate to required size");

	return realloc(ptr, nmemb * size);
}
/* Ensure there is room for `requested' characters after the current
position `ptr' in `buffer'. If not, renew() and update the
pointers as necessary */
void libspectrumMakeRoom(uint8_t **dest, size_t requested, uint8_t **ptr,
						 size_t *allocated) {
	size_t current_length = 0;

	if (*allocated == 0) {

		(*allocated) = requested;
		*dest = new uint8_t[requested];

	} else {
		current_length = *ptr - *dest;

		/* If there's already enough room here, just return */
		if (current_length + requested <= (*allocated))
			return;

		/* Make the new size the maximum of the new needed size and the
	 old allocated size * 2 */
		(*allocated) = current_length + requested > 2 * (*allocated)
						   ? current_length + requested
						   : 2 * (*allocated);

		*dest = static_cast<uint8_t *>(libspectrumReallocN(*dest, *allocated, sizeof(uint8_t)));
	}

	/* Update the secondary pointer to the block */
	*ptr = *dest + current_length;
}

struct LibspectrumSnap {
	LibspectrumSnap() : pages(SNAPSHOT_RAM_PAGES) {}

	/* Which machine are we using here? */
	int machine;

	/* Registers and the like */
	uint16_t pc;

	Common::Array<uint8_t *> pages;
};

/* Error handling */

/* The various errors which can occur */
enum LibspectrumError {

	LIBSPECTRUM_ERROR_NONE = 0,

	LIBSPECTRUM_ERROR_WARNING,
	LIBSPECTRUM_ERROR_MEMORY,
	LIBSPECTRUM_ERROR_UNKNOWN,
	LIBSPECTRUM_ERROR_CORRUPT,
	LIBSPECTRUM_ERROR_SIGNATURE,
	LIBSPECTRUM_ERROR_SLT,     /* .slt data found at end of a .z80 file */
	LIBSPECTRUM_ERROR_INVALID, /* Invalid parameter supplied */

	LIBSPECTRUM_ERROR_LOGIC = -1,
};

enum LibspectrumMachine {

	LIBSPECTRUM_MACHINE_48,
	LIBSPECTRUM_MACHINE_TC2048,
	LIBSPECTRUM_MACHINE_128,
	LIBSPECTRUM_MACHINE_PLUS2,
	LIBSPECTRUM_MACHINE_PENT,
	LIBSPECTRUM_MACHINE_PLUS2A,
	LIBSPECTRUM_MACHINE_PLUS3,

	/* Used by libspectrum_tape_guess_hardware if we can't work out what
   hardware should be used */
	LIBSPECTRUM_MACHINE_UNKNOWN,

	LIBSPECTRUM_MACHINE_16,
	LIBSPECTRUM_MACHINE_TC2068,

	LIBSPECTRUM_MACHINE_SCORP,
	LIBSPECTRUM_MACHINE_PLUS3E,
	LIBSPECTRUM_MACHINE_SE,

	LIBSPECTRUM_MACHINE_TS2068,
	LIBSPECTRUM_MACHINE_PENT512,
	LIBSPECTRUM_MACHINE_PENT1024,
	LIBSPECTRUM_MACHINE_48_NTSC,

	LIBSPECTRUM_MACHINE_128E,

};

enum LibspectrumMachineCapability {
	LIBSPECTRUM_MACHINE_CAPABILITY_128_MEMORY = (1u << 0),
	LIBSPECTRUM_MACHINE_CAPABILITY_PLUS3_MEMORY = (1u << 1),
	LIBSPECTRUM_MACHINE_CAPABILITY_TIMEX_MEMORY = (1u << 2),
	LIBSPECTRUM_MACHINE_CAPABILITY_SCORP_MEMORY = (1u << 3),
	LIBSPECTRUM_MACHINE_CAPABILITY_SE_MEMORY = (1u << 4),
	LIBSPECTRUM_MACHINE_CAPABILITY_PENT512_MEMORY = (1u << 5),
	LIBSPECTRUM_MACHINE_CAPABILITY_PENT1024_MEMORY = (1u << 6),
};

/* Given a machine type, what features does it have? */
int libspectrumMachineCapabilities(LibspectrumMachine type) {
	int capabilities = 0;

	/* 128K Spectrum-style 0x7ffd memory paging */
	switch (type) {
	case LIBSPECTRUM_MACHINE_128:
	case LIBSPECTRUM_MACHINE_PLUS2:
	case LIBSPECTRUM_MACHINE_PLUS2A:
	case LIBSPECTRUM_MACHINE_PLUS3:
	case LIBSPECTRUM_MACHINE_PLUS3E:
	case LIBSPECTRUM_MACHINE_128E:
	case LIBSPECTRUM_MACHINE_PENT:
	case LIBSPECTRUM_MACHINE_PENT512:
	case LIBSPECTRUM_MACHINE_PENT1024:
	case LIBSPECTRUM_MACHINE_SCORP:
		/* FIXME: SE needs to have this capability to be considered a 128k machine
		 */
	case LIBSPECTRUM_MACHINE_SE:
		capabilities |= LIBSPECTRUM_MACHINE_CAPABILITY_128_MEMORY;
		break;
	default:
		break;
	}

	/* +3 Spectrum-style 0x1ffd memory paging */
	switch (type) {
	case LIBSPECTRUM_MACHINE_PLUS2A:
	case LIBSPECTRUM_MACHINE_PLUS3:
	case LIBSPECTRUM_MACHINE_PLUS3E:
	case LIBSPECTRUM_MACHINE_128E:
		capabilities |= LIBSPECTRUM_MACHINE_CAPABILITY_PLUS3_MEMORY;
		break;
	default:
		break;
	}

	/* T[CS]20[46]8-style 0x00fd memory paging */
	switch (type) {
	case LIBSPECTRUM_MACHINE_TC2048:
	case LIBSPECTRUM_MACHINE_TC2068:
	case LIBSPECTRUM_MACHINE_TS2068:
		capabilities |= LIBSPECTRUM_MACHINE_CAPABILITY_TIMEX_MEMORY;
		break;
	default:
		break;
	}

	/* Scorpion-style 0x1ffd memory paging */
	switch (type) {
	case LIBSPECTRUM_MACHINE_SCORP:
		capabilities |= LIBSPECTRUM_MACHINE_CAPABILITY_SCORP_MEMORY;
		break;
	default:
		break;
	}

	/* SE-style 0x7ffd and 0x00fd memory paging */
	switch (type) {
	case LIBSPECTRUM_MACHINE_SE:
		capabilities |= LIBSPECTRUM_MACHINE_CAPABILITY_SE_MEMORY;
		break;
	default:
		break;
	}

	/* Pentagon 512-style memory paging */
	switch (type) {
	case LIBSPECTRUM_MACHINE_PENT512:
	case LIBSPECTRUM_MACHINE_PENT1024:
		capabilities |= LIBSPECTRUM_MACHINE_CAPABILITY_PENT512_MEMORY;
		break;
	default:
		break;
	}

	/* Pentagon 1024-style memory paging */
	switch (type) {
	case LIBSPECTRUM_MACHINE_PENT1024:
		capabilities |= LIBSPECTRUM_MACHINE_CAPABILITY_PENT1024_MEMORY;
		break;
	default:
		break;
	}

	return capabilities;
}

/* Length of the basic .z80 headers */
static const int LIBSPECTRUM_Z80_HEADER_LENGTH = 30;

/* Length of the v2 extensions */
#define LIBSPECTRUM_Z80_V2_LENGTH 23

/* Length of the v3 extensions */
#define LIBSPECTRUM_Z80_V3_LENGTH 54

/* Length of xzx's extensions */
#define LIBSPECTRUM_Z80_V3X_LENGTH 55

/* The constants used for each machine type */
enum {

	/* v2 constants */
	Z80_MACHINE_48_V2 = 0,
	Z80_MACHINE_48_IF1_V2 = 1,
	Z80_MACHINE_48_SAMRAM_V2 = 2,
	Z80_MACHINE_128_V2 = 3,
	Z80_MACHINE_128_IF1_V2 = 4,

	/* v3 constants */
	Z80_MACHINE_48 = 0,
	Z80_MACHINE_48_IF1 = 1,
	Z80_MACHINE_48_SAMRAM = 2,
	Z80_MACHINE_48_MGT = 3,
	Z80_MACHINE_128 = 4,
	Z80_MACHINE_128_IF1 = 5,
	Z80_MACHINE_128_MGT = 6,

	/* Extensions */
	Z80_MACHINE_PLUS3 = 7,
	Z80_MACHINE_PLUS3_XZX_ERROR = 8,
	Z80_MACHINE_PENTAGON = 9,
	Z80_MACHINE_SCORPION = 10,
	Z80_MACHINE_PLUS2 = 12,
	Z80_MACHINE_PLUS2A = 13,
	Z80_MACHINE_TC2048 = 14,
	Z80_MACHINE_TC2068 = 15,
	Z80_MACHINE_TS2068 = 128,

	/* The first extension ID; anything here or greater applies to both
	 v2 and v3 files */
	Z80_MACHINE_FIRST_EXTENSION = Z80_MACHINE_PLUS3,
};

static LibspectrumError readHeader(const uint8_t *buffer, LibspectrumSnap *snap, const uint8_t **data, int *version, int *compressed);

static LibspectrumError getMachineType(LibspectrumSnap *snap, uint8_t type, uint8_t mgt_type, int version);

static LibspectrumError getMachineTypeV2(LibspectrumSnap *snap, uint8_t type);

static LibspectrumError getMachineTypeV3(LibspectrumSnap *snap, uint8_t type, uint8_t mgt_type);

static LibspectrumError getMachineTypeExtension(LibspectrumSnap *snap, uint8_t type);

static LibspectrumError readBlocks(const uint8_t *buffer, size_t buffer_length, LibspectrumSnap *snap, int version, int compressed);

static LibspectrumError readBlock(const uint8_t *buffer, LibspectrumSnap *snap, const uint8_t **next_block, const uint8_t *end, int version, int compressed);

static LibspectrumError readV1Block(const uint8_t *buffer, int is_compressed, uint8_t **uncompressed, const uint8_t **next_block, const uint8_t *end);

static LibspectrumError readV2Block(const uint8_t *buffer, uint8_t **block, size_t *length, int *page, const uint8_t **next_block, const uint8_t *end);

static void uncompressBlock(uint8_t **dest, size_t *dest_length, const uint8_t *src, size_t src_length);

void libspectrumSnapSetMachine(LibspectrumSnap *snap, int val) {
	snap->machine = val;
}

uint8_t *libspectrumSnapPages(LibspectrumSnap *snap, int page) {
	return snap->pages[page];
}

void libspectrumSnapSetPages(LibspectrumSnap *snap, int page, uint8_t *buf) {
	snap->pages[page] = buf;
}

void libspectrumPrintError(LibspectrumError error) {
	switch (error) {
	case LIBSPECTRUM_ERROR_WARNING:
		warning("warning");
		break;
	case LIBSPECTRUM_ERROR_MEMORY:
		warning("memory error");
		break;
	case LIBSPECTRUM_ERROR_UNKNOWN:
		warning("unknown error");
		break;
	case LIBSPECTRUM_ERROR_CORRUPT:
		warning("corruption error");
		break;
	case LIBSPECTRUM_ERROR_SIGNATURE:
		warning("signature error");
		break;
	case LIBSPECTRUM_ERROR_SLT:
		warning("SLT data in Z80 error");
		break;
	case LIBSPECTRUM_ERROR_INVALID:
		warning("invalid parameter error");
		break;
	case LIBSPECTRUM_ERROR_LOGIC:
		warning("logic error");
		break;
	default:
		warning("unhandled error");
		break;
	}
}

int libspectrumSnapMachine(LibspectrumSnap *snap) {
	return snap->machine;
}

/* Read an LSB dword from buffer */
uint32_t libspectrumReadDword(const uint8_t **buffer) {
	uint32_t value;

	value = (*buffer)[0] + (*buffer)[1] * 0x100 + (*buffer)[2] * 0x10000 + (*buffer)[3] * 0x1000000;

	(*buffer) += 4;

	return value;
}

static LibspectrumError getMachineTypeExtension(LibspectrumSnap *snap, uint8_t type) {
	switch (type) {
	case Z80_MACHINE_PLUS3:
	case Z80_MACHINE_PLUS3_XZX_ERROR:
		libspectrumSnapSetMachine(snap, LIBSPECTRUM_MACHINE_PLUS3);
		break;
	case Z80_MACHINE_PENTAGON:
		libspectrumSnapSetMachine(snap, LIBSPECTRUM_MACHINE_PENT);
		break;
	case Z80_MACHINE_SCORPION:
		libspectrumSnapSetMachine(snap, LIBSPECTRUM_MACHINE_SCORP);
		break;
	case Z80_MACHINE_PLUS2:
		libspectrumSnapSetMachine(snap, LIBSPECTRUM_MACHINE_PLUS2);
		break;
	case Z80_MACHINE_PLUS2A:
		libspectrumSnapSetMachine(snap, LIBSPECTRUM_MACHINE_PLUS2A);
		break;
	case Z80_MACHINE_TC2048:
		libspectrumSnapSetMachine(snap, LIBSPECTRUM_MACHINE_TC2048);
		break;
	case Z80_MACHINE_TC2068:
		libspectrumSnapSetMachine(snap, LIBSPECTRUM_MACHINE_TC2068);
		break;
	case Z80_MACHINE_TS2068:
		libspectrumSnapSetMachine(snap, LIBSPECTRUM_MACHINE_TS2068);
		break;
	default:
		libspectrumPrintError(LIBSPECTRUM_ERROR_UNKNOWN);
		warning("%s:get_machine_type: unknown extension machine type %d", __FILE__, type);
		return LIBSPECTRUM_ERROR_UNKNOWN;
	}

	return LIBSPECTRUM_ERROR_NONE;
}

LibspectrumError internalZ80Read(LibspectrumSnap *snap, const uint8_t *buffer, size_t buffer_length);

uint8_t *decompressZ80(uint8_t *raw_data, size_t length) {
	LibspectrumSnap *snap = new LibspectrumSnap;
	for (int i = 0; i < SNAPSHOT_RAM_PAGES; i++)
		libspectrumSnapSetPages(snap, i, nullptr);

	if (internalZ80Read(snap, raw_data, length) != LIBSPECTRUM_ERROR_NONE) {
		return nullptr;
	}

	uint8_t *uncompressed = new uint8_t[0xC000];
	if (uncompressed == nullptr)
		return nullptr;
	Common::copy(snap->pages[5], snap->pages[5] + 0x4000, uncompressed);
	Common::copy(snap->pages[2], snap->pages[2] + 0x4000, uncompressed + 0x4000);
	Common::copy(snap->pages[0], snap->pages[0] + 0x4000, uncompressed + 0x8000);

	for (int i = 0; i < SNAPSHOT_RAM_PAGES; i++)
		if (snap->pages[i] != nullptr)
			delete snap->pages[i];
	delete snap;

	return uncompressed;
}

LibspectrumError internalZ80Read(LibspectrumSnap *snap, const uint8_t *buffer, size_t buffer_length) {
	LibspectrumError error;
	const uint8_t *data;
	int version, compressed = 1;

	error = readHeader(buffer, snap, &data, &version, &compressed);
	if (error != LIBSPECTRUM_ERROR_NONE)
		return error;

	error = readBlocks(data, buffer_length - (data - buffer), snap, version,
					   compressed);
	if (error != LIBSPECTRUM_ERROR_NONE)
		return error;

	return LIBSPECTRUM_ERROR_NONE;
}

static LibspectrumError readHeader(const uint8_t *buffer, LibspectrumSnap *snap, const uint8_t **data, int *version, int *compressed) {
	const uint8_t *header = buffer;
	LibspectrumError error;

	uint8_t header_6 = header[6];
	uint8_t header_7 = header[7] * 0x100;
	snap->pc = header_6 + header_7;

	if (snap->pc == 0) { /* PC == 0x0000 => v2 or greater */

		size_t extra_length;
		const uint8_t *extra_header;

		extra_length = header[LIBSPECTRUM_Z80_HEADER_LENGTH] + header[LIBSPECTRUM_Z80_HEADER_LENGTH + 1] * 0x100;

		switch (extra_length) {
		case LIBSPECTRUM_Z80_V2_LENGTH:
			*version = 2;
			break;
		case LIBSPECTRUM_Z80_V3_LENGTH:
		case LIBSPECTRUM_Z80_V3X_LENGTH:
			*version = 3;
			break;
		default:
			libspectrumPrintError(LIBSPECTRUM_ERROR_UNKNOWN);
			warning("libspectrumReadZ80Header: unknown header length %d", (int)extra_length);
			return LIBSPECTRUM_ERROR_UNKNOWN;
		}

		extra_header = buffer + LIBSPECTRUM_Z80_HEADER_LENGTH + 2;

		snap->pc = extra_header[0] + extra_header[1] * 0x100;

		error = getMachineType(snap, extra_header[2], extra_header[51], *version);
		if (error)
			return error;

		if (extra_header[5] & 0x80) {

			switch (libspectrumSnapMachine(snap)) {
			case LIBSPECTRUM_MACHINE_48:
				libspectrumSnapSetMachine(snap, LIBSPECTRUM_MACHINE_16);
				break;
			case LIBSPECTRUM_MACHINE_128:
				libspectrumSnapSetMachine(snap, LIBSPECTRUM_MACHINE_PLUS2);
				break;
			case LIBSPECTRUM_MACHINE_PLUS3:
				libspectrumSnapSetMachine(snap, LIBSPECTRUM_MACHINE_PLUS2A);
				break;
			default:
				break; /* Do nothing */
			}
		}

		(*data) = buffer + LIBSPECTRUM_Z80_HEADER_LENGTH + 2 + extra_length;

	} else { /* v1 .z80 file */

		libspectrumSnapSetMachine(snap, LIBSPECTRUM_MACHINE_48);
		*version = 1;

		/* Need to flag this for later */
		*compressed = (header[12] & 0x20) ? 1 : 0;

		(*data) = buffer + LIBSPECTRUM_Z80_HEADER_LENGTH;
	}

	return LIBSPECTRUM_ERROR_NONE;
}

static LibspectrumError getMachineType(LibspectrumSnap *snap, uint8_t type, uint8_t mgt_type, int version) {
	LibspectrumError error;

	if (type < Z80_MACHINE_FIRST_EXTENSION) {
		switch (version) {
		case 2:
			error = getMachineTypeV2(snap, type);
			if (error)
				return error;
			break;
		case 3:
			error = getMachineTypeV3(snap, type, mgt_type);
			if (error)
				return error;
			break;
		default:
			libspectrumPrintError(LIBSPECTRUM_ERROR_LOGIC);
			warning("%s:getMachineType: unknown version %d", __FILE__, version);
			return LIBSPECTRUM_ERROR_LOGIC;
		}

	} else {
		error = getMachineTypeExtension(snap, type);
		if (error)
			return error;
	}

	return LIBSPECTRUM_ERROR_NONE;
}

static LibspectrumError getMachineTypeV2(LibspectrumSnap *snap, uint8_t type) {
	switch (type) {
	case Z80_MACHINE_48_V2:
	case Z80_MACHINE_48_SAMRAM_V2:
		libspectrumSnapSetMachine(snap, LIBSPECTRUM_MACHINE_48);
		break;
	case Z80_MACHINE_48_IF1_V2:
		libspectrumSnapSetMachine(snap, LIBSPECTRUM_MACHINE_48);
		break;
	case Z80_MACHINE_128_V2:
		libspectrumSnapSetMachine(snap, LIBSPECTRUM_MACHINE_128);
		break;
	case Z80_MACHINE_128_IF1_V2:
		libspectrumSnapSetMachine(snap, LIBSPECTRUM_MACHINE_128);
		break;
	default:
		libspectrumPrintError(LIBSPECTRUM_ERROR_UNKNOWN);
		warning("%s: getMachineType: unknown v2 machine type %d", __FILE__, type);
		return LIBSPECTRUM_ERROR_UNKNOWN;
	}

	return LIBSPECTRUM_ERROR_NONE;
}

static LibspectrumError getMachineTypeV3(LibspectrumSnap *snap, uint8_t type, uint8_t mgt_type) {
	switch (type) {
	case Z80_MACHINE_48:
	case Z80_MACHINE_48_SAMRAM:
		libspectrumSnapSetMachine(snap, LIBSPECTRUM_MACHINE_48);
		break;
	case Z80_MACHINE_48_IF1:
		libspectrumSnapSetMachine(snap, LIBSPECTRUM_MACHINE_48);
		break;
	case Z80_MACHINE_48_MGT:
		libspectrumSnapSetMachine(snap, LIBSPECTRUM_MACHINE_48);
		break;
	case Z80_MACHINE_128:
		libspectrumSnapSetMachine(snap, LIBSPECTRUM_MACHINE_128);
		break;
	case Z80_MACHINE_128_MGT:
		libspectrumSnapSetMachine(snap, LIBSPECTRUM_MACHINE_128);
		break;
	default:
		libspectrumPrintError(LIBSPECTRUM_ERROR_UNKNOWN);
		warning("%s:getMachineType: unknown v3 machine type %d", __FILE__, type);
		return LIBSPECTRUM_ERROR_UNKNOWN;
	}

	return LIBSPECTRUM_ERROR_NONE;
}

/* Given a 48K memory dump `data', place it into the
 appropriate bits of `snap' for a 48K machine */
LibspectrumError libspectrumSplitTo48kPages(LibspectrumSnap *snap, const uint8_t *data) {
	uint8_t *buffer[3];
	size_t i;

	/* If any of the three pages are already occupied, barf */
	if (libspectrumSnapPages(snap, 5) || libspectrumSnapPages(snap, 2) || libspectrumSnapPages(snap, 0)) {
		libspectrumPrintError(LIBSPECTRUM_ERROR_LOGIC);
		warning("libspectrumSplitTo48kPages: RAM page already in use");
		return LIBSPECTRUM_ERROR_LOGIC;
	}

	for (i = 0; i < 3; i++) {
		buffer[i] = new uint8_t[0x4000];
	}

	libspectrumSnapSetPages(snap, 5, buffer[0]);
	libspectrumSnapSetPages(snap, 2, buffer[1]);
	libspectrumSnapSetPages(snap, 0, buffer[2]);

	/* Finally, do the copies... */
	Common::copy(&data[0x0000], &data[0x0000] + 0x4000, libspectrumSnapPages(snap, 5));
	Common::copy(&data[0x4000], &data[0x4000] + 0x4000, libspectrumSnapPages(snap, 2));
	Common::copy(&data[0x8000], &data[0x8000] + 0x4000, libspectrumSnapPages(snap, 0));

	return LIBSPECTRUM_ERROR_NONE;
}

static LibspectrumError readBlocks(const uint8_t *buffer, size_t buffer_length, LibspectrumSnap *snap, int version, int compressed) {
	const uint8_t *end, *next_block;

	end = buffer + buffer_length;
	next_block = buffer;

	while (next_block < end) {
		LibspectrumError error;
		error = readBlock(next_block, snap, &next_block, end, version, compressed);
		if (error != LIBSPECTRUM_ERROR_NONE)
			return error;
	}

	return LIBSPECTRUM_ERROR_NONE;
}

static LibspectrumError readBlock(const uint8_t *buffer, LibspectrumSnap *snap, const uint8_t **next_block, const uint8_t *end, int version, int compressed) {
	LibspectrumError error;
	uint8_t *uncompressed;

	int capabilities = libspectrumMachineCapabilities(static_cast<LibspectrumMachine>(libspectrumSnapMachine(snap)));

	if (version == 1) {
		error = readV1Block(buffer, compressed, &uncompressed, next_block, end);
		if (error != LIBSPECTRUM_ERROR_NONE)
			return error;

		libspectrumSplitTo48kPages(snap, uncompressed);

		delete uncompressed;

	} else {

		size_t length;
		int page;

		error = readV2Block(buffer, &uncompressed, &length, &page, next_block, end);
		if (error != LIBSPECTRUM_ERROR_NONE)
			return error;

		if (page <= 0 || page > 18) {
			libspectrumPrintError(LIBSPECTRUM_ERROR_UNKNOWN);
			warning("readBlock: unknown page %d", page);
			delete uncompressed;
			return LIBSPECTRUM_ERROR_UNKNOWN;
		}

		/* If it's a ROM page, just throw it away */
		if (page < 3) {
			delete uncompressed;
			return LIBSPECTRUM_ERROR_NONE;
		}

		/* Page 11 is the Multiface ROM unless we're emulating something
	   Scorpion-like */
		if (page == 11 && !(capabilities & LIBSPECTRUM_MACHINE_CAPABILITY_SCORP_MEMORY)) {
			delete uncompressed;
			return LIBSPECTRUM_ERROR_NONE;
		}

		/* Deal with 48K snaps -- first, throw away page 3, as it's a ROM.
	   Then remap the numbers slightly */
		if (!(capabilities & LIBSPECTRUM_MACHINE_CAPABILITY_128_MEMORY)) {
			switch (page) {
			case 3:
				delete uncompressed;
				return LIBSPECTRUM_ERROR_NONE;
			case 4:
				page = 5;
				break;
			case 5:
				page = 3;
				break;
			}
		}

		/* Now map onto RAM page numbers */
		page -= 3;

		if (libspectrumSnapPages(snap, page) == nullptr) {
			libspectrumSnapSetPages(snap, page, uncompressed);
		} else {
			delete uncompressed;
			libspectrumPrintError(LIBSPECTRUM_ERROR_UNKNOWN);
			warning("readBlock: page %d duplicated", page);
			return LIBSPECTRUM_ERROR_CORRUPT;
		}
	}

	return LIBSPECTRUM_ERROR_NONE;
}

static LibspectrumError readV1Block(const uint8_t *buffer, int is_compressed, uint8_t **uncompressed, const uint8_t **next_block, const uint8_t *end) {
	if (is_compressed) {
		const uint8_t *ptr;
		int state;
		size_t uncompressed_length = 0;

		state = 0;
		ptr = buffer;

		while (state != 4) {
			if (ptr == end) {
				libspectrumPrintError(LIBSPECTRUM_ERROR_CORRUPT);
				warning("readV1Block: end marker not found");
				return LIBSPECTRUM_ERROR_CORRUPT;
			}

			switch (state) {
			case 0:
				switch (*ptr++) {
				case 0x00:
					state = 1;
					break;
				default:
					state = 0;
					break;
				}
				break;
			case 1:
				switch (*ptr++) {
				case 0x00:
					state = 1;
					break;
				case 0xed:
					state = 2;
					break;
				default:
					state = 0;
					break;
				}
				break;
			case 2:
				switch (*ptr++) {
				case 0x00:
					state = 1;
					break;
				case 0xed:
					state = 3;
					break;
				default:
					state = 0;
					break;
				}
				break;
			case 3:
				switch (*ptr++) {
				case 0x00:
					state = 4;
					break;
				default:
					state = 0;
					break;
				}
				break;
			default:
				libspectrumPrintError(LIBSPECTRUM_ERROR_LOGIC);
				warning("readV1Block: unknown state %d", state);
				return LIBSPECTRUM_ERROR_LOGIC;
			}
		}

		/* Length passed here is reduced by 4 to remove the end marker */
		uncompressBlock(uncompressed, &uncompressed_length, buffer, (ptr - buffer - 4));

		/* Uncompressed data must be exactly 48Kb long */
		if (uncompressed_length != 0xc000) {
			delete *uncompressed;
			libspectrumPrintError(LIBSPECTRUM_ERROR_CORRUPT);
			warning("readV1Block: data does not uncompress to 48Kb");
			return LIBSPECTRUM_ERROR_CORRUPT;
		}

		*next_block = ptr;

	} else { /* Snap isn't compressed */
		/* Check we've got enough bytes to read */
		if (end - *next_block < 0xc000) {
			libspectrumPrintError(LIBSPECTRUM_ERROR_CORRUPT);
			warning("readV1Block: not enough data in buffer");
			return LIBSPECTRUM_ERROR_CORRUPT;
		}

		*uncompressed = new uint8_t[0xC000];
		Common::copy(buffer, buffer + 0xC000, *uncompressed);
		*next_block = buffer + 0xc000;
	}

	return LIBSPECTRUM_ERROR_NONE;
}

/* The signature used to designate the .slt extensions */
static uint8_t slt_signature[] = "\0\0\0SLT";
static size_t slt_signature_length = 6;

static LibspectrumError readV2Block(const uint8_t *buffer, uint8_t **block, size_t *length, int *page, const uint8_t **next_block, const uint8_t *end) {
	size_t length2;
	length2 = buffer[0] + buffer[1] * 0x100;
	(*page) = buffer[2];

	if (length2 == 0 && *page == 0) {
		if (buffer + 8 < end && !memcmp(buffer, slt_signature, slt_signature_length)) {
			/* Ah, we have what looks like SLT data... */
			*next_block = buffer + 6;
			return LIBSPECTRUM_ERROR_SLT;
		}
	}

	/* A length of 0xffff => 16384 bytes of uncompressed data */
	if (length2 != 0xffff) {
		/* Check we're not going to run over the end of the buffer */
		if (buffer + 3 + length2 > end) {
			libspectrumPrintError(LIBSPECTRUM_ERROR_CORRUPT);
			warning("readV2Block: not enough data in buffer");
			return LIBSPECTRUM_ERROR_CORRUPT;
		}

		*length = 0;
		uncompressBlock(block, length, buffer + 3, length2);
		*next_block = buffer + 3 + length2;

	} else { /* Uncompressed block */
		/* Check we're not going to run over the end of the buffer */
		if (buffer + 3 + 0x4000 > end) {
			libspectrumPrintError(LIBSPECTRUM_ERROR_CORRUPT);
			warning("readV2Block: not enough data in buffer");
			return LIBSPECTRUM_ERROR_CORRUPT;
		}

		*block = new uint8_t[0x4000];
		Common::copy(buffer + 3, buffer + 3 + 0x4000, *block);

		*length = 0x4000;
		*next_block = buffer + 3 + 0x4000;
	}

	return LIBSPECTRUM_ERROR_NONE;
}

static void uncompressBlock(uint8_t **dest, size_t *dest_length, const uint8_t *src, size_t src_length) {
	const uint8_t *in_ptr;
	uint8_t *out_ptr;

	/* Allocate memory for dest if requested */
	if (*dest_length == 0) {
		*dest_length = src_length / 2;
		*dest = new uint8_t[*dest_length];
	}

	in_ptr = src;
	out_ptr = *dest;

	while (in_ptr < src + src_length) {
		/* If we're pointing at the last byte, just copy it across and exit */
		if (in_ptr == src + src_length - 1) {
			libspectrumMakeRoom(dest, 1, &out_ptr, dest_length);
			*out_ptr++ = *in_ptr++;
			continue;
		}

		/* If we're pointing at two successive 0xed bytes, that's a run. If not, just copy the byte across */
		if (*in_ptr == 0xed && *(in_ptr + 1) == 0xed) {
			size_t run_length;
			uint8_t repeated;

			in_ptr += 2;
			run_length = *in_ptr++;
			repeated = *in_ptr++;

			libspectrumMakeRoom(dest, run_length, &out_ptr, dest_length);

			while (run_length--) {
				*out_ptr++ = repeated;
			}
		} else {
			libspectrumMakeRoom(dest, 1, &out_ptr, dest_length);
			*out_ptr++ = *in_ptr++;
		}
	}

	*dest_length = out_ptr - *dest;
}

} // End of namespace Scott
} // End of namespace Glk
