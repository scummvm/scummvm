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
#if 0
#if defined __unix__ && !(defined __APPLE__)
#include <png.h>
#else
#include <libpng/png.h>
#endif

#include <SDL/SDL.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <iconv.h>
#endif

#include "allfiles.h"
#include "platform-dependent.h"
#include "CommonCode/version.h"
#include "sludger.h"
#include "backdrop.h"
#include "cursors.h"
#include "colours.h"
#include "objtypes.h"
#include "region.h"
#include "sprites.h"
#include "sprbanks.h"
#include "people.h"
#include "talk.h"
#include "newfatal.h"
#include "stringy.h"
#include "moreio.h"
#include "statusba.h"
#include "builtin.h"
#include "fonttext.h"
#include "freeze.h"
#include "floor.h"
#include "zbuffer.h"
#include "sound.h"
#include "loadsave.h"
#include "fileset.h"
#include "transition.h"
#include "language.h"
#include "CommonCode/specialsettings.h"
#include "graphics.h"
#include "variable.h"
#include "common/debug.h"
#include "sludge.h"

namespace Sludge {

extern personaAnimation *mouseCursorAnim;
extern spritePalette pastePalette;
extern int dialogValue;
extern unsigned int sceneWidth, sceneHeight;
extern char *launchMe;
extern variable *launchResult;

extern bool reallyWantToQuit;

int numBIFNames = 0;
char * *allBIFNames = NULL;
int numUserFunc = 0;
char * *allUserFunc = NULL;
int numResourceNames = 0;
char * *allResourceNames = NULL;
int selectedLanguage = 0;
int languageNum = -1;

unsigned char *gameIcon = NULL;
int iconW = 0, iconH = 0;

unsigned char *gameLogo = NULL;
int logoW = 0, logoH = 0;

int gameVersion;
int specialSettings;
FILETIME fileTime;
extern int desiredfps;
bool captureAllKeys = false;

unsigned char brightnessLevel = 255;

eventHandlers mainHandlers;
eventHandlers *currentEvents = &mainHandlers;

extern HWND hMainWindow;
extern screenRegion *overRegion;
extern speechStruct *speech;
extern statusStuff *nowStatus;
extern loadedFunction *saverFunc;

loadedFunction *allRunningFunctions = NULL;
screenRegion *lastRegion = NULL;
variableStack *noStack = NULL;
char *loadNow = NULL;
inputType input;
variable *globalVars;
int numGlobals;

const char *sludgeText[] = { "?????", "RETURN", "BRANCH", "BR_ZERO",
		"SET_GLOBAL", "SET_LOCAL", "LOAD_GLOBAL", "LOAD_LOCAL", "PLUS", "MINUS",
		"MULT", "DIVIDE", "AND", "OR", "EQUALS", "NOT_EQ", "MODULUS",
		"LOAD_VALUE", "LOAD_BUILT", "LOAD_FUNC", "CALLIT", "LOAD_STRING",
		"LOAD_FILE", "LOAD_OBJTYPE", "NOT", "LOAD_NULL", "STACK_PUSH",
		"LESSTHAN", "MORETHAN", "NEGATIVE", "U", "LESS_EQUAL", "MORE_EQUAL",
		"INC_LOCAL", "DEC_LOCAL", "INC_GLOBAL", "DEC_GLOBAL", "INDEXSET",
		"INDEXGET", "INC_INDEX", "DEC_INDEX", "QUICK_PUSH" };

void loadHandlers(Common::SeekableReadStream *stream) {
	currentEvents->leftMouseFunction = stream->readUint16BE();
	currentEvents->leftMouseUpFunction = stream->readUint16BE();
	currentEvents->rightMouseFunction = stream->readUint16BE();
	currentEvents->rightMouseUpFunction = stream->readUint16BE();
	currentEvents->moveMouseFunction = stream->readUint16BE();
	currentEvents->focusFunction = stream->readUint16BE();
	currentEvents->spaceFunction = stream->readUint16BE();
}

void saveHandlers(Common::WriteStream *stream) {
	stream->writeUint16BE(currentEvents->leftMouseFunction);
	stream->writeUint16BE(currentEvents->leftMouseUpFunction);
	stream->writeUint16BE(currentEvents->rightMouseFunction);
	stream->writeUint16BE(currentEvents->rightMouseUpFunction);
	stream->writeUint16BE(currentEvents->moveMouseFunction);
	stream->writeUint16BE(currentEvents->focusFunction);
	stream->writeUint16BE(currentEvents->spaceFunction);
}

Common::File *openAndVerify(char *filename, char extra1, char extra2,
		const char *er, int &fileVersion) {
	Common::File *fp = new Common::File();
	if (!fp->open(filename)) {
		fatal("Can't open file", filename);
		return NULL;
	}
	bool headerBad = false;
	if (fp->readByte() != 'S')
		headerBad = true;
	if (fp->readByte() != 'L')
		headerBad = true;
	if (fp->readByte() != 'U')
		headerBad = true;
	if (fp->readByte() != 'D')
		headerBad = true;
	if (fp->readByte() != extra1)
		headerBad = true;
	if (fp->readByte() != extra2)
		headerBad = true;
	if (headerBad) {
		fatal(er, filename);
		return NULL;
	}
	char c;
	c = fp->readByte();
	debug(kSludgeDebugDataLoad, "%c", c);
	while ((c = fp->readByte())) {
		debug(kSludgeDebugDataLoad, "%c", c);
	}

	int majVersion = fp->readByte();
	debug(kSludgeDebugDataLoad, "majVersion %i", majVersion);
	int minVersion = fp->readByte();
	debug(kSludgeDebugDataLoad, "minVersion %i", minVersion);
	fileVersion = majVersion * 256 + minVersion;

	char txtVer[120];

	if (fileVersion > WHOLE_VERSION) {
		sprintf(txtVer, ERROR_VERSION_TOO_LOW_2, majVersion, minVersion);
		fatal(ERROR_VERSION_TOO_LOW_1, txtVer);
		return NULL;
	} else if (fileVersion < MINIM_VERSION) {
		sprintf(txtVer, ERROR_VERSION_TOO_HIGH_2, majVersion, minVersion);
		fatal(ERROR_VERSION_TOO_HIGH_1, txtVer);
		return NULL;
	}
	return fp;
}

bool initSludge(char *filename) {
	int a = 0;
	mouseCursorAnim = makeNullAnim();

	Common::File *fp = openAndVerify(filename, 'G', 'E', ERROR_BAD_HEADER,
			gameVersion);
	if (!fp)
		return false;

	char c = fp->readByte();
	if (c) {
		numBIFNames = fp->readUint16BE();
		debug(kSludgeDebugDataLoad, "numBIFNames %i", numBIFNames);
		allBIFNames = new char *[numBIFNames];
		if (!checkNew(allBIFNames))
			return false;

		for (int fn = 0; fn < numBIFNames; fn++) {
			allBIFNames[fn] = readString(fp);
		}
		numUserFunc = fp->readUint16BE();
		debug(kSludgeDebugDataLoad, "numUserFunc %i", numUserFunc);
		allUserFunc = new char *[numUserFunc];
		if (!checkNew(allUserFunc))
			return false;

		for (int fn = 0; fn < numUserFunc; fn++) {
			allUserFunc[fn] = readString(fp);
		}
		if (gameVersion >= VERSION(1, 3)) {
			numResourceNames = fp->readUint16BE();
			debug(kSludgeDebugDataLoad, "numResourceNames %i",
					numResourceNames);
			allResourceNames = new char *[numResourceNames];
			if (!checkNew(allResourceNames))
				return false;

			for (int fn = 0; fn < numResourceNames; fn++) {
				allResourceNames[fn] = readString(fp);
			}
		}
	}

	winWidth = fp->readUint16BE();
	debug(kSludgeDebugDataLoad, "winWidth : %i", winWidth);
	winHeight = fp->readUint16BE();
	debug(kSludgeDebugDataLoad, "winHeight : %i", winHeight);
	specialSettings = fp->readByte();
	debug(kSludgeDebugDataLoad, "specialSettings : %i", specialSettings);
	desiredfps = 1000 / fp->readByte();

	delete[] readString(fp);  // Unused - was used for registration purposes.

	size_t bytes_read = fp->read(&fileTime, sizeof(FILETIME));
	if (bytes_read != sizeof(FILETIME) && fp->err()) {
		debug("Reading error in initSludge.");
	}

	char *dataFol =
			(gameVersion >= VERSION(1, 3)) ?
					readString(fp) : joinStrings("", "");
	debug(kSludgeDebugDataLoad, "dataFol : %s", dataFol);

	gameSettings.numLanguages =
			(gameVersion >= VERSION(1, 3)) ? (fp->readByte()) : 0;
	debug(kSludgeDebugDataLoad, "numLanguages : %c", gameSettings.numLanguages);
	makeLanguageTable(fp);

	if (gameVersion >= VERSION(1, 6)) {
		fp->readByte();
		// aaLoad
		fp->readByte();
		getFloat(fp);
		getFloat(fp);
	}

	char *checker = readString(fp);
	debug(kSludgeDebugDataLoad, "checker : %s", checker);

	if (strcmp(checker, "okSoFar"))
		return fatal(ERROR_BAD_HEADER, filename);
	delete checker;
	checker = NULL;

	unsigned char customIconLogo = fp->readByte();
	debug(kSludgeDebugDataLoad, "Game icon type: %i", customIconLogo);

	if (customIconLogo & 1) {
		// There is an icon - read it!
		debug(kSludgeDebugDataLoad, "There is an icon - read it!");

#if 0
		int n;
		long file_pointer = ftell(fp);

		png_structp png_ptr;
		png_infop info_ptr, end_info;

		int fileIsPNG = true;

		// Is this a PNG file?

		char tmp[10];
		bytes_read = fread(tmp, 1, 8, fp);
		if (bytes_read != 8 && ferror(fp)) {
			debugOut("Reading error in initSludge.\n");
		}
		if (png_sig_cmp((png_byte *) tmp, 0, 8)) {
			// No, it's old-school HSI
			fileIsPNG = false;
			fseek(fp, file_pointer, SEEK_SET);

			iconW = fp->readUint16BE();
			iconH = fp->readUint16BE();
		} else {
			// Read the PNG header

			png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
			if (!png_ptr) {
				return false;
			}

			info_ptr = png_create_info_struct(png_ptr);
			if (!info_ptr) {
				png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
				return false;
			}

			end_info = png_create_info_struct(png_ptr);
			if (!end_info) {
				png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
				return false;
			}
			png_init_io(png_ptr, fp);       // Tell libpng which file to read
			png_set_sig_bytes(png_ptr, 8);// 8 bytes already read

			png_read_info(png_ptr, info_ptr);

			png_uint_32 width, height;
			int bit_depth, color_type, interlace_type, compression_type, filter_method;
			png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_method);

			iconW = width;
			iconH = height;

			if (bit_depth < 8) png_set_packing(png_ptr);
			png_set_expand(png_ptr);
			if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) png_set_gray_to_rgb(png_ptr);
			if (bit_depth == 16) png_set_strip_16(png_ptr);

			png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);

