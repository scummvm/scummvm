/*
 *      Master sprite draw include file.  Include as main body of each
 *      desired sprite routine, defining macros as appropriate to generate
 *      the correct algorithm:
 *
 *      Set "three_d"     to true for 3d depth coding
 *      Set "bresenham"   to true for Bresenham scaling
 *      Set "packed_attr" to true for packed attribute screen (PANNING rooms)
 *      Set "translate"   to true for 16 color translation version
 *
 *      Set "interface"   to true for special interface version (exclusive
 *                        of all other options)
 *
 *      Define "attribute" to draw to attribute page (in depth code) instead.
 *      Define "monodraw"  to draw in monochrome color
 *
 *      Define "on_black"  to draw only on top of "zero" pixels.
 *
 *      If "packed_attr" is true, then "three_d" MUST be true also.
 *
 *      Parameters required:
 *
 *      NAME            TYPE            USAGE           DESCRIPTION
 *      ===================================================================
 *      series          SeriesPtr       (all)           Series handle
 *      id              int             (all)           Sprite #
 *      buf             buffer *        (all)           Target buffer
 *      target_x        int             (all)           Sprite base X
 *      target_y        int             (all)           Sprite base Y
 *
 *      attr            buffer *        (three_d)       Attribute Buffer
 *      target_depth    int             (three_d)       Sprite depth code
 *
 *      view_port_x     int             (packed_attr)   Work/Orig X mapping
 *      view_port_y     int             (packed_attr)   Work/Orig Y mapping
 *
 *      scale_factor    int             (bresenham)     Scale % size (10-100)
 *
 *      <Set high bit of sprite "id" for mirror imaging>
 *
 *
 *      DRAW ROUTINES                      three_d  packattr  bres.  trans.
 *      ====================================================================
 *      sprite_draw()                         -        -        -      -
 *      sprite_draw_3d()                      *        -        -      -
 *      sprite_draw_3d_scaled()               *        -        *      -
 *      sprite_draw_3d_big()                  *        *        -      -
 *      sprite_draw_3d_scaled_big()           *        *        *      -
 *      sprite_draw_x16()                     -        -        -      *
 *      sprite_draw_3d_x16()                  *        -        -      *
 *      sprite_draw_3d_scaled_x16()           *        -        *      *
 *      sprite_draw_3d_big_x16()              *        *        -      *
 *      sprite_draw_3d_scaled_big_x16()       *        *        *      *
 *
 *      sprite_draw_interface()
 */

/*
 *    Filter out illegal compile combinations
 */

#if packed_attr
#if !three_d
#error Cannot compile packed_attr without three_d!
#endif
#endif

#if interface
#if (three_d || bresenham || translate)
#error Interface is exclusive of all other options
#endif
#endif

#ifdef attribute
#if (!three_d || !bresenham || translate || !packed_attr)
#error Attribute draw must be (three_d,bresenham,!translate,packed_attr)
#endif
#endif


/*
 *    Local variables common to all versions
 */

byte *sprite_ptr;
byte *target_ptr;
int target_wrap;
int xs, ys;
int skip_y, draw_y, max_y, stop_y;
int skip_x, draw_x, max_x, stop_x;
int line_finished;
int mirror;
int draw_count;
SpritePtr sprite;

/*
/*    Local variables for "three_d" depth coding
*/

#if three_d
byte *attr_ptr;
#endif

#define huge_code (packed_attr && translate)

/*
 *    Local variables for "packed_attr" large attribute screens
 */

#if packed_attr
int attr_wrap;
int attr_start;
#endif

/*
 *    Local variables for "bresenham" sprite scaling
 */

#if bresenham
#define   max_bresenham_entries     320
byte      bresenham_table[max_bresenham_entries];
int       bresenham_entries;
int       temp_x, temp_y;
#endif

/*
 *    Local variables for "translate" 16 color thatching translations
 */

#if translate
byte thatch_base;
byte thatch_flag;
#endif

/*
 * In certain configurations, the main draw routine does not need
 * all of the registers for computations; for those variations,
 * certain commonly used variables (skip_x & stop_x) are aliased
 * for clarity. These were originally aliased to CPU registers.
 */

/* SKIP_X and STOP_X were originally aliased to registers (cx, di)
 * in certain configurations to save memory accesses. In C they are
 * always the plain variables.
 */
#define SKIP_X  skip_x
#define STOP_X  stop_x

