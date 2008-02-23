/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# USB Keyboard Driver for PS2
*/

#ifndef __PS2KBD_H__
#define __PS2KBD_H__

#define PS2KBD_FSNAME "usbkbd"
#define PS2KBD_KBDFILE "dev"
#define PS2KBD_DEVFILE (PS2KBD_FSNAME ":" PS2KBD_KBDFILE)


#define PS2KBD_LED_NUMLOCK   1
#define PS2KBD_LED_CAPSLOCK  2
#define PS2KBD_LED_SCRLOCK   4
#define PS2KBD_LED_COMPOSE   8
#define PS2KBD_LED_KANA      16

#define PS2KBD_LED_MASK      0x1F;

#define PS2KBD_ESCAPE_KEY    0x1B

#define PS2KBD_LEFT_CTRL   (1 << 0)
#define PS2KBD_LEFT_SHIFT  (1 << 1)
#define PS2KBD_LEFT_ALT    (1 << 2)
#define PS2KBD_LEFT_GUI    (1 << 3)
#define PS2KBD_RIGHT_CTRL  (1 << 4)
#define PS2KBD_RIGHT_SHIFT (1 << 5)
#define PS2KBD_RIGHT_ALT   (1 << 6)
#define PS2KBD_RIGHT_GUI   (1 << 7)

#define PS2KBD_CTRL    (PS2KBD_LEFT_CTRL | PS2KBD_RIGHT_CTRL)
#define PS2KBD_SHIFT  (PS2KBD_LEFT_SHIFT | PS2KBD_RIGHT_SHIFT)
#define PS2KBD_ALT    (PS2KBD_LEFT_ALT | PS2KBD_RIGHT_ALT)
#define PS2KBD_GUI    (PS2KBD_LEFT_GUI | PS2KBD_RIGHT_GUI)

#define PS2KBD_RAWKEY_UP   0xF0
#define PS2KBD_RAWKEY_DOWN 0xF1

typedef struct _kbd_rawkey

{
  u8 state;
  u8 key;
} kbd_rawkey __attribute__ ((packed));

#define PS2KBD_READMODE_NORMAL 1
#define PS2KBD_READMODE_RAW    2

/* Notes on read mode */
/* In normal readmode (default) read multiples of 1 character off the keyboard file. These are 
   processed by the keymaps so that you get back ASCII data */
/* In raw readmode must read multiples of 2. First byte indicates state (i.e. Up or Down)
   Second byte is the USB key code for that key. This table is presented in the USB HID Usage Tables manaual
   from usb.org */

#define PS2KBD_BLOCKING 1
#define PS2KBD_NONBLOCKING 0

#define PS2KBD_KEYMAP_SIZE 256

typedef struct _kbd_keymap

{
  u8 keymap[PS2KBD_KEYMAP_SIZE];
  u8 shiftkeymap[PS2KBD_KEYMAP_SIZE];
  u8 keycap[PS2KBD_KEYMAP_SIZE];
} kbd_keymap;

/* IOCTLs for the keyboard file driver */

#define PS2KBD_IOCTL_SETREADMODE     1 /* Sets up keymapped or raw mode */
#define PS2KBD_IOCTL_SETLEDS         2 /* Sets the LED state for ALL keyboards connected */
#define PS2KBD_IOCTL_SETREPEATRATE   3 /* Sets the repeat rate of the keyboard */
#define PS2KBD_IOCTL_SETKEYMAP       4 /* Sets the keymap for the standard keys, non shifted and shifted */
#define PS2KBD_IOCTL_SETCTRLMAP      5 /* Sets the control key mapping */
#define PS2KBD_IOCTL_SETALTMAP       6 /* Sets the alt key mapping */
#define PS2KBD_IOCTL_SETSPECIALMAP   7 /* Sets the special key mapping */
#define PS2KBD_IOCTL_SETBLOCKMODE    8 /* Sets whether the keyboard driver blocks on read */
#define PS2KBD_IOCTL_FLUSHBUFFER     9 /* Flush the internal buffer, probably best after a keymap change */
#define PS2KBD_IOCTL_RESETKEYMAP    10 /* Reset keymaps to default states */

/* Note on keymaps. In normal keymap a 0 would indicate no key */
/* Key maps are represented by 3 256*8bit tables. First table maps USB key to a char when not shifted */
/* Second table maps USB key to a char when shifted */
/* Third table contains boolean values. If 1 then the key is shifted/unshifted in capslock, else capslock is ignored */

#endif
