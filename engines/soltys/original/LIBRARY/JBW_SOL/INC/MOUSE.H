#ifndef		__MOUSE__
#define		__MOUSE__

#include	<jbw.h>


extern volatile Boolean MouseCursorWiped;	// mouse cursor inhibit flag
extern	Boolean	Mouse;				// mouse existence flag
extern	word	MouseDelay;			// mouse double click delay
extern	int	Buttons;			// number of mouse buttons
extern	int	MX, MY;				// mouse position on click




EC Boolean	MouseInit	(void);
EC Boolean	MouseTest	(void);		// Mouse cursor in window?
EC void		MouseCursor	(Boolean on);
EC int		MouseX		(void);
EC int		MouseY		(void);
EC void		MouseGoto	(int x, int y);
EC int		MousePressed	(int ButtonMask);
EC int		MousePCount	(int Button);
EC int		MouseRCount	(int Button);

#endif