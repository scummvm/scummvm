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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/backends/platform/psp/osys_psp.cpp $
 * $Id: osys_psp.cpp 43618 2009-08-21 22:44:49Z joostp $
 *
 */

//#define PSP_KB_SHELL	/* Need a hack to properly load the keyboard from the PSP shell */
#ifdef PSP_KB_SHELL
#define PSP_KB_SHELL_PATH 	"ms0:/psp/game4xx/scummvm-1.0.0rc1/"	/* path to kbd.zip */
#endif
//#define PSP_KB_DEBUG
 
#include <malloc.h>
#include "pspkernel.h"
#include <pspgu.h>
#include "png.h"
#include "pspkeyboard.h"
#include "common/keyboard.h"
#include "common/fs.h"
#include "common/unzip.h"

#define K(x)	((short)(Common::KEYCODE_INVALID + (x)))
#define C(x)	((short)(Common::KEYCODE_##x))

// Layout of the keyboard: Order for the boxes is clockwise and then middle: 
//			1						
//		4	5	2				
//			3
// and order of letters is clockwise in each box, plus 2 top letters:
//		e		f	
//			a
//		d		b
//			c
// K(x) is used for ascii values. C(x) is used for keys without ascii values
short PSPKeyboard::_modeChar[MODE_COUNT][5][6] = {
	{	//standard letters
		{ K('a'),  K('b'), K('c'), K('d'), K('f'), K('g') }, 
		{ K('h'),  K('i'), K('l'), K('m'), K('j'), K('k') },
		{ K('o'),  K('n'), K('r'), K('s'), K('p'), K('q') },
		{ K('u'),  K('v'), K('w'), K('y'), K('x'), K('z') },
		{ K('\b'), K('t'), K(' '), K('e'), K(0),   K(0)   }
	},
	{	//capital letters
		{ K('A'),  K('B'), K('C'), K('D'), K('F'), K('G') }, 
		{ K('H'),  K('I'), K('L'), K('M'), K('J'), K('K') },
		{ K('O'),  K('N'), K('R'), K('S'), K('P'), K('Q') },
		{ K('U'),  K('V'), K('W'), K('Y'), K('X'), K('Z') },
		{ K('\b'), K('T'), K(' '), K('E'), K(0),   K(0)   }	
	},
	{	//numbers
		{ K('1'),  K('2'), K('3'), K('4'), K(0),   K(0)   }, 
		{ C(F5),   C(F8),  C(F7),  C(F6),  C(F9),  C(F10) },
		{ K('5'),  K('6'), K('7'), K('8'), K(0),   K(0)   },
		{ C(F1),   C(F4),  C(F3),  C(F2),  K(0),   K(0)   },
		{ K('\b'), K('0'), K(' '), K('9'), K(0),   K(0)   }
	},
	{	//symbols
		{ K('!'),  K(')'), K('?'), K('('), K('<'), K('>') }, 
		{ K('+'),  K('/'), K('='), K('\\'),K('\''),K('"') },
		{ K(':'),  K(']'), K(';'), K('['), K('@'), K('#') },
		{ K('-'),  K('}'), K('_'), K('{'), K('*'), K('$') },
		{ K('\b'), K('.'), K(' '), K(','), K(0),   K(0)   }		
	}
};

// Read function for png library to be able to read from our SeekableReadStream
//
void pngReadStreamRead(png_structp png_ptr, png_bytep data, png_size_t length) {
	Common::SeekableReadStream *file;
	
	file = (Common::SeekableReadStream *)png_ptr->io_ptr;
	
	file->read(data, length);
}

// Constructor
PSPKeyboard::PSPKeyboard() {
	_init = false;			// we're not initialized yet
	_prevButtons = 0;		// Reset previous buttons
	_dirty = true;        	// keyboard needs redrawing
	_mode = 0;              // charset selected. (0 - letters, 1 - uppercase 2 - numbers 3 - symbols)
	_oldCursor = kCenter;	// Center cursor by default
	_moved_x = 20;			// Default starting location
	_moved_y = 50;
	_moved = false;			// Keyboard wasn't moved recently
	_state = kInvisible;	// We start invisible
	_lastState = kInvisible;
}

// Destructor
PSPKeyboard::~PSPKeyboard() {
	if (!_init) return;

	int a;
	for (a = 0; a < guiStringsSize; a++) {
		free(_keyTextures[a].texture);
		free(_keyTextures[a].palette);
		_keyTextures[a].texture = NULL;
		_keyTextures[a].palette = NULL;
	}
	_init = false;
}

