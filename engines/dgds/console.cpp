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

#include "common/memstream.h"
#include "common/system.h"

#include "graphics/palette.h"
#include "graphics/paletteman.h"

#include "image/png.h"

#include "dgds/console.h"
#include "dgds/decompress.h"
#include "dgds/dgds.h"
#include "dgds/globals.h"
#include "dgds/includes.h"
#include "dgds/image.h"
#include "dgds/parser.h"
#include "dgds/resource.h"
#include "dgds/scene.h"
#include "dgds/game_palettes.h"
#include "gui/debugger.h"

namespace Dgds {

Console::Console(DgdsEngine *vm) : _vm(vm) {
	registerCmd("fileinfo", WRAP_METHOD(Console, cmdFileInfo));
	registerCmd("filesearch", WRAP_METHOD(Console, cmdFileSearch));
	registerCmd("filedump", WRAP_METHOD(Console, cmdFileDump));
	registerCmd("imagedump", WRAP_METHOD(Console, cmdImageDump));
	registerCmd("imagedumpall", WRAP_METHOD(Console, cmdImageDumpAll));
	registerCmd("global", WRAP_METHOD(Console, cmdGlobal));
}

bool Console::cmdFileInfo(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s <file>\n", argv[0]);
		return true;
	}

	const Resource &res = _vm->getResourceManager()->getResourceInfo(argv[1]);
	debugPrintf("Resource volume: %d, position: %d, size: %d, checksum: %d\n", res.volume, res.pos, res.size, res.checksum);

	return true;
}

bool Console::cmdFileSearch(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s <file to search>\n", argv[0]);
		return true;
	}

	const ResourceList &resources = _vm->getResourceManager()->getResources();
	for (ResourceList::const_iterator i = resources.begin(), end = resources.end(); i != end; ++i) {
		if (i->_key.contains(argv[1])) {
			Resource res = i->_value;
			debugPrintf("Resource: %s, volume: %d, position: %d, size: %d, checksum: %d\n", i->_key.c_str(), res.volume, res.pos, res.size, res.checksum);
		}
	}

	return true;
}

bool Console::cmdFileDump(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: %s <file> [ignore patches] [unpack] [outputpath] [chunktype]\n", argv[0]);
		debugPrintf("   eg: %s CLLIVING.ADH 0 1 clliving_unpack.adscript SCR:\n", argv[0]);
		return true;
	}

	Common::String fileName = argv[1];
	bool ignorePatches = (argc > 2) && (!scumm_stricmp(argv[2], "true") || !strcmp(argv[2], "1"));
	bool unpack = (argc > 3) && (!scumm_stricmp(argv[3], "true") || !strcmp(argv[3], "1"));
	Common::String dstPath = (argc > 4) ? argv[4] : "";
	Common::String chunkType = (argc > 5) ? argv[5] : "";
	Common::SeekableReadStream *resStream = _vm->getResource(fileName, ignorePatches);
	if (resStream == nullptr) {
		debugPrintf("Resource not found\n");
		return true;
	}
	int32 size = resStream->size();
	byte *data;

	if (!unpack) {
		data = new byte[size];
		resStream->read(data, size);
	} else {
		data = new byte[2000000]; // about 2MB, but maximum container size is around 1.5MB
		byte *ptr = data;
		const char *dot;
		DGDS_EX ex = 0;
		size = 0;

		if ((dot = strrchr(fileName.c_str(), '.'))) {
			ex = MKTAG24(toupper(dot[1]), toupper(dot[2]), toupper(dot[3]));
		}

		DgdsChunkReader chunk(resStream);
		while (chunk.readNextHeader(ex, fileName)) {
			if (!chunkType.empty() && !chunkType.equals(chunk.getIdStr())) {
				if (!chunk.isContainer())
					chunk.skipContent();
				continue;
			}

			chunk.readContent(_vm->getDecompressor());

			memcpy(ptr, chunk.getIdStr(), 4);
			ptr += 4;

			chunk.getContent()->read(ptr, chunk.getContent()->size());
			ptr += chunk.getContent()->size();

			size += 4 + chunk.getContent()->size();
		}

	}

	delete resStream;

	Common::DumpFile out;
	out.open(Common::Path(dstPath + fileName));
	if (!out.isOpen()) {
		warning("Couldn't open path %s%s", dstPath.c_str(), fileName.c_str());
	} else {
		out.write(data, size);
		out.flush();
		out.close();
	}
	delete[] data;

	return true;
}

