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

/*  _______         ____    __         ___    ___
 * \    _  \       \    /  \  /       \   \  /   /       '   '  '
 *  |  | \  \       |  |    ||         |   \/   |         .      .
 *  |  |  |  |      |  |    ||         ||\  /|  |
 *  |  |  |  |      |  |    ||         || \/ |  |         '  '  '
 *  |  |  |  |      |  |    ||         ||    |  |         .      .
 *  |  |_/  /        \  \__//          ||    |  |
 * /_______/ynamic    \____/niversal  /__\  /____\usic   /|  .  . ibliotheque
 *                                                      /  \
 *                                                     / .  \
 * dumb.h - The user header file for DUMB.            / / \  \
 *                                                   | <  /   \_
 * Include this file in any of your files in         |  \/ /\   /
 * which you wish to use the DUMB functions           \_  /  > /
 * and variables.                                       | \ / /
 *                                                      |  ' /
 *                                                       \__/
 */

#ifndef AGS_LIB_DUMB_DUMBFILE_H
#define AGS_LIB_DUMB_DUMBFILE_H

#include "common/stream.h"

namespace AGS3 {

#ifdef __cplusplus
extern "C" {
#endif


#define DUMB_MAJOR_VERSION    0
#define DUMB_MINOR_VERSION    9
#define DUMB_REVISION_VERSION 2

#define DUMB_VERSION (DUMB_MAJOR_VERSION*10000 + DUMB_MINOR_VERSION*100 + DUMB_REVISION_VERSION)

#define DUMB_VERSION_STR "0.9.2"

#define DUMB_NAME "DUMB v" DUMB_VERSION_STR

#define DUMB_YEAR  2003
#define DUMB_MONTH 4
#define DUMB_DAY   2

#define DUMB_YEAR_STR2  "03"
#define DUMB_YEAR_STR4  "2003"
#define DUMB_MONTH_STR1 "4"
#define DUMB_DAY_STR1   "2"

#if DUMB_MONTH < 10
#define DUMB_MONTH_STR2 "0" DUMB_MONTH_STR1
#else
#define DUMB_MONTH_STR2 DUMB_MONTH_STR1
#endif

#if DUMB_DAY < 10
#define DUMB_DAY_STR2 "0" DUMB_DAY_STR1
#else
#define DUMB_DAY_STR2 DUMB_DAY_STR1
#endif


	/* WARNING: The month and day were inadvertently swapped in the v0.8 release.
	 *          Please do not compare this constant against any date in 2002. In
	 *          any case, DUMB_VERSION is probably more useful for this purpose.
	 */
#define DUMB_DATE (DUMB_YEAR*10000 + DUMB_MONTH*100 + DUMB_DAY)

#define DUMB_DATE_STR DUMB_DAY_STR1 "." DUMB_MONTH_STR1 "." DUMB_YEAR_STR4


#undef MIN
#undef MAX
#undef MID

#define MIN(x,y)   (((x) < (y)) ? (x) : (y))
#define MAX(x,y)   (((x) > (y)) ? (x) : (y))
#define MID(x,y,z) MAX((x), MIN((y), (z)))

#undef ABS
#define ABS(x) (((x) >= 0) ? (x) : (-(x)))


#ifdef DEBUGMODE

#ifndef ASSERT
#include <assert.h>
#define ASSERT(n) assert(n)
#endif
#ifndef TRACE
	 // it would be nice if this did actually trace ...
#define TRACE 1 ? (void)0 : (void)printf
#endif

#else

#ifndef ASSERT
#define ASSERT(n)
#endif
#ifndef TRACE
#define TRACE 1 ? (void)0 : (void)printf
#endif

#endif


#define DUMB_ID(a,b,c,d) (((unsigned int)(a) << 24) | \
                          ((unsigned int)(b) << 16) | \
                          ((unsigned int)(c) <<  8) | \
                          ((unsigned int)(d)      ))



#ifndef LONG_LONG
#ifdef __GNUC__
#define LONG_LONG long long
#elif defined _MSC_VER
#define LONG_LONG __int64
#else
#error 64-bit integer type unknown
#endif
#endif

#if __GNUC__ * 100 + __GNUC_MINOR__ >= 301 /* GCC 3.1+ */
#ifndef DUMB_DECLARE_DEPRECATED
#define DUMB_DECLARE_DEPRECATED
#endif
#define DUMB_DEPRECATED __attribute__((__deprecated__))
#else
#define DUMB_DEPRECATED
#endif


