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

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/events.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/platform.h"
#include "common/str-array.h"
#include "common/stream.h"
#include "common/substream.h"
#include "common/system.h"

#include "common/iff_container.h"

#include "audio/audiostream.h"
#include "audio/decoders/aiff.h"
#include "audio/decoders/raw.h"
#include "audio/mixer.h"

#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/managed_surface.h"
#include "graphics/palette.h"
#include "graphics/surface.h"

#include "engines/advancedDetector.h"
#include "engines/util.h"

#include "dgds/console.h"
#include "dgds/decompress.h"
#include "dgds/detection_tables.h"
#include "dgds/dgds.h"
#include "dgds/font.h"
#include "dgds/image.h"
#include "dgds/includes.h"
#include "dgds/movies.h"
#include "dgds/music.h"
#include "dgds/parser.h"
#include "dgds/resource.h"
#include "dgds/sound.h"

namespace Dgds {

Graphics::ManagedSurface resData;
Graphics::Surface bottomBuffer;
Graphics::Surface topBuffer;

Common::MemoryReadStream *soundData;
byte *musicData;
uint32 musicSize;

Common::StringArray _bubbles;
Common::StringArray BMPs;

PFont *_fntP;
FFont *_fntF;

#define DGDS_FILENAME_MAX 12

Common::SeekableReadStream *ttm;
char ttmName[DGDS_FILENAME_MAX + 1];

Common::SeekableReadStream *ads;
char adsName[DGDS_FILENAME_MAX + 1];

DgdsEngine::DgdsEngine(OSystem *syst, const ADGameDescription *gameDesc)
    : Engine(syst) {
	syncSoundSettings();

	_platform = gameDesc->platform;

	if (!strcmp(gameDesc->gameId, "rise"))
		_gameId = GID_DRAGON;
	else if (!strcmp(gameDesc->gameId, "china"))
		_gameId = GID_CHINA;
	else if (!strcmp(gameDesc->gameId, "beamish"))
		_gameId = GID_BEAMISH;

	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "patches");
}

DgdsEngine::~DgdsEngine() {
	DebugMan.clearAllDebugChannels();

	delete _image;
	delete _decompressor;
	delete _resource;
}

void readStrings(Common::SeekableReadStream *stream) {
	uint16 count = stream->readUint16LE();
	debug("        %u:", count);

	for (uint16 k = 0; k < count; k++) {
		byte ch;
		uint16 idx;
		idx = stream->readUint16LE();

		Common::String str;
		while ((ch = stream->readByte()))
			str += ch;
		debug("        %2u: %2u, \"%s\"", k, idx, str.c_str());
	}
}

struct Tag {
	uint16 id;
	Common::String tag;
};

Common::HashMap<uint16, Common::String> *readTags(Common::SeekableReadStream *stream) {
	Common::HashMap<uint16, Common::String> *tags = new Common::HashMap<uint16, Common::String>;
	uint16 count = stream->readUint16LE();
	debug("        %u:", count);

	for (uint16 i = 0; i < count; i++) {
		Common::String string;
		byte c = 0;
		uint16 idx = stream->readUint16LE();
		while (c = stream->readByte())
			string += c;
		debug("        %2u: %2u, \"%s\"", i, idx, string.c_str());

		(*tags)[idx] = string;
	}

	return tags;
}

