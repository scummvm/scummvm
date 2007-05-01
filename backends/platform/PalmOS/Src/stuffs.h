#ifndef STUFFS_H
#define STUFFS_H

enum {
	kOptNone				=	0,
	kOptDeviceARM			=	1 <<	0x00,
	kOptDeviceOS5			=	1 <<	0x01,
	kOptDeviceClie			=	1 <<	0x02,
	kOptDeviceZodiac		=	1 <<	0x03,
	kOptModeWide			=	1 <<	0x04,
	kOptModeLandscape		=	1 <<	0x05,
	kOptMode16Bit			=	1 <<	0x06,
	kOptModeHiDensity		=	1 <<	0x07,
	kOptCollapsible			=	1 <<	0x08,
	kOptDisableOnScrDisp	=	1 <<	0x09,
	kOpt5WayNavigatorV1		=	1 <<	0x0A,
	kOpt5WayNavigatorV2		=	1 <<	0x0B,
	kOptPalmSoundAPI		=	1 <<	0x0C,
	kOptSonyPa1LibAPI		=	1 <<	0x0D,
	kOptGoLcdAPI			=	1 <<	0x0E,
	kOptLightspeedAPI		=	1 <<	0x0F,
	kOptModeRotatable		=	1 <<	0x10,

	kOptDeviceProcX86		=	1 <<	0x1F	// DEBUG only
};

// WARNING : you need to define OPTIONS_DEF()
#define OPTIONS_TST(x)	(OPTIONS_DEF() & (x))
#define OPTIONS_SET(x)	OPTIONS_DEF() |= (x)
#define OPTIONS_RST(x)	OPTIONS_DEF() &= ~(x)

#endif