	 /* Basic Sample Type. Normal range is -0x800000 to 0x7FFFFF. */

	typedef int sample_t;


	/* Library Clean-up Management */

	int dumb_atexit(void (*proc)(void));

	void dumb_exit(void);


	/* File Input Functions */

	typedef struct DUMBFILE_SYSTEM {
		void *(*open)(const char *filename);
		int (*skip)(void *f, long n);
		int (*getc)(void *f);
		long (*getnc)(char *ptr, long n, void *f);
		void (*close)(void *f);
	}
	DUMBFILE_SYSTEM;

	typedef struct DUMBFILE DUMBFILE;

	void register_dumbfile_system(DUMBFILE_SYSTEM *dfs);

	DUMBFILE *dumbfile_open(const char *filename);
	DUMBFILE *dumbfile_open_ex(void *file, DUMBFILE_SYSTEM *dfs);

	long dumbfile_pos(DUMBFILE *f);
	int dumbfile_skip(DUMBFILE *f, long n);

	int dumbfile_getc(DUMBFILE *f);

	int dumbfile_igetw(DUMBFILE *f);
	int dumbfile_mgetw(DUMBFILE *f);

	long dumbfile_igetl(DUMBFILE *f);
	long dumbfile_mgetl(DUMBFILE *f);

	unsigned long dumbfile_cgetul(DUMBFILE *f);
	signed long dumbfile_cgetsl(DUMBFILE *f);

	long dumbfile_getnc(char *ptr, long n, DUMBFILE *f);

	int dumbfile_error(DUMBFILE *f);
	int dumbfile_close(DUMBFILE *f);


	/* stdio File Input Module */

	void dumb_register_stdfiles(void);

	DUMBFILE *dumbfile_open_stdfile(Common::ReadStream *p);


	/* Memory File Input Module */

	DUMBFILE *dumbfile_open_memory(const char *data, long size);


	/* DUH Management */

	typedef struct DUH DUH;

#define DUH_SIGNATURE DUMB_ID('D','U','H','!')

	void unload_duh(DUH *duh);

	DUH *load_duh(const char *filename);
	DUH *read_duh(DUMBFILE *f);

	long duh_get_length(DUH *duh);


	/* Signal Rendering Functions */

	typedef struct DUH_SIGRENDERER DUH_SIGRENDERER;

	DUH_SIGRENDERER *duh_start_sigrenderer(DUH *duh, int sig, int n_channels, long pos);

#ifdef DUMB_DECLARE_DEPRECATED
	typedef void (*DUH_SIGRENDERER_CALLBACK)(void *data, sample_t **samples, int n_channels, long length);
	/* This is deprecated, but is not marked as such because GCC tends to
	 * complain spuriously when the typedef is used later. See comments below.
	 */

	void duh_sigrenderer_set_callback(
		DUH_SIGRENDERER *sigrenderer,
		DUH_SIGRENDERER_CALLBACK callback, void *data
	) DUMB_DEPRECATED;
	/* The 'callback' argument's type has changed for const-correctness. See the
	 * DUH_SIGRENDERER_CALLBACK definition just above. Also note that the samples
	 * in the buffer are now 256 times as large; the normal range is -0x800000 to
	 * 0x7FFFFF. The function has been renamed partly because its functionality
	 * has changed slightly and partly so that its name is more meaningful. The
	 * new one is duh_sigrenderer_set_analyser_callback(), and the typedef for
	 * the function pointer has also changed, from DUH_SIGRENDERER_CALLBACK to
	 * DUH_SIGRENDERER_ANALYSER_CALLBACK. (If you wanted to use this callback to
	 * apply a DSP effect, don't worry; there is a better way of doing this. It
	 * is undocumented, so contact me and I shall try to help. Contact details
	 * are in readme.txt.)
	 */
#endif

