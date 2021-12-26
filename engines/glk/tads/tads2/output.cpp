/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "glk/tads/tads2/character_map.h"
#include "glk/tads/tads2/debug.h"
#include "glk/tads/tads2/error.h"
#include "glk/tads/tads2/memory_cache.h"
#include "glk/tads/tads2/os.h"
#include "glk/tads/tads2/run.h"
#include "glk/tads/tads2/text_io.h"
#include "glk/tads/tads2/vocabulary.h"
#include "glk/tads/os_glk.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

/* Forward declarations */
struct out_stream_info;

/*
 *   use our own isxxx - anything outside the US ASCII range is not reliably
 *   classifiable by the normal C isxxx routines
 */
#define outissp(c) (((uchar)(c)) <= 127 && Common::isSpace((uchar)(c)))
#define outisal(c) (((uchar)(c)) <= 127 && Common::isAlpha((uchar)(c)))
#define outisdg(c) (((uchar)(c)) <= 127 && Common::isDigit((uchar)(c)))
#define outisup(c) (((uchar)(c)) <= 127 && Common::isUpper((uchar)(c)))
#define outislo(c) (((uchar)(c)) <= 127 && Common::isLower((uchar)(c)))


/*
 *   Turn on formatter-level MORE mode, EXCEPT under any of the following
 *   conditions:
 *
 *   - this is a MAC OS port
 *.  - this is an HTML TADS interpreter
 *.  - USE_OS_LINEWRAP is defined
 *
 *   Formatter-level MORE mode and formatter-level line wrapping go together;
 *   you can't have one without the other.  So, if USE_OS_LINEWRAP is
 *   defined, we must also use OS-level MORE mode, which means we don't want
 *   formatter-level MORE mode.
 *
 *   For historical reasons, we check specifically for MAC_OS.  This was the
 *   first platform for which OS-level MORE mode and OS-level line wrapping
 *   were invented; at the time, we foolishly failed to anticipate that more
 *   platforms might eventually come along with the same needs, so we coded a
 *   test for MAC_OS rather than some more abstract marker.  For
 *   compatibility, we retain this specific test.
 *
 *   USE_OS_LINEWRAP is intended as the more abstract marker we should
 *   originally have used.  A port should #define USE_OS_LINEWRAP in its
 *   system-specific os_xxx.h header to turn on OS-level line wrapping and
 *   OS-level MORE mode.  Ports should avoid adding new #ifndef tests for
 *   specific platforms here; we've only retained the MAC_OS test because we
 *   don't want to break the existing MAC_OS port.
 */
#ifndef MAC_OS
# ifndef USE_HTML
#  ifndef USE_OS_LINEWRAP
#   define USE_MORE                   /* activate formatter-level more-mode */
#  endif /* USE_OS_LINEWRAP */
# endif /* USE_HTML */
#endif /* MAC_OS */

/*
 *   In HTML mode, don't use MORE mode.  Note that we explicitly turn MORE
 *   mode OFF, even though we won't have turned it on above, because it might
 *   have been turned on by an os_xxx.h header.  This is here for historical
 *   reasons; in particular, some of the HTML interpreter builds include
 *   headers that were originally written for the normal builds for those
 *   same platforms, and those original headers explicitly #define USE_MORE
 *   somewhere.  So, to be absolutely sure we get it right here, we have to
 *   explicitly turn off USE_MORE when compiling for HTML mode.
 */
#ifdef USE_HTML
# ifdef USE_MORE
#  undef USE_MORE
# endif
#endif

/*
 *   QSPACE is the special character for a quoted space (internally, the
 *   sequence "\ " (backslash-space) is converted to QSPACE).  It must not
 *   be any printable character.  The value here may need to be changed in
 *   the extremely unlikely event that TADS is ever ported to an EBCDIC
 *   machine.
 */
#define QSPACE 26

/*
 *   QTAB is a special hard tab character indicator.  We use this when we
 *   need to generate a hard tab to send to the underlying output layer
 *   (in particular, we use this to send hard tabs to the HTML formatter
 *   when we're in HTML mode).
 */
#define QTAB 25


/* maximum width of the display */
#define MAXWIDTH  OS_MAXWIDTH


/* ------------------------------------------------------------------------ */
/*
 *   Globals and statics.  These should really be moved into a context
 *   structure, so that the output formatter subsystem could be shared
 *   among multiple clients.  For now, there's no practical problem using
 *   statics, because we only need a single output subsystem at one time.
 */

/* current script (command input) file */
extern osfildef *scrfp;

/*
 *   This should be TRUE if the output should have two spaces after a
 *   period (or other such punctuation. It should generally be TRUE for
 *   fixed-width fonts, and FALSE for proportional fonts.
 */
static int doublespace = 1;

/*
 *   Log file handle and name.  If we're copying output to a log file,
 *   these will tell us about the file.
 */
osfildef *logfp;
static char logfname[OSFNMAX];

/* flag indicating whether output has occurred since last check */
static uchar outcnt;

/* flag indicating whether hidden output has occurred */
static uchar hidout;

/* flag indicating whether to show (TRUE) or hide (FALSE) output */
static uchar outflag;

/* flag indicating whether output is hidden for debugging purposes */
int dbghid;

/*
 *   Current recursion level in formatter invocation
 */
static int G_recurse = 0;

/* active stream in current recursion level */
static out_stream_info *G_cur_stream;

/* watchpoint mode flag */
static uchar outwxflag;

/*
 *   User filter function.  When this function is set, we'll invoke this
 *   function for each string that's displayed through the output
 *   formatter.
 */
static objnum G_user_filter = MCMONINV;


/* ------------------------------------------------------------------------ */
/*
 *   Hack to run with TADS 2.0 with minimal reworking.  Rather than using
 *   an allocated output layer context, store our subsystem context
 *   information in some statics.  This is less clean than using a real
 *   context, but doesn't create any practical problems as we don't need
 *   to share the output formatter subsystem among multiple simultaneous
 *   callers.
 */
static runcxdef *runctx;                               /* execution context */
static uchar    *fmsbase;                        /* format string area base */
static uchar    *fmstop;                          /* format string area top */
static objnum    cmdActor;                                 /* current actor */

/* forward declarations of static functions */
static void outstring_stream(out_stream_info *stream, const char *s);
static void outchar_noxlat_stream(out_stream_info *stream, char c);
static char out_parse_entity(char *outbuf, size_t outbuf_size, const char **sp, size_t *slenp);


/* ------------------------------------------------------------------------ */
/*
 *   HTML lexical analysis mode
 */
#define HTML_MODE_NORMAL  0                    /* normal text, not in a tag */
#define HTML_MODE_TAG     1                         /* parsing inside a tag */
#define HTML_MODE_SQUOTE  2           /* in a single-quoted string in a tag */
#define HTML_MODE_DQUOTE  3           /* in a double-quoted string in a tag */

/*
 *   HTML parsing mode flag for <BR> tags.  We defer these until we've
 *   read the full tag in order to obey an HEIGHT attribute we find.  When
 *   we encounter a <BR>, we figure out whether we think we'll need a
 *   flush or a blank line; if we find a HEIGHT attribute, we may change
 *   this opinion.
 */
#define HTML_DEFER_BR_NONE   0                           /* no pending <BR> */
#define HTML_DEFER_BR_FLUSH  1                   /* only need an outflush() */
#define HTML_DEFER_BR_BLANK  2                        /* need an outblank() */

/*
 *   If we're compiling for an HTML-enabled underlying output subsystem,
 *   we want to call the underlying OS layer when switching in and out of
 *   HTML mode.  If the underlying system doesn't process HTML, we don't
 *   need to let it know anything about HTML mode.
 */
#ifdef USE_HTML
# define out_start_html(stream) os_start_html()
# define out_end_html(stream)   os_end_html()
#else
# define out_start_html(stream)
# define out_end_html(stream)
#endif


/* ------------------------------------------------------------------------ */
/*
 *   Output formatter stream state structure.  This structure encapsulates
 *   the state of an individual output stream.
 */
struct out_stream_info {
	/* low-level display routine (va_list version) */
	void (*do_print)(out_stream_info *stream, const char *str);

	/* current line position and output column */
	uchar linepos;
	uchar linecol;

	/* number of lines on the screen (since last MORE prompt) */
	int linecnt;

	/* output buffer */
	char linebuf[MAXWIDTH];

	/*
	 *   attribute buffer - we keep one attribute entry for each character in
	 *   the line buffer
	 */
	int attrbuf[MAXWIDTH];

	/* current attribute for text we're buffering into linebuf */
	int cur_attr;

	/* last attribute we wrote to the osifc layer */
	int os_attr;

	/* CAPS mode - next character output is converted to upper-case */
	uchar capsflag;

	/* NOCAPS mode - next character output is converted to lower-case */
	uchar nocapsflag;

	/* ALLCAPS mode - all characters output are converted to upper-case */
	uchar allcapsflag;

	/* capture information */
	mcmcxdef *capture_ctx;           /* memory context to use for capturing */
	mcmon     capture_obj;                /* object holding captured output */
	uint      capture_ofs;                /* write offset in capture object */
	int       capturing;                 /* true -> we are capturing output */

	/* "preview" state for line flushing */
	int preview;

	/* flag indicating that we just flushed a new line */
	int just_did_nl;

	/* this output stream uses "MORE" mode */
	int use_more_mode;

	/*
	 *   This output stream uses OS-level line wrapping - if this is set,
	 *   the output formatter will not insert a newline at the end of a
	 *   line that it's flushing for word wrapping, but will instead let
	 *   the underlying OS display layer handle the wrapping.
	 */
	int os_line_wrap;

	/*
	 *   Flag indicating that the underlying output system wants to
	 *   receive its output as HTML.
	 *
	 *   If this is true, we'll pass through HTML to the underlying output
	 *   system, and in addition generate HTML sequences for certain
	 *   TADS-native escapes (for example, we'll convert the "\n" sequence
	 *   to a <BR> sequence).
	 *
	 *   If this is false, we'll do just the opposite: we'll remove HTML
	 *   from the output stream and convert it into normal text sequences.
	 */
	int html_target;

	/*
	 *   Flag indicating that the target uses plain text.  If this flag is
	 *   set, we won't add the OS escape codes for highlighted characters.
	 */
	int plain_text_target;

	/*
	 *   Flag indicating that the caller is displaying HTML.  We always
	 *   start off in text mode; the client can switch to HTML mode by
	 *   displaying a special escape sequence, and can switch back to text
	 *   mode by displaying a separate special escape sequence.
	 */
	int html_mode;

	/* current lexical analysis mode */
	unsigned int html_mode_flag;

	/* <BR> defer mode */
	unsigned int html_defer_br;

	/*
	 *   HTML "ignore" mode - we suppress all output when parsing the
	 *   contents of a <TITLE> or <ABOUTBOX> tag
	 */
	int html_in_ignore;

	/*
	 *   HTML <TITLE> mode - when we're in this mode, we're gathering the
	 *   title (i.e., we're inside a <TITLE> tag's contents).  We'll copy
	 *   characters to the title buffer rather than the normal output
	 *   buffer, and then call os_set_title() when we reach the </TITLE>
	 *   tag.
	 */
	int html_in_title;

	/* buffer for the title */
	char html_title_buf[256];

	/* pointer to next available character in title buffer */
	char *html_title_ptr;

	/* quoting level */
	int html_quote_level;

	/* PRE nesting level */
	int html_pre_level;

	/*
	 *   Parsing mode flag for ALT attributes.  If we're parsing a tag
	 *   that allows ALT, such as IMG or SOUND, we'll set this flag, then
	 *   insert the ALT text if we encounter it during parsing.
	 */
	int html_allow_alt;
};

/*
 *   Default output converter.  This is the output converter for the
 *   standard display.  Functions in the public interface that do not
 *   specify an output converter will use this converter by default.
 */
static out_stream_info G_std_disp;

/*
 *   Log file converter.  This is the output converter for a log file.
 *   Whenever we open a log file, we'll initialize this converter; as we
 *   display text to the main display, we'll also copy it to the log file.
 *
 *   We maintain an entire separate conversion context for the log file,
 *   so that we can perform a different set of conversions on it.  We may
 *   want, for example, to pass HTML text through to the OS display
 *   subsystem (this is the case for the HTML-enabled interpreter), but
 *   we'll still want to convert log file output to text.  By keeping a
 *   separate display context for the log file, we can format output to
 *   the log file using an entirely different style than we do for the
 *   display.
 */
static out_stream_info G_log_disp;


/* ------------------------------------------------------------------------ */
/*
 *   low-level output handlers for the standard display and log file
 */