bool Console::dumpImageFrame(const char *fname, int frameno, const char *outpath) {
	Image img(_vm->getResourceManager(), _vm->getDecompressor());
	int maxframe = img.frameCount(fname);
	if (frameno > maxframe) {
		debugPrintf("Image only has %d frames\n", maxframe);
		return false;
	}
	GamePalettes pal(_vm->getResourceManager(), _vm->getDecompressor());
	img.loadBitmap(fname);
	int width = img.width(frameno);
	int height = img.height(frameno);
	if (!width || !height) {
		debugPrintf("Image %s:%d not valid\n", fname, frameno);
		return false;
	}

	Common::DumpFile outf;
	Common::String outfname = Common::String::format("%s-%d.png", fname, frameno);

	if (outpath) {
		Common::Path path(outpath);
		path.joinInPlace(outfname);
		outf.open(path);
	} else {
		outf.open(Common::Path(outfname));
	}
	if (!outf.isOpen()) {
		debugPrintf("Couldn't open %s\n", outfname.c_str());
		return false;
	}

	byte palbuf[768];
	g_system->getPaletteManager()->grabPalette(palbuf, 0, 256);
	::Image::writePNG(outf, *(img.getSurface(frameno)->surfacePtr()), palbuf);
	outf.close();
	debugPrintf("wrote %dx%d png to %s\n", width, height, outfname.c_str());
	return true;
}

bool Console::cmdImageDumpAll(int argc, const char **argv) {
#ifdef USE_PNG
	if (argc < 2) {
		debugPrintf("Usage: %s <imagefilename> [outputdir]\n", argv[0]);
		debugPrintf("  eg: %s CLGAME2.BMP /tmp\n", argv[0]);
		return true;
	}

	const char *fname = argv[1];
	if (!_vm->getResourceManager()->hasResource(fname))  {
		debugPrintf("Resource %s not found\n", fname);
		return true;
	}

	Image img(_vm->getResourceManager(), _vm->getDecompressor());
	int maxframe = img.frameCount(fname);

	const char *outpath = (argc > 2 ? argv[2] : nullptr);

	for (int i = 0; i < maxframe; i++) {
		if (!dumpImageFrame(fname, i, outpath))
			break;
	}
	return true;
#else
	warning("dumpimage needs png support");
	return true;
#endif // USE_PNG

}

bool Console::cmdImageDump(int argc, const char **argv) {
#ifdef USE_PNG
	if (argc < 3) {
		debugPrintf("Usage: %s <imagefilename> <frameno> [outputdir]\n", argv[0]);
		debugPrintf("  eg: %s CLGAME2.BMP 2 /tmp\n", argv[0]);
		return true;
	}

	const char *fname = argv[1];
	int frameno = atoi(argv[2]);

	if (!_vm->getResourceManager()->hasResource(fname))  {
		debugPrintf("Resource %s not found\n", fname);
		return true;
	}

	const char *outpath = (argc > 3 ? argv[3] : nullptr);

	dumpImageFrame(fname, frameno, outpath);
	return true;
#else
	warning("dumpimage needs png support");
	return true;
#endif // USE_PNG
}

bool Console::cmdGlobal(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: %s <num> <val>\n", argv[0]);
		debugPrintf("%s <num> returns the value of a global\n", argv[0]);
		debugPrintf("%s <num> <val> sets the value of a global\n", argv[0]);
		return true;
	}

	GDSScene *scene = _vm->getGDSScene();
	int num = atoi(argv[1]);

	if (argc == 2) {
		int16 val = scene->getGlobal(num);
		debugPrintf("Global %d is %d\n", num, val);
	} else if (argc == 3) {
		int16 newVal = atoi(argv[2]);
		scene->setGlobal(num, newVal);
		debugPrintf("Global %d set to %d\n", num, newVal);
	}

	return true;
}

} // End of namespace Dgds
