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

#ifndef MAGNETIC_DEFS_H
#define MAGNETIC_DEFS_H

#include "common/scummsys.h"
#include "glk/magnetic/magnetic_types.h"

namespace Glk {
namespace Magnetic {

/*****************************************************************************\
* Type definitions for Magnetic
*
* Note: When running into trouble please ensure that these types have the
*       correct number of bits on your system !!!
\*****************************************************************************/

typedef byte type8;
typedef int8 type8s;
typedef uint16 type16;
typedef int16 type16s;
typedef uint32 type32;
typedef int32 type32s;

/****************************************************************************\
* Compile time switches
\****************************************************************************/

/* Switch:  SAVEMEM
   Purpose: Magnetic loads a complete graphics file into memory by default.
            Setting this switch you tell Magnetic to load images on request
            (saving memory, wasting load time)

#define SAVEMEM
*/

/* Switch:  NO_ANIMATION
   Purpose: By default Magnetic plays animated graphics.
            Setting this switch to ignore animations, Magnetic shows the
            static parts of the images anyway!

#define NO_ANIMATION
*/

/****************************************************************************\
* Abstract functions
*
* Note: These functions MUST be implemented by each port of Magnetic!
\****************************************************************************/

/****************************************************************************\
* Function: ms_load_file
*
* Purpose: Load a save game file and restore game
*
* Parameters:   type8s* name            zero terminated string of filename
*                                       typed by player
*               type8*  ptr             pointer to data to save
*               type16 size             number of bytes to save
*
* Result: 0 is successful
*
* Note: You probably want to put in a file requester!
\****************************************************************************/

extern type8 ms_load_file(const char *name, type8 *ptr, type16 size);

/****************************************************************************\
* Function: ms_save_file
*
* Purpose: Save the current game to file
*
* Parameters:   type8s* name            zero terminated string of filename
*                                       typed by player
*               type8*  ptr             pointer to data to save
*               type16  size            number of bytes to save
*
* Result: 0 is successful
*
* Note: You probably want to put in a file requester!
\****************************************************************************/

extern type8 ms_save_file(const char *name, type8 *ptr, type16 size);

/****************************************************************************\
* Function: ms_statuschar
*
* Purpose: Output a single character to the status bar
*
* Parameters:   type8   c               character to be printed
*
* Note: All characters are printed as provided except for:
*       0x0A resets the x position to zero
*       0x09 moves the cursor to the right half of the bar, ie 'width-11'
\****************************************************************************/

extern void ms_statuschar(type8 c);

/****************************************************************************\
* Function: ms_putchar
*
* Purpose: Output a single character to the game/text windows
*
* Parameters:   type8   c               character to be printed
*
* Note: It is highly recommended to buffer the output, see also ms_flush()
\****************************************************************************/

extern void ms_putchar(type8 c);

/****************************************************************************\
* Function: ms_flush
*
* Purpose: Flush the output buffer (if applicable)
*
* Note: see also ms_putchar
\****************************************************************************/

extern void ms_flush(void);

/****************************************************************************\
* Function: ms_getchar
*
* Purpose: Read user input, buffered
*
* Parameters:   type8   trans           if not 0, translate any #undo
*                                       input to a return code of 0
*
* Return: One character
*
* Note: The first time it is called a string should be read and then given
*       back one byte at a time (ie. one for each call) until a '\n' is
*       reached (which will be the last byte sent back before it all restarts)
*       Returning a zero means 'undo' and the rest of the line must then be
*       ignored.
*       Returning 1 means that the opcode should return immediately. This is
*       needed to prevent possible corruption of the game's memory in
*       interpreters which allow a new game to be loaded without restarting.
\****************************************************************************/

extern type8 ms_getchar(type8 trans);

/****************************************************************************\
* Function: ms_showpic
*
* Purpose: Displays or hides a picture
*
* Parameter:    type32  c       number of image to be displayed
*               type8   mode    mode == 0 means gfx off,
*                               mode == 1 gfx on thumbnails,
*                               mode == 2 gfx on normal.
*
* Note: For retrieving the raw data of a picture call ms_extract (see below)
\****************************************************************************/

extern void ms_showpic(type32 c, type8 mode);

/****************************************************************************\
* Function: ms_fatal
*
* Purpose: Handle fatal interpreter error
*
* Parameter:    type8s* txt     message
\****************************************************************************/

extern void ms_fatal(const char *txt);

/****************************************************************************\
* Magnetic core functions
*
* Note: These functions SHOULD be used somewhere in your port!
\****************************************************************************/

/****************************************************************************\
* Function: ms_extract
*
* Purpose: Extract a picture and return a pointer to a raw bitmap
*
* Parameters:   type32  c               number of the picture
*               type16* w               width of picture
*               type16* h               height pf picture
*               type16* pal             array for the palette (16 colours)
*               type8*  is_anim         1 if animated picture, otherwise 0
*                                       OR null (!)
*
* Return: Pointer to bitmap data if successful, otherwise null (also if gfx
*         are disabled!)
*
* Note: All pictures have 16 colours and the palette entries use 3-bit RGB
*       encoded as 00000RRR0GGG0BBB, that is, bits 0-2 give the blue
*       component, bits 4-6 the green and bits 8-10 the red. The bitmap is
*       one byte per pixel, with each byte giving the pixel's index into the
*       palette. The data is given starting from the upper left corner. The
*       image buffer is reused when the next picture is requested, so take
*       care! More information on animated pictures are below!
\****************************************************************************/

extern type8 *ms_extract(type32 c, type16 *w, type16 *h, type16 *pal, type8 *is_anim);

/****************************************************************************\
* Magnetic animated pictures support
*
* Note: Some of the pictures for Wonderland and the Collection Volume 1 games
* are animations. To detect these, pass a pointer to a type8 as the is_anim
* argument to ms_extract().
*
* There are two types of animated images, however almost all images are type1.
* A type1 image consists of four main elements:
* 1) A static picture which is loaded straight at the beginning
* 2) A set of frames with a mask. These frames are just "small pictures", which
*    are coded like the normal static pictures. The image mask determines
*    how the frame is removed after it has been displayed. A mask is exactly
*    1/8 the size of the image and holds 1 bit per pixel, saying "remove pixel"
*    or leave pixel set when frame gets removed. It might be a good idea to check
*    your system documentation for masking operations as your system might be
*    able to use this mask data directly.
* 3) Positioning tables. These hold animation sequences consisting of commands
*    like "Draw frame 12 at (123,456)"
* 4) A playback script, which determines how to use the positioning tables.
*    These scripts are handled inside Magnetic, so no need to worry about.
*    However, details can be found in the ms_animate() function.
*
* A type2 image is like a type1 image, but it does not have a static
* picture, nor does it have frame masking. It just consists of frames.
*
* How to support animations?
* After getting is_anim == 1 you should call ms_animate() immediately, and at
* regular intervals until ms_animate() returns 0. An appropriate interval
* between calls is about 100 milliseconds.
* Each call to ms_animate() will fill in the arguments with the address
* and size of an array of ms_position structures (see below), each of
* which holds an an animation frame number and x and y co-ordinates. To
* display the animation, decode all the animation frames (discussed below)
* from a single call to ms_animate() and display each one over the main picture.
* If your port does not support animations, define NO_ANIMATION.
\****************************************************************************/

/****************************************************************************\
* Function: ms_animate
*
* Purpose: Generate the next frame of an animation
*
* Parameters:   ms_position**   positions  array of ms_position structs
*               type16*         count      size of array
*
* Return: 1 if animation continues, 0 if animation is finfished
*
* Note: The positions array holds size ms_positions structures. BEFORE calling
*       ms_animate again, retrieve the frames for all the ms_positions
*       structures with ms_get_anim_frame and display each one on the static
*       main picture.
\****************************************************************************/

extern type8 ms_animate(struct ms_position **positions, type16 *count);

/****************************************************************************\
* Function: ms_get_anim_frame
*
* Purpose: Extracts the bitmap data of a single animation frame
*
* Parameters:   type16s   number        number of frame (see ms_position struct)
*               type16*   width         width of frame
*               type16*   height        height of frame
*               type8**   mask          pointer to masking data, might be NULL
*
* Return: 1 if animation continues, 0 if animation is finfished
*
* Note: The format of the frame is identical to the main pictures' returned by
*       ms_extract. The mask has one-bit-per-pixel, determing how to handle the
*       removal of the frame.
\****************************************************************************/

extern type8 *ms_get_anim_frame(type16s number, type16 *width, type16 *height, type8 **mask);

/****************************************************************************\
* Function: ms_anim_is_repeating
*
* Purpose: Detects whether an animation is repeating
*
* Return: True if repeating
\****************************************************************************/

extern type8 ms_anim_is_repeating(void);

/****************************************************************************\
* Magnetic Windows hint support
*
* The windowed Magnetic Scolls games included online hints. To add support
* for the hints to your magnetic port, you should implement the ms_showhints
* function. It retrieves a pointer to an array of ms_hint structs
* The root element is always hints[0]. The elcount determines the number
* of items in this topic. You probably want to display those in some kind
* of list interface. The content pointer points to the actual description of
* the items, separated by '\0' terminators. The nodetype is 1 if the items are
* "folders" and 2 if the items are hints. Hints should be displayed one after
* another. For "folder" items, the links array holds the index of the hint in
* the array which is to be displayed on selection. One hint block has exactly
* one type. The parent element determines the "back" target.
\****************************************************************************/

/****************************************************************************\
* Function: ms_showhints
* Purpose: Show the player a hint
*
* Parameters:   ms_hint* hints          pointer to array of ms_hint structs
*
* Return: 0 on error, 1 on success
\****************************************************************************/

extern type8 ms_showhints(struct ms_hint *hints);

/****************************************************************************\
* Magnetic Windows sound support
*
* Wonderland contains music scores that are played when entering specific
* locations in the game. The music data are actually MIDI events and can be
* played through normal MIDI devices. The original game plays the MIDI score
* until the end, even if the location is changed while playing. The playback
* tempo is not included with the MIDI data. The ms_sndextract function
* returns a recommended tempo, however depending on the MIDI implementation
* and operating system, you might need to adapt it.
\****************************************************************************/

/****************************************************************************\
* Function: ms_playmusic
*
* Purpose: Plays (or stops playing) a MIDI music score.
*
* Parameter:    type8 * midi_data       the MIDI data to play
*               type32  length          the length of the MIDI data
*               type16  tempo           the suggested tempo for playing
*
* Note: If midi_data is NULL, all that should happen is that any currently
* playing music is stopped.
* Note: The data returned contain a complete MIDI file header, so if pure
*       memory processing is not applicable you can write the data to a
*       temporary file and use external players or libraries.
\****************************************************************************/

extern void ms_playmusic(type8 *midi_data, type32 length, type16 tempo);

/****************************************************************************\
* Function: ms_init
*
* Purpose: Loads the interpreter with a game
*
* Parameters:   type8s* name            filename of story file
*               type8s* gfxname         filename of graphics file (optional)
*               type8s* hntname         filename of hints file (optional)
*               type8s* sndname         filename of music file (optional)
*
* Return:       0 = failure
*               1 = success (without graphics or graphics failed)
*               2 = success (with graphics)
*
* Note: You must call this function before starting the ms_rungame loop
\****************************************************************************/

extern type8 ms_init(const char *name, const char *gfxname, const char *hntname, const char *sndname);

/****************************************************************************\
* Function: ms_rungame
*
* Purpose: Executes an interpreter instruction
*
* Return: True if successful
*
* Note: You must call this function in a loop like this:
*       while (running) {running=ms_rungame();}
\****************************************************************************/

extern type8 ms_rungame(void);

/****************************************************************************\
* Function: ms_freemen
*
* Purpose: Frees all allocated ressources
\****************************************************************************/

extern void ms_freemem(void);

/****************************************************************************\
* Function: ms_seed
*
* Purpose: Initializes the interpreter's random number generator with
*          the given seed
*
* Parameter:    type32  seed    seed
\****************************************************************************/

extern void ms_seed(type32 seed);

/****************************************************************************\
* Function: ms_is_running
*
* Purpose: Detects if game is running
*
* Return: True, if game is currently running
\****************************************************************************/

extern type8 ms_is_running(void);

/****************************************************************************\
* Function: ms_is_magwin
*
* Purpose: Detects Magnetic Windows games (Wonderland, Collection)
*
* Return: True, if Magnetic Windows game
\****************************************************************************/

extern type8 ms_is_magwin(void);

/****************************************************************************\
* Function: ms_stop
*
* Purpose: Stops further processing of opcodes
\****************************************************************************/

extern void ms_stop(void);

/****************************************************************************\
* Function: ms_status
*
* Purpose: Dumps interperetr state to stderr, ie. registers
\****************************************************************************/

extern void ms_status(void);

/****************************************************************************\
* Function: ms_count
*
* Purpose: Returns the number of executed intructions
*
* Return:  Instruction count
\****************************************************************************/

extern type32 ms_count(void);

extern void write(const char *fmt, ...);

extern void writeChar(char c);

extern int gms_startup_code(int argc, char *argv[]);

extern void gms_main();

} // End of namespace Magnetic
} // End of namespace Glk

#endif
