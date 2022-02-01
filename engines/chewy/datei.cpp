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

#include "common/debug.h"
#include "common/system.h"
#include "chewy/datei.h"
#include "chewy/chewy.h"
#include "chewy/defines.h"
#include "chewy/file.h"

namespace Chewy {

#define GRAFIK 1
#define SPEICHER 2
#define DATEI 3
#define MAUS 4
#define DIALOG 5

#undef MAXPATH
#define MAXPATH 1000

char filename[MAXPATH];
uint8 tmp[10000]; // FIXME

extern int16 modul;
extern int16 fcode;

datei::datei() {
}

datei::~datei() {
}

void datei::assign_filename(const char *fname, const char *ext) {
	strncpy(filename, fname, MAXPATH - 5);
	filename[MAXPATH - 5] = '\0';
	if (!strchr(filename, '.'))
		strcat(filename, ext);
}

uint16 datei::select_pool_item(Stream *stream, uint16 nr) {
	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(stream);
	NewPhead *ph = (NewPhead *)tmp;

	if (rs) {
		rs->seek(0, SEEK_SET);
		if (!ph->load(rs)) {
			error("select_pool_item error");
		} else {
			if (!strncmp(ph->id, "NGS", 3)) {
				if (nr >= ph->PoolAnz)
					nr = ph->PoolAnz - 1;

				rs->seek(-(int)((ph->PoolAnz - nr) * sizeof(uint32)), SEEK_END);
				uint32 tmp1 = rs->readUint32LE();
				rs->seek(tmp1, SEEK_SET);
			}
		}
	}

	return nr;
}

void datei::load_tafmcga(const char *fname, byte *sp, int16 nr) {
	taf_imageheader iheader;
	byte *speicher = sp;
	taf_dateiheader *header = (taf_dateiheader *)tmp;

	assign_filename(fname, ".taf");

	int16 *abmess = (int16 *)speicher;
	speicher += 4;
	if (abmess) {
		Common::File f;
		if (f.open(filename)) {
			if (header->load(&f)) {
				int16 id = get_id(header->id);
				if (id == TAFDATEI) {
					uint32 image = 0;
					if (header->korrekt == 1) {
						uint32 next = header->next;
						uint16 sprcount = 0;
						while ((sprcount <= nr) && (nr <= header->count)) {
							f.seek(next, SEEK_SET);
							if (iheader.load(&f)) {
								next = iheader.next;
								image = iheader.image;
							} else {
								error("load_tafmcga error");
							}
							++sprcount;
						}
					} else {
						f.seek((-(int)((header->count - nr) * sizeof(uint32))), SEEK_END);
						uint32 next = f.readUint32LE();
						f.seek(next, SEEK_SET);

						if (iheader.load(&f)) {
							next = iheader.next;
							image = iheader.image;
						} else {
							error("load_tafmcga error");
						}
					}

					abmess[0] = iheader.width;
					abmess[1] = iheader.height;
					const uint32 size = (uint32)(iheader.height * iheader.width);
					int16 komp = iheader.komp;
					f.seek(image, SEEK_SET);
					read_tbf_image(&f, komp, size, speicher);

				} else {
					error("load_tafmcga error");
				}
			} else {
				error("load_tafmcga error");
			}

			f.close();
		} else {
			error("load_tafmcga error");
		}
	} else {
		error("load_tafmcga error");
	}
}

void datei::load_tafmcga(Stream *handle, int16 komp, uint32 size, byte *speicher) {
	read_tbf_image(handle, komp, size, speicher);
}

void datei::load_full_taf(const char *fname, byte *hi_sp, taf_info *tinfo) {
	taf_dateiheader *header = (taf_dateiheader *)tmp;

	assign_filename(fname, ".taf");

	byte *speicher = hi_sp;
	if (speicher) {
		Common::File f;
		if (f.open(filename)) {
			if (header->load(&f)) {
				int16 id = get_id(header->id);
				if ((id == TAFDATEI) && (header->mode == 19)) {
					uint32 next = header->next;
					uint16 anzahl = header->count;
					uint32 image = 0;
					for (uint16 sprcount = 0; (sprcount < anzahl) && (!modul); sprcount++) {
						tinfo->image[sprcount] = speicher;
						f.seek(next, SEEK_SET);
						taf_imageheader iheader;
						if (iheader.load(&f)) {
							next = iheader.next;
							image = iheader.image;
						} else {
							error("load_full_taf error");
						}
						uint16 *abmess = (uint16 *)speicher;
						abmess[0] = iheader.width;
						abmess[1] = iheader.height;
						speicher += 4;
						uint32 size = (uint32)((uint32)iheader.height) * ((uint32)iheader.width);
						int16 komp = iheader.komp;

						f.seek(image, SEEK_SET);
						read_tbf_image(&f, komp, size, speicher);
						speicher += size;
					}
				} else {
					error("load_full_taf error");
				}
			} else {
				error("load_full_taf error");
			}

			f.close();
		} else {
			error("load_full_taf error");
		}
	} else {
		error("load_full_taf error");
	}
}

void datei::load_korrektur(const char *fname, int16 *sp) {
	taf_dateiheader *header = (taf_dateiheader *)tmp;
	assign_filename(fname, ".taf");

	if (sp) {
		Common::File f;
		if (f.open(filename)) {
			if (header->load(&f)) {
				int16 id = get_id(header->id);
				if ((id == TAFDATEI) && (header->korrekt > 0)) {
					f.seek(-((int)(header->count * sizeof(int16) * 2) * header->korrekt), SEEK_END);
					if ((f.size() - f.pos() / 2) < (int16)(header->count * sizeof(int16) * 2)) {
						error("load_korrektur error");
					} else {
						for (int i = 0; i < header->count * 2; ++i)
							*sp++ = f.readSint16LE();
					}
				} else {
					error("load_korrektur error");
				}
			} else {
				error("load_korrektur error");
			}

			f.close();
		} else {
			error("load_korrektur error");
		}
	} else {
		error("load_korrektur error");
	}
}

void datei::load_tff(const char *fname, byte *speicher) {
	strncpy(filename, fname, MAXPATH - 5);
	filename[MAXPATH - 5] = '\0';

	if (speicher) {
		if (!strchr(filename, '.'))
			strcat(filename, ".tff");

		Common::File f;
		if (f.open(filename)) {
			tff_header *tff = (tff_header *)speicher;
			if (tff->load(&f)) {
				uint32 size = tff->size;
				if (f.read(speicher + sizeof(tff_header), size) != size) {
					error("load_tff error");
				}
			}

			f.close();
		} else {
			error("load_tff error");
		}
	} else {
		error("load_tff error");
	}
}

void datei::read_tbf_image(Stream *stream, int16 komp, uint32 size, byte *sp) {
	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(stream);
	assert(rs);

	byte *speicher = sp;

	if (komp == 1) {
		// Run length encoding using count/value pairs
		for (uint32 pos = 0; pos < size;) {
			uint8 count = rs->readByte();
			uint8 value = rs->readByte();

			for (uint8 i = 0; (i < count) && (pos < size); i++) {
				speicher[pos] = value;
				++pos;
			}
		}
	} else {
		rs->read(speicher, size);
		speicher += size;
	}
}

void datei::void_load(const char *fname, byte *speicher, uint32 size) {
	strncpy(filename, fname, MAXPATH - 1);
	filename[MAXPATH - 1] = '\0';

	Common::File f;
	if (f.open(filename)) {
		if (!f.read(speicher, size)) {
			error("void_load error");
		}

		f.close();
	} else {
		error("void_load error");
	}
}

uint32 datei::load_tmf(Stream *handle, tmf_header *song) {
	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(handle);
	ChunkHead *ch = (ChunkHead *)tmp;
	uint32 size = 0;

	if (rs) {
		rs->seek(-ChunkHead::SIZE(), SEEK_CUR);
		if (!ch->load(rs)) {
			error("load_tmf error");
		} else {
			if (ch->type == TMFDATEI) {
				assert(ch->size > (uint32)tmf_header::SIZE());

				if (!song->load(rs)) {
					error("load_tmf error");
				} else {
					size = ch->size + sizeof(tmf_header);
					byte *speicher = (byte *)song + sizeof(tmf_header);
					speicher += ((uint32)song->pattern_anz) * 1024l;
					for (int16 i = 0; i < 31; ++i) {
						if (song->instrument[i].laenge) {
							song->ipos[i] = speicher;
							speicher += song->instrument[i].laenge;
						}
					}
				}
			} else {
				error("load_tmf error");
			}
		}
	}

	return size;
}

uint32 datei::size(const char *fname, int16 typ) {
	uint32 size = 0;
	tbf_dateiheader *tbfheader = (tbf_dateiheader *)tmp;
	pcx_header *pcxheader = (pcx_header *)tmp;
	mod_header *mh = (mod_header *)tmp;

	strncpy(filename, fname, MAXPATH - 5);
	filename[MAXPATH - 5] = '\0';

	if (!strchr(filename, '.')) {
		switch (typ) {
		case TBFDATEI:
		case TPFDATEI:
			strcat(filename, ".tbf");
			break;
		case PCXDATEI:
			strcat(filename, ".pcx");
			break;
		case TFFDATEI:
			strcat(filename, ".tff");
			break;
		case VOCDATEI:
			strcat(filename, ".voc");
			break;
		case MODDATEI:
			strcat(filename, ".mod");
			break;
		case TMFDATEI:
			strcat(filename, ".tmf");
			typ = 300;
			break;
		default:
			break;
		}
	}

	// SCUMMVM: Note to self, use sizeof(structures) for
	// allocating size, not custom ::SIZE() functions
	Common::File f;
	if (f.open(filename)) {
		switch (typ) {
		case TBFDATEI:
		case TPFDATEI:
			if (chewy_fread(tbfheader, sizeof(tbf_dateiheader), 1, &f)) {
				int16 id = get_id(tbfheader->id);
				if ((id == TBFDATEI) || (id == TPFDATEI)) {
					size = tbfheader->entpsize + 4;
				} else {
					error("size error");
				}
			} else {
				error("size error");
			}
			break;

		case PCXDATEI:
			if (chewy_fread(pcxheader, sizeof(pcx_header), 1, &f)) {
				if ((pcxheader->id == 10) && (pcxheader->version == 5)
				        && (pcxheader->bpp == 8)) {
					uint16 hoehe = (pcxheader->ymax - pcxheader->ymin) + 1;
					uint16 breite = pcxheader->bpz * pcxheader->planes;
					size = (uint32)((long)hoehe) * ((long)breite) + 4;
				} else {
					error("size error");
				}
			} else {
				error("size error");
			}
			break;

		case TFFDATEI: {
			tff_header tff;
			if (tff.load(&f)) {
				int16 id = get_id(tff.id);
				if (id == TFFDATEI) {
					size = tff.size + sizeof(tff_header);
				} else {
					error("size error");
				}
			} else {
				error("size error");
			}
			}
			break;

		case VOCDATEI:
			size = (uint32)f.size() - sizeof(voc_header);
			break;

		case MODDATEI: {
			size = (uint32)f.size();
			int16 id = 0;

			if (chewy_fread(mh, sizeof(mod_header), 1, &f)) {
				if (!strncmp(mh->id, "M.K.", 4))
					id = 1;
				if (!strncmp(mh->id, "M!K!", 4))
					id = 2;
				if (!strncmp(mh->id, "FLT4", 4))
					id = 3;
				if (!strncmp(mh->id, "FLT8", 4))
					id = 4;
			}
			if (id)
				size -= (uint32)sizeof(mod_header);
			else
				size -= (uint32)sizeof(mod15_header);
			size += sizeof(tmf_header);
			}
			break;

		default:
			size = (uint32)f.size();
			break;
		}

		f.close();
	} else {
		error("size error");
		size = 0;
	}

	return size;
}

uint32 datei::get_poolsize(const char *fname, int16 chunk_start, int16 chunk_anz) {
	NewPhead *Nph = (NewPhead *)tmp;
	uint32 size = 0;

	Common::File f;
	if (f.open(fname)) {
		if (!Nph->load(&f)) {
			error("get_poolsize error");
		} else {
			if (!strncmp(Nph->id, "NGS", 3)) {
				select_pool_item(&f, chunk_start);
				f.seek(-ChunkHead::SIZE(), SEEK_CUR);

				for (int16 i = chunk_start; (i < Nph->PoolAnz) && (!modul) && i < (chunk_start + chunk_anz); i++) {
					ChunkHead ch;
					if (!ch.load(&f)) {
						error("get_poolsize error");
					} else {
						if (ch.size > size)
							size = ch.size;

						f.seek(ch.size, SEEK_CUR);
					}
				}
			}
		}

		f.close();
	} else {
		error("get_poolsize error");
	}

	return size;
}

uint32 datei::get_tafinfo(const char *fname, taf_dateiheader **tafheader) {
	uint32 size = 0;
	taf_dateiheader *tdh = (taf_dateiheader *)tmp;
	*tafheader = tdh;

	assign_filename(fname, ".taf");

	Common::File tafFile;
	if (tafFile.open(filename)) {
		if (tdh->load(&tafFile)) {
			int16 id = get_id(tdh->id);
			if ((id == TAFDATEI) && (tdh->mode == 19)) {
				size = tdh->allsize + (((uint32)tdh->count) * 8l);

				size += ((uint32)sizeof(taf_info));
			} else {
				error("get_tafinfo error");
			}
		} else {
			error("get_tafinfo error");
		}

		tafFile.close();
	} else {
		error("get_tafinfo error");
	}

	return size;
}

void datei::load_palette(const char *fname, byte *palette, int16 typ) {
	tbf_dateiheader *tbfheader = (tbf_dateiheader *)tmp;
	pcx_header *pcxheader = (pcx_header *)tmp;
	taf_dateiheader *tafheader = (taf_dateiheader *)tmp;

	strncpy(filename, fname, MAXPATH - 5);
	filename[MAXPATH - 5] = '\0';

	Common::File f;
	if (!strchr(filename, '.')) {
		switch (typ) {
		case TBFDATEI:
		case TPFDATEI:
			strcat(filename, ".tbf");
			break;
		case PCXDATEI:
			strcat(filename, ".pcx");
			break;
		case TAFDATEI:
			strcat(filename, ".taf");
			break;
		default:
			error("load_palette error");
		}
	}

	if (!modul) {
		if (f.open(filename)) {
			switch (typ) {
			case TBFDATEI:
			case TPFDATEI:
				if (chewy_fread(tbfheader, sizeof(tbf_dateiheader), 1, &f)) {
					int16 id = get_id(tbfheader->id);
					if (((id == TBFDATEI) || (id == TPFDATEI)) &&
					        (tbfheader->mode == 19)) {
						for (uint16 i = 0; i < 768; i++)
							palette[i] = tbfheader->palette[i];
					} else {
						error("load_palette error");
					}
				} else {
					error("load_palette error");
				}
				break;

			case PCXDATEI:
				if (chewy_fread(pcxheader, sizeof(pcx_header), 1, &f)) {
					if ((pcxheader->id == 10) && (pcxheader->version == 5)
					        && (pcxheader->bpp == 8)) {
						if (!f.seek(-769L, SEEK_END)) {
							uint8 zeichen = f.readByte();
							if (zeichen == 12) {
								f.seek(-768L, SEEK_END);
								if ((chewy_fread(palette, 768, 1, &f)) != 1) {
									error("load_palette error");
								} else {
									uint16 j = 0;
									for (uint16 i = 0; i < 255; i++) {
										palette[j] >>= 2;
										palette[j + 1] >>= 2;
										palette[j + 2] >>= 2;
										j += 3;
									}
								}
							} else {
								error("load_palette error");
							}
						}
					} else {
						error("load_palette error");
					}
				} else {
					error("load_palette error");
				}
				break;

			case TAFDATEI:
				if (tafheader->load(&f)) {
					int16 id = get_id(tafheader->id);
					if ((id == TAFDATEI) && (tafheader->mode == 19)) {
						for (uint16 i = 0; i < 768; i++)
							palette[i] = tafheader->palette[i];
					} else {
						error("load_palette error");
					}
				} else {
					error("load_palette error");
				}
				break;

			default:
				error("load_palette error");
			}

			f.close();
		} else {
			error("load_palette error");
		}
	}
}

void datei::imsize(const char *fname, uint32 *svekt) {
	taf_dateiheader *header = (taf_dateiheader *)tmp;

	assign_filename(fname, ".taf");

	Common::File f;
	if (f.open(filename)) {
		if (header->load(&f)) {
			int16 id = get_id(header->id);
			if ((id == TAFDATEI) && (header->mode == 19)) {
				uint32 next = header->next;
				uint16 sprcount = 0;
				while (sprcount < header->count) {
					f.seek(next, SEEK_SET);
					taf_imageheader iheader;
					if (iheader.load(&f)) {
						next = iheader.next;
						svekt[sprcount] = (uint32)iheader.width * (uint32)iheader.height;
					} else {
						error("imsize error");
					}
					++sprcount;
				}
			}
		}

		f.close();
	} else {
		error("imsize error");
	}
}

int16 datei::get_id(char *id_code) {
	int16 id = -1;
	if (!(scumm_strnicmp(id_code, "TBF", 3)))
		id = TBFDATEI;
	if (!(scumm_strnicmp(id_code, "TPF", 3)))
		id = TPFDATEI;
	if (!(scumm_strnicmp(id_code, "TAF", 3)))
		id = TAFDATEI;
	if (!(scumm_strnicmp(id_code, "TFF", 3)))
		id = TFFDATEI;

	return id;
}

void datei::fcopy(const char *d_fname, const char *s_fname) {
	assert(!strcmp(d_fname, ADSH_TMP));

	Common::File f;
	if (f.open(s_fname)) {
		Common::SeekableWriteStream *ws = g_engine->_tempFiles.createWriteStreamForMember(ADSH_TMP);
		ws->writeStream(&f);
		delete ws;
	} else {
		error("Could not find - %s", s_fname);
	}
}

} // namespace Chewy