			png_read_update_info(png_ptr, info_ptr);
			png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_method);

		}

		gameIcon = new unsigned char [iconW * iconH * 4];
		if (!gameIcon) return fatal("Can't reserve memory for game icon.");

		int32_t transCol = 63519;
		Uint8 *p = (Uint8 *) gameIcon;

		if (fileIsPNG) {
			unsigned char *row_pointers[iconH];
			for (int i = 0; i < iconH; i++)
			row_pointers[i] = p + 4 * i * iconW;

			png_read_image(png_ptr, (png_byte **) row_pointers);
			png_read_end(png_ptr, NULL);
			png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		} else {

			for (int t2 = 0; t2 < iconH; t2 ++) {
				int t1 = 0;
				while (t1 < iconW) {
					unsigned short c = (unsigned short) fp->readUint16BE();
					if (c & 32) {
						n = fgetc(fp) + 1;
						c -= 32;
					} else {
						n = 1;
					}
					while (n --) {
						*p++ = (Uint8) redValue(c);
						*p++ = (Uint8) greenValue(c);
						*p++ = (Uint8) blueValue(c);
						*p++ = (Uint8)(c == transCol) ? 0 : 255;

						t1++;
					}
				}
			}
		}
#endif
	}

	if (customIconLogo & 2) {
		// There is an logo - read it!
		debug(kSludgeDebugDataLoad, "There is an logo - read it!");
#if 0
		int n;
		long file_pointer = ftell(fp);

		png_structp png_ptr;
		png_infop info_ptr, end_info;

		int fileIsPNG = true;

		// Is this a PNG file?

		char tmp[10];
		bytes_read = fread(tmp, 1, 8, fp);
		if (bytes_read != 8 && ferror(fp)) {
			debugOut("Reading error in initSludge.");
		}
		if (png_sig_cmp((png_byte *) tmp, 0, 8)) {
			// No, it's old-school HSI
			fileIsPNG = false;
			fseek(fp, file_pointer, SEEK_SET);

			logoW = fp->readUint16BE();
			logoH = fp->readUint16BE();
		} else {
			// Read the PNG header

			png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
			if (!png_ptr) {
				return false;
			}

			info_ptr = png_create_info_struct(png_ptr);
			if (!info_ptr) {
				png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
				return false;
			}

			end_info = png_create_info_struct(png_ptr);
			if (!end_info) {
				png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
				return false;
			}
			png_init_io(png_ptr, fp);       // Tell libpng which file to read
			png_set_sig_bytes(png_ptr, 8);// 8 bytes already read

			png_read_info(png_ptr, info_ptr);

			png_uint_32 width, height;
			int bit_depth, color_type, interlace_type, compression_type, filter_method;
			png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_method);

			logoW = width;
			logoH = height;

			if (bit_depth < 8) png_set_packing(png_ptr);
			png_set_expand(png_ptr);
			if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) png_set_gray_to_rgb(png_ptr);
			if (bit_depth == 16) png_set_strip_16(png_ptr);