// Array with file names
const char *PSPKeyboard::_guiStrings[] = {
	"keys4.png", "keys_s4.png",
	"keys_c4.png", "keys_s_c4.png",
	"nums4.png", "nums_s4.png",
	"syms4.png", "syms_s4.png"	
};

// Defines for working with PSP buttons
#define CHANGED(x)	 (buttonsChanged & (x))
#define PRESSED(x)   ((buttonsChanged & (x)) && (pad.Buttons & (x)))
#define UNPRESSED(x) ((buttonsChanged & (x)) && !(pad.Buttons & (x)))
#define DOWN(x)		 (pad.Buttons & (x))
#define UP(x)		 (!(pad.Buttons & (x)))
#define PSP_DPAD	 (PSP_CTRL_DOWN|PSP_CTRL_UP|PSP_CTRL_LEFT|PSP_CTRL_RIGHT)
#define PSP_4BUTTONS (PSP_CTRL_CROSS | PSP_CTRL_CIRCLE | PSP_CTRL_TRIANGLE | PSP_CTRL_SQUARE)

/* 
 *  Attempts to read a character from the controller
 *  Uses the state machine.
 */
bool PSPKeyboard::processInput(Common::Event &event, SceCtrlData &pad, bool &usedInput) {
	usedInput = false;			// Assume we don't use an input
	bool haveEvent = false;		// Whether we have an event for the event manager to process
	event.kbd.flags = 0;
	unsigned int keyDown;
	uint32 buttonsChanged = _prevButtons ^ pad.Buttons;
	
	if (!_init)					// In case we never had init
		goto END;
			
	if (PRESSED(PSP_CTRL_SELECT)) {
		_lastState = _state;
		_state = kMove;			// Check for move or visible state
		usedInput = true;
		goto END;
	}
	
	if (_state == kInvisible)	// Return if we're invisible
		goto END;
	
	if (DOWN(PSP_DPAD | PSP_4BUTTONS | PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER | PSP_CTRL_START))
		usedInput = true;		// for now, we neutralize all button inputs
	
	// Check for moving the keyboard onscreen
	if (_state == kMove) {
		if (UNPRESSED(PSP_CTRL_SELECT)) {
			_state = (_lastState == kInvisible) ? kDefault : kInvisible;			// Back to previous state
			_dirty = true;			
			
			if (_moved) {				// We moved the keyboard. Keep the keyboard onscreen
				_state = kDefault;
				_moved = false;			// reset moved flag
			}
		}
		else if (DOWN(PSP_DPAD)) {
			_moved = true;
			_dirty = true;			
			
			if (DOWN(PSP_CTRL_DOWN)) 
				_moved_y += 5;
			else if (DOWN(PSP_CTRL_UP)) 
				_moved_y -= 5;
			else if (DOWN(PSP_CTRL_LEFT)) 
				_moved_x -= 5;
			else  /* DOWN(PSP_CTRL_RIGHT) */ 
				_moved_x += 5;
		}
		usedInput = true;	// We used up the input (select was held down)
		goto END;
	}		
	

	// Handle 4 buttons + 2 triggers
	if (_state == kDefault || _state == kCornersSelected) {
		unsigned int changed;
		
		if (_state == kDefault) {	// Handle default state
			changed = CHANGED(PSP_4BUTTONS);			// We only care about the 4 buttons
			if (PRESSED(PSP_CTRL_LTRIGGER)) {			// Don't say we used up the input
				_state = kLTriggerDown;
				goto END;
			}
			else if (PRESSED(PSP_CTRL_RTRIGGER)) {		// Don't say we used up the input
				_state = kRTriggerDown;
				goto END;
			}	
			
			if (DOWN(PSP_4BUTTONS))
				usedInput = true;						// Make sure these button presses don't get through
		}
		else { /* _state == kCornersSelected */
			// We care about 4 buttons + triggers (for letter selection)
			changed = CHANGED(PSP_4BUTTONS | PSP_CTRL_RTRIGGER | PSP_CTRL_LTRIGGER);

			if (DOWN(PSP_4BUTTONS | PSP_CTRL_RTRIGGER | PSP_CTRL_LTRIGGER))
				usedInput = true;	// Make sure these button presses don't get through
		}
	
		if (changed) { //pressing a char select button -- both states
		
			int innerChoice;
		
			if (UNPRESSED(PSP_CTRL_TRIANGLE)) {
				innerChoice = 0;
				event.type = Common::EVENT_KEYUP;			// We give priority to key_up
			}
			else if (UNPRESSED(PSP_CTRL_CIRCLE)) {
				innerChoice = 1;
				event.type = Common::EVENT_KEYUP;			// We give priority to key_up
			}
			else if (UNPRESSED(PSP_CTRL_CROSS)) {
				innerChoice = 2;
				event.type = Common::EVENT_KEYUP;			// We give priority to key_up
			}
			else if (UNPRESSED(PSP_CTRL_SQUARE)) {
				innerChoice = 3;
				event.type = Common::EVENT_KEYUP;			// We give priority to key_up
			}
			else if (UNPRESSED(PSP_CTRL_LTRIGGER) && _state == kCornersSelected) {
				innerChoice = 4;
				event.type = Common::EVENT_KEYUP;			// We give priority to key_up
			}		
			else if (UNPRESSED(PSP_CTRL_RTRIGGER) && _state == kCornersSelected) {
				innerChoice = 5;
				event.type = Common::EVENT_KEYUP;			// We give priority to key_up
			}				
			else if (PRESSED(PSP_CTRL_TRIANGLE)) {
				innerChoice = 0;
				event.type = Common::EVENT_KEYDOWN;
			}
			else if (PRESSED(PSP_CTRL_CIRCLE)) {
				innerChoice = 1;				
				event.type = Common::EVENT_KEYDOWN;
			}
			else if (PRESSED(PSP_CTRL_CROSS)) {
				innerChoice = 2;
				event.type = Common::EVENT_KEYDOWN;
			}
			else if (PRESSED(PSP_CTRL_SQUARE)) {
				innerChoice = 3;			
				event.type = Common::EVENT_KEYDOWN;
			}
			else if (PRESSED(PSP_CTRL_LTRIGGER) && _state == kCornersSelected) {
				innerChoice = 4;
				event.type = Common::EVENT_KEYDOWN;			// We give priority to key_up
			}		
			else  /* (UNPRESSED(PSP_CTRL_RTRIGGER)) && _state == kCornersSelected */ {
				innerChoice = 5;
				event.type = Common::EVENT_KEYDOWN;			// We give priority to key_up
			}						

			#define IS_UPPERCASE(x)	((x) >= (unsigned short)'A' && (x) <= (unsigned short)'Z')
			#define TO_LOWER(x)		((x) += 'a'-'A')
		
			//Now grab the value out of the array
			short choice = _modeChar[_mode][_oldCursor][innerChoice];
		
			event.kbd.ascii = choice <= 255 ? choice : 0;
		
			// Handle upper-case which is missing in Common::KeyCode
			if (IS_UPPERCASE(choice)) {
				event.kbd.keycode = (Common::KeyCode) TO_LOWER(choice);
				event.kbd.flags = Common::KBD_SHIFT;
			}
			else
				event.kbd.keycode = (Common::KeyCode) choice;

			haveEvent = (choice != Common::KEYCODE_INVALID) ? true : false;	// We have an event/don't if it's invalid
			usedInput = true;
		}
	}

	// Check for movement of cursor
	if (_state == kDefault || _state == kCornersSelected) {	
		// Check if a dependent button is down
		if (_state == kDefault)
			keyDown = DOWN(PSP_4BUTTONS);
		else /* if (_state == kCornersSelected) */
			keyDown = DOWN(PSP_4BUTTONS | PSP_CTRL_RTRIGGER | PSP_CTRL_LTRIGGER);

		Cursor cursor = _oldCursor;
		
		// Find where the cursor is pointing
		if (keyDown == false) {	// Don't allow movement if a nub-dependent key is down
			cursor = kCenter;
			_state = kDefault;

			if (DOWN(PSP_DPAD)) {
				_state = kCornersSelected;
				usedInput = true;	// Make sure the pressed d-pad is used up
				
				if (DOWN(PSP_CTRL_UP))
					cursor = kUp;
				else if (DOWN(PSP_CTRL_RIGHT))
					cursor = kRight;
				else if (DOWN(PSP_CTRL_DOWN)) 
					cursor = kDown;
				else if (DOWN(PSP_CTRL_LEFT))
					cursor = kLeft;
			}
		}

		if (cursor != _oldCursor) { //If we've moved, update dirty and return
			_dirty = true;
			_oldCursor = cursor;
			usedInput = true;		// We 'used up' the input
			//goto END;				// We didn't find an event
		}			
	}
	
	// Deal with trigger states
	if (_state == kRTriggerDown) {
		usedInput = true;
		
		if (UNPRESSED(PSP_CTRL_RTRIGGER))	{
			_dirty = true;

			if(_mode > 1) 
				_mode = 0;
			else 
				_mode = (_mode == 0) ? 1 : 0;
				
			usedInput = true;
			_state = kDefault;

			goto END;
		}
	}
	else if (_state == kLTriggerDown) {
		usedInput = true;
		
		if (UNPRESSED(PSP_CTRL_LTRIGGER))	{
			_dirty = true;

			if(_mode < 2) 
				_mode = 2;
			else 
				_mode = (_mode == 2) ? 3 : 2;
				
			usedInput = true;
			_state = kDefault;
			
			goto END;
		}
	}	
	
	// Handle start button: enter plus make keyboard invisible
	if (CHANGED(PSP_CTRL_START)) {
		event.kbd.ascii = '\n';
		event.kbd.keycode = Common::KEYCODE_RETURN;
		event.type = DOWN(PSP_CTRL_START) ? Common::EVENT_KEYDOWN : Common::EVENT_KEYUP;
		usedInput = true;					// Don't let start get through
		haveEvent = true;
		if (UP(PSP_CTRL_START))
			_state = kInvisible;			// Make us invisible if unpressed
	}	

END:	
	_prevButtons = pad.Buttons;
	return haveEvent;
}


