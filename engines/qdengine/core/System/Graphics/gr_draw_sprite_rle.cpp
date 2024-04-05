/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "gr_dispatcher.h"
#include "rle_compress.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

void grDispatcher::PutSpr_rle(int x,int y,int sx,int sy,const class rleBuffer* p,int mode,bool alpha_flag)
{
	int px = 0;
	int py = 0;

	int psx = sx;
	int psy = sy;

	if(!clip_rectangle(x,y,px,py,psx,psy)) return;

	if(bytes_per_pixel() == 4){
		int dx = -4;
		int dy = -1;

		x *= 4;

		if(mode & GR_FLIP_HORIZONTAL){
			x += (psx - 1) * 4;
			px = sx - px - psx;
		}
		else 
			dx = 4;

		psx += px;

		if(mode & GR_FLIP_VERTICAL){
			y += psy - 1;
			py = sy - py - psy;
		}
		else 
			dy = 1;

		for(int i = 0; i < psy; i ++){
			unsigned char* scr_buf = reinterpret_cast<unsigned char*>(screenBuf + yTable[y] + x);

			const char* rle_header = p -> header_ptr(py + i);
			const unsigned* rle_data = p -> data_ptr(py + i);

			int j = 0;
			char count = 0;
			while(j < px){
				count = *rle_header++;
				if(count > 0){
					if(count + j <= px){
						j += count;
						rle_data++;
						count = 0;
					}
					else {
						count -= px - j;
						j = px;
					}
				}
				else {
					if(j - count <= px){
						j -= count;
						rle_data -= count;
						count = 0;
					}
					else {
						count += px - j;
						rle_data += px - j;
						j = px;
					}
				}
			}

			if(!alpha_flag){
				while(j < psx){
					if(count > 0){
						while(count && j < psx){
							if(*rle_data)
								*reinterpret_cast<unsigned*>(scr_buf) = *rle_data;
							scr_buf += dx;
							count--;
							j++;
						}
						rle_data++;
					}
					else {
						if(count < 0){
							count = -count;
							while(count && j < psx){
								if(*rle_data)
									*reinterpret_cast<unsigned*>(scr_buf) = *rle_data++;
								else
									rle_data++;

								scr_buf += dx;
								count--;
								j++;
							}
						}
					}
					count = *rle_header++;
				}
			}
			else {
				while(j < psx){
					if(count > 0){
						while(count && j < psx){
							unsigned a = reinterpret_cast<const unsigned char*>(rle_data)[3];
							if(a != 255){
								scr_buf[0] = reinterpret_cast<const unsigned char*>(rle_data)[0] + ((a * scr_buf[0]) >> 8);
								scr_buf[1] = reinterpret_cast<const unsigned char*>(rle_data)[1] + ((a * scr_buf[1]) >> 8);
								scr_buf[2] = reinterpret_cast<const unsigned char*>(rle_data)[2] + ((a * scr_buf[2]) >> 8);
							}
							scr_buf += dx;
							count--;
							j++;
						}
						rle_data++;
					}
					else {
						if(count < 0){
							count = -count;
							while(count && j < psx){
								unsigned a = reinterpret_cast<const unsigned char*>(rle_data)[3];
								if(a != 255){
									scr_buf[0] = reinterpret_cast<const unsigned char*>(rle_data)[0] + ((a * scr_buf[0]) >> 8);
									scr_buf[1] = reinterpret_cast<const unsigned char*>(rle_data)[1] + ((a * scr_buf[1]) >> 8);
									scr_buf[2] = reinterpret_cast<const unsigned char*>(rle_data)[2] + ((a * scr_buf[2]) >> 8);
								}
								rle_data++;
								scr_buf += dx;
								count--;
								j++;
							}
						}
					}
					count = *rle_header++;
				}
			}
			y += dy;
		}
	}
	else if(bytes_per_pixel() == 3){
		int dx = -3;
		int dy = -1;

		x *= 3;

		if(mode & GR_FLIP_HORIZONTAL){
			x += psx * 3 - 3;
			px = sx - px - psx;
		}
		else 
			dx = 3;

		psx += px;

		if(mode & GR_FLIP_VERTICAL){
			y += psy - 1;
			py = sy - py - psy;
		}
		else 
			dy = 1;

		for(int i = 0; i < psy; i ++){
			unsigned char* scr_buf = reinterpret_cast<unsigned char*>(screenBuf + yTable[y] + x);

			const char* rle_header = p -> header_ptr(py + i);
			const unsigned* rle_data = p -> data_ptr(py + i);

			int j = 0;
			char count = 0;
			while(j < px){
				count = *rle_header++;
				if(count > 0){
					if(count + j <= px){
						j += count;
						rle_data++;
						count = 0;
					}
					else {
						count -= px - j;
						j = px;
					}
				}
				else {
					if(j - count <= px){
						j -= count;
						rle_data -= count;
						count = 0;
					}
					else {
						count += px - j;
						rle_data += px - j;
						j = px;
					}
				}
			}

			if(!alpha_flag){
				while(j < psx){
					if(count > 0){
						while(count && j < psx){
							if(*rle_data){
								scr_buf[0] = reinterpret_cast<const unsigned char*>(rle_data)[0];
								scr_buf[1] = reinterpret_cast<const unsigned char*>(rle_data)[1];
								scr_buf[2] = reinterpret_cast<const unsigned char*>(rle_data)[2];
							}
							scr_buf += dx;
							count--;
							j++;
						}
						rle_data++;
					}
					else {
						if(count < 0){
							count = -count;
							while(count && j < psx){
								if(*rle_data){
									scr_buf[0] = reinterpret_cast<const unsigned char*>(rle_data)[0];
									scr_buf[1] = reinterpret_cast<const unsigned char*>(rle_data)[1];
									scr_buf[2] = reinterpret_cast<const unsigned char*>(rle_data)[2];
								}
								scr_buf += dx;
								rle_data++;
								count--;
								j++;
							}
						}
					}
					count = *rle_header++;
				}
			}
			else {
				while(j < psx){
					if(count > 0){
						while(count && j < psx){
							unsigned a = reinterpret_cast<const unsigned char*>(rle_data)[3];
							if(a != 255){
								scr_buf[0] = reinterpret_cast<const unsigned char*>(rle_data)[0] + ((a * scr_buf[0]) >> 8);
								scr_buf[1] = reinterpret_cast<const unsigned char*>(rle_data)[1] + ((a * scr_buf[1]) >> 8);
								scr_buf[2] = reinterpret_cast<const unsigned char*>(rle_data)[2] + ((a * scr_buf[2]) >> 8);
							}
							scr_buf += dx;
							count--;
							j++;
						}
						rle_data++;
					}
					else {
						if(count < 0){
							count = -count;
							while(count && j < psx){
								unsigned a = reinterpret_cast<const unsigned char*>(rle_data)[3];
								if(a != 255){
									scr_buf[0] = reinterpret_cast<const unsigned char*>(rle_data)[0] + ((a * scr_buf[0]) >> 8);
									scr_buf[1] = reinterpret_cast<const unsigned char*>(rle_data)[1] + ((a * scr_buf[1]) >> 8);
									scr_buf[2] = reinterpret_cast<const unsigned char*>(rle_data)[2] + ((a * scr_buf[2]) >> 8);
								}
								scr_buf += dx;
								rle_data++;
								count--;
								j++;
							}
						}
					}
					count = *rle_header++;
				}
			}
			y += dy;
		}
	}
	else if(bytes_per_pixel() == 2){
		int dx = -1;
		int dy = -1;

		x *= 2;

		if(mode & GR_FLIP_HORIZONTAL){
			x += (psx - 1) * 2;
			px = sx - px - psx;
		}
		else 
			dx = 1;

		psx += px;

		if(mode & GR_FLIP_VERTICAL){
			y += psy - 1;
			py = sy - py - psy;
		}
		else 
			dy = 1;

		for(int i = 0; i < psy; i ++){
			unsigned short* scr_buf = reinterpret_cast<unsigned short*>(screenBuf + yTable[y] + x);

			const char* rle_header = p -> header_ptr(py + i);
			const unsigned* rle_data = p -> data_ptr(py + i);

			int j = 0;
			char count = 0;
			while(j < px){
				count = *rle_header++;
				if(count > 0){
					if(count + j <= px){
						j += count;
						rle_data++;
						count = 0;
					}
					else {
						count -= px - j;
						j = px;
					}
				}
				else {
					if(j - count <= px){
						j -= count;
						rle_data -= count;
						count = 0;
					}
					else {
						count += px - j;
						rle_data += px - j;
						j = px;
					}
				}
			}

			if(pixel_format_ == GR_RGB565){
				if(!alpha_flag){
					while(j < psx){
						if(count > 0){
							while(count && j < psx){
								if(*rle_data){
									const unsigned char* rle_buf = (const unsigned char*)rle_data;
									unsigned cl = make_rgb565u(rle_buf[2], rle_buf[1], rle_buf[0]);
									*scr_buf = cl;
								}
								scr_buf += dx;
								count--;
								j++;
							}
							rle_data++;
						}
						else {
							if(count < 0){
								count = -count;
								while(count && j < psx){
									if(*rle_data){
										const unsigned char* rle_buf = (const unsigned char*)rle_data;
										unsigned cl = make_rgb565u(rle_buf[2], rle_buf[1], rle_buf[0]);
										*scr_buf = cl;
									}
									scr_buf += dx;
									rle_data++;
									count--;
									j++;
								}
							}
						}
						count = *rle_header++;
					}
				}
				else {
					while(j < psx){
						if(count > 0){
							while(count && j < psx){
								const unsigned char* rle_buf = (const unsigned char*)rle_data;
								unsigned a = rle_buf[3];
								*scr_buf = alpha_blend_565(make_rgb565u(rle_buf[2], rle_buf[1], rle_buf[0]),*scr_buf,a);
								scr_buf += dx;
								count--;
								j++;
							}
							rle_data++;
						}
						else {
							if(count < 0){
								count = -count;
								while(count && j < psx){
									const unsigned char* rle_buf = (const unsigned char*)rle_data;
									unsigned a = rle_buf[3];
									*scr_buf = alpha_blend_565(make_rgb565u(rle_buf[2], rle_buf[1], rle_buf[0]),*scr_buf,a);
									scr_buf += dx;
									rle_data++;
									count--;
									j++;
								}
							}
						}
						count = *rle_header++;
					}
				}
			}
			else {
				if(!alpha_flag){
					while(j < psx){
						if(count > 0){
							while(count && j < psx){
								if(*rle_data){
									const unsigned char* rle_buf = (const unsigned char*)rle_data;
									unsigned cl = make_rgb555u(rle_buf[2], rle_buf[1], rle_buf[0]);
									*scr_buf = cl;
								}
								scr_buf += dx;
								count--;
								j++;
							}
							rle_data++;
						}
						else {
							if(count < 0){
								count = -count;
								while(count && j < psx){
									if(*rle_data){
										const unsigned char* rle_buf = (const unsigned char*)rle_data;
										unsigned cl = make_rgb555u(rle_buf[2], rle_buf[1], rle_buf[0]);
										*scr_buf = cl;
									}
									scr_buf += dx;
									rle_data++;
									count--;
									j++;
								}
							}
						}
						count = *rle_header++;
					}
				}
				else {
					while(j < psx){
						if(count > 0){
							while(count && j < psx){
								const unsigned char* rle_buf = (const unsigned char*)rle_data;
								unsigned a = rle_buf[3];
								*scr_buf = alpha_blend_555(make_rgb555u(rle_buf[2], rle_buf[1], rle_buf[0]),*scr_buf,a);
								scr_buf += dx;
								count--;
								j++;
							}
							rle_data++;
						}
						else {
							if(count < 0){
								count = -count;
								while(count && j < psx){
									const unsigned char* rle_buf = (const unsigned char*)rle_data;
									unsigned a = rle_buf[3];
									*scr_buf = alpha_blend_555(make_rgb555u(rle_buf[2], rle_buf[1], rle_buf[0]),*scr_buf,a);
									scr_buf += dx;
									rle_data++;
									count--;
									j++;
								}
							}
						}
						count = *rle_header++;
					}
				}
			}
			y += dy;
		}
	}
}

