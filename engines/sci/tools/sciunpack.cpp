/***************************************************************************
 sciunpack.c Copyright (C) 1999, 2000, 2001 Christoph Reichenbach


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Christoph Reichenbach (CJR) [creichen@rbg.informatik.tu-darmstadt.de]

 History:

   990327 - created (CJR)

***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "sciunpack.h"

#include <sciresource.h>
#include <engine.h>
#include <console.h>

/* #define DRAW_GRAPHICS */

#undef HAVE_OBSTACK_H

#ifdef _MSC_VER
#  include <direct.h>
#  define extern __declspec(dllimport) extern
#endif

#ifdef HAVE_GETOPT_H
#	ifndef WIN32
#		include <getopt.h>
#	else
#		include <win32/getopt.h>
#	endif
#endif /* HAVE_GETOPT_H */

#ifdef DRAW_GRAPHICS
#  ifdef HAVE_LIBPNG
#    include "graphics_png.h"
#  endif /* HAVE_LIBPNG */
#endif /* DRAW_GRAPHICS */

#if defined (_MSC_VER) || defined (__BEOS__) || defined(__amigaos4__)
/* [DJ] fchmod is not in Visual C++ RTL - and probably not needed,anyway */
/* [RS] (see comment above, but read MS-DOS instead of Visual C++ RTL) */
#  define fchmod(file,mode)
#  define CREAT_OPTIONS O_BINARY
#endif

#ifndef CREAT_OPTIONS
#  define CREAT_OPTIONS 0x640
#endif


#define ACT_UNPACK 0
#define ACT_WORDS 1
#define ACT_LIST 2
#define ACT_SCRIPTDUMP 3
#define ACT_VOCABDUMP 4

#define ACT_DEFAULT ACT_UNPACK

static int conversion = 0;
static int list = 0;
static int verbose = 0;
static int with_header = 1;
static int color_mode = 0;
static int action = ACT_DEFAULT;
static guint8 midimask = 0x01;  /* MT-32 */

resource_mgr_t *resmgr;

#ifdef WIN32
#define fchmod(arg1, arg2)
#endif

void
print_resource_filename(FILE* file, int type, int number)
{
	if (resmgr->sci_version < SCI_VERSION_1)
		fprintf(file, "%s.%03d", sci_resource_types[type], number);
	else
		fprintf(file, "%d.%s", number, sci_resource_type_suffixes[type]);
}

void
sprint_resource_filename(char* buf, int type, int number)
{
	if (resmgr->sci_version < SCI_VERSION_1)
		sprintf(buf, "%s.%03d", sci_resource_types[type], number);
	else
		sprintf(buf, "%d.%s", number, sci_resource_type_suffixes[type]);
}

#ifdef HAVE_GETOPT_LONG
static struct option options[] = {
	{"conversion", no_argument, &conversion, 1},
	{"version", no_argument, 0, 256},
	{"verbose", no_argument, &verbose, 1},
	{"help", no_argument, 0, 'h'},
	{"output-file", required_argument, 0, 'o'},
	{"unpack", no_argument, &action, ACT_UNPACK},
	{"list", no_argument, &action, ACT_LIST},
	{"words", no_argument, &action, ACT_WORDS},
	{"vocab", no_argument, &action, ACT_VOCABDUMP},
	{"objects", no_argument, &action, ACT_SCRIPTDUMP},
	{"with-header", no_argument, &with_header, 1},
	{"without-header", no_argument, &with_header, 0},
	{"sort-alpha", no_argument, &vocab_sort, SORT_METHOD_ALPHA},
	{"sort-group", no_argument, &vocab_sort, SORT_METHOD_GROUP},
#ifdef DRAW_GRAPHICS
	{"palette-dither", no_argument, &color_mode, SCI_COLOR_DITHER},
	{"palette-interpolate", no_argument, &color_mode, SCI_COLOR_INTERPOLATE},
	{"palette-dither256", no_argument, &color_mode, SCI_COLOR_DITHER256},
#endif /* DRAW_GRAPHICS */
	{"gamedir", required_argument, 0, 'd'},
	{"midimask", required_argument, 0, 'M'},
	{0, 0, 0, 0}};

#endif /* HAVE_GETOPT_LONG */


void unpack_resource(int stype, int snr, char *outfilename);