/* Get maximum clipping coordinates */
max_x = buf->x - 1;
max_y = buf->y - 1;

/* Load up sprite pointer info: check high bit of id for mirror flag */
if (id < 0) {
	mirror = -1;
	id &= 0x7fff;
} else {
	mirror = 1;
}

sprite = &series->index[id - 1];
sprite_ptr = sprite->data;

/* Load up buffer pointer info */
target_ptr = buf->data;
target_wrap = buf->x;

/* Load up attribute pointer info */
#if three_d
attr_ptr = attr->data;
#endif

/* Get attribute buffer wrap boundary */
#if packed_attr
attr_wrap = attr->x;
#endif

/* Get sprite size and (if Bresenham) number of table entries */
#if bresenham
temp_x = sprite->xs;
temp_y = sprite->ys;

bresenham_entries = MAX(temp_x, temp_y);
#else
xs = sprite->xs;
ys = sprite->ys;
#endif

/*
 *    BRESENHAM TABLE
 *
 *    Generate a Bresenham table useable for both X and Y checks.
 *    Each entry is true if that source pixel maps to a drawn pixel
 *    at the given scale factor.
 */

#if bresenham
{
	int bres_acc = 50;  /* Start accumulator at 50 (midpoint) */
	xs = 0;
	ys = 0;

	for (int bi = 0; bi < bresenham_entries; bi++) {
		bres_acc += scale_factor;
		if (bres_acc >= 100) {
			bresenham_table[bi] = true;
			bres_acc -= 100;

			if (bi < temp_x)
				xs++;
			if (bi < temp_y)
				ys++;
		} else {
			bresenham_table[bi] = false;
		}
	}

	/* Adjust target coordinates: coords are passed as middle-lower;
	   must be changed to upper-left based on scaled matte. */
	target_x -= xs / 2;
	target_y -= ys;
	target_y++;
}
#endif


/*
 *    SPRITE CLIPPING
 *
 *    Clip the sprite into the target buffer's boundaries.
 *    Generates skip_x, skip_y, draw_x, draw_y, stop_x, stop_y.
 */

/* X clipping */
{
	int bx = xs;         /* proposed X size */
	int cx = 0;          /* assume no left clipping */
	int ax = target_x;
	int dx = ax + bx - 1; /* proposed right boundary */

	if (ax < 0) {
		/* Left clip needed */
		bx += ax;   /* decrease X size by overflow */
		cx -= ax;   /* increase left clip count */
	}

	dx -= max_x;
	if (dx > 0) {
		bx -= dx;   /* decrease X size by right overflow */
	}

	skip_x = cx;
	draw_x = bx;
	stop_x = cx + bx;

	if (bx <= 0)
		goto all_done;

	/* Mirror adjustments */
	if (mirror != 1) {
		int di = xs;
		ax = target_x;

		ax += di;
		ax--;
		target_x = ax;

		di -= stop_x;		/* subtract original stop_x from size */
		skip_x = -di;

		di += draw_x;
		stop_x = di;
	}
}

/* Y clipping */
{
	int bx = ys;         /* proposed Y size */
	int cx = 0;          /* assume no top clipping */
	int ax = target_y;
	int dx = ax + bx - 1; /* proposed lower boundary */

	if (ax < 0) {
		bx += ax;
		cx -= ax;
	}

	dx -= max_y;
	if (dx > 0) {
		bx -= dx;
	}

	skip_y = cx;
	draw_y = bx;
	stop_y = cx + bx;

	if (bx <= 0)
		goto all_done;
}


/*
 *    POINTER SETUP
 *
 *    Compute the flat starting offset into target (and attr) buffers,
 *    accounting for target_y rows and target_x+skip_x columns.
 *    The original code used 16-bit real-mode segment:offset arithmetic
 *    with 32k wrap-around tricks; in a flat memory model we simply use
 *    integer offsets.
 */

#if translate
{
	int tx = target_x;
	int ty = target_y;
#if packed_attr
	ty += view_port_y;
	tx += view_port_x;
#endif
	/* XOR X & Y for starting thatch polarity, then flip for first pixel */
	thatch_base = (byte)(((tx ^ ty) & 1) ^ 1);
}
#endif

/* Compute starting target pointer */
{
	int start_y = skip_y + target_y;
	int start_x = target_x + skip_x;

	target_ptr += start_y * target_wrap + start_x;
}