#ifdef WIN32
			// Windows wants a BGR bitmap
			if (color_type == PNG_COLOR_TYPE_RGB ||
					color_type == PNG_COLOR_TYPE_RGB_ALPHA)
			png_set_bgr(png_ptr);
#endif

			png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);

			png_read_update_info(png_ptr, info_ptr);
			png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_method);

		}

		if ((logoW != 310) || (logoH != 88)) return fatal("Game logo have wrong dimensions. (Should be 310x88)");

		gameLogo = new unsigned char [logoW * logoH * 4];
		if (!gameLogo) return fatal("Can't reserve memory for game logo.");

		// int32_t transCol = 63519;
		Uint8 *p = (Uint8 *) gameLogo;

		if (fileIsPNG) {
			unsigned char *row_pointers[logoH];
			for (int i = 0; i < logoH; i++)
			row_pointers[i] = p + 4 * i * logoW;

			png_read_image(png_ptr, (png_byte **) row_pointers);
			png_read_end(png_ptr, NULL);
			png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		} else {

			for (int t2 = 0; t2 < logoH; t2 ++) {
				int t1 = 0;
				while (t1 < logoW) {
					unsigned short c = (unsigned short) fp->readUint16BE();
					if (c & 32) {
						n = fgetc(fp) + 1;
						c -= 32;
					} else {
						n = 1;
					}
					while (n --) {
#ifdef WIN32
						// Windows wants a BGR bitmap
						*p++ = (Uint8) blueValue(c);
						*p++ = (Uint8) greenValue(c);
						*p++ = (Uint8) redValue(c);
#else
						*p++ = (Uint8) redValue(c);
						*p++ = (Uint8) greenValue(c);
						*p++ = (Uint8) blueValue(c);
#endif
						*p++ = (Uint8) /*(c == transCol) ? 0 :*/255;

						t1++;
					}
				}
			}
		}
#endif
	}

	numGlobals = fp->readUint16BE();
	debug(kSludgeDebugDataLoad, "numGlobals : %i", numGlobals);

	globalVars = new variable[numGlobals];
	if (!checkNew(globalVars))
		return false;
	for (a = 0; a < numGlobals; a++)
		initVarNew(globalVars[a]);

	// Get the original (untranslated) name of the game and convert it to Unicode.
	// We use this to find saved preferences and saved games.
	setFileIndices(fp, gameSettings.numLanguages, 0);
	char *gameNameOrig = getNumberedString(1);

	char *gameName = encodeFilename(gameNameOrig);

	delete gameNameOrig;

#if 0
	changeToUserDir();

#ifdef _WIN32
	mkdir(gameName);
#else
	mkdir(gameName, 0000777);
#endif

	if (chdir(gameName)) return fatal("This game's preference folder is inaccessible!\nI can't access the following directory (maybe there's a file with the same name, or maybe it's read-protected):", gameName);
#endif
	delete[] gameName;

	// Get user settings
	readIniFile(filename);

	// There's no startup window on Linux and respecting this
	// option from the ini file would disable commandline options.
#if defined __unix__ && !(defined __APPLE__)
#if 0
	if (!showSetupWindow()) return 0;
#endif
	saveIniFile(filename);
#else
	if (!gameSettings.noStartWindow) {
		if (!showSetupWindow()) return 0;
		saveIniFile(filename);
	}
#endif

	// Now set file indices properly to the chosen language.
	languageNum = getLanguageForFileB();
	if (languageNum < 0)
		return fatal("Can't find the translation data specified!");
	setFileIndices(NULL, gameSettings.numLanguages, languageNum);

	if (dataFol[0]) {
		char *dataFolder = encodeFilename(dataFol);
#if 0
#ifdef _WIN32
		mkdir(dataFolder);
#else
		mkdir(dataFolder, 0000777);
#endif

		if (chdir(dataFolder)) return fatal("This game's data folder is inaccessible!\nI can't access the following directory (maybe there's a file with the same name, or maybe it's read-protected):", dataFolder);
#endif
		delete dataFolder;
	}

	positionStatus(10, winHeight - 15);

	return true;
}

extern int cameraX, cameraY;
extern float cameraZoom;