/* move the position the keyboard is currently drawn at */
void PSPKeyboard::moveTo(const int newX, const int newY) {
	_moved_x = newX;
	_moved_y = newY;
}

/* draw the keyboard at the current position */
void PSPKeyboard::render() {
	_dirty = false;
	int x, y;
	
	switch(_oldCursor) {
		case kUp:
			x = 1;
			y = 0;
			break;
		case kRight:
			x = 2;
			y = 1;
			break;
		case kDown:
			x = 1;
			y = 2;
			break;
		case kLeft:
			x = 0;
			y = 1;
			break;
		default:
			x = 1;
			y = 1;
			break;
	}
		
	// Draw the background letters		
	surface_draw_offset(&_keyTextures[_mode<<1], 0, 0, 0, 0, 
			_keyTextures[_mode<<1].texture_width,
			_keyTextures[_mode<<1].texture_height);
			
	// Draw the current Highlighted Selector (orange bit)
	surface_draw_offset(&_keyTextures[(_mode<<1) + 1],
			//Offset from the current draw position to render at
			x * 43, y * 43,
			//internal offset of the image
			x * 64, y * 64,
			//size to render (always the same)
			64, 64);
}


//  Render the given surface at the current screen position offset by screenX, screenY
//  the surface will be internally offset by offsetX,offsetY. And the size of it to be drawn will be intWidth,intHeight
void PSPKeyboard::surface_draw_offset(struct gu_surface* surface, int screenX, int screenY, int offsetX, int offsetY, int intWidth, int intHeight) {
	sceGuAlphaFunc( GU_GREATER, 0, 0xff );
	sceGuEnable( GU_ALPHA_TEST );
	sceGuTexFunc(GU_TFX_BLEND,GU_TCC_RGBA);
	sceGuTexEnvColor(0xFF000000);
	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0 );
	sceGuEnable(GU_BLEND);
	if (surface->paletteSize == 256) {	// 8-bit
		sceGuClutMode(GU_PSM_8888, 0, 0xFF, 0);
		sceGuClutLoad(32, surface->palette); // upload 32*8 entries (256)
		sceGuTexMode(GU_PSM_T8, 0, 0, 0);   // 8-bit image
	} else if (surface->paletteSize == 16) {	// 4-bit
		sceGuClutMode(GU_PSM_8888, 0, 0x0F, 0);
		sceGuClutLoad(2, surface->palette); // upload 2*8 entries (16)
		sceGuTexMode(GU_PSM_T4, 0, 0, 0);   // 4-bit image
	} else {	// 32-bit
		sceGuTexMode(GU_PSM_8888,0,0,GU_FALSE);
	}
	sceGuTexImage(0,surface->surface_width, surface->surface_height,surface->surface_width, surface->texture);
	sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA);

	Vertex* c_vertices = (Vertex*)sceGuGetMemory(2 * sizeof(Vertex));

	c_vertices[0].u 		= offsetX;
	c_vertices[0].v 		= offsetY;
	c_vertices[0].x 		= _moved_x + screenX;
	c_vertices[0].y 		= _moved_y + screenY;
	c_vertices[0].z 		= 0;
	c_vertices[0].color 	= 0xFFFFFFFF;

	c_vertices[1].u 		= offsetX + intWidth;
	c_vertices[1].v 		= offsetY + intHeight;
	c_vertices[1].x 		= _moved_x + screenX + intWidth;
	c_vertices[1].y 		= _moved_y + screenY + intHeight;
	c_vertices[1].z 		= 0;
	c_vertices[1].color 	= 0xFFFFFFFF;

	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,c_vertices);

	sceGuDisable( GU_BLEND );
	sceGuDisable( GU_ALPHA_TEST );
}