#if three_d && packed_attr
{
	/* Compute starting attr pointer for packed (2-pixels-per-byte) buffer.
	   attr_start holds the sub-byte bit offset (0 or 1) for the first pixel. */
	int ay = skip_y + target_y + view_port_y;
	int ax = view_port_x + target_x + skip_x;

	attr_ptr += ay * attr_wrap + (ax / 2);
	attr_start = ax & 1;  /* bit offset within byte (0 or 1) */
}
#endif


/*
 *    MAIN DRAW LOOP
 *
 *    Processes the sprite stream row by row.
 *
 *    draw_y_pos  - vertical draw counter (counts pixels actually drawn/clipped,
 *                  excluding Bresenham-squeezed rows)
 *    scan_y      - vertical scan counter for Bresenham (indexes bresenham_table)
 *    target_row  - pointer to start of current target row
 *    attr_row    - pointer to start of current attr row  (packed_attr only)
 *
 *    Per row:
 *    draw_x_pos  - horizontal draw counter
 *    scan_x      - horizontal scan counter for Bresenham
 *    run_count   - RLE run counter
 *    sprite_byte - current byte read from sprite stream
 *    depth_code  - depth of current sprite pixel (three_d only)
 *    draw_depth  - depth at which we are drawing  (three_d only)
 */