/* standard display printer */
static void do_std_print(out_stream_info *stream, const char *str)
{
	VARUSED(stream);

	/* display the text through the OS layer */
	os_printz(str);
}

/* log file printer */
static void do_log_print(out_stream_info *stream, const char *str)
{
	VARUSED(stream);

	/* display to the log file */
	if (logfp != nullptr && G_os_moremode)
	{
		os_fprintz(logfp, str);
		osfflush(logfp);
	}
}


/* ------------------------------------------------------------------------ */
/*
 *   initialize a generic output formatter state structure
 */
static void out_state_init(out_stream_info *stream)
{
	/* start out at the first column */
	stream->linepos = 0;
	stream->linecol = 0;
	stream->linebuf[0] = '\0';

	/* set normal text attributes */
	stream->cur_attr = 0;
	stream->os_attr = 0;

	/* start out at the first line */
	stream->linecnt = 0;

	/* we're not in either "caps", "nocaps", or "allcaps" mode yet */
	stream->capsflag = stream->nocapsflag = stream->allcapsflag = FALSE;

	/* we're not capturing yet */
	stream->capturing = FALSE;
	stream->capture_obj = MCMONINV;

	/* we aren't previewing a line yet */
	stream->preview = 0;

	/* we haven't flushed a new line yet */
	stream->just_did_nl = FALSE;

	/* presume this stream does not use "MORE" mode */
	stream->use_more_mode = FALSE;

	/* presume this stream uses formatter-level line wrapping */
	stream->os_line_wrap = FALSE;

	/* assume that the underlying system is not HTML-enabled */
	stream->html_target = FALSE;

	/* presume this target accepts OS highlighting sequences */
	stream->plain_text_target = FALSE;

	/* start out in text mode */
	stream->html_mode = FALSE;

	/* start out in "normal" lexical state */
	stream->html_mode_flag = HTML_MODE_NORMAL;

	/* not in an ignored tag yet */
	stream->html_in_ignore = FALSE;

	/* not in title mode yet */
	stream->html_in_title = FALSE;

	/* not yet deferring line breaks */
	stream->html_defer_br = HTML_DEFER_BR_NONE;

	/* not yet in quotes */
	stream->html_quote_level = 0;

	/* not yet in a PRE block */
	stream->html_pre_level = 0;

	/* not in an ALT tag yet */
	stream->html_allow_alt = FALSE;
}


/* ------------------------------------------------------------------------ */
/*
 *   initialize a standard display stream
 */
static void out_init_std(out_stream_info *stream)
{
	/* there's no user output filter function yet */
	out_set_filter(MCMONINV);

	/* initialize the basic stream state */
	out_state_init(stream);

	/* set up the low-level output routine */
	G_std_disp.do_print = do_std_print;

#ifdef USE_MORE
	/*
	 *   We're compiled for MORE mode, and we're not compiling for an
	 *   underlying HTML formatting layer, so use MORE mode for the
	 *   standard display stream.
	 */
	stream->use_more_mode = TRUE;
#else
	/*
	 *   We're compiled for OS-layer (or HTML-layer) MORE handling.  For
	 *   this case, use OS-layer (or HTML-layer) line wrapping as well.
	 */
	stream->os_line_wrap = TRUE;
#endif

#ifdef USE_HTML
	/*
	 *   if we're compiled for HTML mode, set the standard output stream
	 *   so that it knows it has an HTML target - this will ensure that
	 *   HTML tags are passed through to the underlying stream, and that
	 *   we generate HTML equivalents for our own control sequences
	 */
	stream->html_target = TRUE;
#endif
}

/*
 *   initialize a standard log file stream
 */
static void out_init_log(out_stream_info *stream)
{
	/* initialize the basic stream state */
	out_state_init(stream);

	/* set up the low-level output routine */
	stream->do_print = do_log_print;

	/* use plain text in the log file stream */
	stream->plain_text_target = TRUE;
}



/* ------------------------------------------------------------------------ */
/*
 *   table of '&' character name sequences
 */
struct amp_tbl_t {
	/* entity name */
	const char *cname;

	/* HTML Unicode character value */
	uint        html_cval;

	/* native character set expansion */
	char       *expan;
};

/*
 *   HTML entity mapping table.  When we're in non-HTML mode, we keep our
 *   own expansion table so that we can map HTML entity names into the
 *   local character set.
 *
 *   The entries in this table must be in sorted order (by HTML entity
 *   name), because we use a binary search to find an entity name in the
 *   table.
 */
static struct amp_tbl_t amp_tbl[] = {
	{ "AElig", 198, nullptr },
	{ "Aacute", 193, nullptr },
	{ "Abreve", 258, nullptr },
	{ "Acirc", 194, nullptr },
	{ "Agrave", 192, nullptr },
	{ "Alpha", 913, nullptr },
	{ "Aogon", 260, nullptr },
	{ "Aring", 197, nullptr },
	{ "Atilde", 195, nullptr },
	{ "Auml", 196, nullptr },
	{ "Beta", 914, nullptr },
	{ "Cacute", 262, nullptr },
	{ "Ccaron", 268, nullptr },
	{ "Ccedil", 199, nullptr },
	{ "Chi", 935, nullptr },
	{ "Dagger", 8225, nullptr },
	{ "Dcaron", 270, nullptr },
	{ "Delta", 916, nullptr },
	{ "Dstrok", 272, nullptr },
	{ "ETH", 208, nullptr },
	{ "Eacute", 201, nullptr },
	{ "Ecaron", 282, nullptr },
	{ "Ecirc", 202, nullptr },
	{ "Egrave", 200, nullptr },
	{ "Eogon", 280, nullptr },
	{ "Epsilon", 917, nullptr },
	{ "Eta", 919, nullptr },
	{ "Euml", 203, nullptr },
	{ "Gamma", 915, nullptr },
	{ "Iacute", 205, nullptr },
	{ "Icirc", 206, nullptr },
	{ "Igrave", 204, nullptr },
	{ "Iota", 921, nullptr },
	{ "Iuml", 207, nullptr },
	{ "Kappa", 922, nullptr },
	{ "Lacute", 313, nullptr },
	{ "Lambda", 923, nullptr },
	{ "Lcaron", 317, nullptr },
	{ "Lstrok", 321, nullptr },
	{ "Mu", 924, nullptr },
	{ "Nacute", 323, nullptr },
	{ "Ncaron", 327, nullptr },
	{ "Ntilde", 209, nullptr },
	{ "Nu", 925, nullptr },
	{ "OElig", 338, nullptr },
	{ "Oacute", 211, nullptr },
	{ "Ocirc", 212, nullptr },
	{ "Odblac", 336, nullptr },
	{ "Ograve", 210, nullptr },
	{ "Omega", 937, nullptr },
	{ "Omicron", 927, nullptr },
	{ "Oslash", 216, nullptr },
	{ "Otilde", 213, nullptr },
	{ "Ouml", 214, nullptr },
	{ "Phi", 934, nullptr },
	{ "Pi", 928, nullptr },
	{ "Prime", 8243, nullptr },
	{ "Psi", 936, nullptr },
	{ "Racute", 340, nullptr },
	{ "Rcaron", 344, nullptr },
	{ "Rho", 929, nullptr },
	{ "Sacute", 346, nullptr },
	{ "Scaron", 352, nullptr },
	{ "Scedil", 350, nullptr },
	{ "Sigma", 931, nullptr },
	{ "THORN", 222, nullptr },
	{ "Tau", 932, nullptr },
	{ "Tcaron", 356, nullptr },
	{ "Tcedil", 354, nullptr },
	{ "Theta", 920, nullptr },
	{ "Uacute", 218, nullptr },
	{ "Ucirc", 219, nullptr },
	{ "Udblac", 368, nullptr },
	{ "Ugrave", 217, nullptr },
	{ "Upsilon", 933, nullptr },
	{ "Uring", 366, nullptr },
	{ "Uuml", 220, nullptr },
	{ "Xi", 926, nullptr },
	{ "Yacute", 221, nullptr },
	{ "Yuml", 376, nullptr },
	{ "Zacute", 377, nullptr },
	{ "Zcaron", 381, nullptr },
	{ "Zdot", 379, nullptr },
	{ "Zeta", 918, nullptr },
	{ "aacute", 225, nullptr },
	{ "abreve", 259, nullptr },
	{ "acirc", 226, nullptr },
	{ "acute", 180, nullptr },
	{ "aelig", 230, nullptr },
	{ "agrave", 224, nullptr },
	{ "alefsym", 8501, nullptr },
	{ "alpha", 945, nullptr },
	{ "amp", '&', nullptr },
	{ "and", 8743, nullptr },
	{ "ang", 8736, nullptr },
	{ "aogon", 261, nullptr },
	{ "aring", 229, nullptr },
	{ "asymp", 8776, nullptr },
	{ "atilde", 227, nullptr },
	{ "auml", 228, nullptr },
	{ "bdquo", 8222, nullptr },
	{ "beta", 946, nullptr },
	{ "breve", 728, nullptr },
	{ "brvbar", 166, nullptr },
	{ "bull", 8226, nullptr },
	{ "cacute", 263, nullptr },
	{ "cap", 8745, nullptr },
	{ "caron", 711, nullptr },
	{ "ccaron", 269, nullptr },
	{ "ccedil", 231, nullptr },
	{ "cedil", 184, nullptr },
	{ "cent", 162, nullptr },
	{ "chi", 967, nullptr },
	{ "circ", 710, nullptr },
	{ "clubs", 9827, nullptr },
	{ "cong", 8773, nullptr },
	{ "copy", 169, nullptr },
	{ "crarr", 8629, nullptr },
	{ "cup", 8746, nullptr },
	{ "curren", 164, nullptr },
	{ "dArr", 8659, nullptr },
	{ "dagger", 8224, nullptr },
	{ "darr", 8595, nullptr },
	{ "dblac", 733, nullptr },
	{ "dcaron", 271, nullptr },
	{ "deg", 176, nullptr },
	{ "delta", 948, nullptr },
	{ "diams", 9830, nullptr },
	{ "divide", 247, nullptr },
	{ "dot", 729, nullptr },
	{ "dstrok", 273, nullptr },
	{ "eacute", 233, nullptr },
	{ "ecaron", 283, nullptr },
	{ "ecirc", 234, nullptr },
	{ "egrave", 232, nullptr },
	{ "emdash", 8212, nullptr },
	{ "empty", 8709, nullptr },
	{ "endash", 8211, nullptr },
	{ "eogon", 281, nullptr },
	{ "epsilon", 949, nullptr },
	{ "equiv", 8801, nullptr },
	{ "eta", 951, nullptr },
	{ "eth", 240, nullptr },
	{ "euml", 235, nullptr },
	{ "exist", 8707, nullptr },
	{ "fnof", 402, nullptr },
	{ "forall", 8704, nullptr },
	{ "frac12", 189, nullptr },
	{ "frac14", 188, nullptr },
	{ "frac34", 190, nullptr },
	{ "frasl", 8260, nullptr },
	{ "gamma", 947, nullptr },
	{ "ge", 8805, nullptr },
	{ "gt", '>', nullptr },
	{ "hArr", 8660, nullptr },
	{ "harr", 8596, nullptr },
	{ "hearts", 9829, nullptr },
	{ "hellip", 8230, nullptr },
	{ "iacute", 237, nullptr },
	{ "icirc", 238, nullptr },
	{ "iexcl", 161, nullptr },
	{ "igrave", 236, nullptr },
	{ "image", 8465, nullptr },
	{ "infin", 8734, nullptr },
	{ "int", 8747, nullptr },
	{ "iota", 953, nullptr },
	{ "iquest", 191, nullptr },
	{ "isin", 8712, nullptr },
	{ "iuml", 239, nullptr },
	{ "kappa", 954, nullptr },
	{ "lArr", 8656, nullptr },
	{ "lacute", 314, nullptr },
	{ "lambda", 955, nullptr },
	{ "lang", 9001, nullptr },
	{ "laquo", 171, nullptr },
	{ "larr", 8592, nullptr },
	{ "lcaron", 318, nullptr },
	{ "lceil", 8968, nullptr },
	{ "ldq", 8220, nullptr },
	{ "ldquo", 8220, nullptr },
	{ "le", 8804, nullptr },
	{ "lfloor", 8970, nullptr },
	{ "lowast", 8727, nullptr },
	{ "loz", 9674, nullptr },
	{ "lsaquo", 8249, nullptr },
	{ "lsq", 8216, nullptr },
	{ "lsquo", 8216, nullptr },
	{ "lstrok", 322, nullptr },
	{ "lt", '<', nullptr },
	{ "macr", 175, nullptr },
	{ "mdash", 8212, nullptr },
	{ "micro", 181, nullptr },
	{ "middot", 183, nullptr },
	{ "minus", 8722, nullptr },
	{ "mu", 956, nullptr },
	{ "nabla", 8711, nullptr },
	{ "nacute", 324, nullptr },
	{ "nbsp", QSPACE, nullptr },
	{ "ncaron", 328, nullptr },
	{ "ndash", 8211, nullptr },
	{ "ne", 8800, nullptr },
	{ "ni", 8715, nullptr },
	{ "not", 172, nullptr },
	{ "notin", 8713, nullptr },
	{ "nsub", 8836, nullptr },
	{ "ntilde", 241, nullptr },
	{ "nu", 957, nullptr },
	{ "oacute", 243, nullptr },
	{ "ocirc", 244, nullptr },
	{ "odblac", 337, nullptr },
	{ "oelig", 339, nullptr },
	{ "ogon", 731, nullptr },
	{ "ograve", 242, nullptr },
	{ "oline", 8254, nullptr },
	{ "omega", 969, nullptr },
	{ "omicron", 959, nullptr },
	{ "oplus", 8853, nullptr },
	{ "or", 8744, nullptr },
	{ "ordf", 170, nullptr },
	{ "ordm", 186, nullptr },
	{ "oslash", 248, nullptr },
	{ "otilde", 245, nullptr },
	{ "otimes", 8855, nullptr },
	{ "ouml", 246, nullptr },
	{ "para", 182, nullptr },
	{ "part", 8706, nullptr },
	{ "permil", 8240, nullptr },
	{ "perp", 8869, nullptr },
	{ "phi", 966, nullptr },
	{ "pi", 960, nullptr },
	{ "piv", 982, nullptr },
	{ "plusmn", 177, nullptr },
	{ "pound", 163, nullptr },
	{ "prime", 8242, nullptr },
	{ "prod", 8719, nullptr },
	{ "prop", 8733, nullptr },
	{ "psi", 968, nullptr },
	{ "quot", '"', nullptr },
	{ "rArr", 8658, nullptr },
	{ "racute", 341, nullptr },
	{ "radic", 8730, nullptr },
	{ "rang", 9002, nullptr },
	{ "raquo", 187, nullptr },
	{ "rarr", 8594, nullptr },
	{ "rcaron", 345, nullptr },
	{ "rceil", 8969, nullptr },
	{ "rdq", 8221, nullptr },
	{ "rdquo", 8221, nullptr },
	{ "real", 8476, nullptr },
	{ "reg", 174, nullptr },
	{ "rfloor", 8971, nullptr },
	{ "rho", 961, nullptr },
	{ "rsaquo", 8250, nullptr },
	{ "rsq", 8217, nullptr },
	{ "rsquo", 8217, nullptr },
	{ "sacute", 347, nullptr },
	{ "sbquo", 8218, nullptr },
	{ "scaron", 353, nullptr },
	{ "scedil", 351, nullptr },
	{ "sdot", 8901, nullptr },
	{ "sect", 167, nullptr },
	{ "shy", 173, nullptr },
	{ "sigma", 963, nullptr },
	{ "sigmaf", 962, nullptr },
	{ "sim", 8764, nullptr },
	{ "spades", 9824, nullptr },
	{ "sub", 8834, nullptr },
	{ "sube", 8838, nullptr },
	{ "sum", 8721, nullptr },
	{ "sup", 8835, nullptr },
	{ "sup1", 185, nullptr },
	{ "sup2", 178, nullptr },
	{ "sup3", 179, nullptr },
	{ "supe", 8839, nullptr },
	{ "szlig", 223, nullptr },
	{ "tau", 964, nullptr },
	{ "tcaron", 357, nullptr },
	{ "tcedil", 355, nullptr },
	{ "there4", 8756, nullptr },
	{ "theta", 952, nullptr },
	{ "thetasym", 977, nullptr },
	{ "thorn", 254, nullptr },
	{ "thorn", 254, nullptr },
	{ "tilde", 732, nullptr },
	{ "times", 215, nullptr },
	{ "trade", 8482, nullptr },
	{ "uArr", 8657, nullptr },
	{ "uacute", 250, nullptr },
	{ "uarr", 8593, nullptr },
	{ "ucirc", 251, nullptr },
	{ "udblac", 369, nullptr },
	{ "ugrave", 249, nullptr },
	{ "uml", 168, nullptr },
	{ "upsih", 978, nullptr },
	{ "upsilon", 965, nullptr },
	{ "uring", 367, nullptr },
	{ "uuml", 252, nullptr },
	{ "weierp", 8472, nullptr },
	{ "xi", 958, nullptr },
	{ "yacute", 253, nullptr },
	{ "yen", 165, nullptr },
	{ "yuml", 255, nullptr },
	{ "zacute", 378, nullptr },
	{ "zcaron", 382, nullptr },
	{ "zdot", 380, nullptr },
	{ "zeta", 950, nullptr }
};