/* load all the guibits that make up the OSK */
bool PSPKeyboard::load() {
	unsigned char *temp_texture = NULL;
	uint32 *temp_palette = NULL;
	int a;

	if (_init) 
		return true;

	// For the shell, we must use a hack
#ifdef PSP_KB_SHELL
	Common::FSNode node(PSP_KB_SHELL_PATH);
#else /* normal mode */		
	Common::FSNode node(".");				// Look in current directory
#endif	
#ifdef PSP_KB_DEBUG
	fprintf(stderr, "path = %s\n", node.getPath().c_str());
#endif
	
	Common::Archive *fileArchive = NULL;
	Common::Archive *zipArchive = NULL;
	Common::SeekableReadStream * file = 0;
	
	if (node.getChild("kbd").exists() && node.getChild("kbd").isDirectory()) {
#ifdef PSP_KB_DEBUG
		fprintf(stderr, "found directory ./kbd\n");
#endif		
		fileArchive = new Common::FSDirectory(node.getChild("kbd"));
	}
	if (node.getChild("kbd.zip").exists()) {
#ifdef PSP_KB_DEBUG
		fprintf(stderr, "found kbd.zip\n");
#endif		
		zipArchive  = new Common::ZipArchive(node.getChild("kbd.zip"));
	}
	
	// Loop through different png images
	for (a = 0; a < guiStringsSize; a++) {
		uint32 height = 0, width = 0, paletteSize = 0;

#ifdef PSP_KB_DEBUG		
		fprintf(stderr, "load(): Opening %s.\n", _guiStrings[a]);
#endif		
		
		// Look for the file in the kbd directory
		if (fileArchive && fileArchive->hasFile(_guiStrings[a])) {

#ifdef PSP_KB_DEBUG		
			fprintf(stderr, "load(): found it in kbd directory.\n");
#endif			
			
			file = fileArchive->createReadStreamForMember(_guiStrings[a]);
			if (!file) {
				fprintf(stderr, "load(): Can't open kbd/%s for keyboard. No keyboard will load.\n", _guiStrings[a]);
				goto ERROR;
			}
		}
		// We didn't find it. Look for it in the zip file
		else if (zipArchive && zipArchive->hasFile(_guiStrings[a])) {

#ifdef PSP_KB_DEBUG		
			fprintf(stderr, "load(): found it in kbd.zip.\n");		
#endif			
		
			file = zipArchive->createReadStreamForMember(_guiStrings[a]);
			if (!file) {	
				fprintf(stderr, "Can't open %s in kbd.zip for keyboard. No keyboard will load.\n", _guiStrings[a]);
				goto ERROR;
			}
		}
		else {	// Couldn't find the file
			fprintf(stderr, "load(): Can't find %s for keyboard. No keyboard will load.\n", _guiStrings[a]);
			goto ERROR;
		}
		
		if (get_png_image_size(file, &width, &height, &paletteSize) == 0) { // Check image size and palette size
			// Allocate memory for image
#ifdef PSP_KB_DEBUG
			fprintf(stderr, "load(): width=%d, height=%d, paletteSize=%d\n", width, height, paletteSize);
#endif			

			if (paletteSize) {	// 8 or 4-bit image
				uint32 textureSize = 0;
			
				if (paletteSize <= 16) { // 4 bit
					paletteSize = 16;
					textureSize = (width * height)>>1;
				}
				else if (paletteSize <= 256){			// 8-bit image
					paletteSize = 256;
					textureSize = width * height;
				}
				else {
					fprintf(stderr, "Error: palette of %d too big!\n", paletteSize);
					goto ERROR;
				}
#ifdef PSP_KB_DEBUG
				fprintf(stderr, "load(): allocating %d bytes for texture and %d for palette\n", textureSize, paletteSize*4);
#endif				
				temp_texture = (u8 *)malloc(textureSize);
				temp_palette = (uint32 *)memalign(16, paletteSize<<2);
				memset(temp_palette, 0, paletteSize<<2);	// Set to 0 since we might only fill some of it
			}
			else {				// 32-bit image
				temp_texture = (unsigned char *)malloc((width * height)<<2);
			}
			
			// Try to load the image
			file->seek(0);	// Go back to start
			
			if (load_png_image(file, temp_texture, temp_palette) != 0)
				goto ERROR;
			else {	// Success
#ifdef PSP_KB_DEBUG
				fprintf(stderr, "Managed to load the image.\n");
#endif				
				// we need to store the texture in an image of width and heights of 2^n sizes
				_keyTextures[a].texture_width 	= width;						// original size
				_keyTextures[a].texture_height 	= height;
				_keyTextures[a].surface_width 	= convert_pow2(width);		// converted size
				_keyTextures[a].surface_height 	= convert_pow2(height);
				_keyTextures[a].palette			= temp_palette;
				_keyTextures[a].paletteSize 	= paletteSize;
				
				uint32 size;
				
				if (paletteSize == 16) 			// 4 bit
					size = (_keyTextures[a].surface_width * _keyTextures[a].surface_height)>>1;
				else if (paletteSize == 256) 	// 8-bit
					size = _keyTextures[a].surface_width * _keyTextures[a].surface_height;					
				else 							// 32 bit
					size = (_keyTextures[a].surface_width * _keyTextures[a].surface_height)<<2;

#ifdef PSP_KB_DEBUG
				fprintf(stderr, "load(): perm texture width=%d, height=%d, size=%d\n", _keyTextures[a].surface_width, _keyTextures[a].surface_height, size);
#endif				
				_keyTextures[a].texture 	= (unsigned char *)memalign(16, size);	// Allocate memory
				
				block_copy(&_keyTextures[a], temp_texture);			// Copy temp texture to permanent texture

				if (paletteSize == 16)		// 4-bit
					flipNibbles(&_keyTextures[a]);
				
				free(temp_texture);
				
				delete(file);
			}
		}
		else 
			goto ERROR;
	} /* for loop */
	_init = true;
	
	delete(fileArchive);
	delete(zipArchive);
	
	return true;
	
ERROR:

	{
		// Error .. Couldn't get png info from one of the needed files
		free(temp_texture);
		delete(file);
		delete(fileArchive);
		delete(zipArchive);
	
		for (int b = 0; b < a; b++)
		{
			free(_keyTextures[b].texture);
			free(_keyTextures[b].palette);
			_keyTextures[b].texture = NULL;
		}
		_init = false;
		return false;
	}	
}

