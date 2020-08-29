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

void Hugo::DisplayPicture() {
	if (!hugo_hasgraphics()) {
		var[system_status] = STAT_UNAVAILABLE;
		return;
	}

	char filename[MAX_RES_PATH], resname[MAX_RES_PATH];
	g_vm->GetResourceParameters(filename, resname, PICTURE_T);
	Common::String picName = Common::String::format("%s,%s",
		filename, resname);

	// Draw game's picture then move cursor down to the next line
	if (glk_image_draw(mainwin, picName, imagealign_InlineUp, 0)) {
		glk_put_char('\n');
	} else {
		var[system_status] = STAT_LOADERROR;
	}
}

void Hugo::PlayMusic() {
	char filename[MAX_RES_PATH], resname[MAX_RES_PATH];
	char loop_flag = 0;
	long resstart, reslength;

	if (MEM(codeptr+1)==REPEAT_T) loop_flag = true, codeptr++;

	hugo_stopmusic();

	/* If a 0 parameter is passed, i.e. "music 0" */
	if (!GetResourceParameters(filename, resname, MUSIC_T))
	{
		return;
	}

	if (extra_param>=0)
	{
		if (extra_param > 100) extra_param = 100;
		hugo_musicvolume(extra_param);
	}

	if (!(reslength = FindResource(filename, resname)))
		return;

	/* Find out what type of music resource this is */
	resstart = hugo_ftell(resource_file);

	/* Check for MIDI */
	hugo_fseek(resource_file, resstart, SEEK_SET);
	hugo_fread(resname, 4, 1, resource_file);
	if (!memcmp(resname, "MThd", 4))
	{
		resource_type = MIDI_R;
		goto Identified;
	}

	/* Check for XM */
	hugo_fseek(resource_file, resstart, SEEK_SET);
	hugo_fread(resname, 17, 1, resource_file);
	if (!memcmp(resname, "Extended Module: ", 17))
	{
		resource_type = XM_R;
		goto Identified;
	}

	/* Check for S3M */
	hugo_fseek(resource_file, resstart+0x2c, SEEK_SET);
	hugo_fread(resname, 4, 1, resource_file);
	if (!memcmp(resname, "SCRM", 4))
	{
		resource_type = S3M_R;
		goto Identified;
	}

	/* Check for MOD */
	hugo_fseek(resource_file, resstart+1080, SEEK_SET);
	hugo_fread(resname, 4, 1, resource_file);
	resname[4] = '\0';
	/* There are a whole bunch of different MOD identifiers: */
	if (!strcmp(resname+1, "CHN") ||	/* 4CHN, 6CHN, 8CHN */
	    !strcmp(resname+2, "CN") ||		/* 16CN, 32CN */
	    !strcmp(resname, "M.K.") || !strcmp(resname, "M!K!") ||
	    !strcmp(resname, "FLT4") || !strcmp(resname, "CD81") ||
	    !strcmp(resname, "OKTA") || !strcmp(resname, "    "))
	{
		resource_type = MOD_R;
		goto Identified;
	}

	/* Check for MP3 */
/* Assume for now that otherwise unidentified is MP3 */
	else
	{
		resource_type = MP3_R;
		goto Identified;
	}

	/* No file type identified */
	resource_type = UNKNOWN_R;

Identified:
	hugo_fseek(resource_file, resstart, SEEK_SET);

	if (!hugo_playmusic(resource_file, reslength, loop_flag))
		var[system_status] = STAT_LOADERROR;
}

void Hugo::PlaySample() {
	char filename[MAX_RES_PATH], resname[MAX_RES_PATH];
	char loop_flag = 0;
	long reslength;

	if (MEM(codeptr+1)==REPEAT_T) loop_flag = true, codeptr++;

	hugo_stopsample();

	/* If a 0 parameter is passed, i.e. "sound 0" */
	if (!GetResourceParameters(filename, resname, SOUND_T))
	{
		return;
	}

	if (extra_param>=0)
	{
		if (extra_param > 100) extra_param = 100;
		hugo_samplevolume(extra_param);
	}

	if (!(reslength = FindResource(filename, resname)))
		return;

	/* Find out what kind of audio sample this is */
	hugo_fread(resname, 4, 1, resource_file);
	if (!memcmp(resname, "WAVE", 4))
		resource_type = WAVE_R;
	else
		resource_type = UNKNOWN_R;

	hugo_fseek(resource_file, -4, SEEK_CUR);

	if (!hugo_playsample(resource_file, reslength, loop_flag))
		var[system_status] = STAT_LOADERROR;
}

