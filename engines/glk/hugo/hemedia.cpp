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

#include "glk/hugo/hugo.h"

namespace Glk {
namespace Hugo {

int Hugo::loadres(HUGO_FILE infile, int reslen, int type) {
	char buf[4096];
	frefid_t fileref;
	strid_t stream;
	long offset;
	int idVal;
	int i, n;

	offset = hugo_ftell(infile);
	for (i = 0; i < numres[type]; i++)
		if (resids[type][i] == offset)
			return i;

	/* Too many resources loaded... */
	if (numres[type] + 1 == MAXRES)
		return -1;

	idVal = numres[type]++;
	sprintf(buf, "%s%d", type == PIC ? "PIC" : "SND", idVal);
	resids[type][idVal] = offset;

	fileref = glk_fileref_create_by_name(fileusage_Data, buf, 0);
	if (!fileref)
	{
		return -1;
	}

	stream = glk_stream_open_file(fileref, filemode_Write, 0);
	if (!stream)
	{
		glk_fileref_destroy(fileref);
		return -1;
	}

	glk_fileref_destroy(fileref);

	while (reslen > 0)
	{
		n = hugo_fread(buf, 1, reslen < (int)sizeof(buf) ? reslen : sizeof(buf), infile);
		if (n <= 0)
			break;
		glk_put_buffer_stream(stream, buf, n);
		reslen -= n;
	}

	glk_stream_close(stream, NULL);

	return idVal;
}

int Hugo::hugo_hasgraphics() {
	/* Returns true if the current display is capable of graphics display */
	return glk_gestalt(gestalt_Graphics, 0)
		&& glk_gestalt(gestalt_DrawImage, glk_window_get_type(mainwin));
}

void Hugo::initsound() {
	if (!glk_gestalt(gestalt_Sound, 0))
		return;
	schannel = glk_schannel_create(0);
}

void Hugo::initmusic() {
	if (!glk_gestalt(gestalt_Sound, 0) || !glk_gestalt(gestalt_SoundMusic, 0))
		return;
	mchannel = glk_schannel_create(0);
}

int Hugo::hugo_playmusic(HUGO_FILE infile, long reslen, char loop_flag) {
	int idVal;

	if (!mchannel)
		initmusic();
	if (mchannel)
	{
		idVal = loadres(infile, reslen, SND);
		if (idVal < 0)
		{
			hugo_fclose(infile);
			return false;
		}
		glk_schannel_play_ext(mchannel, idVal, loop_flag ? -1 : 1, 0);
	}

	hugo_fclose(infile);
	return true;
}

void Hugo::hugo_musicvolume(int vol) {
	if (!mchannel) initmusic();
	if (!mchannel) return;
	glk_schannel_set_volume(mchannel, (vol * 0x10000) / 100);
}

void Hugo::hugo_stopmusic() {
	if (!mchannel) initmusic();
	if (!mchannel) return;
	glk_schannel_stop(mchannel);
}

int Hugo::hugo_playsample(HUGO_FILE infile, long reslen, char loop_flag) {
	int idVal;

	if (schannel)
	{
		idVal = loadres(infile, reslen, SND);
		if (idVal < 0)
		{
			hugo_fclose(infile);
			return false;
		}
		glk_schannel_play_ext(schannel, idVal, loop_flag ? -1 : 1, 0);
	}

	hugo_fclose(infile);
	return true;
}

void Hugo::hugo_samplevolume(int vol) {
	if (!schannel) initsound();
	if (!schannel) return;
	glk_schannel_set_volume(schannel, (vol * 0x10000) / 100);
}

void Hugo::hugo_stopsample() {
	if (!schannel) initsound();
	if (!schannel) return;
	glk_schannel_stop(schannel);
}

} // End of namespace Hugo
} // End of namespace Glk