void grDispatcher::PutSpr_rle(int x,int y,int sx,int sy,const class rleBuffer* p,int mode,float scale,bool alpha_flag)
{
	int sx_dest = round(float(sx) * scale);
	int sy_dest = round(float(sy) * scale);

	if(sx_dest <= 0 || sy_dest <= 0) return;

	int dx = (sx << 16) / sx_dest;
	int dy = (sy << 16) / sy_dest;
	int fx = (1 << 15);
	int fy = (1 << 15);

	int x0 = 0;
	int x1 = sx_dest - 1;
	int ix = 1;

	int y0 = 0;
	int y1 = sy_dest - 1;
	int iy = 1;

	if(mode & GR_FLIP_VERTICAL){
		y0 = sy_dest - 1,
		y1 = 0;
		iy = -1;
	}

	if(mode & GR_FLIP_HORIZONTAL){
		x0 = sx_dest - 1,
		x1 = 0;
		ix = -1;
	}

	if(bytes_per_pixel() == 2){
		if(!alpha_flag){
			const unsigned char* line_src = rleBuffer::get_buffer(0);

			if(pixel_format_ == GR_RGB565){
				for(int i = y0; i != y1; i += iy){
					p -> decode_line(fy >> 16);

					fy += dy;
					fx = (1 << 15);

					for(int j = x0; j != x1; j += ix){
						if(ClipCheck(x + j,y + i)){
							const unsigned char* src_data = line_src + (fx >> 16) * 3;
							if(src_data[0] || src_data[1] || src_data[2])
								SetPixelFast(x + j,y + i,make_rgb565u(src_data[2], src_data[1], src_data[0]));
						}
						fx += dx;
					}
				}
			}
			else {
				for(int i = y0; i != y1; i += iy){
					p -> decode_line(fy >> 16);

					fy += dy;
					fx = (1 << 15);

					for(int j = x0; j != x1; j += ix){
						if(ClipCheck(x + j,y + i)){
							const unsigned char* src_data = line_src + (fx >> 16) * 3;
							if(src_data[0] || src_data[1] || src_data[2])
								SetPixelFast(x + j,y + i,make_rgb555u(src_data[2], src_data[1], src_data[0]));
						}
						fx += dx;
					}
				}
			}
		}
		else {
			const unsigned char* line_src = rleBuffer::get_buffer(0);

			if(pixel_format_ == GR_RGB565){
				for(int i = y0; i != y1; i += iy){
					p -> decode_line(fy >> 16);

					fy += dy;
					fx = (1 << 15);

					for(int j = x0; j != x1; j += ix){
						if(ClipCheck(x + j,y + i)){
							const unsigned char* src_data = line_src + ((fx >> 16) << 2);

							unsigned a = src_data[3];
							if(a != 255){
								unsigned cl = make_rgb565u(src_data[2], src_data[1], src_data[0]);

								if(a){
									unsigned scl;
									GetPixel(x + j,y + i,scl);

									SetPixelFast(x + j,y + i,alpha_blend_565(cl,scl,a));
								}
								else
									SetPixelFast(x + j,y + i,cl);
							}
						}
						fx += dx;
					}
				}
			}
			else {
				for(int i = y0; i != y1; i += iy){
					p -> decode_line(fy >> 16);

					fy += dy;
					fx = (1 << 15);

					for(int j = x0; j != x1; j += ix){
						if(ClipCheck(x + j,y + i)){
							const unsigned char* src_data = line_src + ((fx >> 16) << 2);

							unsigned a = src_data[3];
							if(a != 255){
								unsigned cl = make_rgb555u(src_data[2], src_data[1], src_data[0]);

								if(a){
									unsigned scl;
									GetPixel(x + j,y + i,scl);

									SetPixelFast(x + j,y + i,alpha_blend_555(cl,scl,a));
								}
								else
									SetPixelFast(x + j,y + i,cl);
							}
						}
						fx += dx;
					}
				}
			}
		}
	}
	else if(bytes_per_pixel() == 3 || bytes_per_pixel() == 4){
		const unsigned char* line_src = rleBuffer::get_buffer(0);

		if(!alpha_flag){
			for(int i = y0; i != y1; i += iy){
				p -> decode_line(fy >> 16);

				fy += dy;
				fx = (1 << 15);

				for(int j = x0; j != x1; j += ix){
					if(ClipCheck(x + j,y + i)){
						int idx = (fx >> 16) << 2;

						unsigned r = line_src[idx + 2];
						unsigned g = line_src[idx + 1];
						unsigned b = line_src[idx + 0];

						if(r || g || b)
							SetPixelFast(x + j,y + i,r,g,b);
					}
					fx += dx;
				}
			}
		}
		else {
			for(int i = y0; i != y1; i += iy){
				p -> decode_line(fy >> 16);

				fy += dy;
				fx = (1 << 15);

				for(int j = x0; j != x1; j += ix){
					if(ClipCheck(x + j,y + i)){
						int idx = (fx >> 16) << 2;

						unsigned a = line_src[idx + 3];
						if(a != 255){
							unsigned sr,sg,sb;
							GetPixel(x + j,y + i,sr,sg,sb);

							unsigned r = line_src[idx + 2] + ((a * sr) >> 8);
							unsigned g = line_src[idx + 1] + ((a * sg) >> 8);
							unsigned b = line_src[idx + 0] + ((a * sb) >> 8);

							SetPixelFast(x + j,y + i,r,g,b);
						}
					}

					fx += dx;
				}
			}
		}
	}
}