{
	/* Absolute starting skip_x, adjusted for mirror direction */
	int abs_skip_x = skip_x * mirror;  /* undo any negative from mirroring */
	/* (The original asm did: mov ax,skip_x / imul mirror to make it always
	   positive regardless of mirror direction.) */
	skip_x = abs_skip_x;

#if three_d
	byte draw_depth = (byte)target_depth;
#endif

#if bresenham
	int scan_x, scan_y = -1;
#endif

	int draw_x_pos, draw_y_pos = -1;
	byte *pixel_ptr;  /* current horizontal position in target */
	byte *target_row = target_ptr;
#if packed_attr
	byte *attr_row = attr_ptr;
#endif

	while (1) { /* row_loop */

		line_finished = false;

#if bresenham
		scan_y++;
		if (scan_y >= temp_y)
			goto all_done;
		if (!bresenham_table[scan_y])
			goto row_finish;
#endif  /* bresenham row gate */

		/* row_valid: */
#if translate
		{
			byte cl = thatch_base ^ 1;
			thatch_base = cl;
			thatch_flag = cl;
		}
#endif

		draw_y_pos++;
		if (draw_y_pos >= stop_y)
			goto all_done;

		if (draw_y_pos < skip_y)
			goto row_finish;

		/* row_draw / pixel_begin: */

#if bresenham
		scan_x = 0;
#endif

		pixel_ptr = target_row;  /* current horizontal position in target */
		draw_x_pos = 0;

		/* pixel_loop: process one sprite row */
		while (1) {

			byte sprite_byte = *sprite_ptr++;

			if (sprite_byte == SS_EOL) {
				/* pixel_EOL */
				line_finished = true;
				break; /* goto row_next */
			}

#ifdef dump_mode
			if (sprite_byte == SS_DUMP) {
				/* pixel_DUMP: skip section */
				int dump_skip, dump_draw;
				dump_skip = *(int *)sprite_ptr; sprite_ptr += 2;

				while (dump_skip > 0) {
					if (draw_x_pos >= STOP_X)
						goto row_next_from_dump;

#if bresenham
					if (!bresenham_table[scan_x])
						goto pixel_DUMP_skip_next;
#endif

					if (draw_x_pos >= SKIP_X) {
#if translate
						thatch_flag ^= 1;
#endif
						pixel_ptr += mirror;
					}

					draw_x_pos++;

#if bresenham
					pixel_DUMP_skip_next :
					scan_x++;
#endif

					dump_skip--;
				}

				/* DUMP draw section */
				dump_draw = *(int *)sprite_ptr; sprite_ptr += 2;
				draw_count = dump_draw;

				while (1) {
					if (draw_x_pos >= STOP_X)
						goto row_next_from_dump;

					sprite_byte = *sprite_ptr++;

#if bresenham
					if (!bresenham_table[scan_x])
						goto pixel_DUMP_draw_next;
#endif

					if (draw_x_pos < SKIP_X)
						goto pixel_DUMP_nodraw;

					if (sprite_byte != SS_SKIP) {
						/* depth / interface / translate checks (shared helper below) */
#if three_d
						{
							byte depth_code;
#if packed_attr
							{
								/* packed attr unpack: 2 pixels per byte, 4 bits each */
								int   attr_col = draw_x_pos - SKIP_X;
								int   byte_off, bit_off;
								byte  attr_byte, full_attr_byte;

								if (mirror == 1) {
									int rem = attr_col & 1;
									byte_off = attr_col / 2;
									bit_off = (-(rem)+attr_start);
									if (bit_off > 1) {
										byte_off++; bit_off &= 1;
									}
								} else {
									byte_off = -(attr_col / 2);
									bit_off = attr_col + attr_start; /* Note: attr_start set at row start for mirror */
									if (bit_off < 0) {
										byte_off--; bit_off += 2;
									}
								}

								attr_byte = attr_row[byte_off];

								/* Invert bottom bit of bit_off, quadruple for shift, rotate right */
								int shift = ((bit_off ^ 1) & 1) << 2;
								/* ror byte by shift */
								attr_byte = (byte)((attr_byte >> shift) | (attr_byte << (8 - shift)));

#ifdef attribute
								full_attr_byte = attr_byte;
#endif

								depth_code = attr_byte & 0x0f;

#ifdef attribute
								{
									byte sprite_depth_bits = depth_code;
									byte stored_depth = draw_depth;
									if (stored_depth > bit_off)  /* reusing bit_off as shift here per asm */
										goto pixel_DUMP_no_attr;
									full_attr_byte = (full_attr_byte & 0xf0) | sprite_depth_bits;
									/* ror full_attr_byte, shift */
									full_attr_byte = (byte)((full_attr_byte >> shift) | (full_attr_byte << (8 - shift)));
									attr_row[byte_off] = full_attr_byte;
pixel_DUMP_no_attr:;
								}
#endif
							}
#else /* !packed_attr */
							{
								depth_code = attr_ptr[pixel_ptr - target_ptr] & 0x0f;
							}
#endif

							if (depth_code == 0)                   goto pixel_DUMP_skip_n_pop;
							if ((byte)draw_depth > depth_code)     goto pixel_DUMP_skip_n_pop;
						}
#endif /* three_d */

#if interface
						{
							byte cur = *pixel_ptr;
							if (cur < 8 || cur > 11)               goto pixel_DUMP_skip_n_pop;
						}
#endif

#if translate
						{
							byte full_color = sprite_byte;
							if (sprite_byte >= 16) {
								if (thatch_flag != 0) {
									sprite_byte >>= 4;
									if (sprite_byte == COLOR_BLACK_THATCH)
										sprite_byte = 0;
								} else {
									sprite_byte &= 0x0f;
								}
							}
						}
#endif

pixel_DUMP_output:
#ifdef on_black
						if (*pixel_ptr != 0)                     goto pixel_DUMP_no_output;
#endif
#ifndef attribute
#ifdef monodraw
						sprite_byte = color;
#endif
						*pixel_ptr = sprite_byte;
#endif
pixel_DUMP_no_output:;

#if translate
						sprite_byte = full_color;  /* restore for thatch counter */
#endif
					}

pixel_DUMP_skip_n_pop:;
pixel_DUMP_skip:
#if translate
					thatch_flag ^= 1;
#endif
					pixel_ptr += mirror;

pixel_DUMP_nodraw:
					draw_x_pos++;

pixel_DUMP_draw_next:
#if bresenham
					scan_x++;
#endif

					draw_count--;
					if (draw_count == 0) {
						/* DUMP done: consume EOL and mark finished */
						sprite_ptr++;  /* skip the EOL byte that follows a DUMP block */
						line_finished = true;
						goto row_next_from_dump;
					}
				}
row_next_from_dump:
				break; /* goto row_next */
			}
#endif /* dump_mode */

			if (sprite_byte == SS_RLE) {
				/* pixel_RLE: process an RLE run-length encoded segment */
				goto pixel_RLE;
			}

			/* Must be IRLE (individual run-length encoded) */
			goto pixel_IRLE;


			/* ============================================================
			 *  RLE HANDLER
			 *  Each RLE packet: [run_count] [run_value]  (until SS_EOL)
			 * ============================================================ */
pixel_RLE:
			while (1) { /* pixel_RLE_loop */

				if (draw_x_pos >= STOP_X)
					break; /* goto row_next */

				{
					byte rc = *sprite_ptr++;
					if (rc == SS_EOL) {
						line_finished = true;
						break; /* goto row_next */
					}
					byte run_count = rc;
					byte run_value = *sprite_ptr++;

					/* pixel_RLE_run_loop: emit run_count pixels of run_value */
					while (1) {

#if bresenham
						if (!bresenham_table[scan_x])
							goto pixel_RLE_run_next;
#endif

						if (draw_x_pos < SKIP_X)   goto pixel_RLE_run_nodraw;
						if (draw_x_pos >= STOP_X)  goto pixel_RLE_run_nodraw;

						/* pixel_RLE_run_yesdraw */
						if (run_value != SS_SKIP) {
							byte out_byte = run_value;

#if three_d
							{
								byte depth_code;
#if packed_attr
								{
									int   attr_col = draw_x_pos - SKIP_X;
									int   byte_off, bit_off;
									byte  attr_byte;

									if (mirror == 1) {
										int rem = attr_col & 1;
										byte_off = attr_col / 2;
										bit_off = (-rem) + attr_start;
										if (bit_off > 1) {
											byte_off++; bit_off &= 1;
										}
									} else {
										byte_off = -(attr_col / 2);
										bit_off = attr_col + attr_start;
										if (bit_off < 0) {
											byte_off--; bit_off += 2;
										}
									}

									attr_byte = attr_row[byte_off];
									int shift = ((bit_off ^ 1) & 1) << 2;
									attr_byte = (byte)((attr_byte >> shift) | (attr_byte << (8 - shift)));

#ifdef attribute
									{
										byte full_ab = attr_byte;
										byte sprite_d = attr_byte & 0x0f;
										if (draw_depth > bit_off)  goto pixel_RLE_no_attr;
										full_ab = (full_ab & 0xf0) | sprite_d;
										full_ab = (byte)((full_ab >> shift) | (full_ab << (8 - shift)));
										attr_row[byte_off] = full_ab;
pixel_RLE_no_attr:;
									}
#endif

									depth_code = attr_byte & 0x0f;
								}
#else
								{
									depth_code = attr_ptr[pixel_ptr - target_ptr] & 0x0f;
								}
#endif

								if (depth_code == 0)               goto pixel_RLE_run_skip_n_pop;
								if ((byte)draw_depth > depth_code) goto pixel_RLE_run_skip_n_pop;
							}
#endif /* three_d */

#if interface
							{
								byte cur = *pixel_ptr;
								if (cur < 8 || cur > 11)           goto pixel_RLE_run_skip_n_pop;
							}
#endif

#if translate
							{
								byte full_color = out_byte;
								if (out_byte >= 16) {
									if (thatch_flag != 0) {
										out_byte >>= 4;
										if (out_byte == COLOR_BLACK_THATCH)
											out_byte = 0;
									} else {
										out_byte &= 0x0f;
									}
								}
								/* pixel_RLE_run_output: */
#ifdef on_black
								if (*pixel_ptr != 0)               goto pixel_RLE_no_output;
#endif
#ifndef attribute
#ifdef monodraw
								out_byte = color;
#endif
								*pixel_ptr = out_byte;
#endif
pixel_RLE_no_output:;
								out_byte = full_color; /* restore for thatching */
								goto pixel_RLE_run_skip_n_pop;
							}
#endif

							/* pixel_RLE_run_output (non-translate path) */
#ifdef on_black
							if (*pixel_ptr != 0)               goto pixel_RLE_no_output2;
#endif
#ifndef attribute
#ifdef monodraw
							out_byte = color;
#endif
							*pixel_ptr = out_byte;
#endif
pixel_RLE_no_output2:;
						}

pixel_RLE_run_skip_n_pop:;
pixel_RLE_run_skip:
#if translate
						thatch_flag ^= 1;
#endif
						pixel_ptr += mirror;

pixel_RLE_run_nodraw:
						draw_x_pos++;

pixel_RLE_run_next:
#if bresenham
						scan_x++;
#endif

						run_count--;
						if (run_count == 0)
							break; /* goto pixel_RLE_loop for next run */
					}
					/* pixel_RLE_run_done -> loop back to pixel_RLE_loop */
				}
			}
			break; /* row_next */


			/* ============================================================
			 *  IRLE HANDLER
			 *  Each IRLE packet is either:
			 *    SS_RUN + [count] + [value]   (a run)
			 *    SS_EOL                       (end of line)
			 *    <literal byte>               (single image pixel)
			 * ============================================================ */
pixel_IRLE:
			while (1) { /* pixel_IRLE_loop */

				if (draw_x_pos >= STOP_X)
					break; /* goto row_next */

				sprite_byte = *sprite_ptr++;

				if (sprite_byte == SS_EOL) {
					line_finished = true;
					break; /* goto row_next (same as pixel_RLE_EOL) */
				}

				if (sprite_byte == SS_RUN) {
					/* pixel_IRLE_run */
					byte run_count = *sprite_ptr++;
					byte run_value = *sprite_ptr++;

					while (1) { /* pixel_IRLE_run_loop */

#if bresenham
						if (!bresenham_table[scan_x])
							goto pixel_IRLE_run_next;
#endif

						if (draw_x_pos < SKIP_X)   goto pixel_IRLE_run_nodraw;
						if (draw_x_pos >= STOP_X)  goto pixel_IRLE_run_nodraw;

						/* pixel_IRLE_run_yesdraw */
						if (run_value != SS_SKIP) {
							byte out_byte = run_value;

#if three_d
							{
								byte depth_code;
#if packed_attr
								{
									int  attr_col = draw_x_pos - SKIP_X;
									int  byte_off, bit_off;
									byte attr_byte;

									if (mirror == 1) {
										int rem = attr_col & 1;
										byte_off = attr_col / 2;
										bit_off = (-rem) + attr_start;
										if (bit_off > 1) {
											byte_off++; bit_off &= 1;
										}
									} else {
										byte_off = -(attr_col / 2);
										bit_off = attr_col + attr_start;
										if (bit_off < 0) {
											byte_off--; bit_off += 2;
										}
									}

									attr_byte = attr_row[byte_off];
									int shift = ((bit_off ^ 1) & 1) << 2;
									attr_byte = (byte)((attr_byte >> shift) | (attr_byte << (8 - shift)));

#ifdef attribute
									{
										byte full_ab = attr_byte;
										byte sprite_d = attr_byte & 0x0f;
										if (draw_depth > bit_off)  goto pixel_IRLE_run_no_attr;
										full_ab = (full_ab & 0xf0) | sprite_d;
										full_ab = (byte)((full_ab >> shift) | (full_ab << (8 - shift)));
										attr_row[byte_off] = full_ab;
pixel_IRLE_run_no_attr:;
									}
#endif

									depth_code = attr_byte & 0x0f;
								}
#else
								{
									depth_code = attr_ptr[pixel_ptr - target_ptr] & 0x0f;
								}
#endif

								if (depth_code == 0)               goto pixel_IRLE_run_skip_n_pop;
								if ((byte)draw_depth > depth_code) goto pixel_IRLE_run_skip_n_pop;
							}
#endif /* three_d */

#if interface
							{
								byte cur = *pixel_ptr;
								if (cur < 8 || cur > 11)           goto pixel_IRLE_run_skip_n_pop;
							}
#endif

#if translate
							{
								byte full_color = out_byte;
								if (out_byte >= 16) {
									if (thatch_flag != 0) {
										out_byte >>= 4;
										if (out_byte == COLOR_BLACK_THATCH)
											out_byte = 0;
									} else {
										out_byte &= 0x0f;
									}
								}
#ifdef on_black
								if (*pixel_ptr != 0)               goto pixel_IRLE_run_no_output;
#endif
#ifndef attribute
#ifdef monodraw
								out_byte = color;
#endif
								*pixel_ptr = out_byte;
#endif
pixel_IRLE_run_no_output:;
								out_byte = full_color;
								goto pixel_IRLE_run_skip_n_pop;
							}
#endif

#ifdef on_black
							if (*pixel_ptr != 0)               goto pixel_IRLE_run_no_output2;
#endif
#ifndef attribute
#ifdef monodraw
							out_byte = color;
#endif
							*pixel_ptr = out_byte;
#endif
pixel_IRLE_run_no_output2:;
						}

pixel_IRLE_run_skip_n_pop:;
pixel_IRLE_run_skip:
#if translate
						thatch_flag ^= 1;
#endif
						pixel_ptr += mirror;

pixel_IRLE_run_nodraw:
						draw_x_pos++;

pixel_IRLE_run_next:
#if bresenham
						scan_x++;
#endif

						run_count--;
						if (run_count == 0)
							break; /* pixel_IRLE_next -> loop */
					}
					continue; /* pixel_IRLE_next: back to pixel_IRLE_loop */
				}

				/* pixel_IRLE_image: sprite_byte is a literal pixel */
				{
					byte out_byte = sprite_byte;

#if bresenham
					if (!bresenham_table[scan_x])
						goto pixel_IRLE_image_next;
#endif

					/* pixel_IRLE_image_clip */
					if (draw_x_pos < SKIP_X)   goto pixel_IRLE_image_nodraw;
					if (draw_x_pos >= STOP_X)  goto pixel_IRLE_image_nodraw;

					if (out_byte != SS_SKIP) {

#if three_d
						{
							byte depth_code;
#if packed_attr
							{
								int  attr_col = draw_x_pos - SKIP_X;
								int  byte_off, bit_off;
								byte attr_byte;

								if (mirror == 1) {
									int rem = attr_col & 1;
									byte_off = attr_col / 2;
									bit_off = (-rem) + attr_start;
									if (bit_off > 1) {
										byte_off++; bit_off &= 1;
									}
								} else {
									byte_off = -(attr_col / 2);
									bit_off = attr_col + attr_start;
									if (bit_off < 0) {
										byte_off--; bit_off += 2;
									}
								}

								attr_byte = attr_row[byte_off];
								int shift = ((bit_off ^ 1) & 1) << 2;
								attr_byte = (byte)((attr_byte >> shift) | (attr_byte << (8 - shift)));

#ifdef attribute
								{
									byte full_ab = attr_byte;
									byte sprite_d = attr_byte & 0x0f;
									if (draw_depth > bit_off)  goto pixel_IRLE_image_no_attr;
									full_ab = (full_ab & 0xf0) | sprite_d;
									full_ab = (byte)((full_ab >> shift) | (full_ab << (8 - shift)));
									attr_row[byte_off] = full_ab;
pixel_IRLE_image_no_attr:;
								}
#endif

								depth_code = attr_byte & 0x0f;
							}
#else
							{
								depth_code = attr_ptr[pixel_ptr - target_ptr] & 0x0f;
							}
#endif

							if (depth_code == 0)               goto pixel_IRLE_image_skip_n_pop;
							if ((byte)draw_depth > depth_code) goto pixel_IRLE_image_skip_n_pop;
						}
#endif /* three_d */

#if interface
						{
							byte cur = *pixel_ptr;
							if (cur < 8 || cur > 11)           goto pixel_IRLE_image_skip_n_pop;
						}
#endif

#if translate
						{
							byte full_color = out_byte;
							if (out_byte >= 16) {
								if (thatch_flag != 0) {
									out_byte >>= 4;
									if (out_byte == COLOR_BLACK_THATCH)
										out_byte = 0;
								} else {
									out_byte &= 0x0f;
								}
							}
							/* pixel_IRLE_image_output: */
#ifdef on_black
							if (*pixel_ptr != 0)               goto pixel_IRLE_image_no_output;
#endif
#ifndef attribute
#ifdef monodraw
							out_byte = color;
#endif
							*pixel_ptr = out_byte;
#endif
pixel_IRLE_image_no_output:;
							goto pixel_IRLE_image_skip_n_pop;
						}
#endif

						/* pixel_IRLE_image_output (non-translate path) */
#ifdef on_black
						if (*pixel_ptr != 0)               goto pixel_IRLE_image_no_output2;
#endif
#ifndef attribute
#ifdef monodraw
						out_byte = color;
#endif
						*pixel_ptr = out_byte;
#endif
pixel_IRLE_image_no_output2:;
					}

pixel_IRLE_image_skip_n_pop:;
pixel_IRLE_image_skip:
#if translate
					thatch_flag ^= 1;
#endif
					pixel_ptr += mirror;

pixel_IRLE_image_nodraw:
					draw_x_pos++;

pixel_IRLE_image_next:
#if bresenham
					scan_x++;
#endif

					continue; /* pixel_IRLE_loop */
				}
			}
			break; /* row_next */

		} /* end pixel_loop / inner while(1) */

		/* row_next: advance to next row */

		/* Advance target row pointer by one scanline */
		target_row += target_wrap;

#if packed_attr
		/* Advance attr row pointer by one scanline */
		attr_row += attr_wrap;
#endif

		/* row_finish: if this line wasn't completed (e.g. right-clipped early),
		   consume the rest of the sprite row stream until SS_EOL. */
row_finish:
		if (!line_finished) {
			while (*sprite_ptr++ != SS_EOL)
				;
		}

	} /* end while(1) row_loop */
}

all_done:;
