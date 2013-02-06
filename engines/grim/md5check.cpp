/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/file.h"
#include "common/md5.h"

#include "gui/error.h"

#include "engines/grim/md5check.h"
#include "engines/grim/grim.h"

namespace Grim {

// Grim retail

const char *gfupd101[] = {
	"bc57c4db2ab814f3bb115ec5702b62f4"
};
const char *year4mus[] = {
	"939e041887c1b14f99ad951ac8e3cb25" //english unpatched
};
const char *year3mus[] = {
	"df556a8fc151f6f48bb68a8cef855e7d", //english unpatched
	"5115ac22556958552a53b5d54b90a68b" //italian unpatched
};
const char *year2mus[] = {
	"775376fccf0415258be6b33e7e6e4985" //english unpatched
};
const char *year1mus[] = {
	"c555c95b0f7557617da364091589d6ea" //english unpatched
};
const char *year0mus[] = {
	"fea10f11abe3ac32d53e4798f80d5ba0" //english unpatched
};
const char *vox0004[] = {
	"ae53a36f77eb0d42a28653b28c345439", //english unpatched
	"57915d4dd3aa5aa44817477b7bd41ce6" //italian unpatched
};
const char *vox0003[] = {
	"7c88ad7c949a516793b584299f33ed23", //english unpatched
	"f900cb063fbd0ef2a0516e9a00291d13" //italian unpatched
};
const char *vox0002[] = {
	"70419878e32377d9d2906fbcb2cb59b5", //english unpatched
	"616e4147c39268b650ce112bf79ea3a1" //italian unpatched
};
const char *vox0001[] = {
	"6a32f8079d2d98d1c10ec8364765fd4b", //english unpatched
	"ec015ac1123315274c3cc51a5626619c" //italian unpatched
};
const char *vox0000[] = {
	"67627f2d9314f9c0ae55dd2099fc052f", //english unpatched
	"c9e1a755f280b3ebff58578c4e5f1e52" //italian unpatched
};
const char *movie04[] = {
	"605d6f2cc4d21fbe702d8e637b5bcfa1", //english unpatched
	"7c8d7e0f4beaca3910658290e8b14cf8" //italian unpatched
};
const char *movie03[] = {
	"113a768e7206016df72f9d2f5c1851b7", //english unpatched
	"0fdb7dda9ebc049eac386d29bface366" //italian unpatched
};
const char *movie02[] = {
	"24a8704a331a03097d195f3597fe21e1", //english unpatched
	"129dad4e40987e91c5549c819d0dad1d" //italian unpatched
};
const char *movie01[] = {
	"548a19a0a4d4ccb8660966d646210820", //english unpatched
	"9fd30d0289f9b4d5c09e821a0a478db5" //italian unpatched
};
const char *movie00[] = {
	"0c6b8e4fa74024c4afdf7758f8d8b1a0" //english unpatched
};
const char *data004[] = {
	"2cdb79d3606965a9a0a3378507488dd7" //english unpatched
};
const char *data003[] = {
	"76dcfc2c21f3412415674c65611fb76d" //english unpatched
};
const char *data002[] = {
	"d236403c0d860961963db0134380001f" //english unpatched
};
const char *data001[] = {
	"84cf3072586fe2840ae16774adb6f5cb", //english unpatched
	"e0b2649e5f2cf5bd6fc61c27bd8d370a" //italian unpatched
};
const char *data000[] = {
	"08e2505a6a7fd90d3920131b1297c60f", //english unpatched
	"e9ddd5106b155c508a20bf85c8802d51" //italian unpatched
};
const char *credits[] = {
	"6dcecad8f01657184f8576aab8fb3f00", //english unpatched
	"1827307248d55c07642342c9213a4723" //italian unpatched
};
const char *local[] = {
	"6142624ce13ea3c9079aa80918010c4a" //italian unpatched
};


// Grim demo

const char *gfdemo01[] = {
	"25d831d57a93eb3ab8edbb07b7b63943"
};
const char *grimdemo[] = {
	"3ba28e7e36a49b5fd01ba98e3c772fe8"
};
const char *sound001[] = {
	"c91a7d405d15fb198a9cb31f89637026"
};
const char *voice001[] = {
	"f24a45079394fee296a0f7fad07c7fad"
};


bool MD5Check::_initted = false;
Common::List<MD5Check::MD5Sum> *MD5Check::_files = NULL;

void MD5Check::init() {
	if (_initted) {
		return;
	}
	_initted = true;
	_files = new Common::List<MD5Sum>();

	#define MD5SUM(filename, sums) _files->push_back(MD5Sum(filename, sums, sizeof(sums) / sizeof(const char *)));

	if (g_grim->getGameType() == GType_GRIM) {
		if (g_grim->getGameFlags() & ADGF_DEMO) {
			MD5SUM("gfdemo01.lab", gfdemo01)
			MD5SUM("grimdemo.mus", grimdemo)
			MD5SUM("sound001.lab", sound001)
			MD5SUM("voice001.lab", voice001)
		} else {
			MD5SUM("gfupd101.exe", gfupd101)
			MD5SUM("year4mus.lab", year4mus)
			MD5SUM("year3mus.lab", year3mus)
			MD5SUM("year2mus.lab", year2mus)
			MD5SUM("year1mus.lab", year1mus)
			MD5SUM("year0mus.lab", year0mus)
			MD5SUM("vox0004.lab", vox0004)
			MD5SUM("vox0003.lab", vox0003)
			MD5SUM("vox0002.lab", vox0002)
			MD5SUM("vox0001.lab", vox0001)
			MD5SUM("vox0000.lab", vox0000)
			MD5SUM("movie04.lab", movie04)
			MD5SUM("movie03.lab", movie03)
			MD5SUM("movie02.lab", movie02)
			MD5SUM("movie01.lab", movie01)
			MD5SUM("movie00.lab", movie00)
			MD5SUM("data004.lab", data004)
			MD5SUM("data003.lab", data003)
			MD5SUM("data002.lab", data002)
			MD5SUM("data001.lab", data001)
			MD5SUM("data000.lab", data000)
			MD5SUM("credits.lab", credits)
			if (g_grim->getGameLanguage() != Common::EN_ANY) {
				MD5SUM("local.lab", local)
			}
		}
	}

	#undef MD5SUM
}

void MD5Check::clear() {
	delete _files;
	_files = NULL;
	_initted = false;
}

bool MD5Check::checkMD5(const MD5Sum &sums, const char *md5) {
	for (int i = 0; i < sums.numSums; ++i) {
		if (strcmp(sums.sums[i], md5) == 0) {
			return true;
		}
	}
	return false;
}

bool MD5Check::checkFiles() {
	init();

	bool ok = true;
	for (Common::List<MD5Sum>::iterator i = _files->begin(); i != _files->end(); ++i) {
		const MD5Sum &sum = *i;;

		Common::File file;
		if (file.open(sum.filename)) {
			Common::String md5 = Common::computeStreamMD5AsString(file);
			if (!checkMD5(sum, md5.c_str())) {
				warning("'%s' may be corrupted. MD5: '%s'", sum.filename, md5.c_str());
				GUI::displayErrorDialog(Common::String::format("The game data file %s may be corrupted.\nIf you are sure it is "
				"not please provide the ResidualVM team the following code, along with the file name, the language and a "
				"description of your game version (i.e. dvd-box or jewelcase):\n%s", sum.filename, md5.c_str()).c_str());
				ok = false;
			}
		} else {
			warning("Could not open %s for checking", sum.filename);
			GUI::displayErrorDialog(Common::String::format("Coul not open the file %s for checking.\nIt may be missing or "
			"you may not have the rights to open it.\nGo to http://wiki.residualvm.org/index.php/Datafiles to see a list "
			"of the needed files.", sum.filename).c_str());
			ok = false;
		}
	}

	return ok;
}

}