/* ------------------------------------------------------------------------ */
/*
 *   turn on CAPS mode for a stream
 */
static void outcaps_stream(out_stream_info *stream)
{
	/* turn on CAPS mode */
	stream->capsflag = TRUE;

	/* turn off NOCAPS and ALLCAPS mode */
	stream->nocapsflag = FALSE;
	stream->allcapsflag = FALSE;
}

/*
 *   turn on NOCAPS mode for a stream
 */
static void outnocaps_stream(out_stream_info *stream)
{
	/* turn on NOCAPS mode */
	stream->nocapsflag = TRUE;

	/* turn off CAPS and ALLCAPS mode */
	stream->capsflag = FALSE;
	stream->allcapsflag = FALSE;
}

/*
 *   turn on or off ALLCAPS mode for a stream
 */
static void outallcaps_stream(out_stream_info *stream, int all_caps)
{
	/* set the ALLCAPS flag */
	stream->allcapsflag = all_caps;

	/* clear the CAPS and NOCAPS flags */
	stream->capsflag = FALSE;
	stream->nocapsflag = FALSE;
}

/* ------------------------------------------------------------------------ */
/*
 *   write a string to a stream
 */
static void stream_print(out_stream_info *stream, char *str)
{
	/* call the stream's do_print method */
	(*stream->do_print)(stream, str);
}

/*
 *   Write out a line
 */
static void t_outline(out_stream_info *stream, int nl,
					  const char *txt, const int *attr)
{
	extern int scrquiet;

	/*
	 *   Check the "script quiet" mode - this indicates that we're reading
	 *   a script and not echoing output to the display.  If this mode is
	 *   on, and we're writing to the display, suppress this write.  If
	 *   the mode is off, or we're writing to another stream (such as the
	 *   log file), show the output as normal.
	 */
	if (!scrquiet || stream != &G_std_disp)
	{
		size_t i;
		char buf[MAXWIDTH];
		char *dst;

		/*
		 *   Check to see if we've reached the end of the screen, and if
		 *   so run the MORE prompt.  Note that we don't make this check
		 *   at all if USE_MORE is undefined, since this means that the OS
		 *   layer code is taking responsibility for pagination issues.
		 *   We also don't display a MORE prompt when reading from a
		 *   script file.
		 *
		 *   Note that we suppress the MORE prompt if nl == 0, since this
		 *   is used to flush a partial line of text without starting a
		 *   new line (for example, when displaying a prompt where the
		 *   input will appear on the same line following the prompt).
		 *
		 *   Skip the MORE prompt if this stream doesn't use it.
		 */
		if (stream->use_more_mode
			&& scrfp == nullptr
			&& G_os_moremode
			&& nl != 0 && nl != 4
			&& stream->linecnt++ >= G_os_pagelength)
		{
			/* display the MORE prompt */
			out_more_prompt();
		}

		/*
		 *   Display the text.  Run through the text in pieces; each time the
		 *   attributes change, set attributes at the osifc level.
		 */
		for (i = 0, dst = buf ; txt[i] != '\0' ; ++i)
		{
			/* if the attribute is changing, notify osifc */
			if (attr != nullptr && attr[i] != stream->os_attr)
			{
				/* flush the preceding text */
				if (dst != buf)
				{
					*dst = '\0';
					stream_print(stream, buf);
				}

				/* set the new attribute */
				os_set_text_attr(attr[i]);

				/* remember this as the last OS attribute */
				stream->os_attr = attr[i];

				/* start with a fresh buffer */
				dst = buf;
			}

			/* buffer this character */
			*dst++ = txt[i];
		}

		/* flush the last chunk of text */
		if (dst != buf)
		{
			*dst = '\0';
			stream_print(stream, buf);
		}
	}
}

/* ------------------------------------------------------------------------ */
/*
 *   Flush the current line to the display.  The 'nl' argument specifies
 *   what kind of flushing to do:
 *
 *   0: flush the current line but do not start a new line; more text will
 *   follow on the current line.  This is used, for example, to flush text
 *   after displaying a prompt and before waiting for user input.
 *
 *   1: flush the line and start a new line.
 *
 *   2: flush the line as though starting a new line, but don't add an
 *   actual newline character to the output, since the underlying OS
 *   display code will handle this.  Instead, add a space after the line.
 *   (This differs from mode 0 in that mode 0 shouldn't add anything at
 *   all after the line.)
 *
 *   3: "preview" mode.  Flush the line, but do not start a new line, and
 *   retain the current text in the buffer.  This is used for systems that
 *   handle the line wrapping in the underlying system code to flush a
 *   partially filled line that will need to be flushed again later.
 *
 *   4: same as mode 0, but used for internal buffer flushes only.  Do not
 *   involve the underlying OS layer in this type of flush - simply flush
 *   our buffers with no separation.
 */

/* flush a given output stream */
static void outflushn_stream(out_stream_info *stream, int nl)
{
	int i;

	/* null-terminate the current output line buffer */
	stream->linebuf[stream->linepos] = '\0';

	/* note the position of the last character to display */
	i = stream->linepos - 1;

	/* if we're adding anything, remove trailing spaces */
	if (nl != 0 && nl != 4)
	{
		/* look for last non-space character */
		for ( ; i >= 0 && outissp(stream->linebuf[i]) ; --i) ;
	}

	/* check the output mode */
	if (nl == 3)
	{
		/*
		 *   this is the special "preview" mode -- only display the part
		 *   that we haven't already previewed for this same line
		 */
		if (i + 1 > stream->preview)
		{
			/* write out the line */
			t_outline(stream, 0, &stream->linebuf[stream->preview],
					  &stream->attrbuf[stream->preview]);

			/* skip past the part we wrote */
			stream->preview += strlen(&stream->linebuf[stream->preview]);
		}
	}
	else
	{
		const char *suffix = nullptr; /* extra text to add after the flushed text */
		int   countnl = 0;               /* true if line counts for [more] paging */

		/* null-terminate the buffer at the current position */
		stream->linebuf[++i] = '\0';

		/* check the mode */
		switch(nl)
		{
		case 0:
		case 3:
		case 4:
			/* no newline - just flush out what we have with no suffix */
			suffix = nullptr;
			break;

		case 1:
			/*
			 *   Add a newline.  If there's nothing in the current line,
			 *   or we just wrote out a newline, do not add an extra
			 *   newline.  Keep all newlines in PRE mode.
			 */
			if (stream->linecol != 0 || !stream->just_did_nl
				|| stream->html_pre_level != 0)
			{
				/* add a newline after the text */
				suffix = "\n";

				/* count the line in the page size */
				countnl = 1;
			}
			else
			{
				/* don't add a newline */
				suffix = nullptr;
			}
			break;

		case 2:
			/*
			 *   we're going to depend on the underlying OS output layer
			 *   to do line breaking, so don't add a newline, but do add a
			 *   space, so that the underlying OS layer knows we have a
			 *   word break here
			 */
			suffix = " ";
			break;
		}

		/*
		 *   display the line, as long as we have something buffered to
		 *   display; even if we don't, display it if our column is
		 *   non-zero and we didn't just do a newline, since this must
		 *   mean that we've flushed a partial line and are just now doing
		 *   the newline
		 */
		if (stream->linebuf[stream->preview] != '\0'
			|| (stream->linecol != 0 && !stream->just_did_nl)
			|| stream->html_pre_level > 0)
		{
			/* write it out */
			t_outline(stream, countnl, &stream->linebuf[stream->preview],
					  &stream->attrbuf[stream->preview]);

			/* write the suffix, if any */
			if (suffix != nullptr)
				t_outline(stream, 0, suffix, nullptr);
		}

		/* generate an HTML line break if necessary */
		if (nl == 1 && stream->html_mode && stream->html_target)
			t_outline(stream, 0, "<BR HEIGHT=0>", nullptr);

		if (nl == 0)
		{
			/* we're not displaying a newline, so flush what we have */
			os_flush();
		}
		else
		{
			/* we displayed a newline, so reset the column position */
			stream->linecol = 0;
		}

		/* reset the line output buffer position */
		stream->linepos = stream->preview = 0;

		/*
		 *   If we just output a newline, note it.  If we didn't just
		 *   output a newline, but we did write out anything else, note
		 *   that we're no longer at the start of a line on the underlying
		 *   output device.
		 */
		if (nl == 1)
			stream->just_did_nl = TRUE;
		else if (stream->linebuf[stream->preview] != '\0')
			stream->just_did_nl = FALSE;
	}

	/*
	 *   If the osifc-level attributes don't match the current attributes,
	 *   bring the osifc layer up to date.  This is necessary in cases where
	 *   we set attributes immediately before asking for input - we
	 *   essentially need to flush the attributes without flushing any text.
	 */
	if (stream->cur_attr != stream->os_attr)
	{
		/* set the osifc attributes */
		os_set_text_attr(stream->cur_attr);

		/* remember the new attributes as the current osifc attributes */
		stream->os_attr = stream->cur_attr;
	}
}