int main(int argc, char** argv)
{
	int retval = 0;
	int i;
	int stype = -1;
	int snr;
	char *resourcenumber_string = 0;
	char *outfilename = 0;
	int optindex = 0;
	int c;
	char *gamedir = sci_getcwd();
	int res_version = SCI_VERSION_AUTODETECT;
	
#ifdef HAVE_GETOPT_LONG
	while ((c = getopt_long(argc, argv, "WOVUvhLcr:o:d:M:", options, &optindex)) > -1) {
#else /* !HAVE_GETOPT_LONG */
	while ((c = getopt(argc, argv, "WOVUvhLcr:o:d:M:")) > -1) {
#endif /* !HAVE_GETOPT_LONG */

		switch (c) {
		case 256:
			printf("sciunpack ("PACKAGE") "VERSION"\n");
			printf("This program is copyright (C) 1999, 2000, 2001 Christoph Reichenbach,\n"
			       " Lars Skovlund, Magnus Reftel\n"
			       "It comes WITHOUT WARRANTY of any kind.\n"
			       "This is free software, released under the GNU General Public License.\n");
			exit(0);

		case 'h': {
			char *gcc_3_0_can_kiss_my_ass =
			       "Usage: sciunpack [options] [-U] <resource.number>\n"
			       "       sciunpack [options] [-U] <resource> <number>\n"
			       "Unpacks resource data\n"
			       "If * is specified instead of <number>, \n"
			       "all resources of given type will be unpacked.\n\n"
			       "       sciunpack [options] -W\n"
			       "Lists vocabulary words\n\n"
			       "       sciunpack [options] -O\n"
			       "Dumps the complete object hierarchy\n\n"
			       "       sciunpack [options] -V\n"
			       "Prints selector names, opcodes, kernel names, and classes\n\n"
			       "\nAvalable operations:\n"
			       " --unpack      -U       Decompress resource\n"
			       " --list        -L       List all resources\n"
			       " --words       -W       List all vocabulary words\n"
			       " --objects     -O       Print all objects\n"
			       " --vocab       -V       Lists the complete vocabulary\n"
			       "\nAvailable options:\n"
			       "General:\n"
			       " --version              Prints the version number\n"
			       " --verbose     -v       Enables additional output\n"
			       " --help        -h       Displays this help message\n"
			       " --midimask    -M       What 'play mask' to use.  Defaults to MT-32 (0x01)\n"

			       "Listing words:\n"
			       " --sort-alpha		sort in alphabetical order\n"
			       " --sort-group		sort in group order\n"
			       "Unpacking:\n"
			       " --convert     -c       Converts selected resources\n"
			       " --output-file -o       Selects output file\n"
			       " --gamedir     -d       Read game resources from dir\n"
			       " --with-header          Forces the SCI header to be written (default)\n"
			       " --without-header       Prevents the two SCI header bytes from being written\n"
#ifdef DRAW_GRAPHICS
			       " --palette-dither       Forces colors in 16 color games to be dithered\n"
			       " --palette-interpolate  Does color interpolation when drawing picture resources\n"
			       " --palette-dither256    Does dithering in 256 colors\n"
#endif /* DRAW_GRAPHICS */
			       "\nAs a default, 'resource.number' is the output filename.\n"
			       "If conversion is enabled, the following resources will be treated specially:\n"
			       "  sound resources:   Will be converted to MIDI, stored in <number>.midi\n"
			       "  script resources:  Will be dissected and  stored in <number>.script\n"
#ifdef DRAW_GRAPHICS
			       "  picture resources: Will be converted to PNG, stored in <number>.png\n"

#endif /* DRAW_GRAPHICS */
				;

			printf(gcc_3_0_can_kiss_my_ass);
			exit(0);
		}

		case 'v':
			verbose = 1;
			break;

		case 'L':
			action = ACT_LIST;
			break;

		case 'W':
			action = ACT_WORDS;
			break;

		case 'V':
			action = ACT_VOCABDUMP;
			break;

		case 'O':
			action = ACT_SCRIPTDUMP;
			break;

		case 'o':
			outfilename = optarg;
			break;

		case 'd':
			if (gamedir) sci_free (gamedir);
			gamedir = sci_strdup (optarg);
			break;

		case 'r':
			res_version = atoi(optarg);
			break;

		case 'c':
			conversion = 1;
			break;

		case 'M':
			midimask = (guint8) strtol(optarg, NULL, 0);
			break;

		case 0: /* getopt_long already did this for us */
		case '?':
			/* getopt_long already printed an error message. */
			break;

		default:
			return -1;
		}
	}

	if (action == ACT_UNPACK) {
		char *resstring = argv[optind];

		if (optind == argc) {
			fprintf(stderr,"Resource identifier required\n");
			return 1;
		}

		if ((resourcenumber_string = (char *) strchr(resstring, '.'))) {
			*resourcenumber_string++ = 0;
		} else if (optind+1 == argc) {
			fprintf(stderr,"Resource number required\n");
			return 1;
		} else resourcenumber_string = argv[optind+1];

		for (i=0; i< 18; i++)
			if ((strcmp(sci_resource_types[i], resstring)==0)) stype = i;
		if (stype==-1) {
			printf("Could not find the resource type '%s'.\n", resstring);
			return 1;
		}
	} /* ACT_UNPACK */

	if (gamedir)
		if (chdir (gamedir)) {
			printf ("Error changing to game directory '%s'\n", gamedir);
			exit(1);
		}

	if (!(resmgr = scir_new_resource_manager(gamedir, res_version,
						 0, 1024*128))) {
		fprintf(stderr,"Could not find any resources; quitting.\n");
		exit(1);
	}

	if (verbose) printf("Autodetect determined: %s\n",
			    sci_version_types[resmgr->sci_version]);


	switch (action) {

	case ACT_LIST: {
		int i;

		if (verbose) {
			for (i=0; i < resmgr->resources_nr; i++) {
				printf("%i: ",i);
				print_resource_filename(stdout,
							resmgr->resources[i].type,
							resmgr->resources[i].number);
				printf("   has size %i\n", resmgr->resources[i].size);
			}

			fprintf(stderr," Reading complete. Actual resource count is %i\n",
				resmgr->resources_nr);
		} else {
			for (i=0; i<resmgr->resources_nr; i++) {
				print_resource_filename(stdout,
							resmgr->resources[i].type,
							resmgr->resources[i].number);
				printf("\n");
			}
		}
		break;
	}

	case ACT_UNPACK: {

		if (!strcmp (resourcenumber_string, "*")) {
			int i;
			for (i=0; i<resmgr->resources_nr; i++)
				if (resmgr->resources[i].type == stype)
					unpack_resource (stype, resmgr->resources[i].number, NULL);
		} else {
			snr = atoi(resourcenumber_string);
			unpack_resource(stype, snr, outfilename);
		}
		break;
	}

	case ACT_WORDS:
		retval = vocab_print();
		break;

	case ACT_SCRIPTDUMP:
		retval = script_dump();
		break;

	case ACT_VOCABDUMP:
		retval = vocab_dump();
		break;

	default:
		fprintf(stderr,"Invalid action %d- internal error!\n", action);
		return 1;
	}


	scir_free_resource_manager(resmgr);
	return retval;
}


void unpack_resource(int stype, int snr, char *outfilename)
{
	char fnamebuffer[12]; /* stores default file name */
	resource_t *found;

	if ((stype == sci_sound) && conversion && (resmgr->sci_version > SCI_VERSION_0)) {
		fprintf(stderr,"MIDI conversion is only supported for SCI version 0\n");
		conversion = 0;
	}

	if (!outfilename) {
		outfilename = fnamebuffer;
		if ((stype == sci_sound) && conversion) {
#ifdef HAVE_OBSTACK_H
			map_MIDI_instruments(resmgr);
#endif
			sprintf(outfilename,"%03d.midi", snr);
		}
#ifdef DRAW_GRAPHICS
		else if ((stype == sci_pic) && conversion)
			sprintf(outfilename,"%03d.png", snr);
#endif /* DRAW_GRAPHICS */
		else
			sprint_resource_filename(outfilename, stype, snr);
	}

	if (verbose) {
		printf("seeking ");
		print_resource_filename(stdout, stype, snr);
		printf("...\n");
	}

	if ((found = scir_find_resource(resmgr, stype, snr, 0))) {

#ifdef DRAW_GRAPHICS
		if ((stype == sci_pic) && conversion) {
			int i;
			picture_t pic = alloc_empty_picture(SCI_RESOLUTION_320X200, SCI_COLORDEPTH_8BPP);
			draw_pic0(pic, 1, 0, found->data);
			if ((i = write_pic_png(outfilename, pic->maps[0]))) {
				fprintf(stderr,"Writing the png failed (%d)\n",i);
			} else if (verbose) printf("Done.\n");
			free_picture(pic);
		} else
#endif /* DRAW_GRAPHICS */
		if ((stype == sci_script) && conversion) {
			sprintf (outfilename, "%03d.script", snr);
                        open_console_file (outfilename);
			script_dissect(resmgr, snr, NULL, 0);
                        close_console_file();
		} else {

/* Visual C++ doesn't allow to specify O_BINARY with creat() */
#ifdef _MSC_VER
                        int outf = open(outfilename, _O_CREAT | _O_BINARY | _O_RDWR);
#else
                        int outf = creat(outfilename, CREAT_OPTIONS);
#endif

#ifdef HAVE_OBSTACK_H
			if ((stype == sci_sound) && conversion) {
				int midilength;
				guint8 *outdata = makeMIDI0(found->data, &midilength, midimask);
				if (!outdata) {
					fprintf(stderr,"MIDI conversion failed. Aborting...\n");
					return;
				}
				if (verbose) printf("MIDI conversion from %d bytes of sound resource"
						    " to a %d bytes MIDI file.\n",
						    found->size, midilength);
				write(outf, outdata, midilength);
				free(outdata);
			} else {
#endif /* HAVE_OBSTACK_H */
				guint8 header = 0x80 | found->type;

				if (with_header) {
					write(outf, &header, 1);
					header = 0x00;
					write(outf, &header, 1);
				}

				write(outf,  found->data, found->size);
#ifdef HAVE_OBSTACK_H
			}
#endif /* HAVE_OBSTACK_H */

			fchmod(outf, 0644);
			close(outf);
			fchmod(outf, 0644);

			if (verbose) printf("Done.\n");
		}

	} else printf("Resource not found.\n");
}