bool checkColourChange(bool reset) {
#if 0
	static GLuint oldPixel;
	static GLuint pixel;
	glReadPixels((GLint)(viewportOffsetX + input.mouseX * viewportWidth / ((float)winWidth / cameraZoom)),
			(GLint)(viewportOffsetY + (((float)winHeight / cameraZoom) - input.mouseY)*viewportHeight / ((float)winHeight / cameraZoom)),
			1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &pixel);

	if (reset || oldPixel != pixel) {
		oldPixel = pixel;
		return true;
	}
#endif
	return false;
}
void sludgeDisplay() {
#if 0
#if defined(HAVE_GLES2)
#ifndef GL_DEPTH24_STENCIL8
#define GL_DEPTH24_STENCIL8 0x88F0
#endif
	// create an FBO
	static GLuint fbo = 0;
	static GLuint fbo_tex = 0;
	static GLuint fbo_rbo = 0;
	static float fbo_tex_w, fbo_tex_h;
	static GLuint fbo_shad, fbo_vert, fbo_frag;
	if (fbo == 0) {
		// create FBO
		int width = 1;
		while (width < realWinWidth) width *= 2;
		int height = 1;
		while (height < realWinHeight) height *= 2;
		glGenFramebuffers(1, &fbo);
		glGenTextures(1, &fbo_tex);
		glGenRenderbuffers(1, &fbo_rbo);
		glBindTexture(GL_TEXTURE_2D, fbo_tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, fbo_rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_tex, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fbo_rbo);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fbo_rbo);
		GLenum ret = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		printf("Screen is %dx%d, FBO(%dx%d) Status = 0x%04X\n", realWinWidth, realWinHeight, width, height, ret);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		fbo_tex_w = (float)realWinWidth / width;
		fbo_tex_h = (float)realWinHeight / height;
		// create shader for blitting the fbo...
		const char _blit_vsh[] = "                              \n\t"
		"attribute highp vec2 aPosition;                        \n\t"
		"attribute highp vec2 aTexCoord;                        \n\t"
		"varying mediump vec2 vTexCoord;                        \n\t"
		"void main(){                                           \n\t"
		"gl_Position = vec4(aPosition.x, aPosition.y, 0.0, 1.0);\n\t"
		"vTexCoord = aTexCoord;                                 \n\t"
		"}                                                      \n\t";

		const char _blit_fsh[] = "                              \n\t"
		"uniform sampler2D uTex;                                \n\t"
		"varying mediump vec2 vTexCoord;                        \n\t"
		"void main(){                                           \n\t"
		"gl_FragColor = texture2D(uTex, vTexCoord);             \n\t"
		"}                                                      \n\t";

		GLint success;
		fbo_frag = glCreateShader(GL_FRAGMENT_SHADER);
		const char *src[1];
		src[0] = _blit_fsh;
		glShaderSource(fbo_frag, 1, src, NULL);
		glCompileShader(fbo_frag);
		glGetShaderiv(fbo_frag, GL_COMPILE_STATUS, &success);
		if (!success) {
			printf("Failed to produce default fragment shader.\n");
		}
		fbo_vert = glCreateShader(GL_VERTEX_SHADER);
		src[0] = _blit_vsh;
		glShaderSource(fbo_vert, 1, src, NULL);
		glCompileShader(fbo_vert);
		glGetShaderiv(fbo_vert, GL_COMPILE_STATUS, &success);
		if (!success) {
			printf("Failed to produce default vertex shader.\n");
		}
		fbo_shad = glCreateProgram();
		glBindAttribLocation(fbo_shad, 0, "aPosition");
		glBindAttribLocation(fbo_shad, 1, "aTexCoord");
		glAttachShader(fbo_shad, fbo_frag);
		glAttachShader(fbo_shad, fbo_vert);
		glLinkProgram(fbo_shad);
		glGetProgramiv(fbo_shad, GL_LINK_STATUS, &success);
		if (!success) {
			printf("Failed to link default program.\n");
		}
		glUniform1i(glGetUniformLocation(fbo_shad, "uTex"), 0);

	}
#endif

	glDepthMask(GL_TRUE);
//	glClearColor(0.5, 0.5, 1.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// Clear The Screen
	glDepthMask(GL_FALSE);
#endif
	drawBackDrop();// Draw the room
	drawZBuffer(cameraX, cameraY, false);
#if 0
	glEnable(GL_DEPTH_TEST);
#endif
	drawPeople();// Then add any moving characters...
#if 0
	glDisable(GL_DEPTH_TEST);
#endif
	viewSpeech();// ...and anything being said
	drawStatusBar();
	displayCursor();

	if (brightnessLevel < 255) fixBrightness();// This is for transitionLevel special effects
#if 0
	glFlush();
#if !defined(HAVE_GLES2)
	SDL_GL_SwapBuffers();
#else
	if (fbo) {
		// blit the FBO now
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GLuint old_prog;
		glGetIntegerv(GL_CURRENT_PROGRAM, (GLint *)&old_prog);
		glUseProgram(fbo_shad);
		glViewport(0, 0, realWinWidth, realWinHeight);
		const float vert[] = {
			-1.0, -1.0, +0.0, +0.0,
			+1.0, -1.0, fbo_tex_w, +0.0,
			-1.0, +1.0, +0.0, fbo_tex_h,
			+1.0, +1.0, fbo_tex_w, fbo_tex_h
		};
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fbo_tex);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (float *)vert);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (float *)vert + 2);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindTexture(GL_TEXTURE_2D, 0);
		glUseProgram(old_prog);
		glViewport(viewportOffsetX, viewportOffsetY, viewportWidth, viewportHeight);
	}
	EGL_SwapBuffers();
	if (fbo) {
		// Rebind FBO now
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	}
#endif
#endif
}

void pauseFunction(loadedFunction *fun) {
	loadedFunction * * huntAndDestroy = &allRunningFunctions;
	while (*huntAndDestroy) {
		if (fun == *huntAndDestroy) {
			(*huntAndDestroy) = (*huntAndDestroy)->next;
			fun->next = NULL;
		} else {
			huntAndDestroy = &(*huntAndDestroy)->next;
		}
	}
}

void restartFunction(loadedFunction *fun) {
	fun->next = allRunningFunctions;
	allRunningFunctions = fun;
}

void killSpeechTimers() {
	loadedFunction *thisFunction = allRunningFunctions;

	while (thisFunction) {
		if (thisFunction->freezerLevel == 0 && thisFunction->isSpeech
				&& thisFunction->timeLeft) {
			thisFunction->timeLeft = 0;
			thisFunction->isSpeech = false;
		}
		thisFunction = thisFunction->next;
	}

	killAllSpeech();
}

void completeTimers() {
	loadedFunction *thisFunction = allRunningFunctions;

	while (thisFunction) {
		if (thisFunction->freezerLevel == 0)
			thisFunction->timeLeft = 0;
		thisFunction = thisFunction->next;
	}
}

void finishFunction(loadedFunction *fun) {
	int a;

	pauseFunction(fun);
	if (fun->stack)
		fatal(ERROR_NON_EMPTY_STACK);
	delete fun->compiledLines;
	for (a = 0; a < fun->numLocals; a++)
		unlinkVar(fun->localVars[a]);
	delete fun->localVars;
	unlinkVar(fun->reg);
	delete fun;
	fun = NULL;
}