/* ------------------------------------------------------------------------ */
/*
 *   Determine if we're showing output.  Returns true if output should be
 *   displayed, false if it should be suppressed.  We'll note the output
 *   for hidden display accounting as needed.
 */
static int out_is_hidden()
{
	/* check the output flag */
	if (!outflag)
	{
		/* trace the hidden output if desired */
		if (dbghid && !hidout)
			trchid();

		/* note the hidden output */
		hidout = 1;

		/*
		 *   unless we're showing hidden text in the debugger, we're
		 *   suppressing output, so return true
		 */
		if (!dbghid)
			return TRUE;
	}

	/* we're not suppressing output */
	return FALSE;
}

/* ------------------------------------------------------------------------ */
/*
 *   Display a blank line to the given stream
 */
static void outblank_stream(out_stream_info *stream)
{
	/* flush the stream */
	outflushn_stream(stream, 1);

	/* if generating for an HTML display target, add an HTML line break */
	if (stream->html_mode && stream->html_target)
		outstring_stream(stream, "<BR>");

	/* write out the newline */
	t_outline(stream, 1, "\n", nullptr);
}

/* ------------------------------------------------------------------------ */
/*
 *   Generate a tab for a "\t" sequence in the game text.
 *
 *   Standard (non-HTML) version: we'll generate enough spaces to take us
 *   to the next tab stop.
 *
 *   HTML version: if we're in native HTML mode, we'll just generate a
 *   <TAB MULTIPLE=4>; if we're not in HTML mode, we'll generate a hard
 *   tab character, which the HTML formatter will interpret as a <TAB
 *   MULTIPLE=4>.
 */
static void outtab_stream(out_stream_info *stream)
{
	/* check to see what the underlying system is expecting */
	if (stream->html_target)
	{
		/* the underlying system is HTML - check for HTML mode */
		if (stream->html_mode)
		{
			/* we're in HTML mode, so use the HTML <TAB> tag */
			outstring_stream(stream, "<TAB MULTIPLE=4>");
		}
		else
		{
			/* we're not in HTML mode, so generate a hard tab character */
			outchar_noxlat_stream(stream, QTAB);
		}
	}
	else
	{
		int maxcol;

		/*
		 *   We're not in HTML mode - expand the tab with spaces.  Figure
		 *   the maximum column: if we're doing our own line wrapping, never
		 *   go beyond the actual display width.
		 */
		maxcol = (stream->os_line_wrap ? OS_MAXWIDTH : G_os_linewidth);

		/* add the spaces */
		do
		{
			stream->attrbuf[stream->linepos] = stream->cur_attr;
			stream->linebuf[stream->linepos++] = ' ';
			++(stream->linecol);
		} while (((stream->linecol + 1) & 3) != 0
				 && stream->linecol < maxcol);
	}
}


/* ------------------------------------------------------------------------ */
/*
 *   Flush a line
 */
static void out_flushline(out_stream_info *stream, int padding)
{
	/*
	 *   check to see if we're using the underlying display layer's line
	 *   wrapping
	 */
	if (stream->os_line_wrap)
	{
		/*
		 *   In the HTML version, we don't need the normal *MORE*
		 *   processing, since the HTML layer will handle that.
		 *   Furthermore, we don't need to provide actual newline breaks
		 *   -- that happens after the HTML is parsed, so we don't have
		 *   enough information here to figure out actual line breaks.
		 *   So, we'll just flush out our buffer whenever it fills up, and
		 *   suppress newlines.
		 *
		 *   Similarly, if we have OS-level MORE processing, don't try to
		 *   figure out where the line breaks go -- just flush our buffer
		 *   without a trailing newline whenever the buffer is full, and
		 *   let the OS layer worry about formatting lines and paragraphs.
		 *
		 *   If we're using padding, use mode 2.  If we don't want padding
		 *   (which is the case if we completely fill up the buffer
		 *   without finding any word breaks), write out in mode 0, which
		 *   just flushes the buffer exactly like it is.
		 */
		outflushn_stream(stream, padding ? 2 : 4);
	}
	else
	{
		/*
		 *   Normal mode - we process the *MORE* prompt ourselves, and we
		 *   are responsible for figuring out where the actual line breaks
		 *   go.  Use outflush() to generate an actual newline whenever we
		 *   flush out our buffer.
		 */
		outflushn_stream(stream, 1);
	}
}


/* ------------------------------------------------------------------------ */
/*
 *   Write a character to an output stream without translation
 */
static void outchar_noxlat_stream(out_stream_info *stream, char c)
{
	int  i;
	int  qspace;

	/* check for the special quoted space character */
	if (c == QSPACE)
	{
		/* it's a quoted space - note it and convert it to a regular space */
		qspace = 1;
		c = ' ';
	}
	else if (c == QTAB)
	{
		/* it's a hard tab - convert it to an ordinary tab */
		c = '\t';
		qspace = 0;
	}
	else
	{
		/* translate any whitespace character to a regular space character */
		if (outissp(c))
			c = ' ';

		/* it's not a quoted space */
		qspace = 0;
	}

	/* check for the caps/nocaps flags */
	if ((stream->capsflag || stream->allcapsflag) && outisal(c))
	{
		/* capsflag is set, so capitalize this character */
		if (outislo(c))
			c = toupper(c);

		/* okay, we've capitalized something; clear flag */
		stream->capsflag = 0;
	}
	else if (stream->nocapsflag && outisal(c))
	{
		/* nocapsflag is set, so minisculize this character */
		if (outisup(c))
			c = tolower(c);

		/* clear the flag now that we've done the job */
		stream->nocapsflag = 0;
	}

	/* if in capture mode, simply capture the character */
	if (stream->capturing)
	{
		uchar *p;

		/* if we have a valid capture object, copy to it */
		if (stream->capture_obj != MCMONINV)
		{
			/* lock the object holding the captured text */
			p = mcmlck(stream->capture_ctx, stream->capture_obj);

			/* make sure the capture object is big enough */
			if (mcmobjsiz(stream->capture_ctx, stream->capture_obj)
				<= stream->capture_ofs)
			{
				/* expand the object by another 256 bytes */
				p = mcmrealo(stream->capture_ctx, stream->capture_obj,
							 (ushort)(stream->capture_ofs + 256));
			}

			/* add this character */
			*(p + stream->capture_ofs++) = c;

			/* unlock the capture object */
			mcmtch(stream->capture_ctx, stream->capture_obj);
			mcmunlck(stream->capture_ctx, stream->capture_obj);
		}

		/*
		 *   we're done - we don't want to actually display the character
		 *   while capturing
		 */
		return;
	}

	/* add the character to out output buffer, flushing as needed */
	if (stream->linecol + 1 < G_os_linewidth)
	{
		/*
		 *   there's room for this character, so add it to the buffer
		 */

		/* ignore non-quoted space at start of line outside of PRE */
		if (outissp(c) && c != '\t' && stream->linecol == 0 && !qspace
			&& stream->html_pre_level == 0)
			return;

		/* is this a non-quoted space not at the start of the line? */
		if (outissp(c) && c != '\t' && stream->linecol != 0 && !qspace
			&& stream->html_pre_level == 0)
		{
			int  pos1 = stream->linepos - 1;
			char p = stream->linebuf[pos1];     /* check previous character */

			/* ignore repeated spaces - collapse into a single space */
			if (outissp(p))
				return;

			/*
			 *   Certain punctuation requires a double space: a period, a
			 *   question mark, an exclamation mark, or a colon; or any of
			 *   these characters followed by any number of single and/or
			 *   double quotes.  First, scan back to before any quotes, if
			 *   are on one now, then check the preceding character; if
			 *   it's one of the punctuation marks requiring a double
			 *   space, add this space a second time.  (In addition to
			 *   scanning back past quotes, scan past parentheses,
			 *   brackets, and braces.)  Don't double the spacing if we're
			 *   not in the normal doublespace mode; some people may
			 *   prefer single spacing after punctuation, so we make this
			 *   a run-time option.
			 */
			if (doublespace)
			{
				/* find the previous relevant punctuation character */
				while (pos1 &&
					   (p == '"' || p == '\'' || p == ')' || p == ']'
						|| p == '}'))
				{
					p = stream->linebuf[--pos1];
				}
				if ( p == '.' || p == '?' || p == '!' || p == ':' )
				{
					/* a double-space is required after this character */
					stream->attrbuf[stream->linepos] = stream->cur_attr;
					stream->linebuf[stream->linepos++] = c;
					++(stream->linecol);
				}
			}
		}

		/* add this character to the buffer */
		stream->attrbuf[stream->linepos] = stream->cur_attr;
		stream->linebuf[stream->linepos++] = c;

		/* advance the output column position */
		++(stream->linecol);
		return;
	}

	/*
	 *   The line would overflow if this character were added.  Find the
	 *   most recent word break, and output the line up to the previous
	 *   word.  Note that if we're trying to output a space, we'll just
	 *   add it to the line buffer.  If the last character of the line
	 *   buffer is already a space, we won't do anything right now.
	 */
	if (outissp(c) && c != '\t' && !qspace)
	{
		/* this is a space, so we're at a word break */
		if (stream->linebuf[stream->linepos - 1] != ' ')
		{
			stream->attrbuf[stream->linepos] = stream->cur_attr;
			stream->linebuf[stream->linepos++] = ' ';
		}
		return;
	}

	/*
	 *   Find the most recent word break: look for a space or dash, starting
	 *   at the end of the line.
	 *
	 *   If we're about to write a hyphen, we want to skip all contiguous
	 *   hyphens, because we want to keep them together as a single
	 *   punctuation mark; then keep going in the normal manner, which will
	 *   keep the hyphens plus the word they're attached to together as a
	 *   single unit.  If spaces precede the sequence of hyphens, include
	 *   the prior word as well.
	 */
	i = stream->linepos - 1;
	if (c == '-')
	{
		/* skip any contiguous hyphens at the end of the line */
		for ( ; i >= 0 && stream->linebuf[i] == '-' ; --i) ;

		/* skip any spaces preceding the sequence of hyphens */
		for ( ; i >= 0 && outissp(stream->linebuf[i]) ; --i) ;
	}

	/*
	 *   Now find the preceding space.  If we're doing our own wrapping
	 *   (i.e., we're not using OS line wrapping), then look for the
	 *   nearest hyphen as well.
	 */
	for ( ; i >= 0 && !outissp(stream->linebuf[i])
		  && !(!stream->os_line_wrap && stream->linebuf[i] == '-') ; --i) ;

	/* check to see if we found a good place to break */
	if (i < 0)
	{
		/*
		 *   we didn't find any good place to break - flush the entire
		 *   line as-is, breaking arbitrarily in the middle of a word
		 */
		out_flushline(stream, FALSE);

		/*
		 *   we've completely cleared out the line buffer, so reset all of
		 *   the line buffer counters
		 */
		stream->linepos = 0;
		stream->linecol = 0;
		stream->linebuf[0] = '\0';
	}
	else
	{
		char brkchar;
		char tmpbuf[MAXWIDTH];
		int tmpattr[MAXWIDTH];
		size_t tmpcnt;

		/* remember the word-break character */
		brkchar = stream->linebuf[i];

		/* null-terminate the line buffer */
		stream->linebuf[stream->linepos] = '\0';

		/* the next line starts after the break - save a copy */
		tmpcnt = strlen(&stream->linebuf[i+1]);
		memcpy(tmpbuf, &stream->linebuf[i+1], tmpcnt + 1);
		memcpy(tmpattr, &stream->attrbuf[i+1], tmpcnt * sizeof(tmpattr[0]));

		/*
		 *   terminate the buffer at the space or after the hyphen,
		 *   depending on where we broke
		 */
		if (outissp(brkchar))
			stream->linebuf[i] = '\0';
		else
			stream->linebuf[i+1] = '\0';

		/* write out everything up to the word break */
		out_flushline(stream, TRUE);

		/* copy the next line into line buffer */
		memcpy(stream->linebuf, tmpbuf, tmpcnt + 1);
		memcpy(stream->attrbuf, tmpattr, tmpcnt * sizeof(tmpattr[0]));
		stream->linepos = tmpcnt;

		/*
		 *   figure what column we're now in - count all of the printable
		 *   characters in the new line
		 */
		for (stream->linecol = 0, i = 0 ; i < stream->linepos ; ++i)
		{
			/* if it's printable, count it */
			if (((unsigned char)stream->linebuf[i]) >= 26)
				++(stream->linecol);
		}
	}

	/* add the new character to buffer */
	stream->attrbuf[stream->linepos] = stream->cur_attr;
	stream->linebuf[stream->linepos++] = c;

	/* advance the column counter */
	++(stream->linecol);
}

