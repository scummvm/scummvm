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

#include "gui/debugger.h"

#include "chewy/chewy.h"
#include "chewy/console.h"
#include "chewy/graphics.h"
#include "chewy/resource.h"
#include "chewy/scene.h"
#include "chewy/sound.h"
#include "chewy/text.h"

namespace Chewy {

Console::Console(ChewyEngine *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("dump",          WRAP_METHOD(Console, Cmd_Dump));
	registerCmd("dump_bg",       WRAP_METHOD(Console, Cmd_DumpBg));
	registerCmd("draw_image",    WRAP_METHOD(Console, Cmd_DrawImage));
	registerCmd("draw_sprite",   WRAP_METHOD(Console, Cmd_DrawSprite));
	registerCmd("play_sound",    WRAP_METHOD(Console, Cmd_PlaySound));
	registerCmd("play_speech",   WRAP_METHOD(Console, Cmd_PlaySpeech));
	registerCmd("play_music",    WRAP_METHOD(Console, Cmd_PlayMusic));
	registerCmd("play_video",    WRAP_METHOD(Console, Cmd_PlayVideo));
	registerCmd("video_info",    WRAP_METHOD(Console, Cmd_VideoInfo));
	registerCmd("error_message", WRAP_METHOD(Console, Cmd_ErrorMessage));
	registerCmd("dialog",        WRAP_METHOD(Console, Cmd_Dialog));
	registerCmd("text",          WRAP_METHOD(Console, Cmd_Text));
	registerCmd("scene",         WRAP_METHOD(Console, Cmd_Scene));
}

Console::~Console() {
}

bool Console::Cmd_Dump(int argc, const char **argv) {
	if (argc < 4) {
		debugPrintf("Usage: dump <file> <resource number> <dump file name>\n");
		return true;
	}

	Common::String filename = argv[1];
	int resNum = atoi(argv[2]);
	Common::String dumpFilename = argv[3];

	Resource *res = new Resource(filename);
	Chunk *chunk = res->getChunk(resNum);
	byte *data = res->getChunkData(resNum);
	uint32 size = chunk->size;

	Common::DumpFile outFile;
	outFile.open(dumpFilename);
	outFile.write(data, size);
	outFile.flush();
	outFile.close();

	delete[] data;
	delete res;

	return true;
}

bool Console::Cmd_DumpBg(int argc, const char **argv) {
	if (argc < 4) {
		debugPrintf("Usage: dump_bg <file> <resource number> <dump file name>\n");
		return true;
	}

	Common::String filename = argv[1];
	int resNum = atoi(argv[2]);
	Common::String dumpFilename = argv[3];

	BackgroundResource *res = new BackgroundResource(filename);
	TBFChunk *image = res->getImage(resNum);

	Common::DumpFile outFile;
	outFile.open(dumpFilename);
	outFile.write(image->data, image->size);
	outFile.flush();
	outFile.close();

	delete[] image->data;
	delete image;
	delete res;

	return true;
}


bool Console::Cmd_DrawImage(int argc, const char **argv) {
	if (argc < 3) {
		debugPrintf("Usage: draw_image <file> <resource number>\n");
		return true;
	}

	Common::String filename = argv[1];
	int resNum = atoi(argv[2]);

	_vm->_graphics->drawImage(filename, resNum);

	return false;
}

bool Console::Cmd_DrawSprite(int argc, const char **argv) {
	if (argc < 3) {
		debugPrintf("Usage: draw_sprite <file> <resource number> [x] [y]\n");
		return true;
	}

	Common::String filename = argv[1];
	int spriteNum = atoi(argv[2]);
	int x = (argc < 4) ? 0 : atoi(argv[3]);
	int y = (argc < 5) ? 0 : atoi(argv[4]);

	_vm->_graphics->drawSprite(filename, spriteNum, x, y);

	return false;
}

bool Console::Cmd_PlaySound(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: play_sound <number>\n");
		return true;
	}

	int resNum = atoi(argv[1]);
	_vm->_sound->playSound(resNum);

	return true;
}

bool Console::Cmd_PlaySpeech(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: play_speech <number>\n");
		return true;
	}

	int resNum = atoi(argv[1]);
	_vm->_sound->playSpeech(resNum);

	return true;
}

bool Console::Cmd_PlayMusic(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: play_music <number>\n");
		return true;
	}

	int resNum = atoi(argv[1]);
	_vm->_sound->playMusic(resNum);

	return true;
}

bool Console::Cmd_PlayVideo(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: play_video <number>\n");
		return true;
	}

	int resNum = atoi(argv[1]);
	_vm->setPlayVideo(resNum);

	return false;
}

bool Console::Cmd_VideoInfo(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: video_info <number>\n");
		return true;
	}

	int resNum = atoi(argv[1]);
	VideoResource *res = new VideoResource("cut.tap");
	VideoChunk *header = res->getVideoHeader(resNum);
	debugPrintf("Size: %d, %d x %d, %d frames, %d ms frame delay, first frame at %d\n", header->size, header->width, header->height, header->frameCount, header->frameDelay, header->firstFrameOffset);
	delete header;
	delete res;

	return true;
}

bool Console::Cmd_ErrorMessage(int argc, const char **argv) {
	if (argc < 3) {
		debugPrintf("Usage: error_message <file> <message number>\n");
		return true;
	}

	Common::String filename = argv[1];
	int resNum = atoi(argv[2]);

	ErrorMessage *res = new ErrorMessage(filename);
	Common::String str = res->getErrorMessage(resNum);
	this->debugPrintf("Error message: %s\n", str.c_str());
	delete res;

	return true;
}

bool Console::Cmd_Dialog(int argc, const char **argv) {
	if (argc < 3) {
		debugPrintf("Usage: dialog <dialog> <entry>\n");
		return true;
	}

	int dialogNum = atoi(argv[1]);
	int entryNum  = atoi(argv[2]);
	uint cur = 0;
	TextEntryList *d = _vm->_text->getDialog(dialogNum, entryNum);

	for (TextEntryList::iterator it = d->begin(); it != d->end(); ++it) {
		this->debugPrintf("Entry %d: speech %d, text '%s'\n", cur, (*it).speechId, (*it).text.c_str());
	}

	d->clear();
	delete d;

	return true;
}

bool Console::Cmd_Text(int argc, const char **argv) {
	if (argc < 3) {
		debugPrintf("Usage: text <dialog> <entry>\n");
		return true;
	}

	int dialogNum = atoi(argv[1]);
	int entryNum = atoi(argv[2]);
	TextEntry *d = _vm->_text->getText(dialogNum, entryNum);

	debugPrintf("Speech %d, text '%s'\n", d->speechId, d->text.c_str());

	delete d;

	return true;
}

bool Console::Cmd_Scene(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Current scene is: %d\n", _vm->_scene->getCurScene());
		debugPrintf("Use scene <scene num> to change the scene\n");
		return true;
	}

	int sceneNum = atoi(argv[1]);
	_vm->_scene->change(sceneNum);

	return false;
}

} // End of namespace Chewy