	typedef void (*DUH_SIGRENDERER_ANALYSER_CALLBACK)(void *data, const sample_t *const *samples, int n_channels, long length);

	void duh_sigrenderer_set_analyser_callback(
		DUH_SIGRENDERER *sigrenderer,
		DUH_SIGRENDERER_ANALYSER_CALLBACK callback, void *data
	);

	int duh_sigrenderer_get_n_channels(DUH_SIGRENDERER *sigrenderer);
	long duh_sigrenderer_get_position(DUH_SIGRENDERER *sigrenderer);

	void duh_sigrenderer_set_sigparam(DUH_SIGRENDERER *sigrenderer, unsigned char id, long value);

	long duh_sigrenderer_get_samples(
		DUH_SIGRENDERER *sigrenderer,
		float volume, float delta,
		long size, sample_t **samples
	);

	void duh_sigrenderer_get_current_sample(DUH_SIGRENDERER *sigrenderer, float volume, sample_t *samples);

	void duh_end_sigrenderer(DUH_SIGRENDERER *sigrenderer);


	/* DUH Rendering Functions */

	long duh_render(
		DUH_SIGRENDERER *sigrenderer,
		int bits, int unsign,
		float volume, float delta,
		long size, void *sptr
	);

#ifdef DUMB_DECLARE_DEPRECATED

	long duh_render_signal(
		DUH_SIGRENDERER *sigrenderer,
		float volume, float delta,
		long size, sample_t **samples
	) DUMB_DEPRECATED;
	/* Please use duh_sigrenderer_get_samples(). Arguments and functionality are
	 * identical.
	 */

	typedef DUH_SIGRENDERER DUH_RENDERER DUMB_DEPRECATED;
	/* Please use DUH_SIGRENDERER instead of DUH_RENDERER. */

	DUH_SIGRENDERER *duh_start_renderer(DUH *duh, int n_channels, long pos) DUMB_DEPRECATED;
	/* Please use duh_start_sigrenderer() instead. Pass 0 for 'sig'. */

	int duh_renderer_get_n_channels(DUH_SIGRENDERER *dr) DUMB_DEPRECATED;
	long duh_renderer_get_position(DUH_SIGRENDERER *dr) DUMB_DEPRECATED;
	/* Please use the duh_sigrenderer_*() equivalents of these two functions. */

	void duh_end_renderer(DUH_SIGRENDERER *dr) DUMB_DEPRECATED;
	/* Please use duh_end_sigrenderer() instead. */

	DUH_SIGRENDERER *duh_renderer_encapsulate_sigrenderer(DUH_SIGRENDERER *sigrenderer) DUMB_DEPRECATED;
	DUH_SIGRENDERER *duh_renderer_get_sigrenderer(DUH_SIGRENDERER *dr) DUMB_DEPRECATED;
	DUH_SIGRENDERER *duh_renderer_decompose_to_sigrenderer(DUH_SIGRENDERER *dr) DUMB_DEPRECATED;
	/* These functions have become no-ops that just return the parameter.
	 * So, for instance, replace
	 *   duh_renderer_encapsulate_sigrenderer(my_sigrenderer)
	 * with
	 *   my_sigrenderer
	 */

#endif


	 /* Impulse Tracker Support */

	extern int dumb_it_max_to_mix;

	typedef struct DUMB_IT_SIGDATA DUMB_IT_SIGDATA;
	typedef struct DUMB_IT_SIGRENDERER DUMB_IT_SIGRENDERER;