/* ------------------------------------------------------------------------ */
/*
 *   Write out a character, translating to the local system character set
 *   from the game's internal character set.
 */
static void outchar_stream(out_stream_info *stream, char c)
{
	outchar_noxlat_stream(stream, cmap_i2n(c));
}

/*
 *   write out a string, translating to the local system character set
 */
static void outstring_stream(out_stream_info *stream, const char *s)
{
	/* write out each character in the string */
	for ( ; *s ; ++s)
		outchar_stream(stream, *s);
}

/*
 *   write out a string without translation
 */
static void outstring_noxlat_stream(out_stream_info *stream, char *s)
{
	for ( ; *s ; ++s)
		outchar_noxlat_stream(stream, *s);
}


/* ------------------------------------------------------------------------ */
/*
 *   Write out an HTML character value, translating to the local character
 *   set.
 */
static void outchar_html_stream(out_stream_info *stream,
								unsigned int htmlchar)
{
	amp_tbl_t *ampptr;

	/*
	 *   search for a mapping entry for this entity, in case it's defined
	 *   in an external mapping file
	 */
	for (ampptr = amp_tbl ;
		 ampptr < amp_tbl + sizeof(amp_tbl)/sizeof(amp_tbl[0]) ; ++ampptr)
	{
		/* if this is the one, stop looking */
		if (ampptr->html_cval == htmlchar)
			break;
	}

	/*
	 *   If we found a mapping table entry, and the entry has an expansion
	 *   from the external character mapping table file, use the external
	 *   expansion; otherwise, use the default expansion.
	 */
	if (ampptr >= amp_tbl + sizeof(amp_tbl)/sizeof(amp_tbl[0])
		|| ampptr->expan == nullptr)
	{
		char xlat_buf[50];

		/*
		 *   there's no external mapping table file expansion -- use the
		 *   default OS mapping routine
		 */
		os_xlat_html4(htmlchar, xlat_buf, sizeof(xlat_buf));
		outstring_noxlat_stream(stream, xlat_buf);
	}
	else
	{
		/*
		 *   use the explicit mapping from the mapping table file
		 */
		outstring_noxlat_stream(stream, ampptr->expan);
	}
}


/* ------------------------------------------------------------------------ */
/*
 *   Enter a recursion level.  Returns TRUE if the caller should proceed
 *   with the operation, FALSE if not.
 *
 *   If we're making a recursive call, thereby re-entering the formatter,
 *   and this stream is not the same as the enclosing stream, we want to
 *   ignore this call and suppress any output to this stream, so we'll
 *   return FALSE.
 */
static int out_push_stream(out_stream_info *stream)
{
	/*
	 *   if we're already in the formatter, and the new stream doesn't
	 *   match the enclosing recursion level's stream, tell the caller to
	 *   abort the operation
	 */
	if (G_recurse != 0 && G_cur_stream != stream)
		return FALSE;

	/* note the active stream */
	G_cur_stream = stream;

	/* count the entry */
	++G_recurse;

	/* tell the caller to proceed */
	return TRUE;
}

/*
 *   Leave a recursion level
 */
static void out_pop_stream()
{
	/* count the exit */
	--G_recurse;
}

/* ------------------------------------------------------------------------ */
/*
 *   nextout() returns the next character in a string, and updates the
 *   string pointer and remaining length.  Returns zero if no more
 *   characters are available in the string.
 */
/* static char nextout(char **s, uint *len); */
#define nextout(s, len) ((char)(*(len) == 0 ? 0 : (--(*(len)), *((*(s))++))))


/* ------------------------------------------------------------------------ */
/*
 *   Get the next character, writing the previous character to the given
 *   output stream if it's not null.
 */
static char nextout_copy(const char **s, size_t *slen,
						 char prv, out_stream_info *stream)
{
	/* if there's a stream, write the previous character to the stream */
	if (stream != nullptr)
		outchar_stream(stream, prv);

	/* return the next character */
	return nextout(s, slen);
}

/* ------------------------------------------------------------------------ */
/*
 *   Read an HTML tag, for our primitive mini-parser.  If 'stream' is not
 *   null, we'll copy each character we read to the output stream.  Returns
 *   the next character after the tag name.
 */
static char read_tag(char *dst, size_t dstlen, int *is_end_tag,
					 const char **s, size_t *slen, out_stream_info *stream)
{
	char c;

	/* skip the opening '<' */
	c = nextout_copy(s, slen, '<', stream);

	/* skip spaces */
	while (outissp(c))
		c = nextout_copy(s, slen, c, stream);

	/* note if this is a closing tag */
	if (c == '/' || c == '\\')
	{
		/* it's an end tag - note it and skip the slash */
		*is_end_tag = TRUE;
		c = nextout_copy(s, slen, c, stream);

		/* skip yet more spaces */
		while (outissp(c))
			c = nextout_copy(s, slen, c, stream);
	}
	else
		*is_end_tag = FALSE;

	/*
	 *   find the end of the tag name - the tag continues to the next space,
	 *   '>', or end of line
	 */
	for ( ; c != '\0' && !outissp(c) && c != '>' ;
		 c = nextout_copy(s, slen, c, stream))
	{
		/* add this to the tag buffer if it fits */
		if (dstlen > 1)
		{
			*dst++ = c;
			--dstlen;
		}
	}

	/* null-terminate the tag name */
	if (dstlen > 0)
		*dst = '\0';

	/* return the next character */
	return c;
}


/* ------------------------------------------------------------------------ */
/*
 *   display a string of a given length to a given stream
 */
