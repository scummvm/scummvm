/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
/*
 Description:   
 
    Console module

 Notes: 
*/

#include "reinherit.h"

/*
   Uses the following modules:
\*--------------------------------------------------------------------------*/
#include "font_mod.h"
#include "cvar_mod.h"
#include "events_mod.h"
#include "gfx_mod.h"

/*
   Begin module
\*--------------------------------------------------------------------------*/
#include "console_mod.h"
#include "console.h"

namespace Saga {

static R_CONSOLEINFO ConInfo = {

	0,
	R_CON_DEFAULTPOS,
	R_CON_DEFAULTLINES,
	R_CON_DEFAULTCMDS
};

static char InputBuf[R_CON_INPUTBUF_LEN];

static R_CON_SCROLLBACK ConScrollback;
static R_CON_SCROLLBACK ConHistory;

static int CV_ConResize = R_CON_DEFAULTPOS;
static int CV_ConDroptime = R_CON_DROPTIME;

int CON_Register(void)
{

	CVAR_Register_I(&CV_ConResize, "con_h",
	    NULL, R_CVAR_NONE, 12, R_CON_DEFAULTPOS);

	CVAR_Register_I(&CV_ConDroptime, "con_droptime",
	    NULL, R_CVAR_NONE, 0, 5000);

	CVAR_Register_I(&ConInfo.line_max, "con_lines",
	    NULL, R_CVAR_NONE, 5, 5000);

	return R_SUCCESS;
}

int CON_Init(void)
{
	return R_SUCCESS;
}

int CON_Shutdown(void)
{

	R_printf(R_STDOUT,
	    "CON_Shutdown(): Deleting console scrollback and command history.\n");

	CON_DeleteScroll(&ConScrollback);
	CON_DeleteScroll(&ConHistory);

	return R_SUCCESS;
}

int CON_Activate(void)
{
	R_EVENT con_event;

	if (ConInfo.active) {
		return R_FAILURE;
	}

	con_event.type = R_CONTINUOUS_EVENT;
	con_event.code = R_CONSOLE_EVENT | R_NODESTROY;
	con_event.op = EVENT_ACTIVATE;
	con_event.time = 0;
	con_event.duration = CV_ConDroptime;

	EVENT_Queue(&con_event);

	ConInfo.active = 1;

	return R_SUCCESS;
}

int CON_Deactivate(void)
{
	R_EVENT con_event;

	if (!ConInfo.active) {
		return R_FAILURE;
	}

	con_event.type = R_CONTINUOUS_EVENT;
	con_event.code = R_CONSOLE_EVENT | R_NODESTROY;
	con_event.op = EVENT_DEACTIVATE;
	con_event.time = 0;
	con_event.duration = CV_ConDroptime;

	EVENT_Queue(&con_event);

	return R_SUCCESS;
}

int CON_IsActive(void)
{

	return ConInfo.active;
}

int CON_Type(int in_char)
/****************************************************************************\
 Responsible for processing character input to the console and maintaining
 the console input buffer.
 Input buffer is processed by EXPR_Parse on enter.
 High ASCII characters are ignored.
\****************************************************************************/
{

	int input_pos = ConInfo.input_pos;
	const char *expr;
	int expr_len;
	int result;
	/*char *lvalue; */

	char *rvalue = NULL;
	R_CVAR_P con_cvar = NULL;

	char *expr_err;
	char *err_str;

	if (ConInfo.y_pos != ConInfo.y_max) {
		/* Ignore keypress until console fully down */
		return R_SUCCESS;
	}

	if ((in_char > 127) || (!in_char)) {
		/* Ignore non-ascii codes */
		return R_SUCCESS;
	}

	switch (in_char) {

	case '\r':

		expr = ConInfo.input_buf;
		CON_Print("> %s", ConInfo.input_buf);

		expr_len = strlen(ConInfo.input_buf);
		result = EXPR_Parse(&expr, &expr_len, &con_cvar, &rvalue);

		CON_AddLine(&ConHistory, ConInfo.hist_max, ConInfo.input_buf);

		memset(ConInfo.input_buf, 0, R_CON_INPUTBUF_LEN);
		ConInfo.input_pos = 0;
		ConInfo.hist_pos = 0;
		if (result != R_SUCCESS) {
			EXPR_GetError(&expr_err);
			CON_Print("Parse error: %s", expr_err);
			break;
		}

		if (rvalue == NULL) {
			CVAR_Print(con_cvar);
			break;
		}

		if (CVAR_IsFunc(con_cvar)) {
			CVAR_Exec(con_cvar, rvalue);
		} else if (CVAR_SetValue(con_cvar, rvalue) != R_SUCCESS) {
			CVAR_GetError(&err_str);
			CON_Print("Illegal assignment: %s.", err_str);
		}

		break;

	case '\b':
		ConInfo.input_buf[input_pos] = 0;

		if (input_pos > 0) {
			ConInfo.input_pos--;
			ConInfo.input_buf[ConInfo.input_pos] = 0;
		}
		break;

	default:
		if (input_pos < R_CON_INPUTBUF_LEN) {
			ConInfo.input_buf[input_pos] = (char)in_char;
			ConInfo.input_pos++;
		}
		break;
	}

	if (rvalue)
		free(rvalue);

	return R_SUCCESS;
}

int CON_Draw(R_SURFACE * ds)
{

	int line_y;

	R_CONSOLE_LINE *walk_ptr;
	R_CONSOLE_LINE *start_ptr;

	int txt_fgcolor;
	int txt_shcolor;

	R_RECT fill_rect;

	int i;

	if (!ConInfo.active) {
		return R_FAILURE;
	}

	if (CV_ConResize != ConInfo.y_max) {
		ConInfo.y_max = CV_ConResize;
		ConInfo.y_pos = CV_ConResize;
	}

	fill_rect.x1 = 0;
	fill_rect.y1 = 0;

	fill_rect.x2 = ds->buf_w - 1;
	fill_rect.y2 = ConInfo.y_pos;

	GFX_DrawRect(ds, &fill_rect, SYSGFX_MatchColor(R_CONSOLE_BGCOLOR));

	txt_fgcolor = SYSGFX_MatchColor(R_CONSOLE_TXTCOLOR);
	txt_shcolor = SYSGFX_MatchColor(R_CONSOLE_TXTSHADOW);

	FONT_Draw(SMALL_FONT_ID,
	    ds,
	    ">", 1,
	    2, ConInfo.y_pos - 10, txt_fgcolor, txt_shcolor, FONT_SHADOW);

	FONT_Draw(SMALL_FONT_ID,
	    ds,
	    ConInfo.input_buf, strlen(ConInfo.input_buf),
	    10, ConInfo.y_pos - 10, txt_fgcolor, txt_shcolor, FONT_SHADOW);

	line_y = ConInfo.y_pos - (R_CON_INPUT_H + R_CON_LINE_H);

	start_ptr = ConScrollback.head;

	for (i = 0; i < ConInfo.line_pos; i++) {
		if (start_ptr->next) {
			start_ptr = start_ptr->next;
		} else {
			break;
		}
	}

	for (walk_ptr = start_ptr; walk_ptr; walk_ptr = walk_ptr->next) {

		FONT_Draw(SMALL_FONT_ID,
		    ds,
		    walk_ptr->str_p,
		    walk_ptr->str_len,
		    2, line_y, txt_fgcolor, txt_shcolor, FONT_SHADOW);

		line_y -= R_CON_LINE_H;

		if (line_y < -R_CON_LINE_H)
			break;
	}

	return R_SUCCESS;
}

int CON_Print(const char *fmt_str, ...)
{

	char vsstr_p[R_CON_PRINTFLIMIT + 1];
	va_list argptr;
	int ret_val;

	va_start(argptr, fmt_str);

	ret_val = vsprintf(vsstr_p, fmt_str, argptr);

	CON_AddLine(&ConScrollback, ConInfo.line_max, vsstr_p);

	va_end(argptr);

	ConInfo.line_pos = 0;

	return ret_val;
}

int CON_CmdUp(void)
{

	R_CONSOLE_LINE *start_ptr = ConHistory.head;
	int i;

	if (!start_ptr) {
		return R_SUCCESS;
	}

	if (ConInfo.hist_pos < ConHistory.lines) {
		ConInfo.hist_pos++;
	}

	for (i = 1; (i < ConInfo.hist_pos); i++) {
		if (start_ptr->next) {
			start_ptr = start_ptr->next;
		} else {
			break;
		}
	}

	memset(ConInfo.input_buf, 0, R_CON_INPUTBUF_LEN);
	strcpy(ConInfo.input_buf, start_ptr->str_p);
	ConInfo.input_pos = start_ptr->str_len - 1;

	R_printf(R_STDOUT, "History pos: %d/%d", ConInfo.hist_pos,
	    ConHistory.lines);

	return R_SUCCESS;
}

int CON_CmdDown(void)
{

	R_CONSOLE_LINE *start_ptr = ConHistory.head;
	int i;

	if (ConInfo.hist_pos == 1) {
		R_printf(R_STDOUT, "Erased input buffer.");
		memset(ConInfo.input_buf, 0, R_CON_INPUTBUF_LEN);
		ConInfo.input_pos = 0;
		ConInfo.hist_pos--;
		return R_SUCCESS;
	} else if (ConInfo.hist_pos) {
		ConInfo.hist_pos--;
	} else {
		return R_SUCCESS;
	}

	for (i = 1; i < ConInfo.hist_pos; i++) {
		if (start_ptr->next) {
			start_ptr = start_ptr->next;
		} else {
			break;
		}
	}

	memset(ConInfo.input_buf, 0, R_CON_INPUTBUF_LEN);
	strcpy(ConInfo.input_buf, start_ptr->str_p);
	ConInfo.input_pos = start_ptr->str_len - 1;

	R_printf(R_STDOUT, "History pos: %d/%d", ConInfo.hist_pos,
	    ConHistory.lines);

	return R_SUCCESS;
}

int CON_PageUp(void)
{

	int n_lines;

	n_lines = (ConInfo.y_max - R_CON_INPUT_H) / R_CON_LINE_H;

	if (ConInfo.line_pos < (ConScrollback.lines - n_lines)) {
		ConInfo.line_pos += n_lines;
	}

	R_printf(R_STDOUT, "Line pos: %d", ConInfo.line_pos);

	return R_SUCCESS;
}

int CON_PageDown(void)
{

	int n_lines;

	n_lines = (ConInfo.y_max - R_CON_INPUT_H) / R_CON_LINE_H;

	if (ConInfo.line_pos > n_lines) {
		ConInfo.line_pos -= n_lines;
	} else {
		ConInfo.line_pos = 0;
	}

	return R_SUCCESS;
}

int CON_DropConsole(double percent)
{

	R_SURFACE *back_buf;

	if (percent > 1.0) {
		percent = 1.0;
	}

	back_buf = SYSGFX_GetBackBuffer();
	CON_SetDropPos(percent);

	CON_Draw(back_buf);

	return R_SUCCESS;
}

int CON_RaiseConsole(double percent)
{

	R_SURFACE *back_buf;

	if (percent >= 1.0) {
		percent = 1.0;
		ConInfo.active = 0;
	}

	back_buf = SYSGFX_GetBackBuffer();

	CON_SetDropPos(1.0 - percent);

	CON_Draw(back_buf);

	return R_SUCCESS;
}

static int CON_SetDropPos(double percent)
{

	double exp_percent;

	if (percent > 1.0)
		percent = 1.0;
	if (percent < 0.0)
		percent = 0.0;

	exp_percent = percent * percent;

	ConInfo.y_pos = (int)(ConInfo.y_max * exp_percent);

	return R_SUCCESS;
}

static int
CON_AddLine(R_CON_SCROLLBACK * scroll, int line_max, const char *constr_p)
{

	int constr_len;
	char *newstr_p;
	R_CONSOLE_LINE *newline_p;
	int del_lines;
	int i;

	constr_len = strlen(constr_p) + 1;

	newstr_p = (char *)malloc(constr_len);
	if (newstr_p == NULL) {
		return R_MEM;
	}

	newline_p = (R_CONSOLE_LINE *)malloc(sizeof(R_CONSOLE_LINE));
	if (newline_p == NULL) {
		return R_MEM;
	}
	newline_p->next = NULL;
	newline_p->prev = NULL;

	strcpy(newstr_p, constr_p);
	newline_p->str_p = newstr_p;
	newline_p->str_len = constr_len;

	if (scroll->head == NULL) {
		scroll->head = newline_p;
		scroll->tail = newline_p;
	} else {
		scroll->head->prev = newline_p;
		newline_p->next = scroll->head;
		scroll->head = newline_p;
	}

	scroll->lines++;

	if (scroll->lines > line_max) {
		del_lines = scroll->lines - line_max;

		for (i = 0; i < del_lines; i++) {
			CON_DeleteLine(scroll);
		}
	}

	return R_SUCCESS;
}

static int CON_DeleteLine(R_CON_SCROLLBACK * scroll)
{

	R_CONSOLE_LINE *temp_p = scroll->tail;

	if (temp_p->prev == NULL) {
		scroll->head = NULL;
		scroll->tail = NULL;
	} else {
		temp_p->prev->next = NULL;
		scroll->tail = temp_p->prev;
	}

	if (temp_p->str_p)
		free(temp_p->str_p);
	free(temp_p);
	scroll->lines--;

	return R_SUCCESS;
}

static int CON_DeleteScroll(R_CON_SCROLLBACK * scroll)
{

	R_CONSOLE_LINE *walk_ptr;
	R_CONSOLE_LINE *temp_ptr;

	for (walk_ptr = scroll->head; walk_ptr; walk_ptr = temp_ptr) {

		if (walk_ptr->str_p)
			free(walk_ptr->str_p);
		temp_ptr = walk_ptr->next;
		free(walk_ptr);
	}

	return R_SUCCESS;
}

} // End of namespace Saga