void grDispatcher::PutSprMask_rle(int x,int y,int sx,int sy,const rleBuffer* p,unsigned mask_color,int mask_alpha,int mode,bool alpha_flag)
{
	int px = 0;
	int py = 0;

	int psx = sx;
	int psy = sy;

	if(!clip_rectangle(x,y,px,py,psx,psy)) return;

	if(bytes_per_pixel() == 4){
		int dx = -4;
		int dy = -1;

		x *= 4;

		if(mode & GR_FLIP_HORIZONTAL){
			x += (psx - 1) * 4;
			px = sx - px - psx;
		}
		else 
			dx = 4;

		psx += px;

		if(mode & GR_FLIP_VERTICAL){
			y += psy - 1;
			py = sy - py - psy;
		}
		else 
			dy = 1;

		unsigned mr,mg,mb;
		split_rgb888(mask_color,mr,mg,mb);

		for(int i = 0; i < psy; i ++){
			unsigned char* scr_buf = reinterpret_cast<unsigned char*>(screenBuf + yTable[y] + x);

			const char* rle_header = p -> header_ptr(py + i);
			const unsigned* rle_data = p -> data_ptr(py + i);

			int j = 0;
			char count = 0;
			while(j < px){
				count = *rle_header++;
				if(count > 0){
					if(count + j <= px){
						j += count;
						rle_data++;
						count = 0;
					}
					else {
						count -= px - j;
						j = px;
					}
				}
				else {
					if(j - count <= px){
						j -= count;
						rle_data -= count;
						count = 0;
					}
					else {
						count += px - j;
						rle_data += px - j;
						j = px;
					}
				}
			}

			if(!alpha_flag){
				while(j < psx){
					if(count > 0){
						while(count && j < psx){
							if(*rle_data){
								scr_buf[2] = ((mr * (255 - mask_alpha)) >> 8) + ((mask_alpha * scr_buf[2]) >> 8);
								scr_buf[1] = ((mg * (255 - mask_alpha)) >> 8) + ((mask_alpha * scr_buf[1]) >> 8);
								scr_buf[0] = ((mb * (255 - mask_alpha)) >> 8) + ((mask_alpha * scr_buf[0]) >> 8);
							}

							scr_buf += dx;
							count--;
							j++;
						}
						rle_data++;
					}
					else {
						if(count < 0){
							count = -count;
							while(count && j < psx){
								if(*rle_data){
									scr_buf[2] = ((mr * (255 - mask_alpha)) >> 8) + ((mask_alpha * scr_buf[2]) >> 8);
									scr_buf[1] = ((mg * (255 - mask_alpha)) >> 8) + ((mask_alpha * scr_buf[1]) >> 8);
									scr_buf[0] = ((mb * (255 - mask_alpha)) >> 8) + ((mask_alpha * scr_buf[0]) >> 8);
								}
								rle_data++;

								scr_buf += dx;
								count--;
								j++;
							}
						}
					}
					count = *rle_header++;
				}
			}
			else {
				while(j < psx){
					if(count > 0){
						while(count && j < psx){
							unsigned a = reinterpret_cast<const unsigned char*>(rle_data)[3];
							if(a != 255){
								a = mask_alpha + ((a * (255 - mask_alpha)) >> 8);

								scr_buf[2] = ((mr * (255 - a)) >> 8) + ((a * scr_buf[2]) >> 8);
								scr_buf[1] = ((mg * (255 - a)) >> 8) + ((a * scr_buf[1]) >> 8);
								scr_buf[0] = ((mb * (255 - a)) >> 8) + ((a * scr_buf[0]) >> 8);
							}
							scr_buf += dx;
							count--;
							j++;
						}
						rle_data++;
					}
					else {
						if(count < 0){
							count = -count;
							while(count && j < psx){
								unsigned a = reinterpret_cast<const unsigned char*>(rle_data)[3];
								if(a != 255){
									a = mask_alpha + ((a * (255 - mask_alpha)) >> 8);

									scr_buf[2] = ((mr * (255 - a)) >> 8) + ((a * scr_buf[2]) >> 8);
									scr_buf[1] = ((mg * (255 - a)) >> 8) + ((a * scr_buf[1]) >> 8);
									scr_buf[0] = ((mb * (255 - a)) >> 8) + ((a * scr_buf[0]) >> 8);
								}
								rle_data++;
								scr_buf += dx;
								count--;
								j++;
							}
						}
					}
					count = *rle_header++;
				}
			}
			y += dy;
		}
	}
	else if(bytes_per_pixel() == 3){
		int dx = -3;
		int dy = -1;

		x *= 3;

		if(mode & GR_FLIP_HORIZONTAL){
			x += psx * 3 - 3;
			px = sx - px - psx;
		}
		else 
			dx = 3;

		psx += px;

		if(mode & GR_FLIP_VERTICAL){
			y += psy - 1;
			py = sy - py - psy;
		}
		else 
			dy = 1;

		unsigned mr,mg,mb;
		split_rgb888(mask_color,mr,mg,mb);

		for(int i = 0; i < psy; i ++){
			unsigned char* scr_buf = reinterpret_cast<unsigned char*>(screenBuf + yTable[y] + x);

			const char* rle_header = p -> header_ptr(py + i);
			const unsigned* rle_data = p -> data_ptr(py + i);

			int j = 0;
			char count = 0;
			while(j < px){
				count = *rle_header++;
				if(count > 0){
					if(count + j <= px){
						j += count;
						rle_data++;
						count = 0;
					}
					else {
						count -= px - j;
						j = px;
					}
				}
				else {
					if(j - count <= px){
						j -= count;
						rle_data -= count;
						count = 0;
					}
					else {
						count += px - j;
						rle_data += px - j;
						j = px;
					}
				}
			}

			if(!alpha_flag){
				while(j < psx){
					if(count > 0){
						while(count && j < psx){
							if(*rle_data){
								scr_buf[2] = ((mr * (255 - mask_alpha)) >> 8) + ((mask_alpha * scr_buf[2]) >> 8);
								scr_buf[1] = ((mg * (255 - mask_alpha)) >> 8) + ((mask_alpha * scr_buf[1]) >> 8);
								scr_buf[0] = ((mb * (255 - mask_alpha)) >> 8) + ((mask_alpha * scr_buf[0]) >> 8);
							}
							scr_buf += dx;
							count--;
							j++;
						}
						rle_data++;
					}
					else {
						if(count < 0){
							count = -count;
							while(count && j < psx){
								if(*rle_data){
									scr_buf[2] = ((mr * (255 - mask_alpha)) >> 8) + ((mask_alpha * scr_buf[2]) >> 8);
									scr_buf[1] = ((mg * (255 - mask_alpha)) >> 8) + ((mask_alpha * scr_buf[1]) >> 8);
									scr_buf[0] = ((mb * (255 - mask_alpha)) >> 8) + ((mask_alpha * scr_buf[0]) >> 8);
								}
								scr_buf += dx;
								rle_data++;
								count--;
								j++;
							}
						}
					}
					count = *rle_header++;
				}
			}
			else {
				while(j < psx){
					if(count > 0){
						while(count && j < psx){
							unsigned a = reinterpret_cast<const unsigned char*>(rle_data)[3];
							if(a != 255){
								a = mask_alpha + ((a * (255 - mask_alpha)) >> 8);

								scr_buf[2] = ((mr * (255 - a)) >> 8) + ((a * scr_buf[2]) >> 8);
								scr_buf[1] = ((mg * (255 - a)) >> 8) + ((a * scr_buf[1]) >> 8);
								scr_buf[0] = ((mb * (255 - a)) >> 8) + ((a * scr_buf[0]) >> 8);
							}
							scr_buf += dx;
							count--;
							j++;
						}
						rle_data++;
					}
					else {
						if(count < 0){
							count = -count;
							while(count && j < psx){
								unsigned a = reinterpret_cast<const unsigned char*>(rle_data)[3];
								if(a != 255){
									a = mask_alpha + ((a * (255 - mask_alpha)) >> 8);

									scr_buf[2] = ((mr * (255 - a)) >> 8) + ((a * scr_buf[2]) >> 8);
									scr_buf[1] = ((mg * (255 - a)) >> 8) + ((a * scr_buf[1]) >> 8);
									scr_buf[0] = ((mb * (255 - a)) >> 8) + ((a * scr_buf[0]) >> 8);
								}
								scr_buf += dx;
								rle_data++;
								count--;
								j++;
							}
						}
					}
					count = *rle_header++;
				}
			}
			y += dy;
		}
	}
	else if(bytes_per_pixel() == 2){
		int dx = -1;
		int dy = -1;

		x *= 2;

		if(mode & GR_FLIP_HORIZONTAL){
			x += (psx - 1) * 2;
			px = sx - px - psx;
		}
		else 
			dx = 1;

		psx += px;

		if(mode & GR_FLIP_VERTICAL){
			y += psy - 1;
			py = sy - py - psy;
		}
		else 
			dy = 1;

		for(int i = 0; i < psy; i ++){
			unsigned short* scr_buf = reinterpret_cast<unsigned short*>(screenBuf + yTable[y] + x);

			const char* rle_header = p -> header_ptr(py + i);
			const unsigned* rle_data = p -> data_ptr(py + i);

			int j = 0;
			char count = 0;
			while(j < px){
				count = *rle_header++;
				if(count > 0){
					if(count + j <= px){
						j += count;
						rle_data++;
						count = 0;
					}
					else {
						count -= px - j;
						j = px;
					}
				}
				else {
					if(j - count <= px){
						j -= count;
						rle_data -= count;
						count = 0;
					}
					else {
						count += px - j;
						rle_data += px - j;
						j = px;
					}
				}
			}

			if(pixel_format_ == GR_RGB565){
				unsigned mr,mg,mb;
				split_rgb565u(mask_color,mr,mg,mb);

				mr = (mr * (255 - mask_alpha)) >> 8;
				mg = (mg * (255 - mask_alpha)) >> 8;
				mb = (mb * (255 - mask_alpha)) >> 8;

				unsigned cl = make_rgb565u(mr,mg,mb);

				if(!alpha_flag){
					while(j < psx){
						if(count > 0){
							while(count && j < psx){
								if(*rle_data){
									*scr_buf = cl;
								}
								scr_buf += dx;
								count--;
								j++;
							}
							rle_data++;
						}
						else {
							if(count < 0){
								count = -count;
								while(count && j < psx){
									if(*rle_data){
										*scr_buf = cl;
									}
									scr_buf += dx;
									rle_data++;
									count--;
									j++;
								}
							}
						}
						count = *rle_header++;
					}
				}
				else {
					while(j < psx){
						if(count > 0){
							while(count && j < psx){
								const unsigned char* rle_buf = (const unsigned char*)rle_data;
								unsigned a = rle_buf[3];

								if(a != 255){
									a = mask_alpha + ((a * (255 - mask_alpha)) >> 8);

									unsigned r = (mr * (255 - a)) >> 8;
									unsigned g = (mg * (255 - a)) >> 8;
									unsigned b = (mb * (255 - a)) >> 8;

									unsigned cl = make_rgb565u(r,g,b);
									*scr_buf = alpha_blend_565(cl,*scr_buf,a);
								}

								scr_buf += dx;
								count--;
								j++;
							}
							rle_data++;
						}
						else {
							if(count < 0){
								count = -count;
								while(count && j < psx){
									const unsigned char* rle_buf = (const unsigned char*)rle_data;
									unsigned a = rle_buf[3];

									if(a != 255){
										a = mask_alpha + ((a * (255 - mask_alpha)) >> 8);

										unsigned r = (mr * (255 - a)) >> 8;
										unsigned g = (mg * (255 - a)) >> 8;
										unsigned b = (mb * (255 - a)) >> 8;

										unsigned cl = make_rgb565u(r,g,b);
										*scr_buf = alpha_blend_565(cl,*scr_buf,a);
									}

									scr_buf += dx;
									rle_data++;
									count--;
									j++;
								}
							}
						}
						count = *rle_header++;
					}
				}
			}
			else {
				unsigned mr,mg,mb;
				split_rgb565u(mask_color,mr,mg,mb);

				mr = (mr * (255 - mask_alpha)) >> 8;
				mg = (mg * (255 - mask_alpha)) >> 8;
				mb = (mb * (255 - mask_alpha)) >> 8;

				unsigned cl = make_rgb555u(mr,mg,mb);

				if(!alpha_flag){
					while(j < psx){
						if(count > 0){
							while(count && j < psx){
								if(*rle_data){
									*scr_buf = cl;
								}
								scr_buf += dx;
								count--;
								j++;
							}
							rle_data++;
						}
						else {
							if(count < 0){
								count = -count;
								while(count && j < psx){
									if(*rle_data){
										*scr_buf = cl;
									}
									scr_buf += dx;
									rle_data++;
									count--;
									j++;
								}
							}
						}
						count = *rle_header++;
					}
				}
				else {
					while(j < psx){
						if(count > 0){
							while(count && j < psx){
								const unsigned char* rle_buf = (const unsigned char*)rle_data;
								unsigned a = rle_buf[3];

								if(a != 255){
									a = mask_alpha + ((a * (255 - mask_alpha)) >> 8);

									unsigned r = (mr * (255 - a)) >> 8;
									unsigned g = (mg * (255 - a)) >> 8;
									unsigned b = (mb * (255 - a)) >> 8;

									unsigned cl = make_rgb555u(r,g,b);
									*scr_buf = alpha_blend_555(cl,*scr_buf,a);
								}

								scr_buf += dx;
								count--;
								j++;
							}
							rle_data++;
						}
						else {
							if(count < 0){
								count = -count;
								while(count && j < psx){
									const unsigned char* rle_buf = (const unsigned char*)rle_data;
									unsigned a = rle_buf[3];

									if(a != 255){
										a = mask_alpha + ((a * (255 - mask_alpha)) >> 8);

										unsigned r = (mr * (255 - a)) >> 8;
										unsigned g = (mg * (255 - a)) >> 8;
										unsigned b = (mb * (255 - a)) >> 8;

										unsigned cl = make_rgb555u(r,g,b);
										*scr_buf = alpha_blend_555(cl,*scr_buf,a);
									}

									scr_buf += dx;
									rle_data++;
									count--;
									j++;
								}
							}
						}
						count = *rle_header++;
					}
				}
			}
			y += dy;
		}
	}
}