	DUMB_IT_SIGDATA *duh_get_it_sigdata(DUH *duh);
	DUH_SIGRENDERER *duh_encapsulate_it_sigrenderer(DUMB_IT_SIGRENDERER *it_sigrenderer, int n_channels, long pos);
	DUMB_IT_SIGRENDERER *duh_get_it_sigrenderer(DUH_SIGRENDERER *sigrenderer);

	DUH_SIGRENDERER *dumb_it_start_at_order(DUH *duh, int n_channels, int startorder);

	void dumb_it_set_loop_callback(DUMB_IT_SIGRENDERER *sigrenderer, int (*callback)(void *data), void *data);
	void dumb_it_set_xm_speed_zero_callback(DUMB_IT_SIGRENDERER *sigrenderer, int (*callback)(void *data), void *data);
	void dumb_it_set_midi_callback(DUMB_IT_SIGRENDERER *sigrenderer, int (*callback)(void *data, int channel, unsigned char byte), void *data);

	int dumb_it_callback_terminate(void *data);
	int dumb_it_callback_midi_block(void *data, int channel, unsigned char byte);

	DUH *dumb_load_it(const char *filename);
	DUH *dumb_load_xm(const char *filename);
	DUH *dumb_load_s3m(const char *filename);
	DUH *dumb_load_mod(const char *filename);

	DUH *dumb_read_it(DUMBFILE *f);
	DUH *dumb_read_xm(DUMBFILE *f);
	DUH *dumb_read_s3m(DUMBFILE *f);
	DUH *dumb_read_mod(DUMBFILE *f);

	int dumb_it_sd_get_n_orders(DUMB_IT_SIGDATA *sd);

	int dumb_it_sd_get_initial_global_volume(DUMB_IT_SIGDATA *sd);
	void dumb_it_sd_set_initial_global_volume(DUMB_IT_SIGDATA *sd, int gv);

	int dumb_it_sd_get_mixing_volume(DUMB_IT_SIGDATA *sd);
	void dumb_it_sd_set_mixing_volume(DUMB_IT_SIGDATA *sd, int mv);

	int dumb_it_sd_get_initial_speed(DUMB_IT_SIGDATA *sd);
	void dumb_it_sd_set_initial_speed(DUMB_IT_SIGDATA *sd, int speed);

	int dumb_it_sd_get_initial_tempo(DUMB_IT_SIGDATA *sd);
	void dumb_it_sd_set_initial_tempo(DUMB_IT_SIGDATA *sd, int tempo);

	int dumb_it_sd_get_initial_channel_volume(DUMB_IT_SIGDATA *sd, int channel);
	void dumb_it_sd_set_initial_channel_volume(DUMB_IT_SIGDATA *sd, int channel, int volume);

	int dumb_it_sr_get_current_order(DUMB_IT_SIGRENDERER *sr);
	int dumb_it_sr_get_current_row(DUMB_IT_SIGRENDERER *sr);

	int dumb_it_sr_get_global_volume(DUMB_IT_SIGRENDERER *sr);
	void dumb_it_sr_set_global_volume(DUMB_IT_SIGRENDERER *sr, int gv);

	int dumb_it_sr_get_tempo(DUMB_IT_SIGRENDERER *sr);
	void dumb_it_sr_set_tempo(DUMB_IT_SIGRENDERER *sr, int tempo);

	int dumb_it_sr_get_speed(DUMB_IT_SIGRENDERER *sr);
	void dumb_it_sr_set_speed(DUMB_IT_SIGRENDERER *sr, int speed);

#define DUMB_IT_N_CHANNELS 64
#define DUMB_IT_N_NNA_CHANNELS 192
#define DUMB_IT_TOTAL_CHANNELS (DUMB_IT_N_CHANNELS + DUMB_IT_N_NNA_CHANNELS)

	/* Channels passed to any of these functions are 0-based */
	int dumb_it_sr_get_channel_volume(DUMB_IT_SIGRENDERER *sr, int channel);
	void dumb_it_sr_set_channel_volume(DUMB_IT_SIGRENDERER *sr, int channel, int volume);