void abortFunction(loadedFunction *fun) {
	int a;

	pauseFunction(fun);
	while (fun->stack)
		trimStack(fun->stack);
	delete fun->compiledLines;
	for (a = 0; a < fun->numLocals; a++)
		unlinkVar(fun->localVars[a]);
	delete fun->localVars;
	unlinkVar(fun->reg);
	if (fun->calledBy)
		abortFunction(fun->calledBy);
	delete fun;
	fun = NULL;
}

int cancelAFunction(int funcNum, loadedFunction *myself, bool &killedMyself) {
	int n = 0;
	killedMyself = false;

	loadedFunction *fun = allRunningFunctions;
	while (fun) {
		if (fun->originalNumber == funcNum) {
			fun->cancelMe = true;
			n++;
			if (fun == myself)
				killedMyself = true;
		}
		fun = fun->next;
	}
	return n;
}

void freezeSubs() {
	loadedFunction *thisFunction = allRunningFunctions;

	while (thisFunction) {
		if (thisFunction->unfreezable) {
			//msgBox ("SLUDGE debugging bollocks!", "Trying to freeze an unfreezable function!");
		} else {
			thisFunction->freezerLevel++;
		}
		thisFunction = thisFunction->next;
	}
}

void unfreezeSubs() {
	loadedFunction *thisFunction = allRunningFunctions;

	while (thisFunction) {
		if (thisFunction->freezerLevel)
			thisFunction->freezerLevel--;
		thisFunction = thisFunction->next;
	}
}