void grDispatcher::PutSprMask_rle(int x,int y,int sx,int sy,const rleBuffer* p,unsigned mask_color,int mask_alpha,int mode,float scale,bool alpha_flag)
{
	int sx_dest = round(float(sx) * scale);
	int sy_dest = round(float(sy) * scale);

	if(sx_dest <= 0 || sy_dest <= 0) return;

	int dx = (sx << 16) / sx_dest;
	int dy = (sy << 16) / sy_dest;
	int fx = (1 << 15);
	int fy = (1 << 15);

	int x0 = 0;
	int x1 = sx_dest - 1;
	int ix = 1;

	int y0 = 0;
	int y1 = sy_dest - 1;
	int iy = 1;

	if(mode & GR_FLIP_VERTICAL){
		y0 = sy_dest - 1,
		y1 = 0;
		iy = -1;
	}

	if(mode & GR_FLIP_HORIZONTAL){
		x0 = sx_dest - 1,
		x1 = 0;
		ix = -1;
	}

	if(bytes_per_pixel() == 2){
		if(!alpha_flag){
			unsigned mr,mg,mb;
			if(pixel_format_ == GR_RGB565) split_rgb565u(mask_color,mr,mg,mb);
			else split_rgb555u(mask_color,mr,mg,mb);

			mr = (mr * (255 - mask_alpha)) >> 8;
			mg = (mg * (255 - mask_alpha)) >> 8;
			mb = (mb * (255 - mask_alpha)) >> 8;

			unsigned mcl = (pixel_format_ == GR_RGB565) ? make_rgb565u(mr,mg,mb) : make_rgb555u(mr,mg,mb);

			const unsigned char* line_src = rleBuffer::get_buffer(0);

			for(int i = y0; i != y1; i += iy){
				p -> decode_line(fy >> 16);

				fy += dy;
				fx = (1 << 15);

				for(int j = x0; j != x1; j += ix){
					if(ClipCheck(x + j,y + i)){
						const unsigned char* src_buf = line_src + ((fx >> 16) << 2);
						if(src_buf[0] || src_buf[1] || src_buf[2]){
							unsigned scl;
							GetPixel(x + j,y + i,scl);

							if(pixel_format_ == GR_RGB565)
								SetPixelFast(x + j,y + i,alpha_blend_565(mcl,scl,mask_alpha));
							else
								SetPixelFast(x + j,y + i,alpha_blend_555(mcl,scl,mask_alpha));
						}
					}
					fx += dx;
				}
			}
		}
		else {
			const unsigned char* line_src = rleBuffer::get_buffer(0);

			if(pixel_format_ == GR_RGB565){
				unsigned mr,mg,mb;
				split_rgb565u(mask_color,mr,mg,mb);

				for(int i = y0; i != y1; i += iy){
					p -> decode_line(fy >> 16);

					fy += dy;
					fx = (1 << 15);

					for(int j = x0; j != x1; j += ix){
						if(ClipCheck(x + j,y + i)){
							const unsigned char* src_buf = line_src + ((fx >> 16) << 2);
							unsigned a = src_buf[3];
							if(a != 255){
								unsigned scl;
								GetPixel(x + j,y + i,scl);

								a = mask_alpha + ((a * (255 - mask_alpha)) >> 8);

								unsigned r = (mr * (255 - a)) >> 8;
								unsigned g = (mg * (255 - a)) >> 8;
								unsigned b = (mb * (255 - a)) >> 8;

								unsigned cl = make_rgb565u(r,g,b);

								SetPixelFast(x + j,y + i,alpha_blend_565(cl,scl,a));
							}
						}
						fx += dx;
					}
				}
			}
			else {
				unsigned mr,mg,mb;
				split_rgb555u(mask_color,mr,mg,mb);

				for(int i = y0; i != y1; i += iy){
					p -> decode_line(fy >> 16);

					fy += dy;
					fx = (1 << 15);

					for(int j = x0; j != x1; j += ix){
						if(ClipCheck(x + j,y + i)){
							const unsigned char* src_buf = line_src + ((fx >> 16) << 2);
							unsigned a = src_buf[3];
							if(a != 255){
								unsigned scl;
								GetPixel(x + j,y + i,scl);

								a = mask_alpha + ((a * (255 - mask_alpha)) >> 8);

								unsigned r = (mr * (255 - a)) >> 8;
								unsigned g = (mg * (255 - a)) >> 8;
								unsigned b = (mb * (255 - a)) >> 8;

								unsigned cl = make_rgb555u(r,g,b);

								SetPixelFast(x + j,y + i,alpha_blend_555(cl,scl,a));
							}
						}
						fx += dx;
					}
				}
			}
		}
	}
	else if(bytes_per_pixel() == 3 || bytes_per_pixel() == 4){
		const unsigned char* line_src = rleBuffer::get_buffer(0);

		unsigned mr,mg,mb;
		split_rgb888(mask_color,mr,mg,mb);

		if(!alpha_flag){
			for(int i = y0; i != y1; i += iy){
				p -> decode_line(fy >> 16);

				fy += dy;
				fx = (1 << 15);

				for(int j = x0; j != x1; j += ix){
					if(ClipCheck(x + j,y + i)){
						int idx = (fx >> 16) << 2;

						if(line_src[idx + 2] || line_src[idx + 1] || line_src[idx + 0]){
							unsigned sr,sg,sb;
							GetPixel(x + j,y + i,sr,sg,sb);

							unsigned r = ((mr * (255 - mask_alpha)) >> 8) + ((mask_alpha * sr) >> 8);
							unsigned g = ((mg * (255 - mask_alpha)) >> 8) + ((mask_alpha * sg) >> 8);
							unsigned b = ((mb * (255 - mask_alpha)) >> 8) + ((mask_alpha * sb) >> 8);

							SetPixelFast(x + j,y + i,r,g,b);
						}
					}
					fx += dx;
				}
			}
		}
		else {
			for(int i = y0; i != y1; i += iy){
				p -> decode_line(fy >> 16);

				fy += dy;
				fx = (1 << 15);

				for(int j = x0; j != x1; j += ix){
					if(ClipCheck(x + j,y + i)){
						int idx = (fx >> 16) << 2;

						unsigned a = line_src[idx + 3];
						if(a != 255){
							unsigned sr,sg,sb;
							GetPixel(x + j,y + i,sr,sg,sb);

							a = mask_alpha + ((a * (255 - mask_alpha)) >> 8);

							unsigned r = ((mr * (255 - a)) >> 8) + ((a * sr) >> 8);
							unsigned g = ((mg * (255 - a)) >> 8) + ((a * sg) >> 8);
							unsigned b = ((mb * (255 - a)) >> 8) + ((a * sb) >> 8);

							SetPixelFast(x + j,y + i,r,g,b);
						}
					}

					fx += dx;
				}
			}
		}
	}
}