static int outformatlen_stream(out_stream_info *stream,
							   const char *s, size_t slen)
{
	char     c;
	int      done = 0;
	char     fmsbuf[40];       /* space for constructing translation string */
	uint     fmslen;
	char    *f = nullptr;
	char    *f1;
	int      infmt = 0;

	/*
	 *   This routine can recurse because of format strings ("%xxx%"
	 *   sequences).  When we recurse, we want to ensure that the
	 *   recursion is directed to the original stream only.  So, note the
	 *   current stream statically in case we re-enter the formatter.
	 */
	if (!out_push_stream(stream))
		return 0;

	/* get the first character */
	c = nextout(&s, &slen);

	/* if we have anything to show, show it */
	while (c != '\0')
	{
		/* check if we're collecting translation string */
		if (infmt)
		{
			/*
			 *   if the string is too long for our buffer, or we've come
			 *   across a backslash (illegal in a format string), or we've
			 *   come across an HTML-significant character ('&' or '<') in
			 *   HTML mode, we must have a stray percent sign; dump the
			 *   whole string so far and act as though we have no format
			 *   string
			 */
			if (c == '\\'
				|| f == &fmsbuf[sizeof(fmsbuf)]
				|| (stream->html_mode && (c == '<' || c == '&')))
			{
				outchar_stream(stream, '%');
				for (f1 = fmsbuf ; f1 < f ; ++f1)
					outchar_stream(stream, *f1);
				infmt = 0;

				/* process this character again */
				continue;
			}
			else if (c == '%' && f == fmsbuf)       /* double percent sign? */
			{
				outchar_stream(stream, '%');       /* send out a single '%' */
				infmt = 0;       /* no longer processing translation string */
			}
			else if (c == '%')   /* found end of string? translate it if so */
			{
				uchar *fms;
				int    initcap = FALSE;
				int    allcaps = FALSE;
				char   fmsbuf_srch[sizeof(fmsbuf)];

				/* null-terminate the string */
				*f = '\0';

				/* check for an init cap */
				if (outisup(fmsbuf[0]))
				{
					/*
					 *   note the initial capital, so that we follow the
					 *   original capitalization in the substituted string
					 */
					initcap = TRUE;

					/*
					 *   if the second letter is capitalized as well,
					 *   capitalize the entire substituted string
					 */
					if (fmsbuf[1] != '\0' && outisup(fmsbuf[1]))
					{
						/* use all caps */
						allcaps = TRUE;
					}
				}

				/* convert the entire string to lower case for searching */
				strcpy(fmsbuf_srch, fmsbuf);
				os_strlwr(fmsbuf_srch);

				/* find the string in the format string table */
				fmslen = strlen(fmsbuf_srch);
				for (fms = fmsbase ; fms < fmstop ; )
				{
					uint propnum;
					uint len;

					/* get the information on this entry */
					propnum = osrp2(fms);
					len = osrp2(fms + 2) - 2;

					/* check for a match */
					if (len == fmslen &&
						!memcmp(fms + 4, fmsbuf_srch, (size_t)len))
					{
						int old_all_caps;

						/* note the current ALLCAPS mode */
						old_all_caps = stream->allcapsflag;

						/*
						 *   we have a match - set the appropriate
						 *   capitalization mode
						 */
						if (allcaps)
							outallcaps_stream(stream, TRUE);
						else if (initcap)
							outcaps_stream(stream);

						/*
						 *   evaluate the associated property to generate
						 *   the substitution text
						 */
						runppr(runctx, cmdActor, (prpnum)propnum, 0);

						/* turn off ALLCAPS mode */
						outallcaps_stream(stream, old_all_caps);

						/* no need to look any further */
						break;
					}

					/* move on to next formatstring if not yet found */
					fms += len + 4;
				}

				/* if we can't find it, dump the format string as-is */
				if (fms == fmstop)
				{
					outchar_stream(stream, '%');
					for (f1 = fmsbuf ; f1 < f ; ++f1)
						outchar_stream(stream, *f1);
					outchar_stream(stream, '%');
				}

				/* no longer reading format string */
				infmt = 0;
			}
			else
			{
				/* copy this character of the format string */
				*f++ = c;
			}

			/* move on to the next character and continue scanning */
			c = nextout(&s, &slen);
			continue;
		}

		/*
		 *   If we're parsing HTML here, and we're inside a tag, skip
		 *   characters until we reach the end of the tag.
		 */
		if (stream->html_mode_flag != HTML_MODE_NORMAL)
		{
			switch(stream->html_mode_flag)
			{
			case HTML_MODE_TAG:
				/*
				 *   keep skipping up to the closing '>', but note when we
				 *   enter any quoted section
				 */
				switch(c)
				{
				case '>':
					/* we've reached the end of the tag */
					stream->html_mode_flag = HTML_MODE_NORMAL;

					/* if we have a deferred <BR>, process it now */
					switch(stream->html_defer_br)
					{
					case HTML_DEFER_BR_NONE:
						/* no deferred <BR> */
						break;

					case HTML_DEFER_BR_FLUSH:
						outflushn_stream(stream, 1);
						break;

					case HTML_DEFER_BR_BLANK:
						outblank_stream(stream);
						break;
					}

					/* no more deferred <BR> pending */
					stream->html_defer_br = HTML_DEFER_BR_NONE;

					/* no more ALT attribute allowed */
					stream->html_allow_alt = FALSE;
					break;

				case '"':
					/* enter a double-quoted string */
					stream->html_mode_flag = HTML_MODE_DQUOTE;
					break;

				case '\'':
					/* enter a single-quoted string */
					stream->html_mode_flag = HTML_MODE_SQUOTE;
					break;

				default:
					/* if it's alphabetic, note the attribute name */
					if (outisal(c))
					{
						char attrname[128];
						char attrval[256];
						char *dst;

						/* gather up the attribute name */
						for (dst = attrname ;
							 dst + 1 < attrname + sizeof(attrname) ; )
						{
							/* store this character */
							*dst++ = c;

							/* get the next character */
							c = nextout(&s, &slen);

							/* if it's not alphanumeric, stop scanning */
							if (!outisal(c) && !outisdg(c))
								break;
						}

						/* null-terminate the result */
						*dst++ = '\0';

						/* gather the value if present */
						if (c == '=')
						{
							char qu;

							/* skip the '=' */
							c = nextout(&s, &slen);

							/* if we have a quote, so note */
							if (c == '"' || c == '\'')
							{
								/* remember the quote */
								qu = c;

								/* skip it */
								c = nextout(&s, &slen);
							}
							else
							{
								/* no quote */
								qu = 0;
							}

							/* read the value */
							for (dst = attrval ;
								 dst + 1 < attrval + sizeof(attrval) ; )
							{
								/* store this character */
								*dst++ = c;

								/* read the next one */
								c = nextout(&s, &slen);
								if (c == '\0')
								{
									/*
									 *   we've reached the end of the
									 *   string, and we're still inside
									 *   this attribute - abandon the
									 *   attribute but note that we're
									 *   inside a quoted string if
									 *   necessary
									 */
									if (qu == '"')
										stream->html_mode_flag =
											HTML_MODE_DQUOTE;
									else if (qu == '\'')
										stream->html_mode_flag =
											HTML_MODE_SQUOTE;
									else
										stream->html_mode_flag
											= HTML_MODE_TAG;

									/* stop scanning the string */
									break;
								}

								/*
								 *   if we're looking for a quote, check
								 *   for the closing quote; otherwise,
								 *   check for alphanumerics
								 */
								if (qu != 0)
								{
									/* if this is our quote, stop scanning */
									if (c == qu)
										break;
								}
								else
								{
									/* if it's non-alphanumeric, we're done */
									if (!outisal(c) && !outisdg(c))
										break;
								}
							}

							/* skip the closing quote, if necessary */
							if (qu != 0 && c == qu)
								c = nextout(&s, &slen);

							/* null-terminate the value string */
							*dst = '\0';
						}
						else
						{
							/* no value */
							attrval[0] = '\0';
						}

						/*
						 *   see if we recognize it, and it's meaningful
						 *   in the context of the current tag
						 */
						if (!scumm_stricmp(attrname, "height")
							&& stream->html_defer_br != HTML_DEFER_BR_NONE)
						{
							int ht;

							/*
							 *   If the height is zero, always treat this
							 *   as a non-blanking flush.  If it's one,
							 *   treat it as we originally planned to.  If
							 *   it's greater than one, add n blank lines.
							 */
							ht = atoi(attrval);
							if (ht == 0)
							{
								/* always use non-blanking flush */
								stream->html_defer_br = HTML_DEFER_BR_FLUSH;
							}
							else if (ht == 1)
							{
								/* keep original setting */
							}
							else
							{
								for ( ; ht > 0 ; --ht)
									outblank_stream(stream);
							}
						}
						else if (!scumm_stricmp(attrname, "alt")
								 && !stream->html_in_ignore
								 && stream->html_allow_alt)
						{
							/* write out the ALT string */
							outstring_stream(stream, attrval);
						}

						/*
						 *   since we already read the next character,
						 *   simply loop back immediately
						 */
						continue;
					}
					break;
				}
				break;

			case HTML_MODE_DQUOTE:
				/* if we've reached the closing quote, return to tag state */
				if (c == '"')
					stream->html_mode_flag = HTML_MODE_TAG;
				break;

			case HTML_MODE_SQUOTE:
				/* if we've reached the closing quote, return to tag state */
				if (c == '\'')
					stream->html_mode_flag = HTML_MODE_TAG;
				break;
			}

			/*
			 *   move on to the next character, and start over with the
			 *   new character
			 */
			c = nextout(&s, &slen);
			continue;
		}

		/*
		 *   If we're in a title, and this isn't the start of a new tag,
		 *   skip the character - we suppress all regular text output
		 *   inside a <TITLE> ... </TITLE> sequence.
		 */
		if (stream->html_in_ignore && c != '<')
		{
			/* check for entities */
			char cbuf[50];
			if (c == '&')
			{
				/* translate the entity */
				c = out_parse_entity(cbuf, sizeof(cbuf), &s, &slen);
			}
			else
			{
				/* it's an ordinary character - copy it out literally */
				cbuf[0] = c;
				cbuf[1] = '\0';

				/* get the next character */
				c = nextout(&s, &slen);
			}

			/*
			 *   if we're gathering a title, and there's room in the title
			 *   buffer for more (always leaving room for a null
			 *   terminator), add this to the title buffer
			 */
			if (stream->html_in_title)
			{
				char *cbp;
				for (cbp = cbuf ; *cbp != '\0' ; ++cbp)
				{
					/* if there's room, add it */
					if (stream->html_title_ptr + 1 <
						stream->html_title_buf
						+ sizeof(stream->html_title_buf))
						*stream->html_title_ptr++ = *cbp;
				}
			}

			/* don't display anything in an ignore section */
			continue;
		}

		if ( c == '%' )                              /* translation string? */
		{
			infmt = 1;
			f = fmsbuf;
		}
		else if ( c == '\\' )                       /* special escape code? */
		{
			c = nextout(&s, &slen);

			if (stream->capturing && c != '^' && c != 'v' && c != '\0')
			{
				outchar_stream(stream, '\\');
				outchar_stream(stream, c);

				/* keep the \- and also put out the next two chars */
				if (c == '-')
				{
					outchar_stream(stream, nextout(&s, &slen));
					outchar_stream(stream, nextout(&s, &slen));
				}
			}
			else
			{
				switch(c)
				{
				case 'H':                                /* HTML mode entry */
					/* turn on HTML mode in the renderer */
					switch(c = nextout(&s, &slen))
					{
					case '-':
						/* if we have an HTML target, notify it */
						if (stream->html_target)
						{
							/* flush its stream */
							outflushn_stream(stream, 0);

							/* tell the OS layer to switch to normal mode */
							out_end_html(stream);
						}

						/* switch to normal mode */
						stream->html_mode = FALSE;
						break;

					case '+':
					default:
						/* if we have an HTML target, notify it */
						if (stream->html_target)
						{
							/* flush the underlying stream */
							outflushn_stream(stream, 0);

							/* tell the OS layer to switch to HTML mode */
							out_start_html(stream);
						}

						/* switch to HTML mode */
						stream->html_mode = TRUE;

						/*
						 *   if the character wasn't a "+", it's not part
						 *   of the "\H" sequence, so display it normally
						 */
						if (c != '+' && c != 0)
							outchar_stream(stream, c);
						break;
					}

					/* this sequence doesn't result in any actual output */
					break;

				case 'n':                                       /* newline? */
					outflushn_stream(stream, 1);        /* yes, output line */
					break;

				case 't':                                           /* tab? */
					outtab_stream(stream);
					break;

				case 'b':                                    /* blank line? */
					outblank_stream(stream);
					break;

				case '\0':                               /* line ends here? */
					done = 1;
					break;

				case ' ':                                   /* quoted space */
					if (stream->html_target && stream->html_mode)
					{
						/*
						 *   we're generating for an HTML target and we're
						 *   in HTML mode - generate the HTML non-breaking
						 *   space
						 */
						outstring_stream(stream, "&nbsp;");
					}
					else
					{
						/*
						 *   we're not in HTML mode - generate our
						 *   internal quoted space character
						 */
						outchar_stream(stream, QSPACE);
					}
					break;

				case '^':                      /* capitalize next character */
					stream->capsflag = 1;
					stream->nocapsflag = 0;
					break;

				case 'v':
					stream->nocapsflag = 1;
					stream->capsflag = 0;
					break;

				case '(':
					/* generate HTML if in the appropriate mode */
					if (stream->html_mode && stream->html_target)
					{
						/* send HTML to the renderer */
						outstring_stream(stream, "<B>");
					}
					else
					{
						/* turn on the 'hilite' attribute */
						stream->cur_attr |= OS_ATTR_HILITE;
					}
					break;

				case ')':
					/* generate HTML if in the appropriate mode */
					if (stream->html_mode && stream->html_target)
					{
						/* send HTML to the renderer */
						outstring_stream(stream, "</B>");
					}
					else
					{
						/* turn off the 'hilite' attribute */
						stream->cur_attr &= ~OS_ATTR_HILITE;
					}
					break;

				case '-':
					outchar_stream(stream, nextout(&s, &slen));
					outchar_stream(stream, nextout(&s, &slen));
					break;

				default:                 /* just pass invalid escapes as-is */
					outchar_stream(stream, c);
					break;
				}
			}
		}
		else if (!stream->html_target
				 && stream->html_mode
				 && (c == '<' || c == '&'))
		{
			/*
			 *   We're in HTML mode, but the underlying target does not
			 *   accept HTML sequences.  It appears we're at the start of
			 *   an "&" entity or a tag sequence, so parse it, remove it,
			 *   and replace it (if possible) with a text-only equivalent.
			 */
			if (c == '<')
			{
				/* read the tag */
				char tagbuf[50];
				int is_end_tag;
				c = read_tag(tagbuf, sizeof(tagbuf), &is_end_tag,
							 &s, &slen, nullptr);

				/*
				 *   Check to see if we recognize the tag.  We only
				 *   recognize a few simple tags that map easily to
				 *   character mode.
				 */
				if (!scumm_stricmp(tagbuf, "br"))
				{
					/*
					 *   line break - if there's anything buffered up,
					 *   just flush the current line, otherwise write out
					 *   a blank line
					 */
					if (stream->html_in_ignore)
						/* suppress breaks in ignore mode */;
					else if (stream->linepos != 0)
						stream->html_defer_br = HTML_DEFER_BR_FLUSH;
					else
						stream->html_defer_br = HTML_DEFER_BR_BLANK;
				}
				else if (!scumm_stricmp(tagbuf, "b")
						 || !scumm_stricmp(tagbuf, "i")
						 || !scumm_stricmp(tagbuf, "em")
						 || !scumm_stricmp(tagbuf, "strong"))
				{
					int attr = 0;

					/* choose the attribute flag */
					switch (tagbuf[0])
					{
					case 'b':
					case 'B':
						attr = OS_ATTR_BOLD;
						break;

					case 'i':
					case 'I':
						attr = OS_ATTR_ITALIC;
						break;

					case 'e':
					case 'E':
						attr = OS_ATTR_EM;
						break;

					case 's':
					case 'S':
						attr = OS_ATTR_STRONG;
						break;
					}

					/* bold on/off - send out appropriate os-layer code */
					if (stream->html_in_ignore)
					{
						/* suppress any change in 'ignore' mode */
					}
					else if (!is_end_tag)
					{
						/* turn on the selected attribute */
						stream->cur_attr |= attr;
					}
					else
					{
						/* turn off the selected attribute */
						stream->cur_attr &= ~attr;
					}
				}
				else if (!scumm_stricmp(tagbuf, "p"))
				{
					/* paragraph - send out a blank line */
					if (!stream->html_in_ignore)
						outblank_stream(stream);
				}
				else if (!scumm_stricmp(tagbuf, "tab"))
				{
					/* tab - send out a \t */
					if (!stream->html_in_ignore)
						outtab_stream(stream);
				}
				else if (!scumm_stricmp(tagbuf, "img") || !scumm_stricmp(tagbuf, "sound"))
				{
					/* IMG and SOUND - allow ALT attributes */
					stream->html_allow_alt = TRUE;
				}
				else if (!scumm_stricmp(tagbuf, "hr"))
				{
					int rem;

					if (!stream->html_in_ignore)
					{
						/* start a new line */
						outflushn_stream(stream, 1);

						/* write out underscores to the display width */
						for (rem = G_os_linewidth - 1 ; rem > 0 ; )
						{
							char dashbuf[100];
							int cur;

							/* do as much as we can on this pass */
							cur = rem;
							if ((size_t)cur > sizeof(dashbuf) - 1)
								cur = sizeof(dashbuf) - 1;

							/* do a buffer-full of dashes */
							memset(dashbuf, '_', cur);
							dashbuf[cur] = '\0';
							outstring_stream(stream, dashbuf);

							/* deduct this from the total */
							rem -= cur;
						}

						/* put a blank line after the underscores */
						outblank_stream(stream);
					}
				}
				else if (!scumm_stricmp(tagbuf, "q"))
				{
					unsigned int htmlchar;

					if (!stream->html_in_ignore)
					{
						/* if it's an open quote, increment the level */
						if (!is_end_tag)
							++(stream->html_quote_level);

						/* add the open quote */
						htmlchar =
							(!is_end_tag
							 ? ((stream->html_quote_level & 1) == 1
								? 8220 : 8216)
							 : ((stream->html_quote_level & 1) == 1
								? 8221 : 8217));

						/*
						 *   write out the HTML character, translated to
						 *   the local character set
						 */
						outchar_html_stream(stream, htmlchar);

						/* if it's a close quote, decrement the level */
						if (is_end_tag)
							--(stream->html_quote_level);
					}
				}
				else if (!scumm_stricmp(tagbuf, "title"))
				{
					/*
					 *   Turn ignore mode on or off as appropriate, and
					 *   turn on or off title mode as well.
					 */
					if (is_end_tag)
					{
						/*
						 *   note that we're leaving an ignore section and
						 *   a title section
						 */
						--(stream->html_in_ignore);
						--(stream->html_in_title);

						/*
						 *   if we're no longer in a title, call the OS
						 *   layer to tell it the title string, in case it
						 *   wants to change the window title or otherwise
						 *   make use of the title
						 */
						if (stream->html_in_title == 0)
						{
							/* null-terminate the title string */
							*stream->html_title_ptr = '\0';

							/* tell the OS about the title */
							os_set_title(stream->html_title_buf);
						}
					}
					else
					{
						/*
						 *   if we aren't already in a title, set up to
						 *   capture the title into the title buffer
						 */
						if (!stream->html_in_title)
							stream->html_title_ptr = stream->html_title_buf;

						/*
						 *   note that we're in a title and in an ignore
						 *   section, since nothing within gets displayed
						 */
						++(stream->html_in_ignore);
						++(stream->html_in_title);
					}
				}
				else if (!scumm_stricmp(tagbuf, "aboutbox"))
				{
					/* turn ignore mode on or off as appropriate */
					if (is_end_tag)
						--(stream->html_in_ignore);
					else
						++(stream->html_in_ignore);
				}
				else if (!scumm_stricmp(tagbuf, "pre"))
				{
					/* count the nesting level if starting PRE mode */
					if (!is_end_tag)
						stream->html_pre_level += 1;

					/* surround the PRE block with line breaks */
					outblank_stream(stream);

					/* count the nesting level if ending PRE mode */
					if (is_end_tag && stream->html_pre_level != 0)
						stream->html_pre_level -= 1;
				}

				/* suppress everything up to the next '>' */
				stream->html_mode_flag = HTML_MODE_TAG;

				/*
				 *   continue with the current character; since we're in
				 *   html tag mode, we'll skip everything until we get to
				 *   the closing '>'
				 */
				continue;
			}
			else if (c == '&')
			{
				/* parse it */
				char  xlat_buf[50];
				c = out_parse_entity(xlat_buf, sizeof(xlat_buf), &s, &slen);

				/* write it out (we've already translated it) */
				outstring_noxlat_stream(stream, xlat_buf);

				/* proceed with the next character */
				continue;
			}
		}
		else if (stream->html_target && stream->html_mode && c == '<')
		{
			/*
			 *   We're in HTML mode, and we have an underlying HTML target.
			 *   We don't need to do much HTML interpretation at this level.
			 *   However, we do need to keep track of when we're in a PRE
			 *   block, so that we can pass whitespaces and newlines through
			 *   to the underlying HTML engine without filtering when we're
			 *   in preformatted text.
			 */
			char tagbuf[50];
			int is_end_tag;
			c = read_tag(tagbuf, sizeof(tagbuf), &is_end_tag,
						 &s, &slen, stream);

			/* check for special tags */
			if (!scumm_stricmp(tagbuf, "pre"))
			{
				/* count the nesting level */
				if (!is_end_tag)
					stream->html_pre_level += 1;
				else if (is_end_tag && stream->html_pre_level != 0)
					stream->html_pre_level -= 1;
			}

			/* copy the last character after the tag to the stream */
			outchar_stream(stream, c);
		}
		else
		{
			/* normal character */
			outchar_stream(stream, c);
		}

		/* move on to the next character, unless we're finished */
		if (done)
			c = '\0';
		else
			c = nextout(&s, &slen);
	}

	/* if we ended up inside what looked like a format string, dump string */
	if (infmt)
	{
		outchar_stream(stream, '%');
		for (f1 = fmsbuf ; f1 < f ; ++f1)
			outchar_stream(stream, *f1);
	}

	/* exit a recursion level */
	out_pop_stream();

	/* success */
	return 0;
}