	typedef struct DUMB_IT_CHANNEL_STATE DUMB_IT_CHANNEL_STATE;

	struct DUMB_IT_CHANNEL_STATE {
		int channel; /* 0-based; meaningful for NNA channels */
		int sample; /* 1-based; 0 if nothing playing, then other fields undef */
		int freq; /* in Hz */
		float volume; /* 1.0 maximum; affected by ALL factors, inc. mixing vol */
		unsigned char pan; /* 0-64, 100 for surround */
		signed char subpan; /* use (pan + subpan/256.0f) or ((pan<<8)+subpan) */
		unsigned char filter_cutoff;    /* 0-127    cutoff=127 AND resonance=0 */
		unsigned char filter_subcutoff; /* 0-255      -> no filters (subcutoff */
		unsigned char filter_resonance; /* 0-127        always 0 in this case) */
		/* subcutoff only changes from zero if filter envelopes are in use. The
		 * calculation (filter_cutoff + filter_subcutoff/256.0f) gives a more
		 * accurate filter cutoff measurement as a float. It would often be more
		 * useful to use a scaled int such as ((cutoff<<8) + subcutoff).
		 */
	};

	/* Values of 64 or more will access NNA channels here. */
	void dumb_it_sr_get_channel_state(DUMB_IT_SIGRENDERER *sr, int channel, DUMB_IT_CHANNEL_STATE *state);


	/* Signal Design Helper Values */

	/* Use pow(DUMB_SEMITONE_BASE, n) to get the 'delta' value to transpose up by
	 * n semitones. To transpose down, use negative n.
	 */
#define DUMB_SEMITONE_BASE 1.059463094359295309843105314939748495817

	 /* Use pow(DUMB_QUARTERTONE_BASE, n) to get the 'delta' value to transpose up
	  * by n quartertones. To transpose down, use negative n.
	  */
#define DUMB_QUARTERTONE_BASE 1.029302236643492074463779317738953977823

	  /* Use pow(DUMB_PITCH_BASE, n) to get the 'delta' value to transpose up by n
	   * units. In this case, 256 units represent one semitone; 3072 units
	   * represent one octave. These units are used by the sequence signal (SEQU).
	   */
#define DUMB_PITCH_BASE 1.000225659305069791926712241547647863626


	   /* Signal Design Function Types */

	typedef void sigdata_t;
	typedef void sigrenderer_t;

	typedef sigdata_t *(*DUH_LOAD_SIGDATA)(DUH *duh, DUMBFILE *file);

	typedef sigrenderer_t *(*DUH_START_SIGRENDERER)(
		DUH *duh,
		sigdata_t *sigdata,
		int n_channels,
		long pos
		);

	typedef void (*DUH_SIGRENDERER_SET_SIGPARAM)(
		sigrenderer_t *sigrenderer,
		unsigned char id, long value
		);

	typedef long (*DUH_SIGRENDERER_GET_SAMPLES)(
		sigrenderer_t *sigrenderer,
		float volume, float delta,
		long size, sample_t **samples
		);

	typedef void (*DUH_SIGRENDERER_GET_CURRENT_SAMPLE)(
		sigrenderer_t *sigrenderer,
		float volume,
		sample_t *samples
		);

	typedef void (*DUH_END_SIGRENDERER)(sigrenderer_t *sigrenderer);

	typedef void (*DUH_UNLOAD_SIGDATA)(sigdata_t *sigdata);


	/* Signal Design Function Registration */

	typedef struct DUH_SIGTYPE_DESC {
		long type;
		DUH_LOAD_SIGDATA                   load_sigdata;
		DUH_START_SIGRENDERER              start_sigrenderer;
		DUH_SIGRENDERER_SET_SIGPARAM       sigrenderer_set_sigparam;
		DUH_SIGRENDERER_GET_SAMPLES        sigrenderer_get_samples;
		DUH_SIGRENDERER_GET_CURRENT_SAMPLE sigrenderer_get_current_sample;
		DUH_END_SIGRENDERER                end_sigrenderer;
		DUH_UNLOAD_SIGDATA                 unload_sigdata;
	}
	DUH_SIGTYPE_DESC;