void grDispatcher::PutSpr_rle_rot(const Vect2i& pos, const Vect2i& size, const rleBuffer* data, bool has_alpha, int mode, float angle)
{
	unsigned char* buf = (unsigned char*)temp_buffer(size.x * size.y * 4);

	unsigned char* buf_ptr = buf;
	for(int i = 0; i < size.y; i++){
		data->decode_line(i, buf_ptr);
		buf_ptr += size.x * 4;
	}

	if(!has_alpha){
		unsigned* p = (unsigned*)buf;
		buf_ptr = buf + 3;
		for(int i = 0; i < size.y; i++){
			for(int j = 0; j < size.x; j++){
				if(!*p++)
					*buf_ptr = 255;

				buf_ptr += 4;
			}
		}
	}

	PutSpr_rot(pos, size, buf, true, mode, angle);
}

void grDispatcher::PutSpr_rle_rot(const Vect2i& pos, const Vect2i& size, const rleBuffer* data, bool has_alpha, int mode, float angle, const Vect2f& scale)
{
	unsigned char* buf = (unsigned char*)temp_buffer(size.x * size.y * 4);

	unsigned char* buf_ptr = buf;
	for(int i = 0; i < size.y; i++){
		data->decode_line(i, buf_ptr);
		buf_ptr += size.x * 4;
	}

	if(!has_alpha){
		unsigned* p = (unsigned*)buf;
		buf_ptr = buf + 3;
		for(int i = 0; i < size.y; i++){
			for(int j = 0; j < size.x; j++){
				if(!*p++)
					*buf_ptr = 255;

				buf_ptr += 4;
			}
		}
	}

	PutSpr_rot(pos, size, buf, true, mode, angle, scale);
}