// Copy texture from regular size image to power of 2 size image
//
void PSPKeyboard::block_copy(gu_surface* surface, u8 *texture) {
	u32 stride = 0, width = 0;
	
	switch(surface->paletteSize) {
		case 16:	// 4-bit
			width = surface->texture_width >> 1;
			stride = (surface->surface_width - surface->texture_width)>>1;
			break;
		case 256:	// 8-bit
			width = surface->texture_width;
			stride = surface->surface_width - surface->texture_width;
			break;
		case 0:		// 32-bit
			width = surface->texture_width << 2;			
			stride = (surface->surface_width - surface->texture_width)<<2;
			break;
		default:
			fprintf(stderr, "Error in block_copy: bad value in paletteSize = %d\n", surface->paletteSize);
			return;
	}
	
	u8 *src  = texture;
	u8 *dest = surface->texture;	

	for (unsigned int y = 0 ; y < surface->texture_height ; y++) {
		memcpy(dest, src, width);
		dest += width;
		src += width;

		// skip at the end of each line
		if (stride > 0) {
			dest += stride;
		}
	}
}

// Convert 4 bit images to match weird PSP format
//
void PSPKeyboard::flipNibbles(gu_surface* surface) {
	u32 *dest = (u32 *)surface->texture;
	
	for (u32 y = 0 ; y < surface->texture_height ; y++) {
		for (u32 x = 0; x < (surface->surface_width >> 3); x++) {
			u32 val = *dest;
			*dest++ = ((val >> 4) & 0x0F0F0F0F) | ((val << 4) & 0xF0F0F0F0);
		}	
	}

}