void readSDS(Common::SeekableReadStream *stream) {
	uint32 mark;

	// Debug
	/*uint32 pos = stream->pos();
					byte *tmp = new byte[stream->size()];
					stream->read(tmp, stream->size());
					Common::hexdump(tmp, stream->size());
					stream->seek(pos, SEEK_SET);*/

	mark = stream->readUint32LE();
	debug("    0x%X", mark);

	char version[7];
	stream->read(version, sizeof(version));
	debug("    %s", version);

	uint16 idx;
	idx = stream->readUint16LE();
	debug("    S%d.SDS", idx);

	// gross hack to grep the strings.
	_bubbles.clear();

	bool inside = false;
	Common::String txt;
	while (1) {
		char buf[4];
		stream->read(buf, sizeof(buf));
		if (stream->pos() >= stream->size())
			break;
		if (Common::isPrint(buf[0]) && Common::isPrint(buf[1]) && Common::isPrint(buf[2]) && Common::isPrint(buf[3])) {
			inside = true;
		}
		stream->seek(-3, SEEK_CUR);

		if (inside) {
			if (buf[0] == '\0') {
				// here's where we do a clever thing. we want Pascal like strings.
				uint16 pos = txt.size() + 1;
				stream->seek(-pos - 2, SEEK_CUR);
				uint16 len = stream->readUint16LE();
				stream->seek(pos, SEEK_CUR);

				// gotcha!
				if (len == pos) {
					//if (resource == 0)
						_bubbles.push_back(txt);
					debug("    \"%s\"", txt.c_str());
				}
				// let's hope the string wasn't shorter than 4 chars...
				txt.clear();
				inside = false;
			} else {
				txt += buf[0];
			}
		}
	}
#if 0
						idx = stream->readUint16LE();
						debug("    %d", idx);

						idx = stream->readUint16LE();
						debug("    %d", idx);

						uint16 count;
						while (1) {
							uint16 code;
							code = stream->readUint16LE();
							count = stream->readUint16LE();
							idx = stream->readUint16LE();

							debugN("\tOP: 0x%8.8x %2u %2u\n", code, count, idx);

							uint16 pitch = (count+1)&(~1); // align to word.
							if ((stream->pos()+pitch) >= stream->size()) break;

							if (code == 0 && count == 0) break;

							stream->skip(pitch);
						}

						Common::String sval;
						byte ch;

						do {
							ch = stream->readByte();
							sval += ch;
						} while (ch != 0);

						debug("\"%s\"", sval.c_str());
#endif
#if 0
						// probe for the .ADS name. are these shorts?
						uint count;
						count = 0;
						while (1) {
							uint16 x;
							x = stream->readUint16LE();
							if ((x & 0xFF00) != 0)
								break;
							debug("      %u: %u|0x%4.4X", count++, x, x);
						}
						stream->seek(-2, SEEK_CUR);

						// .ADS name.
						Common::String ads;
						byte ch;
						while ((ch = stream->readByte()))
							ads += ch;
						debug("    %s", ads.c_str());

						stream->hexdump(6);
						stream->skip(6);

						int w, h;

						w = stream->readSint16LE();
						h = stream->readSint16LE();
						debug("    %dx%d", w, h);

						// probe for the strings. are these shorts?
						count = 0;
						while (1) {
							uint16 x;
							x = stream->readUint16LE();
							if ((x & 0xFF00) != 0)
								break;
							if (stream->pos() >= stream->size()) break;
							debug("      %u: %u|0x%4.4X", count++, x, x);
						}
						stream->seek(-4, SEEK_CUR);
						// here we are.

						uint16 len;
						len = stream->readSint16LE();
						Common::String txt;
						for (uint16 j=0; j<len; j++) {
							ch = stream->readByte();
							txt += ch;
							debug("      \"%s\"", txt.c_str());
						}
#endif
}