void grDispatcher::PutSprMask_rle_rot(const Vect2i& pos, const Vect2i& size, const rleBuffer* data, bool has_alpha, unsigned mask_color, int mask_alpha, int mode, float angle)
{
	unsigned char* buf = (unsigned char*)temp_buffer(size.x * size.y * 4);

	unsigned char* buf_ptr = buf;
	for(int i = 0; i < size.y; i++){
		data->decode_line(i, buf_ptr);
		buf_ptr += size.x * 4;
	}

	if(!has_alpha){
		unsigned* p = (unsigned*)buf;
		buf_ptr = buf + 3;
		for(int i = 0; i < size.y; i++){
			for(int j = 0; j < size.x; j++){
				if(!*p++)
					*buf_ptr = 255;

				buf_ptr += 4;
			}
		}
	}

	PutSprMask_rot(pos, size, buf, true, mask_color, mask_alpha, mode, angle);
}

void grDispatcher::PutSprMask_rle_rot(const Vect2i& pos, const Vect2i& size, const rleBuffer* data, bool has_alpha, unsigned mask_color, int mask_alpha, int mode, float angle, const Vect2f& scale)
{
	unsigned char* buf = (unsigned char*)temp_buffer(size.x * size.y * 4);

	unsigned char* buf_ptr = buf;
	for(int i = 0; i < size.y; i++){
		data->decode_line(i, buf_ptr);
		buf_ptr += size.x * 4;
	}

	if(!has_alpha){
		unsigned* p = (unsigned*)buf;
		buf_ptr = buf + 3;
		for(int i = 0; i < size.y; i++){
			for(int j = 0; j < size.x; j++){
				if(!*p++)
					*buf_ptr = 255;

				buf_ptr += 4;
			}
		}
	}

	PutSprMask_rot(pos, size, buf, true, mask_color, mask_alpha, mode, angle, scale);
}

inline bool rle_alpha_b(unsigned pixel){ return (reinterpret_cast<unsigned char*>(&pixel)[3] < 200); }
inline bool rle_alpha_b16(unsigned short pixel){ return pixel < 200; }