/* ------------------------------------------------------------------------ */
/*
 *   Parse an HTML entity markup
 */
static char out_parse_entity(char *outbuf, size_t outbuf_size, const char **sp, size_t *slenp) {
	char  ampbuf[10];
	char *dst;
	const char *orig_s;
	size_t orig_slen;
	const amp_tbl_t *ampptr;
	size_t lo, hi, cur;
	char  c;

	/*
	 *   remember where the part after the '&' begins, so we can come back
	 *   here later if necessary
	 */
	orig_s = *sp;
	orig_slen = *slenp;

	/* get the character after the ampersand */
	c = nextout(sp, slenp);

	/* if it's numeric, parse the number */
	if (c == '#')
	{
		uint val;

		/* skip the '#' */
		c = nextout(sp, slenp);

		/* check for hex */
		if (c == 'x' || c == 'X')
		{
			/* skip the 'x' */
			c = nextout(sp, slenp);

			/* read the hex number */
			for (val = 0 ; Common::isXDigit((uchar)c) ; c = nextout(sp, slenp))
			{
				/* accumulate the current digit into the value */
				val *= 16;
				if (outisdg(c))
					val += c - '0';
				else if (c >= 'a' && c <= 'f')
					val += c - 'a' + 10;
				else
					val += c - 'A' + 10;
			}
		}
		else
		{
			/* read the number */
			for (val = 0 ; outisdg(c) ; c = nextout(sp, slenp))
			{
				/* accumulate the current digit into the value */
				val *= 10;
				val += c - '0';
			}
		}

		/* if we found a ';' at the end, skip it */
		if (c == ';')
			c = nextout(sp, slenp);

		/* translate the character into the output buffer */
		os_xlat_html4(val, outbuf, outbuf_size);

		/* we're done with this character */
		return c;
	}

	/*
	 *   Parse the sequence after the '&'.  Parse up to the closing
	 *   semicolon, or any non-alphanumeric, or until we fill up the buffer.
	 */
	for (dst = ampbuf ;
		 c != '\0' && (outisdg(c) || outisal(c))
			 && dst < ampbuf + sizeof(ampbuf) - 1 ;
		 *dst++ = c, c = nextout(sp, slenp)) ;

	/* null-terminate the name */
	*dst = '\0';

	/* do a binary search for the name */
	lo = 0;
	hi = sizeof(amp_tbl)/sizeof(amp_tbl[0]) - 1;
	for (;;)
	{
		int diff;

		/* if we've converged, look no further */
		if (lo > hi || lo >= sizeof(amp_tbl)/sizeof(amp_tbl[0]))
		{
			ampptr = nullptr;
			break;
		}

		/* split the difference */
		cur = lo + (hi - lo)/2;
		ampptr = &amp_tbl[cur];

		/* see where we are relative to the target item */
		diff = strcmp(ampptr->cname, ampbuf);
		if (diff == 0)
		{
			/* this is it */
			break;
		}
		else if (diff > 0)
		{
			/* make sure we don't go off the end */
			if (cur == hi && cur == 0)
			{
				/* we've failed to find it */
				ampptr = nullptr;
				break;
			}

			/* this one is too high - check the lower half */
			hi = (cur == hi ? hi - 1 : cur);
		}
		else
		{
			/* this one is too low - check the upper half */
			lo = (cur == lo ? lo + 1 : cur);
		}
	}

	/* skip to the appropriate next character */
	if (c == ';')
	{
		/* name ended with semicolon - skip the semicolon */
		c = nextout(sp, slenp);
	}
	else if (ampptr != nullptr)
	{
		int skipcnt;

		/* found the name - skip its exact length */
		skipcnt = strlen(ampptr->cname);
		for (*sp = orig_s, *slenp = orig_slen ; skipcnt != 0 ;
			 c = nextout(sp, slenp), --skipcnt) ;
	}

	/* if we found the entry, write out the character */
	if (ampptr != nullptr)
	{
		/*
		 *   if this one has an external mapping table entry, use the mapping
		 *   table entry; otherwise, use the default OS routine mapping
		 */
		if (ampptr->expan != nullptr)
		{
			/*
			 *   we have an explicit expansion from the mapping table file -
			 *   use it
			 */
			size_t copylen = strlen(ampptr->expan);
			if (copylen > outbuf_size - 1)
				copylen = outbuf_size - 1;

			memcpy(outbuf, ampptr->expan, copylen);
			outbuf[copylen] = '\0';
		}
		else
		{
			/*
			 *   there's no mapping table expansion - use the default OS code
			 *   expansion
			 */
			os_xlat_html4(ampptr->html_cval, outbuf, outbuf_size);
		}
	}
	else
	{
		/*
		 *   didn't find it - output the '&' literally, then back up and
		 *   output the entire sequence following
		 */
		*sp = orig_s;
		*slenp = orig_slen;
		c = nextout(sp, slenp);

		/* fill in the '&' return value */
		outbuf[0] = '&';
		outbuf[1] = '\0';
	}

	/* return the next character */
	return c;
}



/* ------------------------------------------------------------------------ */
/*
 *   Initialize the output formatter
 */
void out_init()
{
	/* not yet hiding output */
	outflag = 1;
	outcnt = 0;
	hidout = 0;

	/* initialize the standard display stream */
	out_init_std(&G_std_disp);

	/* initialize the log file stream */
	out_init_log(&G_log_disp);
}


/* ------------------------------------------------------------------------ */
/*
 *   initialize the property translation table
 */
void tiosetfmt(tiocxdef *ctx, runcxdef *rctx, uchar *fbase, uint flen)
{
	VARUSED(ctx);
	fmsbase = fbase;
	fmstop = fbase + flen;
	runctx = rctx;
}


/* ------------------------------------------------------------------------ */
/*
 *   Map an HTML entity to a local character value.  The character table
 *   reader will call this routine during initialization if it finds HTML
 *   entities in the mapping table file.  We'll remember these mappings
 *   for use in translating HTML entities to the local character set.
 *
 *   Note that the standard run-time can only display a single character
 *   set, so every HTML entity that we display must be mapped to the
 *   single active native character set.
 */
void tio_set_html_expansion(unsigned int html_char_val,
							const char *expansion, size_t expansion_len)
{
	amp_tbl_t *p;

	/* find the character value */
	for (p = amp_tbl ;
		 p < amp_tbl + sizeof(amp_tbl)/sizeof(amp_tbl[0]) ; ++p)
	{
		/* if this is the one, store it */
		if (p->html_cval == html_char_val)
		{
			/* allocate space for it */
			p->expan = (char *)osmalloc(expansion_len + 1);

			/* save it */
			memcpy(p->expan, expansion, expansion_len);
			p->expan[expansion_len] = '\0';

			/* no need to look any further */
			return;
		}
	}
}


/* ------------------------------------------------------------------------ */
/*
 *   Write out a c-style (null-terminated) string.
 */
int outformat(const char *s) {
	return outformatlen(s, strlen(s));
}


/* ------------------------------------------------------------------------ */
/*
 *   This routine sends out a string, one character at a time (via outchar).
 *   Escape codes ('\n', and so forth) are handled here.
 */
int outformatlen(const char *s, uint slen) {
	char     c;
	uint     orig_slen;
	const char *orig_s;
	int      ret;
	int      called_filter;

	/* presume we'll return success */
	ret = 0;

	/* presume we won't call the filter function */
	called_filter = FALSE;

	/* if there's a user filter function to invoke, call it */
	if (G_user_filter != MCMONINV)
	{
		/* push the string */
		runpstr(runctx, s, slen, 1);

		/* call the filter */
		runfn(runctx, G_user_filter, 1);

		/*
		 *   note that we called the filter, so that we'll remove the
		 *   result of the filter from the stack before we return
		 */
		called_filter = TRUE;

		/* if the result is a string, use it in place of the original text */
		if (runtostyp(runctx) == DAT_SSTRING)
		{
			runsdef val;
			uchar *p;

			/* pop the value */
			runpop(runctx, &val);

			/*
			 *   get the text from the string, and use it as a replacement
			 *   for the original string
			 */
			p = val.runsv.runsvstr;
			slen = osrp2(p) - 2;
			s = (char *)(p + 2);

			/*
			 *   push the string back onto the stack - this will ensure
			 *   that the string stays referenced while we're working, so
			 *   that the garbage collector won't delete it
			 */
			runrepush(runctx, &val);
		}
	}

	/* remember the original string, before we scan the first character */
	orig_s = s;
	orig_slen = slen;

	/* get the first character to display */
	c = nextout(&s, &slen);

	/* if the string is non-empty, note that we've displayed something */
	if (c != 0)
		outcnt = 1;

	/* check to see if we're hiding output */
	if (out_is_hidden())
		goto done;

	/* if the debugger is showing watchpoints, suppress all output */
	if (outwxflag)
		goto done;

	/* display the string */
	ret = outformatlen_stream(&G_std_disp, orig_s, orig_slen);

	/* if there's a log file, write to the log file as well */
	if (logfp != nullptr)
	{
		outformatlen_stream(&G_log_disp, orig_s, orig_slen);
		osfflush(logfp);
	}

done:
	/* if we called the filter, remove the result from the stack */
	if (called_filter)
		rundisc(runctx);

	/* return the result from displaying to the screen */
	return ret;
}

