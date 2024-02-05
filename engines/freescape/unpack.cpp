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

/*

This code was modified from https://github.com/w4kfu/unEXEPACK/blob/master/unpack.c

*/

#include "common/file.h"
#include "common/memstream.h"

#include "freescape/freescape.h"

namespace Freescape {

#define DOS_SIGNATURE 0x5A4D
#define EXEPACK_SIGNATURE 0x4252

struct memstream {
	unsigned char *buf;
	unsigned int length;
	unsigned int pos;
};

struct dos_header {
	unsigned short e_magic;
	unsigned short e_cblp;
	unsigned short e_cp;
	unsigned short e_crlc;
	unsigned short e_cparhdr;
	unsigned short e_minalloc;
	unsigned short e_maxalloc;
	unsigned short e_ss;
	unsigned short e_sp;
	unsigned short e_csum;
	unsigned short e_ip;
	unsigned short e_cs;
	unsigned short e_lfarlc;
	unsigned short e_ovno;
};

struct exepack_header {
	unsigned short real_ip;
	unsigned short real_cs;
	unsigned short mem_start;
	unsigned short exepack_size;
	unsigned short real_sp;
	unsigned short real_ss;
	unsigned short dest_len;
	unsigned short skip_len;
	unsigned short signature;
};

void reverse(unsigned char *s, size_t length);
void unpack_data(unsigned char *unpacked_data, unsigned char *buf, unsigned int *unpacked_data_size, unsigned int packed_data_len);
Common::MemoryReadStream *unpack(struct memstream *ms);
unsigned char *create_reloc_table(struct memstream *ms, struct dos_header *dh, struct exepack_header *eh, unsigned int *reloc_table_size);
Common::MemoryReadStream *writeExe(struct dos_header *dh, unsigned char *unpacked_data, unsigned int unpacked_data_size, unsigned char *reloc, size_t reloc_size, size_t padding);
Common::MemoryReadStream *craftexec(struct dos_header *dh, struct exepack_header *eh, unsigned char *unpacked_data, unsigned int unpacked_data_size, unsigned char *reloc, unsigned int reloc_size);

// Utils
int test_dos_header(struct memstream *ms);
void msopen(Common::File &file, struct memstream *ms);
unsigned int msread(struct memstream *ms, void *buf, unsigned int count);
int mscanread(struct memstream *ms, unsigned int count);
unsigned int msgetavailable(struct memstream *ms);
void msseek(struct memstream *ms, unsigned int offset);
void msclose(struct memstream *ms);
void *memmem(void *l, size_t l_len, const void *s, size_t s_len);

void reverse(unsigned char *s, size_t length) {
	size_t i, j;
	unsigned char c;

	if (length == 0x00) {
		return;
	}
	for (i = 0, j = length - 1; i < j; i++, j--) {
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}

/* buf is already reversed, because EXEPACK use backward processing */
void unpack_data(unsigned char *unpacked_data, unsigned char *buf, unsigned int *unpacked_data_size, unsigned int packed_data_len) {
	unsigned char opcode;
	unsigned short count;
	unsigned char fillbyte;
	unsigned char *save_buf = NULL;
	unsigned char *save_unp = NULL;
	unsigned int cur_unpacked_data_size = 0x00;

	save_buf = buf;
	save_unp = unpacked_data;
	while (*buf == 0xFF) {
		buf++;
	}
	while (1) {
		opcode = *buf++;
		count = *(buf) * 0x100 + *(buf + 1);
		buf += 2;
		if ((opcode & 0xFE) == 0xB0) {
			fillbyte = *buf++;
			if ((cur_unpacked_data_size + count) > *unpacked_data_size) {
				debug("overflow");
			}
			memset(unpacked_data, fillbyte, count);
			unpacked_data += count;
			cur_unpacked_data_size += count;
		}
		else if ((opcode & 0xFE) == 0xB2) {
			if ((cur_unpacked_data_size + count) > *unpacked_data_size) {
				debug("overflow");
			}
			memcpy(unpacked_data, buf, count);
			unpacked_data += count;
			cur_unpacked_data_size += count;
			buf += count;
		}
		else {
			debug("unknown opcode");
		}
		if ((opcode & 1) == 1) {
			break;
		}
		if (buf - save_buf >= packed_data_len) {
			break;
		}
	}
	if (buf - save_buf < packed_data_len) {
		if ((packed_data_len - (buf - save_buf)) > (*unpacked_data_size - (unpacked_data - save_unp))) {
			debug("Data left are too large!");
		}
		memcpy(unpacked_data, buf, packed_data_len - (buf - save_buf));
		cur_unpacked_data_size += packed_data_len - (buf - save_buf);
	}
	*unpacked_data_size = cur_unpacked_data_size;
}

unsigned char *create_reloc_table(struct memstream *ms, struct dos_header *dh, struct exepack_header *eh, unsigned int *reloc_table_size) {
	unsigned int exepack_offset = 0x00;
	unsigned int reloc_length;
	int nb_reloc;
	unsigned char *buf_reloc = NULL;
	unsigned char *reloc = NULL;
	int i, j;
	unsigned short count = 0x00;
	unsigned short entry;
	unsigned int reloc_position = 0x00;

	exepack_offset = (dh->e_cparhdr + dh->e_cs) * 16;
	msseek(ms, exepack_offset);
	reloc = (unsigned char*)memmem(ms->buf + exepack_offset, msgetavailable(ms), "Packed file is corrupt", strlen("Packed file is corrupt"));
	if (!reloc) {
		debug("Cannot find string \"Packed file is corrupt\", is it really EXEPACK ?");
	}

	reloc_length = (unsigned int)(eh->exepack_size - ((reloc - (ms->buf + exepack_offset)) & 0xFFFFFFFF) + strlen("Packed file is corrupt"));
	nb_reloc = (reloc_length - 16 * sizeof (unsigned short)) / 2;
	*reloc_table_size = nb_reloc * 2 * sizeof(unsigned short);
	buf_reloc = (unsigned char*)malloc(sizeof (char) * *reloc_table_size);
	assert(buf_reloc);
	reloc += strlen("Packed file is corrupt");
	msseek(ms, (reloc - ms->buf) & 0xFFFFFFFF);
	for (i = 0; i < 16; i++) {
		if (msread(ms, &count, sizeof (unsigned short)) != sizeof (unsigned short)) {
			debug("msread failed");
		}
		for (j = 0; j < count; j++) {
			if (msread(ms, &entry, sizeof (unsigned short)) != sizeof (unsigned short)) {
				debug("msread failed");
			}
			if (reloc_position >= *reloc_table_size) {
				debug("overflow");
			}
			*(unsigned short*)(buf_reloc + reloc_position) = entry;
			reloc_position += 2;
			if (reloc_position >= *reloc_table_size) {
				debug("overflow");
			}
			*(unsigned short*)(buf_reloc + reloc_position) = (i * 0x1000) & 0xFFFF;
			reloc_position += 2;
		}
	}
	*reloc_table_size = reloc_position;
	return buf_reloc;
}

Common::MemoryReadStream *writeExe(struct dos_header *dh, unsigned char *unpacked_data, unsigned int unpacked_data_size, unsigned char *reloc, size_t reloc_size, size_t padding) {
	Common::MemoryWriteStreamDynamic buf(DisposeAfterUse::NO);

	buf.write(dh, sizeof (struct dos_header));
	buf.write(reloc, reloc_size);
	for (size_t i = 0; i < padding; i++) {
		buf.write("\x00", 1);
	}
	buf.write(unpacked_data, unpacked_data_size);
	return (new Common::MemoryReadStream(buf.getData(), buf.size()));
}

Common::MemoryReadStream *craftexec(struct dos_header *dh, struct exepack_header *eh, unsigned char *unpacked_data, unsigned int unpacked_data_size, unsigned char *reloc, unsigned int reloc_size) {
	struct dos_header dhead;
	int header_size;
	int total_length;
	int padding_length;

	memset(&dhead, 0, sizeof (struct dos_header));
	header_size = sizeof (struct dos_header) + reloc_size;
	dhead.e_magic = DOS_SIGNATURE;
	dhead.e_cparhdr = (header_size / 16) & 0xFFFF;
	dhead.e_cparhdr = (dhead.e_cparhdr / 32 + 1) * 32;
	padding_length = dhead.e_cparhdr * 16 - header_size;
	total_length = header_size + padding_length + unpacked_data_size;
	dhead.e_ss = eh->real_ss;
	dhead.e_sp = eh->real_sp;
	dhead.e_ip = eh->real_ip;
	dhead.e_cs = eh->real_cs;
	dhead.e_minalloc = dh->e_minalloc;
	dhead.e_maxalloc = 0xFFFF;
	dhead.e_lfarlc = sizeof (struct dos_header);
	dhead.e_crlc = (reloc_size / (2 * sizeof (unsigned short))) & 0xFFFF;
	dhead.e_cblp = total_length % 512;
	dhead.e_cp = (total_length / 512 + 1) & 0xFFFF;
	//print_dos_header(&dhead);
	return writeExe(&dhead, unpacked_data, unpacked_data_size, reloc, reloc_size, padding_length);
}

Common::MemoryReadStream *unpack(struct memstream *ms) {
	struct dos_header dh;
	struct exepack_header eh;
	unsigned int exepack_offset = 0x00;
	unsigned char *unpacked_data = NULL;
	unsigned int unpacked_data_size = 0x00;
	unsigned int packed_data_start;
	unsigned int packed_data_end;
	unsigned int packed_data_len;
	unsigned int reloc_size;
	unsigned char *reloc = NULL;

	if (msread(ms, &dh, sizeof (struct dos_header)) != sizeof (struct dos_header)) {
		return nullptr;
	}
	//print_dos_header(&dh);
	exepack_offset = (dh.e_cparhdr + dh.e_cs) * 16;
	msseek(ms, exepack_offset);
	if (msread(ms, &eh, sizeof (struct exepack_header)) != sizeof (struct exepack_header)) {
		return nullptr;
	}
	//print_exepack_header(&eh);
	if ((eh.signature != EXEPACK_SIGNATURE && eh.skip_len != EXEPACK_SIGNATURE) || eh.exepack_size == 0x00) {
		debug("This is not a valid EXEPACK executable");
		return nullptr;
	}
	debug("Header exepack = %X\n", exepack_offset);
	//print_exepack_header(&eh);
	unpacked_data_size = eh.dest_len * 16;
	unpacked_data = (unsigned char*)malloc(sizeof (char) * unpacked_data_size);
	assert(unpacked_data);
	memset(unpacked_data, 0x00, sizeof (char) * unpacked_data_size);
	packed_data_start = dh.e_cparhdr * 16;
	packed_data_end = exepack_offset;
	packed_data_len = packed_data_end - packed_data_start;
	msseek(ms, packed_data_start);
	if (mscanread(ms, packed_data_len) == 0x00) {
		free(unpacked_data);
		return nullptr;
	}
	reverse(ms->buf + packed_data_start, packed_data_len);
	unpack_data(unpacked_data, ms->buf + packed_data_start, &unpacked_data_size, packed_data_len);
	reverse(unpacked_data, unpacked_data_size);
	reloc = create_reloc_table(ms, &dh, &eh, &reloc_size);
	Common::MemoryReadStream *ret = craftexec(&dh, &eh, unpacked_data, unpacked_data_size, reloc, reloc_size);
	free(unpacked_data);
	return ret;
}

void *memmem(void *l, size_t l_len, const void *s, size_t s_len) {
	char *cur, *last;
	char *cl = (char *)l;
	const char *cs = (const char *)s;

	if (l_len == 0 || s_len == 0) {
		return NULL;
	}
	if (l_len < s_len) {
		return NULL;
	}
	if (s_len == 1) {
		return (void *)memchr(l, (int)*cs, l_len);
	}
	last = cl + l_len - s_len;
	for (cur = (char *)cl; cur <= last; cur++) {
		if (cur[0] == cs[0] && memcmp(cur, cs, s_len) == 0) {
			return cur;
		}
	}
	return NULL;
}

void msopen(Common::File &file, struct memstream *ms) {
	assert(ms);

	ms->buf = (unsigned char*)malloc(sizeof (char) * file.size());
	assert(ms->buf);

	assert(file.read(ms->buf, file.size()) == file.size());
	ms->pos = 0x00;
	ms->length = file.size();
}

unsigned int msread(struct memstream *ms, void *buf, unsigned int count) {
	unsigned int length;

	if (buf == NULL) {
		return 0;
	}
	if (ms->pos > ms->length) {
		debug("invalid read");
	}
	if (count < (ms->length - ms->pos)) {
		length = count;
	}
	else {
		length = ms->length - ms->pos;
	}
	if (length > 0) {
		memcpy(buf, ms->buf + ms->pos, length);
	}
	ms->pos += length;
	return length;
}

int mscanread(struct memstream *ms, unsigned int count) {
	if (ms->pos > ms->length) {
		return 0;
	}
	if (count > (ms->length - ms->pos)) {
		return 0;
	}
	return 1;
}

unsigned int msgetavailable(struct memstream *ms) {
	if (ms->pos > ms->length) {
		return 0;
	}
	return ms->length - ms->pos;
}

void msseek(struct memstream *ms, unsigned int offset) {
	if (offset > ms->length) {
		debug("invalid seek : 0x%X", offset);
	}
	ms->pos = offset;
}

void msclose(struct memstream *ms) {
	if (ms != NULL) {
		if (ms->buf != NULL) {
			free(ms->buf);
			ms->buf = NULL;
		}
	}
}

int test_dos_header(struct memstream *ms) {
	struct dos_header dh;

	if (ms == NULL) {
		return 0;
	}
	if (msread(ms, &dh, sizeof (struct dos_header)) != sizeof (struct dos_header)) {
		return 0;
	}
	msseek(ms, 0x00);
	if (dh.e_magic != DOS_SIGNATURE) {
		return 0;
	}
	/* at least one page */
	if (dh.e_cp == 0) {
		return 0;
	}
	/* last page must not hold 0 bytes */
	if (dh.e_cblp == 0) {
		return 0;
	}
	/* not even number of paragraphs */
	if (dh.e_cparhdr % 2 != 0) {
		return 0;
	}
	return 1;
}

Common::MemoryReadStream *unpackEXE(Common::File &file) {
	struct memstream ms;
	msopen(file, &ms);
	if (test_dos_header(&ms) == 0) {
		msclose(&ms);
		return nullptr;
	}
	Common::MemoryReadStream *ret = unpack(&ms);
	msclose(&ms);
	return ret;
}

}