void parseFileInner(Common::Platform platform, Common::SeekableReadStream &file, const char *name, int resource, Decompressor *decompressor) {
	const char *dot;
	DGDS_EX ex = 0;

	if ((dot = strrchr(name, '.'))) {
		ex = MKTAG24(dot[1], dot[2], dot[3]);
	}

	uint parent = 0;

	DgdsParser ctx(file, name);
	if (isFlatfile(platform, ex)) {
		uint16 tcount;
		uint16 *tw, *th;
		uint32 *toffset;
		Common::String line;

		switch (ex) {
		case EX_RST: {
			uint32 mark;

			mark = file.readUint32LE();
			debug("    0x%X", mark);

			// elaborate guesswork. who knows it might be true.
			while (!file.eos()) {
				uint16 idx;
				uint16 vals[7];

				idx = file.readUint16LE();
				debugN("  #%u:\t", idx);
				if (idx == 0)
					break;
				for (int i = 0; i < ARRAYSIZE(vals); i++) {
					vals[i] = file.readUint16LE();
					if (i != 0)
						debugN(", ");
					debugN("%u", vals[i]);
				}
				debug(".");
			}
			debug("-");

			while (!file.eos()) {
				uint16 idx;
				uint16 vals[2];
				idx = file.readUint16LE();
				debugN("  #%u:\t", idx);
				for (int i = 0; i < ARRAYSIZE(vals); i++) {
					vals[i] = file.readUint16LE();
					if (i != 0)
						debugN(", ");
					debugN("%u", vals[i]);
				}
				debug(".");
				if (idx == 0)
					break;
			}
			debug("-");
		} break;
		case EX_SCR: {
			/* Unknown image format (Amiga). */
			byte tag[5];
			file.read(tag, 4); /* maybe */
			tag[4] = '\0';

			uint16 pitch, planes;
			pitch = file.readUint16BE();  /* always 200 (320x200 screen). */
			planes = file.readUint16BE(); /* always 5 (32 color). */

			debug("    \"%s\" pitch:%u bpp:%u size: %u bytes",
			      tag, pitch, planes,
			      SCREEN_WIDTH * planes * SCREEN_HEIGHT / 8);
		} break;
		case EX_BMP: {
			/* Unknown image format (Amiga). */
			tcount = file.readUint16BE();
			tw = new uint16[tcount];
			th = new uint16[tcount];

			uint32 packedSize, unpackedSize;
			unpackedSize = file.readUint32BE();
			debug("        [%u] %u =", tcount, unpackedSize);

			uint32 sz = 0;
			toffset = new uint32[tcount];
			for (uint16 k = 0; k < tcount; k++) {
				tw[k] = file.readUint16BE();
				th[k] = file.readUint16BE();
				debug("        %ux%u ~@%u", tw[k], th[k], sz);

				toffset[k] = sz;
				sz += uint(tw[k] + 15) / 16 * th[k] * 5;
			}
			debug("    ~= [%u]", sz);

			/* this is a wild guess. */
			byte version[13];
			file.read(version, 12);
			version[12] = '\0';
			debug("    %s", version);

			unpackedSize = file.readUint32BE();
			packedSize = file.readUint32BE();
			debug("        %u -> %u",
			      packedSize, unpackedSize);
		} break;
		case EX_INS: {
			/* AIFF sound sample (Amiga). */
			byte *dest = new byte[file.size()];
			file.read(dest, file.size());
			soundData = new Common::MemoryReadStream(dest, file.size(), DisposeAfterUse::YES);
		} break;
		case EX_SNG:
			/* IFF-SMUS music (Amiga). */
			break;
		case EX_AMG:
			/* (Amiga). */
			line = file.readLine();
			while (!file.eos() && !line.empty()) {
				debug("    \"%s\"", line.c_str());
				line = file.readLine();
			}
			break;
		case EX_VIN:
			line = file.readLine();
			while (!file.eos()) {
				if (!line.empty())
					debug("    \"%s\"", line.c_str());
				line = file.readLine();
			}
			break;
		default:
			break;
		}
		int leftover = file.size() - file.pos();
		file.hexdump(leftover);
		file.skip(leftover);
	} else {
		uint16 tcount;
		uint16 scount;
		uint16 *tw = 0, *th = 0;
		uint32 *toffset = 0;

		uint16 *mtx;
		uint16 mw, mh;

		scount = 0;

		DgdsChunk chunk;
		while (chunk.readHeader(ctx)) {
			if (chunk.container) {
				parent = chunk._id;
				continue;
			}

			Common::SeekableReadStream *stream;

			bool packed = chunk.isPacked(ex);
			stream = packed ? chunk.decodeStream(ctx, decompressor) : chunk.readStream(ctx);

			switch (ex) {
			case EX_TDS:
				/* Heart of China. */
				if (chunk.isSection(ID_THD)) {
					uint32 mark;

					mark = stream->readUint32LE();
					debug("    0x%X", mark);

					char version[7];
					stream->read(version, sizeof(version));
					debug("    \"%s\"", version);

					byte ch;
					Common::String bmpName;
					while ((ch = stream->readByte()))
						bmpName += ch;
					debug("    \"%s\"", bmpName.c_str());

					Common::String personName;
					while ((ch = stream->readByte()))
						personName += ch;
					debug("    \"%s\"", personName.c_str());
				}
				break;
			case EX_DDS:
				/* Heart of China. */
				if (chunk.isSection(ID_DDS)) {
					uint32 mark;

					mark = stream->readUint32LE();
					debug("    0x%X", mark);

					char version[7];
					stream->read(version, sizeof(version));
					debug("    \"%s\"", version);

					byte ch;
					Common::String tag;
					while ((ch = stream->readByte()))
						tag += ch;
					debug("    \"%s\"", tag.c_str());
				}
				break;
			case EX_SDS:
				if (chunk.isSection(ID_SDS)) {
					readSDS(stream);
				}
				break;
			case EX_TTM:
				if (chunk.isSection(ID_VER)) {
					char version[5];

					stream->read(version, sizeof(version));
					debug("        %s", version);
				} else if (chunk.isSection(ID_PAG)) {
					uint16 pages;
					pages = stream->readUint16LE();
					debug("        %u", pages);
				} else if (chunk.isSection(ID_TT3)) {
					if (resource == 0) {
						uint32 size = stream->size();
						byte *dest = new byte[size];
						stream->read(dest, size);
						ttm = new Common::MemoryReadStream(dest, size, DisposeAfterUse::YES);
						Common::strlcpy(ttmName, name, sizeof(ttmName));
					} else {
						while (!stream->eos()) {
							uint16 code;
							byte count;
							uint op;

							code = stream->readUint16LE();
							count = code & 0x000F;
							op = code & 0xFFF0;

							debugN("\tOP: 0x%4.4x %2u ", op, count);
							if (count == 0x0F) {
								Common::String sval;
								byte ch[2];

								do {
									ch[0] = stream->readByte();
									ch[1] = stream->readByte();
									sval += ch[0];
									sval += ch[1];
								} while (ch[0] != 0 && ch[1] != 0);

								debugN("\"%s\"", sval.c_str());
							} else {
								int ival;

								for (byte k = 0; k < count; k++) {
									ival = stream->readSint16LE();

									if (k == 0)
										debugN("%d", ival);
									else
										debugN(", %d", ival);
								}
							}
							debug(" ");
						}
					}
				} else if (chunk.isSection(ID_TAG)) {
					uint16 count;

					count = stream->readUint16LE();
					debug("        %u", count);
					// something fishy here. the first two entries sometimes are an empty string or non-text junk.
					// most of the time entries have text (sometimes with garbled characters).
					// this parser is likely not ok. but the NUL count seems to be ok.
					for (uint16 k = 0; k < count; k++) {
						byte ch;
						uint16 idx;
						Common::String str;

						idx = stream->readUint16LE();
						while ((ch = stream->readByte()))
							str += ch;
						debug("        %2u: %2u, \"%s\"", k, idx, str.c_str());
					}
				}
				break;
			case EX_GDS:
				if (chunk.isSection(ID_INF)) {
					//stream->hexdump(stream->size());
					uint32 mark;
					char version[7];

					mark = stream->readUint32LE();
					debug("    0x%X", mark);

					stream->read(version, sizeof(version));
					debug("    \"%s\"", version);

				} else if (chunk.isSection(ID_SDS)) {
					//stream->hexdump(stream->size());

					uint32 x;
					x = stream->readUint32LE();
					debug("    %u", x);

					while (!stream->eos()) {
						uint16 x2;
						do {
							do {
								x2 = stream->readUint16LE();
								debugN("        %u: %u|%u, ", x2, (x2 & 0xF), (x2 >> 4));
								if (stream->pos() >= stream->size())
									break;
							} while ((x2 & 0x80) != 0x80);
							debug("-");
							if (stream->pos() >= stream->size())
								break;
						} while ((x2 & 0xF0) != 0xF0);
					}
				}
				break;
			case EX_ADS:
			case EX_ADL:
			case EX_ADH:
				if (chunk.isSection(ID_VER)) {
					char version[5];

					stream->read(version, sizeof(version));
					debug("        %s", version);
				} else if (chunk.isSection(ID_RES)) {
					debug("res0");
					if (resource == 0) {
						readTags(stream);
					} else {
						readStrings(stream);
					}
				} else if (chunk.isSection(ID_SCR)) {
					if (resource == 0) {
						uint32 size = stream->size();
						byte *dest = new byte[size];
						stream->read(dest, size);
						ads = new Common::MemoryReadStream(dest, size, DisposeAfterUse::YES);
						Common::strlcpy(adsName, name, sizeof(adsName));
					} else {
						/* this is either a script, or a property sheet, i can't decide. */
						while (!stream->eos()) {
							uint16 code;
							code = stream->readUint16LE();
							if ((code & 0xFF00) == 0) {
								uint16 tag = (code & 0xFF);
								debug("          PUSH %d (0x%4.4X)", tag, tag); // ADS:TAG or TTM:TAG id.
							} else {
								const char *desc = "";
								switch (code) {
								case 0xF010:
								case 0xF200:
								case 0xFDA8:
								case 0xFE98:
								case 0xFF88:
								case 0xFF10:
									debug("          INT 0x%4.4X\t;", code);
									continue;

								case 0xFFFF:
									debug("          INT 0x%4.4X\t; return", code);
									debug("-");
									continue;

								case 0x0190:
								case 0x1070:
								case 0x1340:
								case 0x1360:
								case 0x1370:
								case 0x1420:
								case 0x1430:
								case 0x1500:
								case 0x1520:
								case 0x2000:
								case 0x2010:
								case 0x2020:
								case 0x3010:
								case 0x3020:
								case 0x30FF:
								case 0x4000:
								case 0x4010:
									desc = "?";
									break;

								case 0x1330:
									break;
								case 0x1350:
									desc = "? (res,rtag)";
									break;

								case 0x1510:
									desc = "? ()";
									break;
								case 0x2005:
									desc = "? (res,rtag,?,?)";
									break;

								default:
									break;
								}
								debug("          OP 0x%4.4X\t;%s", code, desc);
							}
						}
						assert(stream->size() == stream->pos());
						//stream->hexdump(stream->size());
					}
				} else if (chunk.isSection(ID_TAG)) {
					readStrings(stream);
				}
				break;
			case EX_REQ:
				if (parent == ID_TAG) {
					if (chunk.isSection(ID_REQ)) {
						readStrings(stream);
					} else if (chunk.isSection(ID_GAD)) {
						readStrings(stream);
					}
				} else if (parent == ID_REQ) {
					if (chunk.isSection(ID_REQ)) {
					} else if (chunk.isSection(ID_GAD)) {
					}
				}
				break;
			case EX_SNG:
				/* DOS. */
				if (chunk.isSection(ID_SNG)) {
					musicSize = stream->size();

					debug("        %2u: %u bytes", scount, musicSize);

					musicData = (uint8 *)malloc(musicSize);
					stream->read(musicData, musicSize);
					scount++;
				} else if (chunk.isSection(ID_INF)) {
					uint32 count;
					count = stream->size() / 2;
					debug("        [%u]", count);
					for (uint32 k = 0; k < count; k++) {
						uint16 idx;
						idx = stream->readUint16LE();
						debug("        %2u: %u", k, idx);
					}
				}
				break;
			case EX_SX:
				/* Macintosh. */
				if (chunk.isSection(ID_INF)) {
					uint16 type, count;

					type = stream->readUint16LE();
					count = stream->readUint16LE();

					debug("        %u [%u]:", type, count);
					for (uint16 k = 0; k < count; k++) {
						uint16 idx;
						idx = stream->readUint16LE();
						debug("        %2u: %u", k, idx);
					}
				} else if (chunk.isSection(ID_TAG)) {
					readStrings(stream);
				} else if (chunk.isSection(ID_FNM)) {
					readStrings(stream);
				} else if (chunk.isSection(ID_DAT)) {
					uint16 idx, type;
					byte compression;
					uint32 unpackSize;
					idx = stream->readUint16LE();
					type = stream->readUint16LE();
					compression = stream->readByte();
					unpackSize = stream->readUint32LE();
					//debug("        #%2u: (0x%X?) %s %u", idx, type, compressionDescr[compression], unpackSize);

					musicSize = unpackSize;
					debug("        %2u: %u bytes", scount, musicSize);

					musicData = (uint8 *)malloc(musicSize);
					decompressor->decompress(compression, musicData, musicSize, stream, stream->size() - stream->pos());

					scount++;
				}
				break;
			case EX_PAL:
				/* DOS & Macintosh. */
				// Handled in Image::setPalette
				error("Should not be here");
				break;
			case EX_FNT:
				if (resource == 0) {
					if (chunk.isSection(ID_FNT)) {
						byte magic = stream->readByte();
						stream->seek(-1, SEEK_CUR);
						debug("    magic: %u", magic);

						if (magic != 0xFF)
							_fntF = FFont::load(*stream);
						else
							_fntP = PFont::load(*stream, decompressor);
					}
				}
				break;
			case EX_SCR:
				// Handled in Image::loadScreen
				error("Should not be here");
				break;
			case EX_BMP:
				// Handled in Image::loadBitmap
				error("Should not be here");
				break;
			default:
				break;
			}
			int leftover = stream->size() - stream->pos();
			//stream->hexdump(leftover);
			stream->skip(leftover);

			delete stream;
		}
	}

	if (ex == EX_BMP) {
		BMPs.push_back(Common::String(name));
		debug("BMPs: %s", name);
	}

	debug("  [%u:%u] --", file.pos(), ctx.bytesRead);
}

