/*
Copyright (C) 2003-2005 The Pentagram team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "ultima8/misc/pent_include.h"

#include "ultima8/games/game_detector.h"
#include "ultima8/filesys/file_system.h"
#include "ultima8/games/game_info.h"
#include "raw_archive.h"
#include "md5.h"

#include "gamemd5.h"

bool GameDetector::detect(std::string path, GameInfo *info) {
	FileSystem *fs = FileSystem::get_instance();
	if (!fs->AddVirtualPath("@detect", path))
		return false;

	IDataSource *ids;


	// Strategy: find eusecode.flx, fusecode.flx or gusecode.flx,
	// compute its MD5, and check it against our MD5 table.
	// Should that fail, try a manual check to at least identify the
	// game type and its language.

	ids = fs->ReadFile("@detect/usecode/eusecode.flx");
	if (!ids)
		ids = fs->ReadFile("@detect/usecode/fusecode.flx");
	if (!ids)
		ids = fs->ReadFile("@detect/usecode/gusecode.flx");
	if (!ids)
		ids = fs->ReadFile("@detect/usecode/jusecode.flx");
	if (!ids)
		return false; // all games have usecode

	Pentagram::md5_file(ids, info->md5, 0);
	delete ids;

	std::string md5s = info->getPrintableMD5();

	int i = 0;
	while (Pentagram::md5table[i].md5) {
		if (md5s == Pentagram::md5table[i].md5) {
			info->type = Pentagram::md5table[i].type;
			info->language = Pentagram::md5table[i].language;
			info->version = Pentagram::md5table[i].version;
			return true;
		}
		i++;
	}

	perr << "MD5-based game autodetection failed (" << md5s << "). "
	     << std::endl << "Trying manual detection." << std::endl;



	// game type
	if (info->type == GameInfo::GAME_UNKNOWN) {

		ids = fs->ReadFile("@detect/static/u8gumps.flx"); // random U8 file
		if (ids) {
			info->type = GameInfo::GAME_U8;
			delete ids;
			ids = 0;
		}

	}

	if (info->type == GameInfo::GAME_UNKNOWN) {

		ids = fs->ReadFile("@detect/static/help1.dat"); // random remorse file
		if (ids) {
			info->type = GameInfo::GAME_REMORSE;
			delete ids;
			ids = 0;
		}

	}

	if (info->type == GameInfo::GAME_UNKNOWN) {

		ids = fs->ReadFile("@detect/static/help1.bmp"); // random regret file
		if (ids) {
			info->type = GameInfo::GAME_REGRET;
			delete ids;
			ids = 0;
		}

	}

	//TODO: game version
	info->version = 999;


	// game language

	// detect using eusecode/fusecode/gusecode
	if (info->language == GameInfo::GAMELANG_UNKNOWN) {
		ids = fs->ReadFile("@detect/usecode/eusecode.flx");
		if (ids) {
			if (info->type == GameInfo::GAME_U8) {
				// distinguish between english and spanish
				RawArchive *f = new RawArchive(ids);
				const char *buf = reinterpret_cast<const char *>((f->get_object_nodel(183)));
				unsigned int size = f->get_size(183);
				if (buf) {
					for (unsigned int i = 0; i + 9 < size; ++i) {
						if (strncmp(buf + i, "tableware", 9) == 0) {
							info->language = GameInfo::GAMELANG_ENGLISH;
							break;
						}
						if (strncmp(buf + i, "vajilla", 7) == 0) {
							info->language = GameInfo::GAMELANG_SPANISH;
							break;
						}
					}
				}
				delete f;
				ids = 0; // ids is deleted when f is deleted
			}

			// if still unsure, English
			if (info->language == GameInfo::GAMELANG_UNKNOWN)
				info->language = GameInfo::GAMELANG_ENGLISH;

			delete ids;
			ids = 0;
		}
	}
	if (info->language == GameInfo::GAMELANG_UNKNOWN) {
		ids = fs->ReadFile("@detect/usecode/fusecode.flx");
		if (ids) {
			info->language = GameInfo::GAMELANG_FRENCH;
			delete ids;
			ids = 0;
		}
	}
	if (info->language == GameInfo::GAMELANG_UNKNOWN) {
		ids = fs->ReadFile("@detect/usecode/gusecode.flx");
		if (ids) {
			info->language = GameInfo::GAMELANG_GERMAN;
			delete ids;
			ids = 0;
		}
	}
	if (info->language == GameInfo::GAMELANG_UNKNOWN) {
		ids = fs->ReadFile("@detect/usecode/jusecode.flx");
		if (ids) {
			info->language = GameInfo::GAMELANG_JAPANESE;
			delete ids;
			ids = 0;
		}
	}

	fs->RemoveVirtualPath("@detect");

	return (info->type != GameInfo::GAME_UNKNOWN &&
	        /* info->version != 0 && */
	        info->language != GameInfo::GAMELANG_UNKNOWN);
}