bool continueFunction(loadedFunction *fun) {
	bool keepLooping = true;
	bool advanceNow;
	unsigned int param;
	sludgeCommand com;

	if (fun->cancelMe) {
		abortFunction(fun);
		return true;
	}

//	if (numBIFNames) newDebug ("*** Function:", allUserFunc[fun->originalNumber]);

	//debugOut ("SLUDGER: continueFunction\n");

	while (keepLooping) {
		advanceNow = true;
		param = fun->compiledLines[fun->runThisLine].param;
		com = fun->compiledLines[fun->runThisLine].theCommand;
//		fprintf (stderr, "com: %d param: %d (%s)\n", com, param,
//				(com < numSludgeCommands) ? sludgeText[com] : ERROR_UNKNOWN_MCODE); fflush(stderr);

		if (numBIFNames) {
			setFatalInfo(
					(fun->originalNumber < numUserFunc) ?
							allUserFunc[fun->originalNumber] :
							"Unknown user function",
					(com < numSludgeCommands) ?
							sludgeText[com] : ERROR_UNKNOWN_MCODE);
//			newDebug (
//				(com < numSludgeCommands) ? sludgeText[com] : "Unknown SLUDGE machine code",
//				param);
		}

		//debugOut ("SLUDGER: continueFunction - in da loop: %s\n", sludgeText[com]);

		switch (com) {
		case SLU_RETURN:
			if (fun->calledBy) {
				loadedFunction *returnTo = fun->calledBy;
				if (fun->returnSomething)
					copyVariable(fun->reg, returnTo->reg);
				finishFunction(fun);
				fun = returnTo;
				restartFunction(fun);
			} else {
				finishFunction(fun);
				advanceNow = false;   // So we don't do anything else with "fun"
				keepLooping = false;    // So we drop out of the loop
			}
			break;

		case SLU_CALLIT:
			switch (fun->reg.varType) {
			case SVT_FUNC:
				pauseFunction(fun);
				if (numBIFNames)
					setFatalInfo(
							(fun->originalNumber < numUserFunc) ?
									allUserFunc[fun->originalNumber] :
									"Unknown user function",
							(fun->reg.varData.intValue < numUserFunc) ?
									allUserFunc[fun->reg.varData.intValue] :
									"Unknown user function");

				if (!startNewFunctionNum(fun->reg.varData.intValue, param, fun,
						fun->stack))
					return false;
				fun = allRunningFunctions;
				advanceNow = false;   // So we don't do anything else with "fun"
				break;

			case SVT_BUILT: {
				debug(kSludgeDebugStackMachine, "Built-in init value: %i",
						fun->reg.varData.intValue);
				builtReturn br = callBuiltIn(fun->reg.varData.intValue, param,
						fun);

				switch (br) {
				case BR_ERROR:
					return fatal(
							"Unknown error. This shouldn't happen. Please notify the SLUDGE developers.");

				case BR_PAUSE:
					pauseFunction(fun);
					// No break!

				case BR_KEEP_AND_PAUSE:
					keepLooping = false;
					break;

				case BR_ALREADY_GONE:
					keepLooping = false;
					advanceNow = false;
					break;

				case BR_CALLAFUNC: {
					int i = fun->reg.varData.intValue;
					setVariable(fun->reg, SVT_INT, 1);
					pauseFunction(fun);
					if (numBIFNames)
						setFatalInfo(
								(fun->originalNumber < numUserFunc) ?
										allUserFunc[fun->originalNumber] :
										"Unknown user function",
								(i < numUserFunc) ?
										allUserFunc[i] :
										"Unknown user function");
					if (!startNewFunctionNum(i, 0, fun, noStack, false))
						return false;
					fun = allRunningFunctions;
					advanceNow = false; // So we don't do anything else with "fun"
				}
					break;

				default:
					break;
				}
			}
				break;

			default:
				return fatal(ERROR_CALL_NONFUNCTION);
			}
			break;

			// These all grab things and shove 'em into the register

		case SLU_LOAD_NULL:
			setVariable(fun->reg, SVT_NULL, 0);
			break;

		case SLU_LOAD_FILE:
			setVariable(fun->reg, SVT_FILE, param);
			break;

		case SLU_LOAD_VALUE:
			setVariable(fun->reg, SVT_INT, param);
			break;

		case SLU_LOAD_LOCAL:
			if (!copyVariable(fun->localVars[param], fun->reg))
				return false;
			break;

		case SLU_AND:
			setVariable(fun->reg, SVT_INT,
					getBoolean(fun->reg) && getBoolean(fun->stack->thisVar));
			trimStack(fun->stack);
			break;

		case SLU_OR:
			setVariable(fun->reg, SVT_INT,
					getBoolean(fun->reg) || getBoolean(fun->stack->thisVar));
			trimStack(fun->stack);
			break;

		case SLU_LOAD_FUNC:
			setVariable(fun->reg, SVT_FUNC, param);
			break;

		case SLU_LOAD_BUILT:
			setVariable(fun->reg, SVT_BUILT, param);
			break;

		case SLU_LOAD_OBJTYPE:
			setVariable(fun->reg, SVT_OBJTYPE, param);
			break;

		case SLU_UNREG:
			if (dialogValue != 1)
				fatal(ERROR_HACKER);
			break;

		case SLU_LOAD_STRING:
			if (!loadStringToVar(fun->reg, param)) {
				return false;
			}
			break;

		case SLU_INDEXGET:
		case SLU_INCREMENT_INDEX:
		case SLU_DECREMENT_INDEX:
			switch (fun->stack->thisVar.varType) {
			case SVT_NULL:
				if (com == SLU_INDEXGET) {
					setVariable(fun->reg, SVT_NULL, 0);
					trimStack(fun->stack);
				} else {
					return fatal(ERROR_INCDEC_UNKNOWN);
				}
				break;

			case SVT_FASTARRAY:
			case SVT_STACK:
				if (fun->stack->thisVar.varData.theStack->first == NULL) {
					return fatal(ERROR_INDEX_EMPTY);
				} else {
					int ii;
					if (!getValueType(ii, SVT_INT, fun->reg))
						return false;
					variable *grab =
							(fun->stack->thisVar.varType == SVT_FASTARRAY) ?
									fastArrayGetByIndex(
											fun->stack->thisVar.varData.fastArray,
											ii) :
									stackGetByIndex(
											fun->stack->thisVar.varData.theStack->first,
											ii);

					trimStack(fun->stack);

					if (!grab) {
						setVariable(fun->reg, SVT_NULL, 0);
					} else {
						int ii;
						switch (com) {
						case SLU_INCREMENT_INDEX:
							if (!getValueType(ii, SVT_INT, *grab))
								return false;
							setVariable(fun->reg, SVT_INT, ii);
							grab->varData.intValue = ii + 1;
							break;

						case SLU_DECREMENT_INDEX:
							if (!getValueType(ii, SVT_INT, *grab))
								return false;
							setVariable(fun->reg, SVT_INT, ii);
							grab->varData.intValue = ii - 1;
							break;

						default:
							if (!copyVariable(*grab, fun->reg))
								return false;
						}
					}
				}
				break;

			default:
				return fatal(ERROR_INDEX_NONSTACK);
			}
			break;

		case SLU_INDEXSET:
			switch (fun->stack->thisVar.varType) {
			case SVT_STACK:
				if (fun->stack->thisVar.varData.theStack->first == NULL) {
					return fatal(ERROR_INDEX_EMPTY);
				} else {
					int ii;
					if (!getValueType(ii, SVT_INT, fun->reg))
						return false;
					if (!stackSetByIndex(
							fun->stack->thisVar.varData.theStack->first, ii,
							fun->stack->next->thisVar)) {
						return false;
					}
					trimStack(fun->stack);
					trimStack(fun->stack);
				}
				break;

			case SVT_FASTARRAY: {
				int ii;
				if (!getValueType(ii, SVT_INT, fun->reg))
					return false;
				variable *v = fastArrayGetByIndex(
						fun->stack->thisVar.varData.fastArray, ii);
				if (v == NULL)
					return fatal("Not within bounds of fast array.");
				if (!copyVariable(fun->stack->next->thisVar, *v))
					return false;
				trimStack(fun->stack);
				trimStack(fun->stack);
			}
				break;

			default:
				return fatal(ERROR_INDEX_NONSTACK);
			}
			break;

			// What can we do with the register? Well, we can copy it into a local
			// variable, a global or onto the stack...

		case SLU_INCREMENT_LOCAL: {
			int ii;
			if (!getValueType(ii, SVT_INT, fun->localVars[param]))
				return false;
			setVariable(fun->reg, SVT_INT, ii);
			setVariable(fun->localVars[param], SVT_INT, ii + 1);
		}
			break;

		case SLU_INCREMENT_GLOBAL: {
			int ii;
			if (!getValueType(ii, SVT_INT, globalVars[param]))
				return false;
			setVariable(fun->reg, SVT_INT, ii);
			setVariable(globalVars[param], SVT_INT, ii + 1);
		}
			break;

		case SLU_DECREMENT_LOCAL: {
			int ii;
			if (!getValueType(ii, SVT_INT, fun->localVars[param]))
				return false;
			setVariable(fun->reg, SVT_INT, ii);
			setVariable(fun->localVars[param], SVT_INT, ii - 1);
		}
			break;

		case SLU_DECREMENT_GLOBAL: {
			int ii;
			if (!getValueType(ii, SVT_INT, globalVars[param]))
				return false;
			setVariable(fun->reg, SVT_INT, ii);
			setVariable(globalVars[param], SVT_INT, ii - 1);
		}
			break;

		case SLU_SET_LOCAL:
			if (!copyVariable(fun->reg, fun->localVars[param]))
				return false;
			break;

		case SLU_SET_GLOBAL:
//			newDebug ("  Copying TO global variable", param);
//			newDebug ("  Global type at the moment", globalVars[param].varType);
			if (!copyVariable(fun->reg, globalVars[param]))
				return false;
//			newDebug ("  New type", globalVars[param].varType);
			break;

		case SLU_LOAD_GLOBAL:
//			newDebug ("  Copying FROM global variable", param);
//			newDebug ("  Global type at the moment", globalVars[param].varType);
			if (!copyVariable(globalVars[param], fun->reg))
				return false;
			break;

		case SLU_STACK_PUSH:
			if (!addVarToStack(fun->reg, fun->stack))
				return false;
			break;

		case SLU_QUICK_PUSH:
			if (!addVarToStackQuick(fun->reg, fun->stack))
				return false;
			break;

		case SLU_NOT:
			setVariable(fun->reg, SVT_INT, !getBoolean(fun->reg));
			break;

		case SLU_BR_ZERO:
			if (!getBoolean(fun->reg)) {
				advanceNow = false;
				fun->runThisLine = param;
			}
			break;

		case SLU_BRANCH:
			advanceNow = false;
			fun->runThisLine = param;
			break;

		case SLU_NEGATIVE: {
			int i;
			if (!getValueType(i, SVT_INT, fun->reg))
				return false;
			setVariable(fun->reg, SVT_INT, -i);
		}
			break;

			// All these things rely on there being somet' on the stack

		case SLU_MULT:
		case SLU_PLUS:
		case SLU_MINUS:
		case SLU_MODULUS:
		case SLU_DIVIDE:
		case SLU_EQUALS:
		case SLU_NOT_EQ:
		case SLU_LESSTHAN:
		case SLU_MORETHAN:
		case SLU_LESS_EQUAL:
		case SLU_MORE_EQUAL:
			if (fun->stack) {
				int firstValue, secondValue;

				switch (com) {
				case SLU_PLUS:
					addVariablesInSecond(fun->stack->thisVar, fun->reg);
					trimStack(fun->stack);
					break;

				case SLU_EQUALS:
					compareVariablesInSecond(fun->stack->thisVar, fun->reg);
					trimStack(fun->stack);
					break;

				case SLU_NOT_EQ:
					compareVariablesInSecond(fun->stack->thisVar, fun->reg);
					trimStack(fun->stack);
					fun->reg.varData.intValue = !fun->reg.varData.intValue;
					break;

				default:
					if (!getValueType(firstValue, SVT_INT, fun->stack->thisVar))
						return false;
					if (!getValueType(secondValue, SVT_INT, fun->reg))
						return false;
					trimStack(fun->stack);

					switch (com) {
					case SLU_MULT:
						setVariable(fun->reg, SVT_INT,
								firstValue * secondValue);
						break;

					case SLU_MINUS:
						setVariable(fun->reg, SVT_INT,
								firstValue - secondValue);
						break;

					case SLU_MODULUS:
						setVariable(fun->reg, SVT_INT,
								firstValue % secondValue);
						break;

					case SLU_DIVIDE:
						setVariable(fun->reg, SVT_INT,
								firstValue / secondValue);
						break;

					case SLU_LESSTHAN:
						setVariable(fun->reg, SVT_INT,
								firstValue < secondValue);
						break;

					case SLU_MORETHAN:
						setVariable(fun->reg, SVT_INT,
								firstValue > secondValue);
						break;

					case SLU_LESS_EQUAL:
						setVariable(fun->reg, SVT_INT,
								firstValue <= secondValue);
						break;

					case SLU_MORE_EQUAL:
						setVariable(fun->reg, SVT_INT,
								firstValue >= secondValue);
						break;

					default:
						break;
					}
				}
			} else {
				return fatal(ERROR_NOSTACK);
			}
			break;

		default:
			return fatal(ERROR_UNKNOWN_CODE);
		}

		if (advanceNow)
			fun->runThisLine++;

	}
	return true;
}