void DgdsEngine::parseFile(Common::String filename, int resource) {
	//filename.toLowercase();

	if (filename.hasSuffix(".SNG") || filename.hasSuffix(".sng")) // TODO: Mac sound
		return;

	Common::SeekableReadStream *stream = _resource->getResource(filename);
	parseFileInner(_platform, *stream, filename.c_str(), resource, _decompressor);
	delete stream;
}

int delay = 0;

struct Channel {
	Audio::AudioStream *stream;
	Audio::SoundHandle handle;
	byte volume;
};

struct Channel _channels[2];

void DgdsEngine::playSfx(const char *fileName, byte channel, byte volume) {
	parseFile(fileName);
	if (soundData) {
		Channel *ch = &_channels[channel];
		Audio::AudioStream *input = Audio::makeAIFFStream(soundData, DisposeAfterUse::YES);
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &ch->handle, input, -1, volume);
		soundData = 0;
	}
}

void DgdsEngine::stopSfx(byte channel) {
	if (_mixer->isSoundHandleActive(_channels[channel].handle)) {
		_mixer->stopHandle(_channels[channel].handle);
		_channels[channel].stream = 0;
	}
}

bool DgdsEngine::playPCM(byte *data, uint32 size) {
	_mixer->stopAll();

	if (!data)
		return false;

	byte numParts;
	byte *trackPtr[0xFF];
	uint16 trackSiz[0xFF];
	numParts = loadSndTrack(DIGITAL_PCM, trackPtr, trackSiz, data, size);
	if (numParts == 0)
		return false;

	for (byte part = 0; part < numParts; part++) {
		byte *ptr = trackPtr[part];

		bool digital_pcm = false;
		if (READ_LE_UINT16(ptr) == 0x00FE) {
			digital_pcm = true;
		}
		ptr += 2;

		if (!digital_pcm)
			continue;

		uint16 rate, length, first, last;
		rate = READ_LE_UINT16(ptr);

		length = READ_LE_UINT16(ptr + 2);
		first = READ_LE_UINT16(ptr + 4);
		last = READ_LE_UINT16(ptr + 6);
		ptr += 8;

		ptr += first;
		debug(" - Digital PCM: %u Hz, [%u]=%u:%u",
		      rate, length, first, last);
		trackPtr[part] = ptr;
		trackSiz[part] = length;

		Channel *ch = &_channels[part];
		byte volume = 255;
		Audio::AudioStream *input = Audio::makeRawStream(trackPtr[part], trackSiz[part],
		                                                 rate, Audio::FLAG_UNSIGNED, DisposeAfterUse::NO);
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &ch->handle, input, -1, volume);
	}
	return true;
}