void Hugo::PlayVideo() {
	char filename[MAX_RES_PATH], resname[MAX_RES_PATH];
	char loop_flag = 0, background = 0;
	int volume = 100;
	long resstart, reslength;

#if defined (COMPILE_V25)
	var[system_status] = STAT_UNAVAILABLE;
#endif
	if (MEM(codeptr+1)==REPEAT_T) loop_flag = true, codeptr++;

#if !defined (COMPILE_V25)
	hugo_stopvideo();
#endif

	/* If a 0 parameter is passed, i.e. "video 0" */
	if (!GetResourceParameters(filename, resname, VIDEO_T))
	{
		return;
	}

	if (MEM(codeptr-1)==COMMA_T)
	{
		background = (char)GetValue();
		codeptr++;	/* eol */
	}

	if (extra_param>=0)
	{
		if (extra_param > 100) extra_param = 100;
		volume = extra_param;
	}

	if (!(reslength = FindResource(filename, resname)))
		return;

	/* Find out what type of video resource this is */
	resstart = hugo_ftell(resource_file);

	/* Check for MPEG */
	hugo_fseek(resource_file, resstart, SEEK_SET);
	hugo_fread(resname, 4, 1, resource_file);
	if (resname[2]==0x01 && (unsigned char)resname[3]==0xba)
	{
		resource_type = MPEG_R;
		goto Identified;
	}

	/* Check for AVI */
	hugo_fseek(resource_file, resstart+8, SEEK_SET);
	hugo_fread(resname, 4, 1, resource_file);
	if (!memcmp(resname, "AVI ", 4))
	{
		resource_type = AVI_R;
		goto Identified;
	}

	/* No file type identified */
	resource_type = UNKNOWN_R;

Identified:
	hugo_fseek(resource_file, resstart, SEEK_SET);

#if !defined (COMPILE_V25)
	if (!hugo_playvideo(resource_file, reslength, loop_flag, background, volume))
		var[system_status] = STAT_LOADERROR;
#else
	fclose(resource_file);
	resource_file = nullptr;
#endif
}