bool runSludge() {

	loadedFunction *thisFunction = allRunningFunctions;
	loadedFunction *nextFunction;

	while (thisFunction) {
		nextFunction = thisFunction->next;

		if (!thisFunction->freezerLevel) {
			if (thisFunction->timeLeft) {
				if (thisFunction->timeLeft < 0) {
					if (!stillPlayingSound(
							findInSoundCache(speech->lastFile))) {
						thisFunction->timeLeft = 0;
					}
				} else if (!--(thisFunction->timeLeft)) {
				}
			} else {
				if (thisFunction->isSpeech) {
					thisFunction->isSpeech = false;
					killAllSpeech();
				}
				if (!continueFunction(thisFunction))
					return false;
			}
		}

		thisFunction = nextFunction;
	}

	if (loadNow) {
		if (loadNow[0] == ':') {
			saveGame(loadNow + 1);
			setVariable(saverFunc->reg, SVT_INT, 1);
		} else {
			if (!loadGame(loadNow))
				return false;
		}
		delete loadNow;
		loadNow = NULL;
	}

	return true;
}

bool loadFunctionCode(loadedFunction *newFunc) {

	debug(kSludgeDebugDataLoad, "Current address: %i", bigDataFile->pos());
	unsigned int numLines, numLinesRead;

	if (!openSubSlice(newFunc->originalNumber))
		return false;

	debug(kSludgeDebugDataLoad, "Load function code");

	newFunc->unfreezable = bigDataFile->readByte();
	numLines = bigDataFile->readUint16BE();
	debug(kSludgeDebugDataLoad, "numLines: %i", numLines);
	newFunc->numArgs = bigDataFile->readUint16BE();
	debug(kSludgeDebugDataLoad, "numArgs: %i", newFunc->numArgs);
	newFunc->numLocals = bigDataFile->readUint16BE();
	debug(kSludgeDebugDataLoad, "numLocals: %i", newFunc->numLocals);
	newFunc->compiledLines = new lineOfCode[numLines];
	if (!checkNew(newFunc->compiledLines))
		return false;

	for (numLinesRead = 0; numLinesRead < numLines; numLinesRead++) {
		newFunc->compiledLines[numLinesRead].theCommand = (sludgeCommand) bigDataFile->readByte();
		newFunc->compiledLines[numLinesRead].param = bigDataFile->readUint16BE();
		debug(kSludgeDebugDataLoad, "command line %i: %i", numLinesRead,
				newFunc->compiledLines[numLinesRead].theCommand);
	}
	finishAccess();

	// Now we need to reserve memory for the local variables
	newFunc->localVars = new variable[newFunc->numLocals];
	if (!checkNew(newFunc->localVars))
		return false;
	for (int a = 0; a < newFunc->numLocals; a++) {
		initVarNew(newFunc->localVars[a]);
	}

	return true;
}

int startNewFunctionNum(unsigned int funcNum, unsigned int numParamsExpected,
		loadedFunction *calledBy, variableStack *&vStack, bool returnSommet) {
	loadedFunction *newFunc = new loadedFunction;
	checkNew(newFunc);
	newFunc->originalNumber = funcNum;

	loadFunctionCode(newFunc);

	if (newFunc->numArgs != (int) numParamsExpected)
		return fatal("Wrong number of parameters!");
	if (newFunc->numArgs > newFunc->numLocals)
		return fatal("More arguments than local variable space!");

	// Now, lets copy the parameters from the calling function's stack...

	while (numParamsExpected) {
		numParamsExpected--;
		if (vStack == NULL)
			return fatal(
					"Corrupted file!The stack's empty and there were still parameters expected");
		copyVariable(vStack->thisVar, newFunc->localVars[numParamsExpected]);
		trimStack(vStack);
	}

	newFunc->cancelMe = false;
	newFunc->timeLeft = 0;
	newFunc->returnSomething = returnSommet;
	newFunc->calledBy = calledBy;
	newFunc->stack = NULL;
	newFunc->freezerLevel = 0;
	newFunc->runThisLine = 0;
	newFunc->isSpeech = 0;
	initVarNew(newFunc->reg);

	restartFunction(newFunc);
	return 1;
}