void grDispatcher::DrawSprContour(int x,int y,int sx,int sy,const class rleBuffer* p,int contour_color,int mode,bool alpha_flag)
{
	int px = 0;
	int py = 0;

	int psx = sx;
	int psy = sy;

	if(!clip_rectangle(x,y,px,py,psx,psy)) return;

	if(bytes_per_pixel() == 4){
		int dx = -1;
		int dy = -1;

		x *= 4;

		if(mode & GR_FLIP_HORIZONTAL){
			x += (psx - 1) * 4;
			px = sx - px - psx;
		}
		else 
			dx = 1;

		if(mode & GR_FLIP_VERTICAL){
			y += psy - 1;
			py = sy - py - psy;
		}
		else 
			dy = 1;

		const unsigned* data0 = reinterpret_cast<const unsigned*>(rleBuffer::get_buffer(0));
		const unsigned* data1 = reinterpret_cast<const unsigned*>(rleBuffer::get_buffer(1));

//		px <<= 2;
//		psx <<= 2;

		int ii;
		for(int i = 0; i < psy; i ++){
			unsigned* scr_buf = reinterpret_cast<unsigned*>(screenBuf + yTable[y] + x);
			unsigned* scr_buf_prev = (i) ? reinterpret_cast<unsigned*>(screenBuf + yTable[y - dy] + x) : scr_buf;
			p -> decode_line(py + i,i & 1);

			const unsigned* data_ptr = (i & 1) ? data1 + px : data0 + px;
			const unsigned* data_ptr_prev = (i & 1) ? data0 + px : data1 + px;

			if(!alpha_flag){
				unsigned pixel = 0;
				for(int j = 0; j < psx; j ++){
					pixel = data_ptr[j];

					if(!pixel && j && data_ptr[j - 1]){
						*(scr_buf - dx) = contour_color;
					}
					if((pixel && (!j || !data_ptr[j - 1]))){
						*scr_buf = contour_color;
					}
					else {
						if(pixel && (!i || !data_ptr_prev[j])){
							*scr_buf = contour_color;
						}
					}
					if(!pixel && i && data_ptr_prev[j]){
						*scr_buf_prev = contour_color;
					}

					scr_buf += dx;
					scr_buf_prev += dx;
				}
				if(pixel) *(scr_buf - dx) = contour_color;
			}
			else {
				bool pixel = false;
				for(int j = 0; j < psx; j ++){
					pixel = rle_alpha_b(data_ptr[j]);

					if(!pixel && j && rle_alpha_b(data_ptr[j - 1])){
						*(scr_buf - dx) = contour_color;
					}
					if((pixel && (!j || !rle_alpha_b(data_ptr[j - 1])))){
						*scr_buf = contour_color;
					}
					else {
						if(pixel && (!i || !rle_alpha_b(data_ptr_prev[j]))){
							*scr_buf = contour_color;
						}
					}
					if(!pixel && i && rle_alpha_b(data_ptr_prev[j])){
						*scr_buf_prev = contour_color;
					}

					scr_buf += dx;
					scr_buf_prev += dx;
				}
				if(pixel) *(scr_buf - dx) = contour_color;
			}

			y += dy;
			ii = i;
		}
		unsigned* scr_buf_prev = reinterpret_cast<unsigned*>(screenBuf + yTable[y - dy] + x);
		const unsigned* data_ptr_prev = (ii & 1) ? data0 + px : data1 + px;
		if(!alpha_flag){
			for(int j = 0; j < psx; j ++){
				if(data_ptr_prev[j])
					*scr_buf_prev = contour_color;
				scr_buf_prev += dx;
			}
		}
		else {
			for(int j = 0; j < psx; j ++){
				if(rle_alpha_b(data_ptr_prev[j]))
					*scr_buf_prev = contour_color;
				scr_buf_prev += dx;
			}
		}

		return;
	}
	if(bytes_per_pixel() == 3){
		int dx = -3;
		int dy = -1;

		x *= 3;

		if(mode & GR_FLIP_HORIZONTAL){
			x += psx * 3 - 3;
			px = sx - px - psx;
		}
		else 
			dx = 3;

		if(mode & GR_FLIP_VERTICAL){
			y += psy - 1;
			py = sy - py - psy;
		}
		else 
			dy = 1;

		const unsigned* data0 = reinterpret_cast<const unsigned*>(rleBuffer::get_buffer(0));
		const unsigned* data1 = reinterpret_cast<const unsigned*>(rleBuffer::get_buffer(1));

		for(int i = 0; i < psy; i ++){
			unsigned char* scr_buf = reinterpret_cast<unsigned char*>(screenBuf + yTable[y] + x);
			unsigned char* scr_buf_prev = (i) ? reinterpret_cast<unsigned char*>(screenBuf + yTable[y - dy] + x) : scr_buf;
			p -> decode_line(py + i,i & 1);

			const unsigned* data_ptr = (i & 1) ? data1 + px : data0 + px;
			const unsigned* data_ptr_prev = (i & 1) ? data0 + px : data1 + px;

			if(!alpha_flag){
				unsigned pixel = 0;
				for(int j = 0; j < psx; j ++){
					pixel = data_ptr[j];

					if(!pixel && j && data_ptr[j - 1]){
						(scr_buf - dx)[0] = reinterpret_cast<const unsigned char*>(&contour_color)[0];
						(scr_buf - dx)[1] = reinterpret_cast<const unsigned char*>(&contour_color)[1];
						(scr_buf - dx)[2] = reinterpret_cast<const unsigned char*>(&contour_color)[2];
					}
					if((pixel && (!j || !data_ptr[j - 1]))){
						scr_buf[0] = reinterpret_cast<const unsigned char*>(&contour_color)[0];
						scr_buf[1] = reinterpret_cast<const unsigned char*>(&contour_color)[1];
						scr_buf[2] = reinterpret_cast<const unsigned char*>(&contour_color)[2];
					}
					else {
						if(pixel && (!i || !data_ptr_prev[j])){
							scr_buf[0] = reinterpret_cast<const unsigned char*>(&contour_color)[0];
							scr_buf[1] = reinterpret_cast<const unsigned char*>(&contour_color)[1];
							scr_buf[2] = reinterpret_cast<const unsigned char*>(&contour_color)[2];
						}
					}
					if(!pixel && i && data_ptr_prev[j]){
						scr_buf_prev[0] = reinterpret_cast<const unsigned char*>(&contour_color)[0];
						scr_buf_prev[1] = reinterpret_cast<const unsigned char*>(&contour_color)[1];
						scr_buf_prev[2] = reinterpret_cast<const unsigned char*>(&contour_color)[2];
					}

					scr_buf += dx;
					scr_buf_prev += dx;
				}
				if(pixel){
					(scr_buf - dx)[0] = reinterpret_cast<const unsigned char*>(&contour_color)[0];
					(scr_buf - dx)[1] = reinterpret_cast<const unsigned char*>(&contour_color)[1];
					(scr_buf - dx)[2] = reinterpret_cast<const unsigned char*>(&contour_color)[2];
				}
			}
			else {
				bool pixel = false;
				for(int j = 0; j < psx; j ++){
					pixel = rle_alpha_b(data_ptr[j]);

					if(!pixel && j && rle_alpha_b(data_ptr[j - 1])){
						(scr_buf - dx)[0] = reinterpret_cast<const unsigned char*>(&contour_color)[0];
						(scr_buf - dx)[1] = reinterpret_cast<const unsigned char*>(&contour_color)[1];
						(scr_buf - dx)[2] = reinterpret_cast<const unsigned char*>(&contour_color)[2];
					}
					if((pixel && (!j || !rle_alpha_b(data_ptr[j - 1])))){
						scr_buf[0] = reinterpret_cast<const unsigned char*>(&contour_color)[0];
						scr_buf[1] = reinterpret_cast<const unsigned char*>(&contour_color)[1];
						scr_buf[2] = reinterpret_cast<const unsigned char*>(&contour_color)[2];
					}
					else {
						if(pixel && (!i || !rle_alpha_b(data_ptr_prev[j]))){
							scr_buf[0] = reinterpret_cast<const unsigned char*>(&contour_color)[0];
							scr_buf[1] = reinterpret_cast<const unsigned char*>(&contour_color)[1];
							scr_buf[2] = reinterpret_cast<const unsigned char*>(&contour_color)[2];
						}
					}
					if(!pixel && i && rle_alpha_b(data_ptr_prev[j])){
						scr_buf_prev[0] = reinterpret_cast<const unsigned char*>(&contour_color)[0];
						scr_buf_prev[1] = reinterpret_cast<const unsigned char*>(&contour_color)[1];
						scr_buf_prev[2] = reinterpret_cast<const unsigned char*>(&contour_color)[2];
					}

					scr_buf += dx;
					scr_buf_prev += dx;
				}
				if(pixel){
					(scr_buf - dx)[0] = reinterpret_cast<const unsigned char*>(&contour_color)[0];
					(scr_buf - dx)[1] = reinterpret_cast<const unsigned char*>(&contour_color)[1];
					(scr_buf - dx)[2] = reinterpret_cast<const unsigned char*>(&contour_color)[2];
				}
			}

			y += dy;
		}
		unsigned char* scr_buf_prev = reinterpret_cast<unsigned char*>(screenBuf + yTable[y - dy] + x);
		const unsigned* data_ptr_prev = (psy & 1) ? data0 + px : data1 + px;
		if(!alpha_flag){
			for(int j = 0; j < psx; j ++){
				if(data_ptr_prev[j]){
					scr_buf_prev[0] = reinterpret_cast<const unsigned char*>(&contour_color)[0];
					scr_buf_prev[1] = reinterpret_cast<const unsigned char*>(&contour_color)[1];
					scr_buf_prev[2] = reinterpret_cast<const unsigned char*>(&contour_color)[2];
				}
				scr_buf_prev += dx;
			}
		}
		else {
			for(int j = 0; j < psx; j ++){
				if(rle_alpha_b(data_ptr_prev[j])){
					scr_buf_prev[0] = reinterpret_cast<const unsigned char*>(&contour_color)[0];
					scr_buf_prev[1] = reinterpret_cast<const unsigned char*>(&contour_color)[1];
					scr_buf_prev[2] = reinterpret_cast<const unsigned char*>(&contour_color)[2];
				}
				scr_buf_prev += dx;
			}
		}
	}
	else if(bytes_per_pixel() == 2){
		int dx = -1;
		int dy = -1;

		x *= 2;

		if(mode & GR_FLIP_HORIZONTAL){
			x += (psx - 1) * 2;
			px = sx - px - psx;
		}
		else 
			dx = 1;

		if(mode & GR_FLIP_VERTICAL){
			y += psy - 1;
			py = sy - py - psy;
		}
		else 
			dy = 1;

		const unsigned short* data0 = reinterpret_cast<const unsigned short*>(rleBuffer::get_buffer(0));
		const unsigned short* data1 = reinterpret_cast<const unsigned short*>(rleBuffer::get_buffer(1));

		px <<= 1;
		psx <<= 1;

		for(int i = 0; i < psy; i ++){
			unsigned short* scr_buf = reinterpret_cast<unsigned short*>(screenBuf + yTable[y] + x);
			unsigned short* scr_buf_prev = (i) ? reinterpret_cast<unsigned short*>(screenBuf + yTable[y - dy] + x) : scr_buf;
			p -> decode_line(py + i,i & 1);

			const unsigned short* data_ptr = (i & 1) ? data1 + px : data0 + px;
			const unsigned short* data_ptr_prev = (i & 1) ? data0 + px : data1 + px;

			if(!alpha_flag){
				unsigned pixel = 0;
				for(int j = 0; j < psx; j += 2){
					pixel = data_ptr[j];

					if(!pixel && j && data_ptr[j - 2]){
						*(scr_buf - dx) = contour_color;
					}
					if((pixel && (!j || !data_ptr[j - 2]))){
						*scr_buf = contour_color;
					}
					else {
						if(pixel && (!i || !data_ptr_prev[j])){
							*scr_buf = contour_color;
						}
					}
					if(!pixel && i && data_ptr_prev[j]){
						*scr_buf_prev = contour_color;
					}

					scr_buf += dx;
					scr_buf_prev += dx;
				}
				if(pixel) *(scr_buf - dx) = contour_color;
			}
			else {
				bool pixel = false;
				for(int j = 0; j < psx; j += 2){
					pixel = rle_alpha_b16(data_ptr[j + 1]);

					if(!pixel && j && rle_alpha_b16(data_ptr[j - 1])){
						*(scr_buf - dx) = contour_color;
					}
					if((pixel && (!j || !rle_alpha_b16(data_ptr[j - 1])))){
						*scr_buf = contour_color;
					}
					else {
						if(pixel && (!i || !rle_alpha_b16(data_ptr_prev[j + 1]))){
							*scr_buf = contour_color;
						}
					}
					if(!pixel && i && rle_alpha_b16(data_ptr_prev[j + 1])){
						*scr_buf_prev = contour_color;
					}

					scr_buf += dx;
					scr_buf_prev += dx;
				}
				if(pixel) *(scr_buf - dx) = contour_color;
			}

			y += dy;
		}
		unsigned short* scr_buf_prev = reinterpret_cast<unsigned short*>(screenBuf + yTable[y - dy] + x);
		const unsigned short* data_ptr_prev = (psy & 1) ? data0 + px : data1 + px;
		if(!alpha_flag){
			for(int j = 0; j < psx; j += 2){
				if(data_ptr_prev[j])
					*scr_buf_prev = contour_color;
				scr_buf_prev += dx;
			}
		}
		else {
			for(int j = 0; j < psx; j += 2){
				if(rle_alpha_b16(data_ptr_prev[j + 1]))
					*scr_buf_prev = contour_color;
				scr_buf_prev += dx;
			}
		}
	}
}