/* ------------------------------------------------------------------------ */
/*
 *   Display a blank line
 */
void outblank()
{
	/* note that we've displayed something */
	outcnt = 1;

	/* check to see if we're hiding output */
	if (out_is_hidden())
		return;

	/* generate the newline to the standard display */
	outblank_stream(&G_std_disp);

	/* if we're logging, generate the newline to the log file as well */
	if (logfp != nullptr)
	{
		outblank_stream(&G_log_disp);
		osfflush(logfp);
	}
}


/* ------------------------------------------------------------------------ */
/*
 *   outcaps() - sets an internal flag which makes the next letter output
 *   a capital, whether it came in that way or not.  Set the same state in
 *   both formatters (standard and log).
 */
void outcaps(void)
{
	outcaps_stream(&G_std_disp);
	outcaps_stream(&G_log_disp);
}

/*
 *   outnocaps() - sets the next letter to a miniscule, whether it came in
 *   that way or not.
 */
void outnocaps(void)
{
	outnocaps_stream(&G_std_disp);
	outnocaps_stream(&G_log_disp);
}

/* ------------------------------------------------------------------------ */
/*
 *   Open a log file
 */
int tiologopn(tiocxdef *ctx, char *fn)
{
	/* if there's an old log file, close it */
	if (tiologcls(ctx))
		return 1;

	/* save the filename for later */
	strcpy(logfname, fn);

	/* open the new file */
	logfp = osfopwt(fn, OSFTLOG);

	/*
	 *   Reset the log file's output formatter state, since we're opening
	 *   a new file.
	 */
	out_init_log(&G_log_disp);

	/*
	 *   Set the log file's HTML source mode flag to the same value as is
	 *   currently being used in the main display stream, so that it will
	 *   interpret source markups the same way that the display stream is
	 *   going to.
	 */
	G_log_disp.html_mode = G_std_disp.html_mode;

	/* return 0 on success, non-zero on failure */
	return (logfp == nullptr);
}

/*
 *   Close the log file
 */
int tiologcls(tiocxdef *ctx)
{
	/* if we have a file, close it */
	if (logfp != nullptr)
	{
		/* close the handle */
		osfcls(logfp);

		/* set the system file type to "log file" */
		os_settype(logfname, OSFTLOG);

		/* forget about our log file handle */
		logfp = nullptr;
	}

	/* success */
	return 0;
}

/* ------------------------------------------------------------------------ */
/*
 *   Write text explicitly to the log file.  This can be used to add
 *   special text (such as prompt text) that would normally be suppressed
 *   from the log file.  When more mode is turned off, we don't
 *   automatically copy text to the log file; any text that the caller
 *   knows should be in the log file during times when more mode is turned
 *   off can be explicitly added with this function.
 *
 *   If nl is true, we'll add a newline at the end of this text.  The
 *   caller should not include any newlines in the text being displayed
 *   here.
 */
void out_logfile_print(const char *txt, int nl)
{
	/* if there's no log file, there's nothing to do */
	if (logfp == nullptr)
		return;

	/* add the text */
	os_fprintz(logfp, txt);

	/* add a newline if desired */
	if (nl)
	{
		/* add a normal newline */
		os_fprintz(logfp, "\n");

		/* if the logfile is an html target, write an HTML line break */
		if (G_log_disp.html_target && G_log_disp.html_mode)
			os_fprintz(logfp, "<BR HEIGHT=0>\n");
	}

	/* flush the output */
	osfflush(logfp);
}

/* ------------------------------------------------------------------------ */
/*
 *   Set the current MORE mode
 */
int setmore(int state)
{
	int oldstate = G_os_moremode;

	G_os_moremode = state;
	return oldstate;
}

/* ------------------------------------------------------------------------ */
/*
 *   Run the MORE prompt.  If the output layer takes responsibility for
 *   pagination issues (i.e., USE_MORE is defined), we'll simply display
 *   the prompt and wait for input.  Otherwise, the OS layer controls the
 *   MORE prompt, so we'll call the OS-layer function to display the
 *   prompt.
 */
void out_more_prompt()
{
#ifdef USE_MORE
	/*
	 *   USE_MORE defined - we take responsibility for pagination.  Show
	 *   our default MORE prompt and wait for a keystroke.
	 */

	int done;
	int next_page = FALSE;

	/* display the "MORE" prompt */
	os_printz("[More]");
	os_flush();

	/* wait for an acceptable keystroke */
	for (done = FALSE ; !done ; )
	{
		os_event_info_t evt;

		/* get an event */
		switch(os_get_event(0, FALSE, &evt))
		{
		case OS_EVT_KEY:
			switch(evt.key[0])
			{
			case ' ':
				/* stop waiting, show one page */
				done = TRUE;
				next_page = TRUE;
				break;

			case '\r':
			case '\n':
				/* stop waiting, show one line */
				done = TRUE;
				next_page = FALSE;
				break;

			default:
				/* ignore any other keystrokes */
				break;
			}
			break;

		case OS_EVT_EOF:
			/* end of file - there's nothing to wait for now */
			done = TRUE;
			next_page = TRUE;

			/* don't use more prompts any more, as the user can't respond */
			G_os_moremode = FALSE;
			break;

		default:
			/* ignore other events */
			break;
		}
	}

	/*
	 *   Remove the prompt from the screen by backing up and overwriting
	 *   it with spaces.  (Note that this assumes that we're running in
	 *   some kind of terminal or character mode with a fixed-pitch font;
	 *   if that's not the case, the OS layer should be taking
	 *   responsibility for pagination anyway, so this code shouldn't be
	 *   in use in the first place.)
	 */
	os_printz("\r      \r");

	/*
	 *   if they pressed the space key, it means that we should show an
	 *   entire new page, so reset the line count to zero; otherwise,
	 *   we'll want to display another MORE prompt at the very next line,
	 *   so leave the line count alone
	 */
	if (next_page)
		G_std_disp.linecnt = 0;

#else /* USE_MORE */

	/*
	 *   USE_MORE is undefined - this means that the OS layer is taking
	 *   all responsibility for pagination.  We must ask the OS layer to
	 *   display the MORE prompt, because we can't make any assumptions
	 *   about what the prompt looks like.
	 */

	os_more_prompt();

#endif /* USE_MORE */
}

/* ------------------------------------------------------------------------ */
/*
 *   reset output
 */
void outreset(void)
{
	G_std_disp.linecnt = 0;
}

/* ------------------------------------------------------------------------ */
/*
 *   Determine if HTML mode is active.  Returns true if so, false if not.
 *   Note that this merely indicates whether an "\H+" sequence is
 *   currently active -- this will return true after an "\H+" sequence,
 *   even on text-only interpreters.
 */
int tio_is_html_mode()
{
	/* return the current HTML mode flag for the standard display stream */
	return G_std_disp.html_mode;
}


/* ------------------------------------------------------------------------ */
/*
 *   Capture routines.  Capture affects only the standard display output
 *   stream; there's no need to capture information redundantly in the log
 *   file stream.
 */

/*
 *   Begin/end capturing
 */
void tiocapture(tiocxdef *tioctx, mcmcxdef *memctx, int flag)
{
	if (flag)
	{
		/* create a new object if necessary */
		if (G_std_disp.capture_obj == MCMONINV)
		{
			mcmalo(memctx, 256, &G_std_disp.capture_obj);
			mcmunlck(memctx, G_std_disp.capture_obj);
		}

		/* remember the memory context */
		G_std_disp.capture_ctx = memctx;
	}

	/*
	 *   remember capture status in the standard output stream as well as
	 *   the log stream
	 */
	G_std_disp.capturing = flag;
	G_log_disp.capturing = flag;
}

/* clear all captured output */
void tioclrcapture(tiocxdef *tioctx)
{
	G_std_disp.capture_ofs = 0;
}

/* clear captured output back to a given size */
void tiopopcapture(tiocxdef *tioctx, uint orig_size)
{
	G_std_disp.capture_ofs = orig_size;
}

/* get the object handle of the captured output */
mcmon tiogetcapture(tiocxdef *ctx)
{
	return G_std_disp.capture_obj;
}

/* get the amount of text captured */
uint tiocapturesize(tiocxdef *ctx)
{
	return G_std_disp.capture_ofs;
}

/* ------------------------------------------------------------------------ */
/*
 *   set the current actor
 */
void tiosetactor(tiocxdef *ctx, objnum actor)
{
	VARUSED(ctx);
	cmdActor = actor;
}

/*
 *   get the current actor
 */
objnum tiogetactor(tiocxdef *ctx)
{
	VARUSED(ctx);
	return cmdActor;
}

/* ------------------------------------------------------------------------ */
/*
 *   Flush the output line.  We'll write to both the standard display and
 *   the log file, as needed.
 */
void outflushn(int nl)
{
	/* flush the display stream */
	outflushn_stream(&G_std_disp, nl);

	/* flush the log stream, if we have an open log file */
	if (logfp != nullptr)
	{
		outflushn_stream(&G_log_disp, nl);
		osfflush(logfp);
	}
}

/*
 *   flush the current line, and start a new line
 */
void outflush(void)
{
	/* use the common flushing routine in mode 1 (regular newline) */
	outflushn(1);
}

/* ------------------------------------------------------------------------ */
/*
 *   Hidden text routines
 */

/*
 *   outhide - hide output in the standard display stream
 */
void outhide(void)
{
	outflag = 0;
	outcnt = 0;
	hidout = 0;
}

/*
 *   Check output status.  Indicate whether output is currently hidden,
 *   and whether any hidden output has occurred.
 */
void outstat(int *hidden, int *output_occurred)
{
	*hidden = !outflag;
	*output_occurred = outcnt;
}

/* set the flag to indicate that output has occurred */
void outsethidden(void)
{
	outcnt = 1;
	hidout = 1;
}

/*
 *   outshow() - turns output back on, and returns TRUE (1) if any output
 *   has occurred since the last outshow(), FALSE (0) otherwise.
 */
int outshow(void)
{
	/* turn output back on */
	outflag = 1;

	/* if we're debugging, note the end of hidden output */
	if (dbghid && hidout)
	{
		hidout = 0;
		trcsho();
	}

	/* return the flag indicating whether hidden output occurred */
	return outcnt;
}

/* ------------------------------------------------------------------------ */
/*
 *   start/end watchpoint evaluation - suppress all dstring output
 */
void outwx(int flag)
{
	outwxflag = flag;
}


/* ------------------------------------------------------------------------ */
/*
 *   Set the user filter function.  Setting this to MCMONINV clears the
 *   filter.
 */
void out_set_filter(objnum filter_fn)
{
	/* remember the filter function */
	G_user_filter = filter_fn;
}

/* ------------------------------------------------------------------------ */
/*
 *   Set the double-space mode
 */
void out_set_doublespace(int dbl)
{
	/* remember the new setting */
	doublespace = dbl;
}

int tio_askfile(const char *prompt, char *reply, int replen, int prompt_type, os_filetype_t file_type) {
	// let the OS layer handle it
	return os_askfile(prompt, reply, replen, prompt_type, file_type);
}

int tio_input_dialog(int icon_id, const char *prompt, int standard_button_set,
	const char **buttons, int button_count,
	int default_index, int cancel_index) {
	// call the OS implementation
	return os_input_dialog(icon_id, prompt, standard_button_set,
		buttons, button_count,
		default_index, cancel_index);
}

void outfmt(tiocxdef *ctx, uchar *txt) {
	uint len;

	VARUSED(ctx);

	/* read the length prefix */
	len = osrp2(txt) - 2;
	txt += 2;

	/* write out the string */
	tioputslen(ctx, (char *)txt, len);
}

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk
