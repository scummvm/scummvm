#ifndef	__INTREGS__
#define	__INTREGS__

#define		RegsPtr	((RegsOnStack far *) (((long) _SS << 16) | _SP))

typedef struct {word C: 1;
		      : 1;
		word P: 1;
		      : 1;
		word A: 1;
		      : 1;
		word Z: 1;
		word S: 1;
		word T: 1;
		word I: 1;
		word D: 1;
		word O: 1;	} Flags;

typedef struct { word BP, DI, SI, DS, ES,
		 DX, CX, BX, AX, IP, CS; Flags F; } RegsOnStack;

#endif