static void user_warning_fn(png_structp png_ptr, png_const_charp warning_msg) {
	// ignore PNG warnings
}

/* Get the width and height of a png image */
int PSPKeyboard::get_png_image_size(Common::SeekableReadStream *file, uint32 *png_width, uint32 *png_height, u32 *paletteSize) {
	png_structp png_ptr;
	png_infop info_ptr;
	unsigned int sig_read = 0;
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		return -1;
	}
	png_set_error_fn(png_ptr, (png_voidp) NULL, (png_error_ptr) NULL, user_warning_fn);
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return -1;
	}
	// Set the png lib to use our read function
	png_set_read_fn(png_ptr, (void *)file, pngReadStreamRead);

	png_set_sig_bytes(png_ptr, sig_read);
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, int_p_NULL, int_p_NULL);
	if (color_type & PNG_COLOR_MASK_PALETTE) 
		*paletteSize = info_ptr->num_palette;
	else
		*paletteSize = 0;
	
	png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

	*png_width = width;
	*png_height = height;
	return 0;
}

// Load a texture from a png image
//
int PSPKeyboard::load_png_image(Common::SeekableReadStream *file, unsigned char *ImageBuffer, uint32 *palette) {
	png_structp png_ptr;
	png_infop info_ptr;
	unsigned int sig_read = 0;
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;
	size_t x, y;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		fprintf(stderr, "load_png_image(): Error: couldn't create read struct to load keyboard.\n");
		return -1;
	}
	// Use dummy error function
	png_set_error_fn(png_ptr, (png_voidp) NULL, (png_error_ptr) NULL, user_warning_fn);
	
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return -1;
	}
	
	// Set the png lib to use our read function
	png_set_read_fn(png_ptr, (void *)file, pngReadStreamRead);
	
	png_set_sig_bytes(png_ptr, sig_read);
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, int_p_NULL, int_p_NULL);

	// Strip off 16 bit channels. Not really needed but whatever
	png_set_strip_16(png_ptr);

	if (color_type == PNG_COLOR_TYPE_PALETTE) {
		// We copy the palette
		uint32 *dstPal = palette;
		png_colorp srcPal = info_ptr->palette;
		for(int i=0; i < info_ptr->num_palette; i++) {
			unsigned char alphaVal = (i < info_ptr->num_trans) ? info_ptr->trans[i] : 0xFF;	// Load alpha if it's there
			*dstPal++ = GU_ARGB(alphaVal, srcPal->red, srcPal->green, srcPal->blue);
			srcPal++;
		}

		unsigned char *line = (unsigned char*) malloc(info_ptr->rowbytes);
		if (!line) {
			png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
			return -1;
		}
		
		for (y = 0; y < height; y++) {
			png_read_row(png_ptr, line, png_bytep_NULL);
			memcpy(&ImageBuffer[y * info_ptr->rowbytes], line, info_ptr->rowbytes);
		}
		
		free(line);
	} else {	// Not a palettized image
		if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_gray_1_2_4_to_8(png_ptr);
		// Convert trans channel to alpha for 32 bits
		if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);
		png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);

		u32* line = (u32*) malloc(width<<2);
		if (!line) {
			png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
			return -1;
		}

		u32* Image = (u32 *)ImageBuffer;
		for (y = 0; y < height; y++) {
			png_read_row(png_ptr, (u8*) line, png_bytep_NULL);
			for (x = 0; x < width; x++)
			{
				Image[y*width + x] = line[x];
			}
		}
		
		free(line);
	}
	
	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
	
	return 0;
}

// Function to get the lowest power of 2 higher than our image size
//
uint32 PSPKeyboard::convert_pow2(uint32 size) {
	uint32 pow_counter = 0;

	for ( ; pow_counter < 32 ; pow_counter++) {
		// Find the first value which is higher
		if ((size >> pow_counter) == 0)	{
			// take already good values into account
			if (((uint32) 1 << pow_counter) != size)
				return ((uint32)1 << pow_counter);
			else
				return ((uint32)1 << (pow_counter-1));
		}
	}
	return 0;
}