long Hugo::FindResource(const char *filename, const char *resname) {
	char resource_in_file[MAX_RES_PATH];
	int i, len;
	int rescount;
	unsigned int startofdata;
	long resposition, reslength;
#if defined (GLK)
	frefid_t fref;
#endif
/* Previously, resource positions were written as 24 bits, which meant that
   a given resource couldn't start after 16,777,216 bytes or be more than
   that length.  The new resource file format (designated by 'r') corrects this. */
	int res_32bits = true;

	resource_file = nullptr;

	strcpy(loaded_filename, filename);
	strcpy(loaded_resname, resname);
	if (!strcmp(filename, "")) strcpy(loaded_filename, resname);

	/* See if the file is supposed to be in a resourcefile to
	   begin with
	*/
	if (!strcmp(filename, ""))
		goto NotinResourceFile;


	/* Open the resourcefile */
	//strupr(filename);

#if !defined (GLK)
	/* stdio implementation */
	if (!(resource_file = TrytoOpen(filename, "rb", "games")))
		if (!(resource_file = TrytoOpen(filename, "rb", "object")))
		{
			var[system_status] = STAT_NOFILE;
			return 0;
		}
#else
	/* Glk implementation */
	fref = glk_fileref_create_by_name(fileusage_Data | fileusage_BinaryMode,
		filename, 0);
	if (glk_fileref_does_file_exist(fref))
		resource_file = glk_stream_open_file(fref, filemode_Read, 0);
	else
		resource_file = nullptr;
	glk_fileref_destroy(fref);
	if (!resource_file)
	{
		var[system_status] = STAT_NOFILE;
		return 0;
	}
#endif

	/* Read the resourcefile header */
	/* if (fgetc(resource_file)!='R') goto ResfileError; */
	i = hugo_fgetc(resource_file);
	if (i=='r')
		res_32bits = true;
	else if (i=='R')
		res_32bits = false;
	else
		goto ResfileError;
	/* Read and ignore the resource file version. */
	hugo_fgetc(resource_file);
	rescount = hugo_fgetc(resource_file);
	rescount += hugo_fgetc(resource_file)*256;
	startofdata = hugo_fgetc(resource_file);
	startofdata += (unsigned int)hugo_fgetc(resource_file)*256;
	if (hugo_ferror(resource_file))
		goto ResfileError;


	/* Now skim through the list of resources in the resourcefile to
	   see if we have a match
	*/
	for (i=1; i<=rescount; i++)
	{
		len = hugo_fgetc(resource_file);
		if (hugo_ferror(resource_file))
			goto ResfileError;

		if (!(hugo_fgets(resource_in_file, len+1, resource_file)))
			goto ResfileError;

		resposition = (long)hugo_fgetc(resource_file);
		resposition += (long)hugo_fgetc(resource_file)*256L;
		resposition += (long)hugo_fgetc(resource_file)*65536L;
		if (res_32bits)
		{
			resposition += (long)hugo_fgetc(resource_file)*16777216L;
		}

		reslength = (long)hugo_fgetc(resource_file);
		reslength += (long)hugo_fgetc(resource_file)*256L;
		reslength += (long)hugo_fgetc(resource_file)*65536L;
		if (res_32bits)
		{
			reslength += (long)hugo_fgetc(resource_file)*16777216L;
		}
		if (hugo_ferror(resource_file)) goto ResfileError;

		if (!strcmp(resname, resource_in_file))
		{
			if (hugo_fseek(resource_file, (long)startofdata+resposition, SEEK_SET))
				goto ResfileError;
			return reslength;
		}
	}

ResfileError:

	var[system_status] = STAT_NORESOURCE;

#if defined (DEBUGGER)
	SwitchtoDebugger();
	sprintf(debug_line, "Unable to find \"%s\" in \"%s\"", resname, filename);
	DebugMessageBox("Resource Error", debug_line);
	SwitchtoGame();
#endif
	hugo_fclose(resource_file);
	resource_file = nullptr;


	/* If we get here, we've either been unable to find the named
	   resource in the given resourcefile, or no resourcefile was
	   given
	*/
NotinResourceFile:

#if !defined (GLK)
	/* stdio implementation */
	if (!(resource_file = TrytoOpen(resname, "rb", "resource")))
		if (!(resource_file = TrytoOpen(resname, "rb", "source")))
		{
			if (!strcmp(filename, ""))
				var[system_status] = STAT_NOFILE;
			else
				var[system_status] = STAT_NORESOURCE;
			return 0;
		}
#else
	/* Glk implementation */
	fref = glk_fileref_create_by_name(fileusage_Data | fileusage_BinaryMode,
		resname, 0);
	if (glk_fileref_does_file_exist(fref))
		resource_file = glk_stream_open_file(fref, filemode_Read, 0);
	else
		resource_file = nullptr;
	glk_fileref_destroy(fref);
	if (!resource_file)
	{
		if (!strcmp(filename, ""))
			var[system_status] = STAT_NOFILE;
		else
			var[system_status] = STAT_NORESOURCE;
		return 0;
	}
#endif

	/* resource_file here refers to a resource in an individual
	   on-disk file, not a consolidated resource file
	*/
	hugo_fseek(resource_file, 0, SEEK_END);
	reslength = hugo_ftell(resource_file);
	hugo_fseek(resource_file, 0, SEEK_SET);
	if (hugo_ferror(resource_file))
	{
		hugo_fclose(resource_file);
		resource_file = nullptr;
		return false;
	}

	return reslength;
}

int Hugo::GetResourceParameters(char *filename, char *resname, int restype) {
	int f;
	
	var[system_status] = 0;

	extra_param = -1;

	codeptr++;		/* token--i.e., 'picture', etc. */

	f = GetValue();

	/* If a 0 parameter is passed for "music 0", etc. */
	if (!f && MEM(codeptr)!=COMMA_T)
	{
		++codeptr;
		return 0;
	}

	strcpy(filename, GetWord((unsigned int)f));

	if (MEM(codeptr++)!=EOL_T)	/* two or more parameters */
	{
		strupr(filename);
		strcpy(resname, GetWord(GetValue()));
		if (MEM(codeptr++)==COMMA_T)
		{
			extra_param = GetValue();
			codeptr++;
		}
	}
	else				/* only one parameter */
	{
		strcpy(resname, filename);
		strcpy(filename, "");
	}

	return true;
}

} // End of namespace Hugo
} // End of namespace Glk
