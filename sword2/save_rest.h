/* Copyright (C) 1994-2003 Revolution Software Ltd
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef	SAVE_REST_H
#define	SAVE_REST_H

#include "sword2/memory.h"

namespace Sword2 {

#define	SAVE_DESCRIPTION_LEN	64

uint32 SaveGame(uint16 slotNo, uint8 *description);
uint32 RestoreGame(uint16 slotNo);
uint32 GetSaveDescription(uint16 slotNo, uint8 *description);
bool SaveExists(uint16 slotNo);
void FillSaveBuffer(mem *buffer, uint32 size, uint8 *desc);
uint32 RestoreFromBuffer(mem *buffer, uint32 size);
uint32 FindBufferSize(void);

// Save & Restore error codes

#define SR_OK			0x00000000	// No worries
#define SR_ERR_FILEOPEN		0x00000001	// can't open file - Couldn't
						// create file for saving, or
						// couldn't find file for
						// loading.
#define SR_ERR_INCOMPATIBLE	0x00000002	// (RestoreGame only)
						// incompatible savegame data.
						// Savegame file is obsolete.
						// (Won't happen after
						// development stops)
#define SR_ERR_READFAIL		0x00000003	// (RestoreGame only) failed on
						// reading savegame file -
						// Something screwed up during
						// the fread()
#define SR_ERR_WRITEFAIL	0x00000004	// (SaveGame only) failed on
						// writing savegame file -
						// Something screwed up during
						// the fwrite() - could be
						// hard-drive full..?

} // End of namespace Sword2

#endif