int lastFramesPerSecond = -1;
int thisFramesPerSecond = -1;
#if 0
Uint32 lastSeconds = 0;
#endif

bool handleInput() {
	static int l = 0;
#if 0
	static Uint32 theTime;

	theTime = SDL_GetTicks() / 1000;
	if (lastSeconds != theTime) {
		lastSeconds = theTime;
		lastFramesPerSecond = thisFramesPerSecond;
		thisFramesPerSecond = 1;
	} else {
		thisFramesPerSecond ++;
	}
//	lastFramesPerSecond = theTime.wSecond;
#endif
	if (launchMe) {
		if (l) {
			// Still paused because of spawned thingy...
		} else {
			l = 1;

			setVariable(*launchResult, SVT_INT, 0/*launch(launchMe) > 31*/); //TODO:false value
			launchMe = NULL;
			launchResult = NULL;
		}
		return true;
	} else {
		l = 0;
	}

	if (!overRegion)
		getOverRegion();

	if (input.justMoved) {
		if (currentEvents->moveMouseFunction) {
			if (!startNewFunctionNum(currentEvents->moveMouseFunction, 0, NULL,
					noStack))
				return false;
		}
	}
	input.justMoved = false;

	if (lastRegion != overRegion && currentEvents->focusFunction) {
		variableStack *tempStack = new variableStack;
		if (!checkNew(tempStack))
			return false;

		initVarNew(tempStack->thisVar);
		if (overRegion) {
			setVariable(tempStack->thisVar, SVT_OBJTYPE,
					overRegion->thisType->objectNum);
		} else {
			setVariable(tempStack->thisVar, SVT_INT, 0);
		}
		tempStack->next = NULL;
		if (!startNewFunctionNum(currentEvents->focusFunction, 1, NULL,
				tempStack))
			return false;
	}
	if (input.leftRelease && currentEvents->leftMouseUpFunction) {
		if (!startNewFunctionNum(currentEvents->leftMouseUpFunction, 0, NULL,
				noStack))
			return false;
	}
	if (input.rightRelease && currentEvents->rightMouseUpFunction) {
		if (!startNewFunctionNum(currentEvents->rightMouseUpFunction, 0, NULL,
				noStack))
			return false;
	}
	if (input.leftClick && currentEvents->leftMouseFunction)
		if (!startNewFunctionNum(currentEvents->leftMouseFunction, 0, NULL,
				noStack))
			return false;
	if (input.rightClick && currentEvents->rightMouseFunction) {
		if (!startNewFunctionNum(currentEvents->rightMouseFunction, 0, NULL,
				noStack))
			return false;
	}
	if (input.keyPressed && currentEvents->spaceFunction) {
		char *tempString = NULL;
		switch (input.keyPressed) {
		case 127:
			tempString = copyString("BACKSPACE");
			break;
		case 9:
			tempString = copyString("TAB");
			break;
		case 13:
			tempString = copyString("ENTER");
			break;
		case 27:
			tempString = copyString("ESCAPE");
			break;
			/*
			 case 1112:  tempString = copyString ("ALT+F1");     break;
			 case 1113:  tempString = copyString ("ALT+F2");     break;
			 case 1114:  tempString = copyString ("ALT+F3");     break;
			 case 1115:  tempString = copyString ("ALT+F4");     break;
			 case 1116:  tempString = copyString ("ALT+F5");     break;
			 case 1117:  tempString = copyString ("ALT+F6");     break;
			 case 1118:  tempString = copyString ("ALT+F7");     break;
			 case 1119:  tempString = copyString ("ALT+F8");     break;
			 case 1120:  tempString = copyString ("ALT+F9");     break;
			 case 1121:  tempString = copyString ("ALT+F10");    break;
			 case 1122:  tempString = copyString ("ALT+F11");    break;
			 case 1123:  tempString = copyString ("ALT+F12");    break;

			 case 2019:  tempString = copyString ("PAUSE");      break;
			 */
		case 63276:
			tempString = copyString("PAGE UP");
			break;
		case 63277:
			tempString = copyString("PAGE DOWN");
			break;
		case 63275:
			tempString = copyString("END");
			break;
		case 63273:
			tempString = copyString("HOME");
			break;
		case 63234:
			tempString = copyString("LEFT");
			break;
		case 63232:
			tempString = copyString("UP");
			break;
		case 63235:
			tempString = copyString("RIGHT");
			break;
		case 63233:
			tempString = copyString("DOWN");
			break;
			/*
			 case 2045:   tempString = copyString ("INSERT");     break;
			 case 2046:   tempString = copyString ("DELETE");     break;
			 */
		case 63236:
			tempString = copyString("F1");
			break;
		case 63237:
			tempString = copyString("F2");
			break;
		case 63238:
			tempString = copyString("F3");
			break;
		case 63239:
			tempString = copyString("F4");
			break;
		case 63240:
			tempString = copyString("F5");
			break;
		case 63241:
			tempString = copyString("F6");
			break;
		case 63242:
			tempString = copyString("F7");
			break;
		case 63243:
			tempString = copyString("F8");
			break;
		case 63244:
			tempString = copyString("F9");
			break;
		case 63245:
			tempString = copyString("F10");
			break;
		case 63246:
			tempString = copyString("F11");
			break;
		case 63247:
			tempString = copyString("F12");
			break;

		default:
			if (input.keyPressed >= 256) {
				//if (captureAllKeys) {
				tempString = copyString("ABCDEF");
				sprintf(tempString, "%i", input.keyPressed);
				//}
			} else {
				tempString = copyString(" ");
				tempString[0] = input.keyPressed;
			}
		}

		if (tempString) {
			variableStack *tempStack = new variableStack;
			if (!checkNew(tempStack))
				return false;
			initVarNew(tempStack->thisVar);
			makeTextVar(tempStack->thisVar, tempString);
			delete tempString;
			tempString = NULL;
			tempStack->next = NULL;
			if (!startNewFunctionNum(currentEvents->spaceFunction, 1, NULL,
					tempStack))
				return false;
		}
	}
	input.rightClick = false;
	input.leftClick = false;
	input.rightRelease = false;
	input.leftRelease = false;
	input.keyPressed = 0;
	lastRegion = overRegion;
	return runSludge();
}

} // End of namespace Sludge