	void dumb_register_sigtype(DUH_SIGTYPE_DESC *desc);


	// Decide where to put these functions; new heading?

	sigdata_t *duh_get_raw_sigdata(DUH *duh, int sig, long type);

	DUH_SIGRENDERER *duh_encapsulate_raw_sigrenderer(sigrenderer_t *vsigrenderer, DUH_SIGTYPE_DESC *desc, int n_channels, long pos);
	sigrenderer_t *duh_get_raw_sigrenderer(DUH_SIGRENDERER *sigrenderer, long type);


	/* Sample Buffer Allocation Helpers */

	sample_t **create_sample_buffer(int n_channels, long length);
	void destroy_sample_buffer(sample_t **samples);


	/* Silencing Helper */

	void dumb_silence(sample_t *samples, long length);


	/* Click Removal Helpers */

	typedef struct DUMB_CLICK_REMOVER DUMB_CLICK_REMOVER;

	DUMB_CLICK_REMOVER *dumb_create_click_remover(void);
	void dumb_record_click(DUMB_CLICK_REMOVER *cr, long pos, sample_t step);
	void dumb_remove_clicks(DUMB_CLICK_REMOVER *cr, sample_t *samples, long length, float halflife);
	sample_t dumb_click_remover_get_offset(DUMB_CLICK_REMOVER *cr);
	void dumb_destroy_click_remover(DUMB_CLICK_REMOVER *cr);

	DUMB_CLICK_REMOVER **dumb_create_click_remover_array(int n);
	void dumb_record_click_array(int n, DUMB_CLICK_REMOVER **cr, long pos, sample_t *step);
	void dumb_record_click_negative_array(int n, DUMB_CLICK_REMOVER **cr, long pos, sample_t *step);
	void dumb_remove_clicks_array(int n, DUMB_CLICK_REMOVER **cr, sample_t **samples, long length, float halflife);
	void dumb_click_remover_get_offset_array(int n, DUMB_CLICK_REMOVER **cr, sample_t *offset);
	void dumb_destroy_click_remover_array(int n, DUMB_CLICK_REMOVER **cr);


	/* Resampling Helpers */

#define DUMB_RQ_ALIASING 0
#define DUMB_RQ_LINEAR   1
#define DUMB_RQ_CUBIC    2
#define DUMB_RQ_N_LEVELS 3
	extern int dumb_resampling_quality;

	typedef struct DUMB_RESAMPLER DUMB_RESAMPLER;

	typedef void (*DUMB_RESAMPLE_PICKUP)(DUMB_RESAMPLER *resampler, void *data);

	struct DUMB_RESAMPLER {
		sample_t *src;
		long pos;
		int subpos;
		long start, end;
		int dir;
		DUMB_RESAMPLE_PICKUP pickup;
		void *pickup_data;
		int min_quality;
		int max_quality;
		/* Everything below this point is internal: do not use. */
		sample_t x[3];
		int overshot;
	};

	void dumb_reset_resampler(DUMB_RESAMPLER *resampler, sample_t *src, long pos, long start, long end);
	DUMB_RESAMPLER *dumb_start_resampler(sample_t *src, long pos, long start, long end);
	long dumb_resample(DUMB_RESAMPLER *resampler, sample_t *dst, long dst_size, float volume, float delta);
	sample_t dumb_resample_get_current_sample(DUMB_RESAMPLER *resampler, float volume);
	void dumb_end_resampler(DUMB_RESAMPLER *resampler);


	/* DUH Construction */

	DUH *make_duh(
		long length,
		int n_signals,
		DUH_SIGTYPE_DESC *desc[],
		sigdata_t *sigdata[]
	);


#ifdef __cplusplus
}
#endif

} // namespace AGS3

#endif