void DgdsEngine::playMusic(const char *fileName) {
	//stopMusic();

	parseFile(fileName);
	if (musicData) {
		uint32 tracks;
		tracks = availableSndTracks(musicData, musicSize);
		if ((tracks & TRACK_MT32))
			_midiPlayer->play(musicData, musicSize);
		if ((tracks & DIGITAL_PCM))
			playPCM(musicData, musicSize);
	}
}

Common::Error DgdsEngine::run() {
	initGraphics(SCREEN_WIDTH, SCREEN_HEIGHT);

	soundData = 0;
	musicData = 0;

	_console = new Console(this);
	_resource = new ResourceManager();
	_decompressor = new Decompressor();
	_image = new Image(_resource, _decompressor);
	_midiPlayer = new DgdsMidiPlayer();
	assert(_midiPlayer);

	setDebugger(_console);

	bottomBuffer.create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());
	topBuffer.create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());
	resData.create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());

	debug("DgdsEngine::init");

	g_system->fillScreen(0);

	Common::EventManager *eventMan = g_system->getEventManager();
	Common::Event ev;

	ADSInterpreter interpADS(this);
	TTMInterpreter interpTTM(this);

	TTMState title1State, title2State;
	ADSState introState;

	if (getGameId() == GID_DRAGON) {
		TTMData title1Data, title2Data;
		ADSData introData;
		interpTTM.load("TITLE1.TTM", &title1Data);
		interpTTM.load("TITLE2.TTM", &title2Data);
		interpADS.load("INTRO.ADS", &introData);

		interpTTM.init(&title1State, &title1Data);
		interpTTM.init(&title2State, &title2Data);
		interpADS.init(&introState, &introData);

		parseFile("DRAGON.FNT");
		parseFile("S55.SDS");
	} else if (getGameId() == GID_CHINA) {
		ADSData introData;
		interpADS.load("TITLE.ADS", &introData);

		interpADS.init(&introState, &introData);

		parseFile("HOC.FNT");
	} else if (getGameId() == GID_BEAMISH) {
		ADSData introData;
		interpADS.load("TITLE.ADS", &introData);

		interpADS.init(&introState, &introData);

		//parseFile("HOC.FNT");
	}

	_console->attach();

	while (!shouldQuit()) {
		if (eventMan->pollEvent(ev)) {
			if (ev.type == Common::EVENT_KEYDOWN) {
				switch (ev.kbd.keycode) {
					/*
					case Common::KEYCODE_TAB:	sid++;					break;
					case Common::KEYCODE_UP:	if (id > 0) id--; bk=0;			break;
					case Common::KEYCODE_DOWN:	if (id < BMPs.size()) id++; bk=0;	break;
					case Common::KEYCODE_LEFT:	if (bk > 0) bk--;			break;
					case Common::KEYCODE_RIGHT:	if (bk < (_tcount-1)) bk++;		break;
					*/
				case Common::KEYCODE_ESCAPE:
					return Common::kNoError;
				default:
					break;
				}
			}
		}

		//		browse(_platform, _rmfName, this);

		if (getGameId() == GID_DRAGON) {
			if (!interpTTM.run(&title1State))
				if (!interpTTM.run(&title2State))
					if (!interpADS.run(&introState))
						return Common::kNoError;
		} else if (getGameId() == GID_CHINA || getGameId() == GID_BEAMISH) {
			if (!interpADS.run(&introState))
				return Common::kNoError;
		}

		g_system->delayMillis(40);
	}
	return Common::kNoError;
}

} // End of namespace Dgds
