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

#ifndef DAAD_ADP_PREFIX_H
#define DAAD_ADP_PREFIX_H

/*
 * The vendored ADP interpreter exports a large number of globals and helper
 * functions with no namespace and no prefix (Free, Abort, StrCopy, MemCopy,
 * RandInt, charset, palette, interpreter, screenWidth, dmg, bitmap, ...).
 * ScummVM links every engine into a single binary, so those names are a
 * collision hazard with other engines and with the C library.
 *
 * Rather than patching the vendored sources (which would make every ADP
 * update a merge), every ADP translation unit is compiled with
 *
 *     -include engines/daad/adp_prefix.h
 *
 * which renames them to adp_*.
 * Our own shim files include this header explicitly,
 * but only AFTER their ScummVM includes.
 * Several of these names (screenWidth, palette, charset, ...)
 * would otherwise be substituted inside ScummVM's own headers.
 *
 * The list is generated from the declarations in the adp/include headers,
 * as well as the the globals that src-common expects the video backend to define.
 *
 * Names that already carry an ADP-specific prefix (DDB_, DMG_, VID_, SCR_,
 * DIM_, File_, Memory_, OS*) are deliberately left alone.
 */

/*
 * ADP's os_lib.cpp calls the Microsoft spellings of the case insensitive
 * string comparisons; upstream's makefiles map them to strcasecmp/strncasecmp.
 * Those are not available on every ScummVM target, so use ScummVM's portable
 * versions instead (defined in common/util.cpp).
 */
int scumm_stricmp(const char *s1, const char *s2);
int scumm_strnicmp(const char *s1, const char *s2, unsigned int n);

#define stricmp  scumm_stricmp
#define strnicmp scumm_strnicmp