void grDispatcher::DrawSprContour(int x,int y,int sx,int sy,const class rleBuffer* p,int contour_color,int mode,float scale,bool alpha_flag)
{
	int sx_dest = round(float(sx) * scale);
	int sy_dest = round(float(sy) * scale);

	if(!sx_dest || !sy_dest) return;

	int dx = (sx << 16) / sx_dest;
	int dy = (sy << 16) / sy_dest;
	int fx = (1 << 15);
	int fy = (1 << 15);

	int x0 = 0;
	int x1 = sx_dest;
	int ix = 1;

	int y0 = 0;
	int y1 = sy_dest;
	int iy = 1;

	if(mode & GR_FLIP_VERTICAL){
		y0 = sy_dest,
		y1 = 0;
		iy = -1;
	}

	if(mode & GR_FLIP_HORIZONTAL){
		x0 = sx_dest,
		x1 = 0;
		ix = -1;
	}

	if(bytes_per_pixel() == 2){
		if(!alpha_flag){
			const unsigned short* line0 = reinterpret_cast<const unsigned short*>(rleBuffer::get_buffer(0));
			const unsigned short* line1 = reinterpret_cast<const unsigned short*>(rleBuffer::get_buffer(1));

			for(int i = y0; i != y1; i += iy){
				p -> decode_line(fy >> 16,i & 1);
				const unsigned short* line_src = (i & 1) ? line1 : line0;
				const unsigned short* line_src_prev = (i & 1) ? line0 : line1;

				fy += dy;
				fx = (1 << 15);

				unsigned cl = 0;
				for(int j = x0; j != x1; j += ix){
					if(ClipCheck(x + j,y + i)){
						cl = line_src[(fx >> 16) << 1];
						if(!cl && j != x0 && line_src[((fx - dx) >> 16) << 1])
							SetPixel(x + j - ix,y + i,contour_color);

						if(cl && (j == x0 || !line_src[((fx - dx) >> 16) << 1])){
							SetPixelFast(x + j,y + i,contour_color);
						}
						else {
							if(cl && (i == y0 || !line_src_prev[(fx >> 16) << 1]))
								SetPixelFast(x + j,y + i,contour_color);
						}

						if(!cl && i != y0 && line_src_prev[(fx >> 16) << 1])
							SetPixel(x + j,y + i - iy,contour_color);
					}
					fx += dx;
				}
				if(cl) SetPixel(x + x1 - ix,y + i,contour_color);
			}
			fx = (1 << 15);
			for(int j = x0; j != x1; j += ix){
				const unsigned short* line_src_prev = (y1 & 1) ? line0 : line1;
				if(line_src_prev[(fx >> 16) << 1])
					SetPixel(x + j,y + y1 - iy,contour_color);
				fx += dx;
			}
		}
		else {
			const unsigned short* line0 = reinterpret_cast<const unsigned short*>(rleBuffer::get_buffer(0));
			const unsigned short* line1 = reinterpret_cast<const unsigned short*>(rleBuffer::get_buffer(1));

			for(int i = y0; i != y1; i += iy){
				p -> decode_line(fy >> 16,i & 1);
				const unsigned short* line_src = (i & 1) ? line1 : line0;
				const unsigned short* line_src_prev = (i & 1) ? line0 : line1;

				fy += dy;
				fx = (1 << 15);

				bool cl = false;
				for(int j = x0; j != x1; j += ix){
					if(ClipCheck(x + j,y + i)){
						cl = rle_alpha_b16(line_src[((fx >> 16) << 1) + 1]);
						if(!cl && j != x0 && rle_alpha_b16(line_src[(((fx - dx) >> 16) << 1) + 1]))
							SetPixel(x + j - ix,y + i,contour_color);

						if(cl && (j == x0 || !rle_alpha_b16(line_src[(((fx - dx) >> 16) << 1) + 1]))){
							SetPixelFast(x + j,y + i,contour_color);
						}
						else {
							if(cl && (i == y0 || !rle_alpha_b16(line_src_prev[((fx >> 16) << 1) + 1])))
								SetPixelFast(x + j,y + i,contour_color);
						}

						if(!cl && i != y0 && rle_alpha_b16(line_src_prev[((fx >> 16) << 1) + 1]))
							SetPixel(x + j,y + i - iy,contour_color);
					}
					fx += dx;
				}
				if(cl) SetPixel(x + x1 - ix,y + i,contour_color);
			}
			fx = (1 << 15);
			for(int j = x0; j != x1; j += ix){
				const unsigned short* line_src_prev = (y1 & 1) ? line0 : line1;
				if(rle_alpha_b16(line_src_prev[((fx >> 16) << 1) + 1]))
					SetPixel(x + j,y + y1 - iy,contour_color);
				fx += dx;
			}
		}
	}
	else if(bytes_per_pixel() == 3 || bytes_per_pixel() == 4){
		if(!alpha_flag){
			const unsigned* line0 = reinterpret_cast<const unsigned*>(rleBuffer::get_buffer(0));
			const unsigned* line1 = reinterpret_cast<const unsigned*>(rleBuffer::get_buffer(1));

			for(int i = y0; i != y1; i += iy){
				p -> decode_line(fy >> 16,i & 1);
				const unsigned* line_src = (i & 1) ? line1 : line0;
				const unsigned* line_src_prev = (i & 1) ? line0 : line1;

				fy += dy;
				fx = (1 << 15);

				unsigned cl = 0;
				for(int j = x0; j != x1; j += ix){
					if(ClipCheck(x + j,y + i)){
						cl = line_src[fx >> 16];
						if(!cl && j != x0 && line_src[(fx - dx) >> 16])
							SetPixel(x + j - ix,y + i,contour_color);

						if(cl && (j == x0 || !line_src[(fx - dx) >> 16])){
							SetPixelFast(x + j,y + i,contour_color);
						}
						else {
							if(cl && (i == y0 || !line_src_prev[fx >> 16]))
								SetPixelFast(x + j,y + i,contour_color);
						}

						if(!cl && i != y0 && line_src_prev[fx >> 16])
							SetPixel(x + j,y + i - iy,contour_color);
					}
					fx += dx;
				}
				if(cl) SetPixel(x + x1 - ix,y + i,contour_color);
			}
			fx = (1 << 15);
			for(int j = x0; j != x1; j += ix){
				const unsigned* line_src_prev = (y1 & 1) ? line0 : line1;
				if(line_src_prev[fx >> 16])
					SetPixel(x + j,y + y1 - iy,contour_color);
				fx += dx;
			}
		}
		else {
			const unsigned* line0 = reinterpret_cast<const unsigned*>(rleBuffer::get_buffer(0));
			const unsigned* line1 = reinterpret_cast<const unsigned*>(rleBuffer::get_buffer(1));

			for(int i = y0; i != y1; i += iy){
				p -> decode_line(fy >> 16,i & 1);
				const unsigned* line_src = (i & 1) ? line1 : line0;
				const unsigned* line_src_prev = (i & 1) ? line0 : line1;

				fy += dy;
				fx = (1 << 15);

				bool cl = false;
				for(int j = x0; j != x1; j += ix){
					if(ClipCheck(x + j,y + i)){
						cl = rle_alpha_b(line_src[fx >> 16]);
						if(!cl && j != x0 && rle_alpha_b(line_src[(fx - dx) >> 16]))
							SetPixel(x + j - ix,y + i,contour_color);

						if(cl && (j == x0 || !rle_alpha_b(line_src[(fx - dx) >> 16]))){
							SetPixelFast(x + j,y + i,contour_color);
						}
						else {
							if(cl && (i == y0 || !rle_alpha_b(line_src_prev[fx >> 16])))
								SetPixelFast(x + j,y + i,contour_color);
						}

						if(!cl && i != y0 && rle_alpha_b(line_src_prev[fx >> 16]))
							SetPixel(x + j,y + i - iy,contour_color);
					}
					fx += dx;
				}
				if(cl) SetPixel(x + x1 - ix,y + i,contour_color);
			}
			fx = (1 << 15);
			for(int j = x0; j != x1; j += ix){
				const unsigned* line_src_prev = (y1 & 1) ? line0 : line1;
				if(rle_alpha_b(line_src_prev[fx >> 16]))
					SetPixel(x + j,y + y1 - iy,contour_color);
				fx += dx;
			}
		}
	}
}