#define Abort                      adp_Abort
#define AllocateArena              adp_AllocateArena
#define AllocateBlock              adp_AllocateBlock
#define AllocateBlockInPool        adp_AllocateBlockInPool
#define CGAPaletteCyan             adp_CGAPaletteCyan
#define CGAPaletteRed              adp_CGAPaletteRed
#define CPCPalette                 adp_CPCPalette
#define CPC_CloseDisk              adp_CPC_CloseDisk
#define CPC_CreateDisk             adp_CPC_CreateDisk
#define CPC_DeleteFile             adp_CPC_DeleteFile
#define CPC_DumpInfo               adp_CPC_DumpInfo
#define CPC_FindFile               adp_CPC_FindFile
#define CPC_FindFirstFile          adp_CPC_FindFirstFile
#define CPC_FindNextFile           adp_CPC_FindNextFile
#define CPC_GetFreeSpace           adp_CPC_GetFreeSpace
#define CPC_IsSpectrumDisk         adp_CPC_IsSpectrumDisk
#define CPC_OpenDisk               adp_CPC_OpenDisk
#define CPC_ReadFile               adp_CPC_ReadFile
#define CPC_WriteFile              adp_CPC_WriteFile
#define ChangeExtension            adp_ChangeExtension
#define Commodore64Palette         adp_Commodore64Palette
#define DebugPrintfImpl            adp_DebugPrintfImpl
#define DefaultCharset             adp_DefaultCharset
#define DefaultPalette             adp_DefaultPalette
#define DumpMemory                 adp_DumpMemory
#define EGAPalette                 adp_EGAPalette
#define FAT_ChangeDirectory        adp_FAT_ChangeDirectory
#define FAT_CloseDisk              adp_FAT_CloseDisk
#define FAT_CreateDisk             adp_FAT_CreateDisk
#define FAT_DumpInfo               adp_FAT_DumpInfo
#define FAT_FindFile               adp_FAT_FindFile
#define FAT_FindFirstFile          adp_FAT_FindFirstFile
#define FAT_FindNextFile           adp_FAT_FindNextFile
#define FAT_GetCWD                 adp_FAT_GetCWD
#define FAT_GetFileName            adp_FAT_GetFileName
#define FAT_GetFreeSpace           adp_FAT_GetFreeSpace
#define FAT_GetVolumeLabel         adp_FAT_GetVolumeLabel
#define FAT_MakeDirectory          adp_FAT_MakeDirectory
#define FAT_OpenDisk               adp_FAT_OpenDisk
#define FAT_ReadFile               adp_FAT_ReadFile
#define FAT_RemoveDirectory        adp_FAT_RemoveDirectory
#define FAT_RemoveFile             adp_FAT_RemoveFile
#define FAT_SetVolumeLabel         adp_FAT_SetVolumeLabel
#define FAT_WriteFile              adp_FAT_WriteFile
#define Free                       adp_Free
#define FreeArena                  adp_FreeArena
#define GetMaxAllocatableBlockSize adp_GetMaxAllocatableBlockSize
#define IntToHex2                  adp_IntToHex2
#define IsAlphaNumeric             adp_IsAlphaNumeric
#define IsDelimiter                adp_IsDelimiter
#define IsSpace                    adp_IsSpace
#define LongToChar                 adp_LongToChar
#define MSXPalette                 adp_MSXPalette
#define MemClear                   adp_MemClear
#define MemComp                    adp_MemComp
#define MemCopy                    adp_MemCopy
#define MemMove                    adp_MemMove
#define MemSet                     adp_MemSet
#define PCWDefaultPalette          adp_PCWDefaultPalette
#define Pal2RGB                    adp_Pal2RGB
#define RGB2Pal                    adp_RGB2Pal
#define RandInt                    adp_RandInt
#define RandReset                  adp_RandReset
#define RandSeed                   adp_RandSeed
#define RandSeedFromClock          adp_RandSeedFromClock
#define RandSetDefaultSeed         adp_RandSetDefaultSeed
#define StrCat                     adp_StrCat
#define StrComp                    adp_StrComp
#define StrCopy                    adp_StrCopy
#define StrIComp                   adp_StrIComp
#define StrLen                     adp_StrLen
#define StrRChr                    adp_StrRChr
#define ToLower                    adp_ToLower
#define ToUpper                    adp_ToUpper
#define TracePrintf                adp_TracePrintf
#define WinText_AddSpan            adp_WinText_AddSpan
#define WinText_ClearRect          adp_WinText_ClearRect
#define WinText_FindMenuOption     adp_WinText_FindMenuOption
#define WinText_FindText           adp_WinText_FindText
#define WinText_ListMenuOptions    adp_WinText_ListMenuOptions
#define WinText_Scroll             adp_WinText_Scroll
#define ZIP_Extract                adp_ZIP_Extract
#define ZIP_Finalize               adp_ZIP_Finalize
#define ZIP_Read                   adp_ZIP_Read
#define ZIP_Seek                   adp_ZIP_Seek
#define ZIP_Store                  adp_ZIP_Store
#define ZXSpectrumCharacterSet     adp_ZXSpectrumCharacterSet
#define ZXSpectrumPalette          adp_ZXSpectrumPalette
#define attributes                 adp_attributes
#define audioData                  adp_audioData
#define audioDataSize              adp_audioDataSize
#define backBuffer                 adp_backBuffer
#define beepSample                 adp_beepSample
#define beepSampleSize             adp_beepSampleSize
#define bitmap                     adp_bitmap
#define bufferedEntry              adp_bufferedEntry
#define bufferedHAM6               adp_bufferedHAM6
#define bufferedIndex              adp_bufferedIndex
#define buffering                  adp_buffering
#define charWidth                  adp_charWidth
#define charset                    adp_charset
#define charsetInitialized         adp_charsetInitialized
#define clickSample                adp_clickSample
#define clickSampleSize            adp_clickSampleSize
#define columnWidth                adp_columnWidth
#define defaultCharWidth           adp_defaultCharWidth
#define dmg                        adp_dmg
#define exitGame                   adp_exitGame
#define fix16                      adp_fix16
#define fix32                      adp_fix32
#define frontBuffer                adp_frontBuffer
#define graphicsBuffer             adp_graphicsBuffer
#define interpreter                adp_interpreter
#define lineHeight                 adp_lineHeight
#define mainLoopCallback           adp_mainLoopCallback
#define nativeImageMode            adp_nativeImageMode
#define palette                    adp_palette
#define pcxPalette                 adp_pcxPalette
#define pcxPictureData             adp_pcxPictureData
#define pcxPictureHeight           adp_pcxPictureHeight
#define pcxPictureSize             adp_pcxPictureSize
#define pcxPictureWidth            adp_pcxPictureWidth
#define pictureData                adp_pictureData
#define quit                       adp_quit
#define read16                     adp_read16
#define read16BE                   adp_read16BE
#define read16LE                   adp_read16LE
#define read32                     adp_read32
#define read32BE                   adp_read32BE
#define read32LE                   adp_read32LE
#define screenCellWidth            adp_screenCellWidth
#define screenHeight               adp_screenHeight
#define screenMachine              adp_screenMachine
#define screenMode                 adp_screenMode
#define screenWidth                adp_screenWidth
#define stride                     adp_stride
#define supportsOpenFileDialog     adp_supportsOpenFileDialog
#define textBuffer                 adp_textBuffer
#define textInput                  adp_textInput
#define videoInitialized           adp_videoInitialized
#define waitingForKey              adp_waitingForKey
#define write16                    adp_write16
#define write32                    adp_write32
#define xCoordMultiplier           adp_xCoordMultiplier
#define xmsgFilePresent            adp_xmsgFilePresent
#define yCoordMultiplier           adp_yCoordMultiplier
#define zxsPictureAttributes       adp_zxsPictureAttributes
#define zxsPictureBitmap           adp_zxsPictureBitmap
#define zxsPictureHeight           adp_zxsPictureHeight
#define zxsPictureMirror           adp_zxsPictureMirror
#define zxsPictureWidth            adp_zxsPictureWidth
#endif
