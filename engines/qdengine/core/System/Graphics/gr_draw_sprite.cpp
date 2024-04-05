/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include <memory.h>

#include "gr_dispatcher.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

void grDispatcher::PutSpr_a(int x,int y,int sx,int sy,const unsigned char* p,int mode,float scale)
{
	int i,j,sx_dest,sy_dest;

	sx_dest = round(float(sx) * scale);
	sy_dest = round(float(sy) * scale);

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
		sx *= 4;
		if(pixel_format_ == GR_ARGB1555){
			for(i = y0; i != y1; i += iy){
				const unsigned char* line_src = p + ((fy >> 16) * sx);

				fy += dy;
				fx = (1 << 15);

				for(j = x0; j != x1; j += ix){
					const unsigned char* src_data = line_src + (fx >> 16) * 4;
					unsigned a = src_data[3];

					if(a != 255 && ClipCheck(x + j,y + i)){
						if(a){
							unsigned sc;
							GetPixel(x + j,y + i,sc);
							SetPixel(x + j, y + i, alpha_blend_555(make_rgb555u(src_data[2], src_data[1], src_data[0]),sc,a));
						}
						else 
							SetPixel(x + j, y + i, make_rgb555u(src_data[2], src_data[1], src_data[0]));
					}
					fx += dx;
				}
			}
		}
		else {
			for(i = y0; i != y1; i += iy){
				const unsigned char* line_src = p + ((fy >> 16) * sx);

				fy += dy;
				fx = (1 << 15);

				for(j = x0; j != x1; j += ix){
					const unsigned char* src_data = line_src + (fx >> 16) * 4;
					unsigned a = src_data[3];

					if(a != 255 && ClipCheck(x + j,y + i)){
						if(a){
							unsigned sc;
							GetPixel(x + j,y + i,sc);
							SetPixel(x + j, y + i, alpha_blend_565(make_rgb565u(src_data[2], src_data[1], src_data[0]),sc,a));
						}
						else 
							SetPixel(x + j, y + i, make_rgb565u(src_data[2], src_data[1], src_data[0]));
					}
					fx += dx;
				}
			}
		}
	}
	else if(bytes_per_pixel() == 3 || bytes_per_pixel() == 4){
		int sx3 = sx * 4;

		for(i = y0; i != y1; i += iy){
			const unsigned char* line_src = p + ((fy >> 16) * sx3);

			fy += dy;
			fx = (1 << 15);

			for(j = x0; j != x1; j += ix){
				int idx = (fx >> 16) << 2;
				unsigned a = line_src[idx + 3];
				if(a != 255 && ClipCheck(x + j,y + i)){
					unsigned sr,sg,sb;
					GetPixel(x + j,y + i,sr,sg,sb);

					unsigned r = line_src[idx + 2] + ((a * sr) >> 8);
					unsigned g = line_src[idx + 1] + ((a * sg) >> 8);
					unsigned b = line_src[idx + 0] + ((a * sb) >> 8);

					SetPixel(x + j,y + i,r,g,b);
				}

				fx += dx;
			}
		}
		return;
	}
}

void grDispatcher::PutSpr(int x,int y,int sx,int sy,const unsigned char* p,int mode,float scale)
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
		const unsigned short* src = reinterpret_cast<const unsigned short*>(p);

		for(int i = y0; i != y1; i += iy){
			const unsigned short* line_src = src + ((fy >> 16) * sx);

			fy += dy;
			fx = (1 << 15);

			for(int j = x0; j != x1; j += ix){
				unsigned cl = line_src[fx >> 16];
				if(cl)
					SetPixel(x + j,y + i,cl);
				fx += dx;
			}
		}
		return;
	}

	if(bytes_per_pixel() == 3 || bytes_per_pixel() == 4){
		int sx3 = sx * 3;
		for(int i = y0; i != y1; i += iy){
			const unsigned char* line_src = p + ((fy >> 16) * sx3);

			fy += dy;
			fx = (1 << 15);

			for(int j = x0; j != x1; j += ix){
				int idx = (fx >> 16) * 3;

				unsigned r = line_src[idx + 2];
				unsigned g = line_src[idx + 1];
				unsigned b = line_src[idx + 0];

				if(r || g || b)
					SetPixel(x + j,y + i,r,g,b);

				fx += dx;
			}
		}
		return;
	}
/*
	if(bytes_per_pixel() == 4){
		const unsigned* src = reinterpret_cast<const unsigned*>(p);

		for(int i = y0; i != y1; i += iy){
			const unsigned* line_src = src + ((fy >> 16) * sx);

			fy += dy;
			fx = (1 << 15);

			for(int j = x0; j != x1; j += ix){
				unsigned cl = line_src[fx >> 16];
				if(cl)
					SetPixel(x + j,y + i,cl);
				fx += dx;
			}
		}
		return;
	}
*/
}

void grDispatcher::PutSpr_a(int x,int y,int sx,int sy,const unsigned char* p,int mode)
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

		if(mode & GR_FLIP_VERTICAL){
			y += psy - 1;
			py = sy - py - psy;
		}
		else 
			dy = 1;

		int px3 = px * 4;
		int sx3 = sx * 4;

		const unsigned char* data_ptr = p + py * sx3;

		for(int i = 0; i < psy; i ++){
			unsigned char* scr_buf = reinterpret_cast<unsigned char*>(screenBuf + yTable[y] + x);
			const unsigned char* data_line = data_ptr + px3;

			for(int j = 0; j < psx; j ++){
				unsigned a = data_line[3];
				if(a != 255){
					if(a){
						scr_buf[0] = data_line[0] + ((a * scr_buf[0]) >> 8);
						scr_buf[1] = data_line[1] + ((a * scr_buf[1]) >> 8);
						scr_buf[2] = data_line[2] + ((a * scr_buf[2]) >> 8);
					}
					else {
						scr_buf[0] = data_line[0];
						scr_buf[1] = data_line[1];
						scr_buf[2] = data_line[2];
					}
				}
				scr_buf += dx;
				data_line += 4;
			}
			data_ptr += sx3;
			y += dy;
		}
		return;
	}
	if(bytes_per_pixel() == 3){
		int dx = -3;
		int dy = -1;

		x *= 3;

		if(mode & GR_FLIP_HORIZONTAL){
			x += (psx - 1) * 3;
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

		int px3 = px * 4;
		int sx3 = sx * 4;

		const unsigned char* data_ptr = p + py * sx3;

		for(int i = 0; i < psy; i ++){
			unsigned char* scr_buf = reinterpret_cast<unsigned char*>(screenBuf + yTable[y] + x);
			const unsigned char* data_line = data_ptr + px3;

			for(int j = 0; j < psx; j ++){
				unsigned a = data_line[3];
				if(a != 255){
					if(a){
						scr_buf[0] = data_line[0] + ((a * scr_buf[0]) >> 8);
						scr_buf[1] = data_line[1] + ((a * scr_buf[1]) >> 8);
						scr_buf[2] = data_line[2] + ((a * scr_buf[2]) >> 8);
					}
					else {
						scr_buf[0] = data_line[0];
						scr_buf[1] = data_line[1];
						scr_buf[2] = data_line[2];
					}
				}
				scr_buf += dx;
				data_line += 4;
			}
			data_ptr += sx3;
			y += dy;
		}
		return;
	}
	if(bytes_per_pixel() == 2){
		int dx = -1;
		int dy = -1;

		if(mode & GR_FLIP_HORIZONTAL){
			x += psx - 1;
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

		x <<= 1;
		sx <<= 2;
		px <<= 2;

		const unsigned char* data_ptr = p + py * sx;

		if(pixel_format_ == GR_RGB565){
			for(int i = 0; i < psy; i ++){
				unsigned short* scr_buf = reinterpret_cast<unsigned short*>(screenBuf + yTable[y] + x);
				const unsigned char* data_line = data_ptr + px;

				for(int j = 0; j < psx; j ++){
					unsigned a = data_line[3];
					if(a != 255){
						if(a)
							*scr_buf = alpha_blend_565(make_rgb565u(data_line[2], data_line[1], data_line[0]),*scr_buf,a);
						else 
							*scr_buf = make_rgb565u(data_line[2], data_line[1], data_line[0]);
					}

					scr_buf += dx;
					data_line += 4;
				}
				data_ptr += sx;
				y += dy;
			}
		}
		else {
			for(int i = 0; i < psy; i ++){
				unsigned short* scr_buf = reinterpret_cast<unsigned short*>(screenBuf + yTable[y] + x);
				const unsigned char* data_line = data_ptr + px;

				for(int j = 0; j < psx; j ++){
					unsigned a = data_line[3];
					if(a != 255){
						if(a)
							*scr_buf = alpha_blend_555(make_rgb555u(data_line[2], data_line[1], data_line[0]),*scr_buf,a);
						else 
							*scr_buf = make_rgb555u(data_line[2], data_line[1], data_line[0]);
					}

					scr_buf += dx;
					data_line += 4;
				}
				data_ptr += sx;
				y += dy;
			}
		}
		return;
	}
}

void grDispatcher::PutSpr_rot(const Vect2i& pos, const Vect2i& size, const unsigned char* data, bool has_alpha, int mode, float angle)
{
	const int F_PREC = 16;

	int xc = pos.x + size.x/2;
	int yc = pos.y + size.y/2;

	float sn = sinf(angle);
	float cs = cosf(angle);

	int sx = round(fabs(cs) * float(size.x) + fabs(sn) * float(size.y)) + 2;
	int sy = round(fabs(sn) * float(size.x) + fabs(cs) * float(size.y)) + 2;

	int x0 = xc - sx/2;
	int y0 = yc - sy/2;

	int dx = 0;
	int dy = 0;

	if(!(round(R2G(angle)) % 90)){
		int angle_num = round(cycleAngle(angle) / (M_PI/2.f));
		switch(angle_num){
			case 1:
				dy = -2;
				break;
			case 2:
				dx = -2;
				dy = -2;
				break;
			case 3:
				dx = -2;
				break;
		}
	}

	if(!clip_rectangle(x0, y0, sx, sy))
		return;

	int sin_a = round(sn * float(1 << F_PREC));
	int cos_a = round(cs * float(1 << F_PREC));

	if(has_alpha){
		if(bytes_per_pixel() == 4 || bytes_per_pixel() == 3){
			for(int y = 0; y <= sy; y++){
				unsigned char* screen_ptr = (unsigned char*)(screenBuf + yTable[y + y0] + x0 * bytes_per_pixel());

				int xx = (x0 - xc) * cos_a + (y + y0 - yc) * sin_a + ((size.x + 1 + dx) << (F_PREC - 1));
				int yy = (y + y0 - yc) * cos_a - (x0 - xc) * sin_a + ((size.y + 1 + dy) << (F_PREC - 1));

				for(int x = 0; x <= sx; x++){
					int xb = (xx >> F_PREC);
					int yb = (yy >> F_PREC);

					if(xb >= 0 && xb < size.x && yb >= 0 && yb < size.y){
						if(mode & GR_FLIP_HORIZONTAL)
							xb = size.x - xb - 1;
						if(mode & GR_FLIP_VERTICAL)
							yb = size.y - yb - 1;

						const unsigned char* data_ptr = data + size.x * 4 * yb + xb * 4;

						unsigned a = data_ptr[3];
						if(a != 255){
							if(a){
								screen_ptr[0] = data_ptr[0] + ((a * screen_ptr[0]) >> 8);
								screen_ptr[1] = data_ptr[1] + ((a * screen_ptr[1]) >> 8);
								screen_ptr[2] = data_ptr[2] + ((a * screen_ptr[2]) >> 8);
							}
							else {
								screen_ptr[0] = data_ptr[0];
								screen_ptr[1] = data_ptr[1];
								screen_ptr[2] = data_ptr[2];
							}
						}
					}

					xx += cos_a;
					yy -= sin_a;

					screen_ptr += bytes_per_pixel();
				}
			}
		}
		else if(bytes_per_pixel() == 2){
			if(pixel_format_ == GR_RGB565){
				for(int y = 0; y <= sy; y++){
					unsigned short* screen_ptr = (unsigned short*)(screenBuf + yTable[y + y0] + x0 * 2);

					int xx = (x0 - xc) * cos_a + (y + y0 - yc) * sin_a + ((size.x + 1 + dx) << (F_PREC - 1));
					int yy = (y + y0 - yc) * cos_a - (x0 - xc) * sin_a + ((size.y + 1 + dy) << (F_PREC - 1));

					for(int x = 0; x <= sx; x++){
						int xb = (xx >> F_PREC);
						int yb = (yy >> F_PREC);

						if(xb >= 0 && xb < size.x && yb >= 0 && yb < size.y){
							if(mode & GR_FLIP_HORIZONTAL)
								xb = size.x - xb - 1;
							if(mode & GR_FLIP_VERTICAL)
								yb = size.y - yb - 1;

							const unsigned char* data_ptr = data + size.x * 4 * yb + xb * 4;

							unsigned a = data_ptr[3];
							if(a != 255){
								if(a)
									*screen_ptr = alpha_blend_565(make_rgb565u(data_ptr[2], data_ptr[1], data_ptr[0]),*screen_ptr,a);
								else 
									*screen_ptr = make_rgb565u(data_ptr[2], data_ptr[1], data_ptr[0]);
							}
						}

						xx += cos_a;
						yy -= sin_a;

						screen_ptr++;
					}
				}
			}
			else if(pixel_format_ == GR_ARGB1555){
				for(int y = 0; y <= sy; y++){
					unsigned short* screen_ptr = (unsigned short*)(screenBuf + yTable[y + y0] + x0 * 2);

					int xx = (x0 - xc) * cos_a + (y + y0 - yc) * sin_a + ((size.x + 1 + dx) << (F_PREC - 1));
					int yy = (y + y0 - yc) * cos_a - (x0 - xc) * sin_a + ((size.y + 1 + dy) << (F_PREC - 1));

					for(int x = 0; x <= sx; x++){
						int xb = (xx >> F_PREC);
						int yb = (yy >> F_PREC);

						if(xb >= 0 && xb < size.x && yb >= 0 && yb < size.y){
							if(mode & GR_FLIP_HORIZONTAL)
								xb = size.x - xb - 1;
							if(mode & GR_FLIP_VERTICAL)
								yb = size.y - yb - 1;

							const unsigned char* data_ptr = data + size.x * 4 * yb + xb * 4;

							unsigned a = data_ptr[3];
							if(a != 255){
								if(a)
									*screen_ptr = alpha_blend_555(make_rgb555u(data_ptr[2], data_ptr[1], data_ptr[0]),*screen_ptr,a);
								else 
									*screen_ptr = make_rgb555u(data_ptr[2], data_ptr[1], data_ptr[0]);
							}
						}

						xx += cos_a;
						yy -= sin_a;

						screen_ptr++;
					}
				}
			}
		}
	}
	else {
		if(bytes_per_pixel() == 4 || bytes_per_pixel() == 3){
			for(int y = 0; y <= sy; y++){
				unsigned char* screen_ptr = (unsigned char*)(screenBuf + yTable[y + y0] + x0 * bytes_per_pixel());

				int xx = (x0 - xc) * cos_a + (y + y0 - yc) * sin_a + ((size.x + 1 + dx) << (F_PREC - 1));
				int yy = (y + y0 - yc) * cos_a - (x0 - xc) * sin_a + ((size.y + 1 + dy) << (F_PREC - 1));

				for(int x = 0; x <= sx; x++){
					int xb = (xx >> F_PREC);
					int yb = (yy >> F_PREC);

					if(xb >= 0 && xb < size.x && yb >= 0 && yb < size.y){
						if(mode & GR_FLIP_HORIZONTAL)
							xb = size.x - xb - 1;
						if(mode & GR_FLIP_VERTICAL)
							yb = size.y - yb - 1;

						const unsigned char* data_ptr = data + size.x * 3 * yb + xb * 3;
						if(data_ptr[0] || data_ptr[1] || data_ptr[2]){
							screen_ptr[0] = data_ptr[0];
							screen_ptr[1] = data_ptr[1];
							screen_ptr[2] = data_ptr[2];
						}
					}

					xx += cos_a;
					yy -= sin_a;

					screen_ptr += bytes_per_pixel();
				}
			}
		}
		else if(bytes_per_pixel() == 2){
			if(pixel_format_ == GR_RGB565){
				for(int y = 0; y <= sy; y++){
					unsigned short* screen_ptr = (unsigned short*)(screenBuf + yTable[y + y0] + x0 * 2);

					int xx = (x0 - xc) * cos_a + (y + y0 - yc) * sin_a + ((size.x + 1 + dx) << (F_PREC - 1));
					int yy = (y + y0 - yc) * cos_a - (x0 - xc) * sin_a + ((size.y + 1 + dy) << (F_PREC - 1));

					for(int x = 0; x <= sx; x++){
						int xb = (xx >> F_PREC);
						int yb = (yy >> F_PREC);

						if(xb >= 0 && xb < size.x && yb >= 0 && yb < size.y){
							if(mode & GR_FLIP_HORIZONTAL)
								xb = size.x - xb - 1;
							if(mode & GR_FLIP_VERTICAL)
								yb = size.y - yb - 1;

							const unsigned char* data_ptr = data + size.x * 3 * yb + xb * 3;
							if(data_ptr[0] || data_ptr[1] || data_ptr[2])
								*screen_ptr = make_rgb565u(data_ptr[2], data_ptr[1], data_ptr[0]);
						}

						xx += cos_a;
						yy -= sin_a;

						screen_ptr++;
					}
				}
			}
			else if(pixel_format_ == GR_ARGB1555){
				for(int y = 0; y <= sy; y++){
					unsigned short* screen_ptr = (unsigned short*)(screenBuf + yTable[y + y0] + x0 * 2);

					int xx = (x0 - xc) * cos_a + (y + y0 - yc) * sin_a + ((size.x + 1 + dx) << (F_PREC - 1));
					int yy = (y + y0 - yc) * cos_a - (x0 - xc) * sin_a + ((size.y + 1 + dy) << (F_PREC - 1));

					for(int x = 0; x <= sx; x++){
						int xb = (xx >> F_PREC);
						int yb = (yy >> F_PREC);

						if(xb >= 0 && xb < size.x && yb >= 0 && yb < size.y){
							if(mode & GR_FLIP_HORIZONTAL)
								xb = size.x - xb - 1;
							if(mode & GR_FLIP_VERTICAL)
								yb = size.y - yb - 1;

							const unsigned char* data_ptr = data + size.x * 3 * yb + xb * 3;
							if(data_ptr[0] || data_ptr[1] || data_ptr[2])
								*screen_ptr = make_rgb555u(data_ptr[2], data_ptr[1], data_ptr[0]);
						}

						xx += cos_a;
						yy -= sin_a;

						screen_ptr++;
					}
				}
			}
		}
	}
}

void grDispatcher::PutSpr_rot(const Vect2i& pos, const Vect2i& size, const unsigned char* data, bool has_alpha, int mode, float angle, const Vect2f& scale)
{
	const int F_PREC = 16;

	int xc = pos.x + round(float(size.x) * scale.x / 2.f);
	int yc = pos.y + round(float(size.y) * scale.y / 2.f);

	float sn = sinf(angle);
	float cs = cosf(angle);

	int sx = round(fabs(cs) * float(size.x) * scale.x + fabs(sn) * float(size.y) * scale.y) + 2;
	int sy = round(fabs(sn) * float(size.x) * scale.x + fabs(cs) * float(size.y) * scale.y) + 2;

	int x0 = xc - sx/2;
	int y0 = yc - sy/2;

	if(!clip_rectangle(x0, y0, sx, sy))
		return;

	int sin_a = round(sinf(angle) * float(1 << F_PREC));
	int cos_a = round(cosf(angle) * float(1 << F_PREC));

	Vect2i iscale = Vect2i(round(scale.x * float(1 << F_PREC)), round(scale.y * float(1 << F_PREC)));
	Vect2i scaled_size = Vect2i(iscale.x * size.x, iscale.y * size.y);

	if(has_alpha){
		if(bytes_per_pixel() == 4 || bytes_per_pixel() == 3){
			for(int y = 0; y <= sy; y++){
				unsigned char* screen_ptr = (unsigned char*)(screenBuf + yTable[y + y0] + x0 * bytes_per_pixel());

				int xx = (x0 - xc) * cos_a + (y + y0 - yc) * sin_a + scaled_size.x/2 + (1 << (F_PREC - 1));
				int yy = (y + y0 - yc) * cos_a - (x0 - xc) * sin_a + scaled_size.y/2 + (1 << (F_PREC - 1));

				for(int x = 0; x <= sx; x++){
					int xb = xx / iscale.x;
					int yb = yy / iscale.y;

					if(xb >= 0 && xb < size.x && yb >= 0 && yb < size.y){
						if(mode & GR_FLIP_HORIZONTAL)
							xb = size.x - xb - 1;
						if(mode & GR_FLIP_VERTICAL)
							yb = size.y - yb - 1;

						const unsigned char* data_ptr = data + size.x * 4 * yb + xb * 4;

						unsigned a = data_ptr[3];
						if(a != 255){
							if(a){
								screen_ptr[0] = data_ptr[0] + ((a * screen_ptr[0]) >> 8);
								screen_ptr[1] = data_ptr[1] + ((a * screen_ptr[1]) >> 8);
								screen_ptr[2] = data_ptr[2] + ((a * screen_ptr[2]) >> 8);
							}
							else {
								screen_ptr[0] = data_ptr[0];
								screen_ptr[1] = data_ptr[1];
								screen_ptr[2] = data_ptr[2];
							}
						}
					}

					xx += cos_a;
					yy -= sin_a;

					screen_ptr += bytes_per_pixel();
				}
			}
		}
		else if(bytes_per_pixel() == 2){
			if(pixel_format_ == GR_RGB565){
				for(int y = 0; y <= sy; y++){
					unsigned short* screen_ptr = (unsigned short*)(screenBuf + yTable[y + y0] + x0 * 2);

					int xx = (x0 - xc) * cos_a + (y + y0 - yc) * sin_a + scaled_size.x/2 + (1 << (F_PREC - 1));
					int yy = (y + y0 - yc) * cos_a - (x0 - xc) * sin_a + scaled_size.y/2 + (1 << (F_PREC - 1));

					for(int x = 0; x <= sx; x++){
						int xb = xx / iscale.x;
						int yb = yy / iscale.y;

						if(xb >= 0 && xb < size.x && yb >= 0 && yb < size.y){
							if(mode & GR_FLIP_HORIZONTAL)
								xb = size.x - xb - 1;
							if(mode & GR_FLIP_VERTICAL)
								yb = size.y - yb - 1;

							const unsigned char* data_ptr = data + size.x * 4 * yb + xb * 4;

							unsigned a = data_ptr[3];
							if(a != 255){
								if(a)
									*screen_ptr = alpha_blend_565(make_rgb565u(data_ptr[2], data_ptr[1], data_ptr[0]),*screen_ptr,a);
								else 
									*screen_ptr = make_rgb565u(data_ptr[2], data_ptr[1], data_ptr[0]);
							}
						}

						xx += cos_a;
						yy -= sin_a;

						screen_ptr++;
					}
				}
			}
			else if(pixel_format_ == GR_ARGB1555){
				for(int y = 0; y <= sy; y++){
					unsigned short* screen_ptr = (unsigned short*)(screenBuf + yTable[y + y0] + x0 * 2);

					int xx = (x0 - xc) * cos_a + (y + y0 - yc) * sin_a + scaled_size.x/2 + (1 << (F_PREC - 1));
					int yy = (y + y0 - yc) * cos_a - (x0 - xc) * sin_a + scaled_size.y/2 + (1 << (F_PREC - 1));

					for(int x = 0; x <= sx; x++){
						int xb = xx / iscale.x;
						int yb = yy / iscale.y;

						if(xb >= 0 && xb < size.x && yb >= 0 && yb < size.y){
							if(mode & GR_FLIP_HORIZONTAL)
								xb = size.x - xb - 1;
							if(mode & GR_FLIP_VERTICAL)
								yb = size.y - yb - 1;

							const unsigned char* data_ptr = data + size.x * 4 * yb + xb * 4;

							unsigned a = data_ptr[3];
							if(a != 255){
								if(a)
									*screen_ptr = alpha_blend_555(make_rgb555u(data_ptr[2], data_ptr[1], data_ptr[0]),*screen_ptr,a);
								else 
									*screen_ptr = make_rgb555u(data_ptr[2], data_ptr[1], data_ptr[0]);
							}
						}

						xx += cos_a;
						yy -= sin_a;

						screen_ptr++;
					}
				}
			}
		}
	}
	else {
		if(bytes_per_pixel() == 4 || bytes_per_pixel() == 3){
			for(int y = 0; y <= sy; y++){
				unsigned char* screen_ptr = (unsigned char*)(screenBuf + yTable[y + y0] + x0 * bytes_per_pixel());

				int xx = (x0 - xc) * cos_a + (y + y0 - yc) * sin_a + scaled_size.x/2 + (1 << (F_PREC - 1));
				int yy = (y + y0 - yc) * cos_a - (x0 - xc) * sin_a + scaled_size.y/2 + (1 << (F_PREC - 1));

				for(int x = 0; x <= sx; x++){
					int xb = xx / iscale.x;
					int yb = yy / iscale.y;

					if(xb >= 0 && xb < size.x && yb >= 0 && yb < size.y){
						if(mode & GR_FLIP_HORIZONTAL)
							xb = size.x - xb - 1;
						if(mode & GR_FLIP_VERTICAL)
							yb = size.y - yb - 1;

						const unsigned char* data_ptr = data + size.x * 3 * yb + xb * 3;
						screen_ptr[0] = data_ptr[0];
						screen_ptr[1] = data_ptr[1];
						screen_ptr[2] = data_ptr[2];
					}

					xx += cos_a;
					yy -= sin_a;

					screen_ptr += bytes_per_pixel();
				}
			}
		}
		else if(bytes_per_pixel() == 2){
			if(pixel_format_ == GR_RGB565){
				for(int y = 0; y <= sy; y++){
					unsigned short* screen_ptr = (unsigned short*)(screenBuf + yTable[y + y0] + x0 * 2);

					int xx = (x0 - xc) * cos_a + (y + y0 - yc) * sin_a + scaled_size.x/2 + (1 << (F_PREC - 1));
					int yy = (y + y0 - yc) * cos_a - (x0 - xc) * sin_a + scaled_size.y/2 + (1 << (F_PREC - 1));

					for(int x = 0; x <= sx; x++){
						int xb = xx / iscale.x;
						int yb = yy / iscale.y;

						if(xb >= 0 && xb < size.x && yb >= 0 && yb < size.y){
							if(mode & GR_FLIP_HORIZONTAL)
								xb = size.x - xb - 1;
							if(mode & GR_FLIP_VERTICAL)
								yb = size.y - yb - 1;

							const unsigned char* data_ptr = data + size.x * 3 * yb + xb * 3;
							*screen_ptr = make_rgb565u(data_ptr[2], data_ptr[1], data_ptr[0]);
						}

						xx += cos_a;
						yy -= sin_a;

						screen_ptr++;
					}
				}
			}
			else if(pixel_format_ == GR_ARGB1555){
				for(int y = 0; y <= sy; y++){
					unsigned short* screen_ptr = (unsigned short*)(screenBuf + yTable[y + y0] + x0 * 2);

					int xx = (x0 - xc) * cos_a + (y + y0 - yc) * sin_a + scaled_size.x/2 + (1 << (F_PREC - 1));
					int yy = (y + y0 - yc) * cos_a - (x0 - xc) * sin_a + scaled_size.y/2 + (1 << (F_PREC - 1));

					for(int x = 0; x <= sx; x++){
						int xb = xx / iscale.x;
						int yb = yy / iscale.y;

						if(xb >= 0 && xb < size.x && yb >= 0 && yb < size.y){
							if(mode & GR_FLIP_HORIZONTAL)
								xb = size.x - xb - 1;
							if(mode & GR_FLIP_VERTICAL)
								yb = size.y - yb - 1;

							const unsigned char* data_ptr = data + size.x * 3 * yb + xb * 3;
							*screen_ptr = make_rgb555u(data_ptr[2], data_ptr[1], data_ptr[0]);
						}

						xx += cos_a;
						yy -= sin_a;

						screen_ptr++;
					}
				}
			}
		}
	}
}

void grDispatcher::PutSprMask_rot(const Vect2i& pos, const Vect2i& size, const unsigned char* data, bool has_alpha, unsigned mask_color, int mask_alpha, int mode, float angle)
{
	const int F_PREC = 16;

	int xc = pos.x + size.x/2;
	int yc = pos.y + size.y/2;

	float sn = sinf(angle);
	float cs = cosf(angle);

	int sx = round(fabs(cs) * float(size.x) + fabs(sn) * float(size.y)) + 2;
	int sy = round(fabs(sn) * float(size.x) + fabs(cs) * float(size.y)) + 2;

	int x0 = xc - sx/2;
	int y0 = yc - sy/2;

	if(!clip_rectangle(x0, y0, sx, sy))
		return;

	int sin_a = round(sn * float(1 << F_PREC));
	int cos_a = round(cs * float(1 << F_PREC));

	if(has_alpha){
		if(bytes_per_pixel() == 4 || bytes_per_pixel() == 3){
			unsigned mr,mg,mb;
			split_rgb888(mask_color,mr,mg,mb);

			for(int y = 0; y <= sy; y++){
				unsigned char* screen_ptr = (unsigned char*)(screenBuf + yTable[y + y0] + x0 * bytes_per_pixel());

				int xx = (x0 - xc) * cos_a + (y + y0 - yc) * sin_a + (size.x + 1)*(1 << (F_PREC - 1));
				int yy = (y + y0 - yc) * cos_a - (x0 - xc) * sin_a + (size.y + 1)*(1 << (F_PREC - 1));

				for(int x = 0; x <= sx; x++){
					int xb = (xx >> F_PREC);
					int yb = (yy >> F_PREC);

					if(xb >= 0 && xb < size.x && yb >= 0 && yb < size.y){
						if(mode & GR_FLIP_HORIZONTAL)
							xb = size.x - xb - 1;
						if(mode & GR_FLIP_VERTICAL)
							yb = size.y - yb - 1;

						const unsigned char* data_ptr = data + size.x * 4 * yb + xb * 4;

						unsigned a = data_ptr[3];
						if(a != 255){
							a = mask_alpha + ((a * (255 - mask_alpha)) >> 8);

							screen_ptr[0] = ((mb * (255 - a)) >> 8) + ((a * screen_ptr[0]) >> 8);
							screen_ptr[1] = ((mg * (255 - a)) >> 8) + ((a * screen_ptr[1]) >> 8);
							screen_ptr[2] = ((mr * (255 - a)) >> 8) + ((a * screen_ptr[2]) >> 8);
						}
					}

					xx += cos_a;
					yy -= sin_a;

					screen_ptr += bytes_per_pixel();
				}
			}
		}
		else if(bytes_per_pixel() == 2){
			if(pixel_format_ == GR_RGB565){
				unsigned mr,mg,mb;
				split_rgb565u(mask_color,mr,mg,mb);

				for(int y = 0; y <= sy; y++){
					unsigned short* screen_ptr = (unsigned short*)(screenBuf + yTable[y + y0] + x0 * 2);

					int xx = (x0 - xc) * cos_a + (y + y0 - yc) * sin_a + (size.x + 1)*(1 << (F_PREC - 1));
					int yy = (y + y0 - yc) * cos_a - (x0 - xc) * sin_a + (size.y + 1)*(1 << (F_PREC - 1));

					for(int x = 0; x <= sx; x++){
						int xb = (xx >> F_PREC);
						int yb = (yy >> F_PREC);

						if(xb >= 0 && xb < size.x && yb >= 0 && yb < size.y){
							if(mode & GR_FLIP_HORIZONTAL)
								xb = size.x - xb - 1;
							if(mode & GR_FLIP_VERTICAL)
								yb = size.y - yb - 1;

							const unsigned char* data_ptr = data + size.x * 4 * yb + xb * 4;

							unsigned a = data_ptr[3];
							if(a != 255){
								a = mask_alpha + ((a * (255 - mask_alpha)) >> 8);

								unsigned r = (mr * (255 - a)) >> 8;
								unsigned g = (mg * (255 - a)) >> 8;
								unsigned b = (mb * (255 - a)) >> 8;

								unsigned cl = make_rgb565u(r,g,b);

								*screen_ptr = alpha_blend_565(cl,*screen_ptr,a);
							}
						}

						xx += cos_a;
						yy -= sin_a;

						screen_ptr++;
					}
				}
			}
			else if(pixel_format_ == GR_ARGB1555){
				unsigned mr,mg,mb;
				split_rgb555u(mask_color,mr,mg,mb);

				for(int y = 0; y <= sy; y++){
					unsigned short* screen_ptr = (unsigned short*)(screenBuf + yTable[y + y0] + x0 * 2);

					int xx = (x0 - xc) * cos_a + (y + y0 - yc) * sin_a + (size.x + 1)*(1 << (F_PREC - 1));
					int yy = (y + y0 - yc) * cos_a - (x0 - xc) * sin_a + (size.y + 1)*(1 << (F_PREC - 1));

					for(int x = 0; x <= sx; x++){
						int xb = (xx >> F_PREC);
						int yb = (yy >> F_PREC);

						if(xb >= 0 && xb < size.x && yb >= 0 && yb < size.y){
							if(mode & GR_FLIP_HORIZONTAL)
								xb = size.x - xb - 1;
							if(mode & GR_FLIP_VERTICAL)
								yb = size.y - yb - 1;

							const unsigned char* data_ptr = data + size.x * 4 * yb + xb * 4;

							unsigned a = data_ptr[3];
							if(a != 255){
								a = mask_alpha + ((a * (255 - mask_alpha)) >> 8);

								unsigned r = (mr * (255 - a)) >> 8;
								unsigned g = (mg * (255 - a)) >> 8;
								unsigned b = (mb * (255 - a)) >> 8;

								unsigned cl = make_rgb555u(r,g,b);

								*screen_ptr = alpha_blend_555(cl,*screen_ptr,a);
							}
						}

						xx += cos_a;
						yy -= sin_a;

						screen_ptr++;
					}
				}
			}
		}
	}
	else {
		if(bytes_per_pixel() == 4 || bytes_per_pixel() == 3){
			unsigned mr,mg,mb;
			split_rgb888(mask_color,mr,mg,mb);

			mr = (mr * (255 - mask_alpha)) >> 8;
			mg = (mg * (255 - mask_alpha)) >> 8;
			mb = (mb * (255 - mask_alpha)) >> 8;

			for(int y = 0; y <= sy; y++){
				unsigned char* screen_ptr = (unsigned char*)(screenBuf + yTable[y + y0] + x0 * bytes_per_pixel());

				int xx = (x0 - xc) * cos_a + (y + y0 - yc) * sin_a + (size.x + 1)*(1 << (F_PREC - 1));
				int yy = (y + y0 - yc) * cos_a - (x0 - xc) * sin_a + (size.y + 1)*(1 << (F_PREC - 1));

				for(int x = 0; x <= sx; x++){
					int xb = (xx >> F_PREC);
					int yb = (yy >> F_PREC);

					if(xb >= 0 && xb < size.x && yb >= 0 && yb < size.y){
						if(mode & GR_FLIP_HORIZONTAL)
							xb = size.x - xb - 1;
						if(mode & GR_FLIP_VERTICAL)
							yb = size.y - yb - 1;

						const unsigned char* data_ptr = data + size.x * 3 * yb + xb * 3;
						if(data_ptr[0] || data_ptr[1] || data_ptr[2]){
							screen_ptr[2] = mr + ((mask_alpha * screen_ptr[2]) >> 8);
							screen_ptr[1] = mg + ((mask_alpha * screen_ptr[1]) >> 8);
							screen_ptr[0] = mb + ((mask_alpha * screen_ptr[0]) >> 8);
						}
					}

					xx += cos_a;
					yy -= sin_a;

					screen_ptr += bytes_per_pixel();
				}
			}
		}
		else if(bytes_per_pixel() == 2){
			if(pixel_format_ == GR_RGB565){
				unsigned mr,mg,mb;
				split_rgb565u(mask_color,mr,mg,mb);

				mr = (mr * (255 - mask_alpha)) >> 8;
				mg = (mg * (255 - mask_alpha)) >> 8;
				mb = (mb * (255 - mask_alpha)) >> 8;

				unsigned mcl = make_rgb565u(mr,mg,mb);

				for(int y = 0; y <= sy; y++){
					unsigned short* screen_ptr = (unsigned short*)(screenBuf + yTable[y + y0] + x0 * 2);

					int xx = (x0 - xc) * cos_a + (y + y0 - yc) * sin_a + (size.x + 1)*(1 << (F_PREC - 1));
					int yy = (y + y0 - yc) * cos_a - (x0 - xc) * sin_a + (size.y + 1)*(1 << (F_PREC - 1));

					for(int x = 0; x <= sx; x++){
						int xb = (xx >> F_PREC);
						int yb = (yy >> F_PREC);

						if(xb >= 0 && xb < size.x && yb >= 0 && yb < size.y){
							if(mode & GR_FLIP_HORIZONTAL)
								xb = size.x - xb - 1;
							if(mode & GR_FLIP_VERTICAL)
								yb = size.y - yb - 1;

							const unsigned char* data_ptr = data + size.x * 3 * yb + xb * 3;
							if(data_ptr[0] || data_ptr[1] || data_ptr[2]){
								*screen_ptr = alpha_blend_565(mcl, *screen_ptr, mask_alpha);
							}
						}

						xx += cos_a;
						yy -= sin_a;

						screen_ptr++;
					}
				}
			}
			else if(pixel_format_ == GR_ARGB1555){
				unsigned mr,mg,mb;
				split_rgb555u(mask_color,mr,mg,mb);

				mr = (mr * (255 - mask_alpha)) >> 8;
				mg = (mg * (255 - mask_alpha)) >> 8;
				mb = (mb * (255 - mask_alpha)) >> 8;

				unsigned mcl = make_rgb555u(mr,mg,mb);

				for(int y = 0; y <= sy; y++){
					unsigned short* screen_ptr = (unsigned short*)(screenBuf + yTable[y + y0] + x0 * 2);

					int xx = (x0 - xc) * cos_a + (y + y0 - yc) * sin_a + (size.x + 1)*(1 << (F_PREC - 1));
					int yy = (y + y0 - yc) * cos_a - (x0 - xc) * sin_a + (size.y + 1)*(1 << (F_PREC - 1));

					for(int x = 0; x <= sx; x++){
						int xb = (xx >> F_PREC);
						int yb = (yy >> F_PREC);

						if(xb >= 0 && xb < size.x && yb >= 0 && yb < size.y){
							if(mode & GR_FLIP_HORIZONTAL)
								xb = size.x - xb - 1;
							if(mode & GR_FLIP_VERTICAL)
								yb = size.y - yb - 1;

							const unsigned char* data_ptr = data + size.x * 3 * yb + xb * 3;
							if(data_ptr[0] || data_ptr[1] || data_ptr[2]){
								*screen_ptr = alpha_blend_565(mcl, *screen_ptr, mask_alpha);
							}
						}

						xx += cos_a;
						yy -= sin_a;

						screen_ptr++;
					}
				}
			}
		}
	}
}

void grDispatcher::PutSprMask_rot(const Vect2i& pos, const Vect2i& size, const unsigned char* data, bool has_alpha, unsigned mask_color, int mask_alpha, int mode, float angle, const Vect2f& scale)
{
	const int F_PREC = 16;

	int xc = pos.x + round(float(size.x) * scale.x / 2.f);
	int yc = pos.y + round(float(size.y) * scale.y / 2.f);

	float sn = sinf(angle);
	float cs = cosf(angle);

	int sx = round(fabs(cs) * float(size.x) * scale.x + fabs(sn) * float(size.y) * scale.y) + 2;
	int sy = round(fabs(sn) * float(size.x) * scale.x + fabs(cs) * float(size.y) * scale.y) + 2;

	int x0 = xc - sx/2;
	int y0 = yc - sy/2;

	if(!clip_rectangle(x0, y0, sx, sy))
		return;

	int sin_a = round(sinf(angle) * float(1 << F_PREC));
	int cos_a = round(cosf(angle) * float(1 << F_PREC));

	Vect2i iscale = Vect2i(round(scale.x * float(1 << F_PREC)), round(scale.y * float(1 << F_PREC)));
	Vect2i scaled_size = Vect2i(iscale.x * size.x, iscale.y * size.y);

	if(has_alpha){
		if(bytes_per_pixel() == 4 || bytes_per_pixel() == 3){
			unsigned mr,mg,mb;
			split_rgb888(mask_color,mr,mg,mb);

			for(int y = 0; y <= sy; y++){
				unsigned char* screen_ptr = (unsigned char*)(screenBuf + yTable[y + y0] + x0 * bytes_per_pixel());

				int xx = (x0 - xc) * cos_a + (y + y0 - yc) * sin_a + scaled_size.x/2 + (1 << (F_PREC - 1));
				int yy = (y + y0 - yc) * cos_a - (x0 - xc) * sin_a + scaled_size.y/2 + (1 << (F_PREC - 1));

				for(int x = 0; x <= sx; x++){
					int xb = xx / iscale.x;
					int yb = yy / iscale.y;

					if(xb >= 0 && xb < size.x && yb >= 0 && yb < size.y){
						if(mode & GR_FLIP_HORIZONTAL)
							xb = size.x - xb - 1;
						if(mode & GR_FLIP_VERTICAL)
							yb = size.y - yb - 1;

						const unsigned char* data_ptr = data + size.x * 4 * yb + xb * 4;

						unsigned a = data_ptr[3];
						if(a != 255){
							a = mask_alpha + ((a * (255 - mask_alpha)) >> 8);

							screen_ptr[0] = ((mb * (255 - a)) >> 8) + ((a * screen_ptr[0]) >> 8);
							screen_ptr[1] = ((mg * (255 - a)) >> 8) + ((a * screen_ptr[1]) >> 8);
							screen_ptr[2] = ((mr * (255 - a)) >> 8) + ((a * screen_ptr[2]) >> 8);
						}
					}

					xx += cos_a;
					yy -= sin_a;

					screen_ptr += bytes_per_pixel();
				}
			}
		}
		else if(bytes_per_pixel() == 2){
			if(pixel_format_ == GR_RGB565){
				unsigned mr,mg,mb;
				split_rgb565u(mask_color,mr,mg,mb);

				for(int y = 0; y <= sy; y++){
					unsigned short* screen_ptr = (unsigned short*)(screenBuf + yTable[y + y0] + x0 * 2);

					int xx = (x0 - xc) * cos_a + (y + y0 - yc) * sin_a + scaled_size.x/2 + (1 << (F_PREC - 1));
					int yy = (y + y0 - yc) * cos_a - (x0 - xc) * sin_a + scaled_size.y/2 + (1 << (F_PREC - 1));

					for(int x = 0; x <= sx; x++){
						int xb = xx / iscale.x;
						int yb = yy / iscale.y;

						if(xb >= 0 && xb < size.x && yb >= 0 && yb < size.y){
							if(mode & GR_FLIP_HORIZONTAL)
								xb = size.x - xb - 1;
							if(mode & GR_FLIP_VERTICAL)
								yb = size.y - yb - 1;

							const unsigned char* data_ptr = data + size.x * 4 * yb + xb * 4;

							unsigned a = data_ptr[3];
							if(a != 255){
								a = mask_alpha + ((a * (255 - mask_alpha)) >> 8);

								unsigned r = (mr * (255 - a)) >> 8;
								unsigned g = (mg * (255 - a)) >> 8;
								unsigned b = (mb * (255 - a)) >> 8;

								unsigned cl = make_rgb565u(r,g,b);

								*screen_ptr = alpha_blend_565(cl,*screen_ptr,a);
							}
						}

						xx += cos_a;
						yy -= sin_a;

						screen_ptr++;
					}
				}
			}
			else if(pixel_format_ == GR_ARGB1555){
				unsigned mr,mg,mb;
				split_rgb555u(mask_color,mr,mg,mb);

				for(int y = 0; y <= sy; y++){
					unsigned short* screen_ptr = (unsigned short*)(screenBuf + yTable[y + y0] + x0 * 2);

					int xx = (x0 - xc) * cos_a + (y + y0 - yc) * sin_a + scaled_size.x/2 + (1 << (F_PREC - 1));
					int yy = (y + y0 - yc) * cos_a - (x0 - xc) * sin_a + scaled_size.y/2 + (1 << (F_PREC - 1));

					for(int x = 0; x <= sx; x++){
						int xb = xx / iscale.x;
						int yb = yy / iscale.y;

						if(xb >= 0 && xb < size.x && yb >= 0 && yb < size.y){
							if(mode & GR_FLIP_HORIZONTAL)
								xb = size.x - xb - 1;
							if(mode & GR_FLIP_VERTICAL)
								yb = size.y - yb - 1;

							const unsigned char* data_ptr = data + size.x * 4 * yb + xb * 4;

							unsigned a = data_ptr[3];
							if(a != 255){
								a = mask_alpha + ((a * (255 - mask_alpha)) >> 8);

								unsigned r = (mr * (255 - a)) >> 8;
								unsigned g = (mg * (255 - a)) >> 8;
								unsigned b = (mb * (255 - a)) >> 8;

								unsigned cl = make_rgb555u(r,g,b);

								*screen_ptr = alpha_blend_555(cl,*screen_ptr,a);
							}
						}

						xx += cos_a;
						yy -= sin_a;

						screen_ptr++;
					}
				}
			}
		}
	}
	else {
		if(bytes_per_pixel() == 4 || bytes_per_pixel() == 3){
			unsigned mr,mg,mb;
			split_rgb888(mask_color,mr,mg,mb);

			mr = (mr * (255 - mask_alpha)) >> 8;
			mg = (mg * (255 - mask_alpha)) >> 8;
			mb = (mb * (255 - mask_alpha)) >> 8;

			for(int y = 0; y <= sy; y++){
				unsigned char* screen_ptr = (unsigned char*)(screenBuf + yTable[y + y0] + x0 * bytes_per_pixel());

				int xx = (x0 - xc) * cos_a + (y + y0 - yc) * sin_a + scaled_size.x/2 + (1 << (F_PREC - 1));
				int yy = (y + y0 - yc) * cos_a - (x0 - xc) * sin_a + scaled_size.y/2 + (1 << (F_PREC - 1));

				for(int x = 0; x <= sx; x++){
					int xb = xx / iscale.x;
					int yb = yy / iscale.y;

					if(xb >= 0 && xb < size.x && yb >= 0 && yb < size.y){
						if(mode & GR_FLIP_HORIZONTAL)
							xb = size.x - xb - 1;
						if(mode & GR_FLIP_VERTICAL)
							yb = size.y - yb - 1;

						const unsigned char* data_ptr = data + size.x * 3 * yb + xb * 3;
						screen_ptr[0] = data_ptr[0];
						screen_ptr[1] = data_ptr[1];
						screen_ptr[2] = data_ptr[2];
					}

					xx += cos_a;
					yy -= sin_a;

					screen_ptr += bytes_per_pixel();
				}
			}
		}
		else if(bytes_per_pixel() == 2){
			if(pixel_format_ == GR_RGB565){
				unsigned mr,mg,mb;
				split_rgb565u(mask_color,mr,mg,mb);

				mr = (mr * (255 - mask_alpha)) >> 8;
				mg = (mg * (255 - mask_alpha)) >> 8;
				mb = (mb * (255 - mask_alpha)) >> 8;

				for(int y = 0; y <= sy; y++){
					unsigned short* screen_ptr = (unsigned short*)(screenBuf + yTable[y + y0] + x0 * 2);

					int xx = (x0 - xc) * cos_a + (y + y0 - yc) * sin_a + scaled_size.x/2 + (1 << (F_PREC - 1));
					int yy = (y + y0 - yc) * cos_a - (x0 - xc) * sin_a + scaled_size.y/2 + (1 << (F_PREC - 1));

					for(int x = 0; x <= sx; x++){
						int xb = xx / iscale.x;
						int yb = yy / iscale.y;

						if(xb >= 0 && xb < size.x && yb >= 0 && yb < size.y){
							if(mode & GR_FLIP_HORIZONTAL)
								xb = size.x - xb - 1;
							if(mode & GR_FLIP_VERTICAL)
								yb = size.y - yb - 1;

							const unsigned char* data_ptr = data + size.x * 3 * yb + xb * 3;
							*screen_ptr = make_rgb565u(data_ptr[2], data_ptr[1], data_ptr[0]);
						}

						xx += cos_a;
						yy -= sin_a;

						screen_ptr++;
					}
				}
			}
			else if(pixel_format_ == GR_ARGB1555){
				unsigned mr,mg,mb;
				split_rgb555u(mask_color,mr,mg,mb);

				mr = (mr * (255 - mask_alpha)) >> 8;
				mg = (mg * (255 - mask_alpha)) >> 8;
				mb = (mb * (255 - mask_alpha)) >> 8;

				for(int y = 0; y <= sy; y++){
					unsigned short* screen_ptr = (unsigned short*)(screenBuf + yTable[y + y0] + x0 * 2);

					int xx = (x0 - xc) * cos_a + (y + y0 - yc) * sin_a + scaled_size.x/2 + (1 << (F_PREC - 1));
					int yy = (y + y0 - yc) * cos_a - (x0 - xc) * sin_a + scaled_size.y/2 + (1 << (F_PREC - 1));

					for(int x = 0; x <= sx; x++){
						int xb = xx / iscale.x;
						int yb = yy / iscale.y;

						if(xb >= 0 && xb < size.x && yb >= 0 && yb < size.y){
							if(mode & GR_FLIP_HORIZONTAL)
								xb = size.x - xb - 1;
							if(mode & GR_FLIP_VERTICAL)
								yb = size.y - yb - 1;

							const unsigned char* data_ptr = data + size.x * 3 * yb + xb * 3;
							*screen_ptr = make_rgb555u(data_ptr[2], data_ptr[1], data_ptr[0]);
						}

						xx += cos_a;
						yy -= sin_a;

						screen_ptr++;
					}
				}
			}
		}
	}
}

void grDispatcher::PutSpr(int x,int y,int sx,int sy,const unsigned char* p,int mode)
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
			x += psx * 4 - 4;
			px = sx - px - psx;
		}
		else 
			dx = 4;

		if(mode & GR_FLIP_VERTICAL){
			y += psy - 1;
			py = sy - py - psy;
		}
		else 
			dy = 1;

		int px3 = px * 3;
		int sx3 = sx * 3;

		const unsigned char* data_ptr = p + py * sx3;

		for(int i = 0; i < psy; i ++){
			unsigned char* scr_buf = reinterpret_cast<unsigned char*>(screenBuf + yTable[y] + x);
			const unsigned char* data_line = data_ptr + px3;

			for(int j = 0; j < psx; j ++){
				if(data_line[0] || data_line[1] || data_line[2]){
					scr_buf[0] = data_line[0];
					scr_buf[1] = data_line[1];
					scr_buf[2] = data_line[2];
				}
				scr_buf += dx;
				data_line += 3;
			}

			data_ptr += sx3;
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

		if(mode & GR_FLIP_VERTICAL){
			y += psy - 1;
			py = sy - py - psy;
		}
		else 
			dy = 1;

		int px3 = px * 3;
		int sx3 = sx * 3;

		const unsigned char* data_ptr = p + py * sx3;

		for(int i = 0; i < psy; i ++){
			unsigned char* scr_buf = reinterpret_cast<unsigned char*>(screenBuf + yTable[y] + x);
			const unsigned char* data_line = data_ptr + px3;

			for(int j = 0; j < psx; j ++){
				if(data_line[0] || data_line[1] || data_line[2]){
					scr_buf[0] = data_line[0];
					scr_buf[1] = data_line[1];
					scr_buf[2] = data_line[2];
				}
				scr_buf += dx;
				data_line += 3;
			}

			data_ptr += sx3;
			y += dy;
		}
	}
	else if(bytes_per_pixel() == 2){
		int dx = -1;
		int dy = -1;

		if(mode & GR_FLIP_HORIZONTAL){
			x += psx - 1;
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

		x <<= 1;

		sx *= 3;
		px *= 3;

		const unsigned char* data_ptr = p + py * sx;

		if(pixel_format_ == GR_RGB565){
			for(int i = 0; i < psy; i ++){
				unsigned short* scr_buf = reinterpret_cast<unsigned short*>(screenBuf + yTable[y] + x);
				const unsigned char* data_line = data_ptr + px;

				for(int j = 0; j < psx; j ++){
					if(*data_line)
						*scr_buf = make_rgb565u(data_line[2], data_line[1], data_line[0]);
					scr_buf += dx;
					data_line += 3;
				}

				data_ptr += sx;
				y += dy;
			}
		}
		else {
			for(int i = 0; i < psy; i ++){
				unsigned short* scr_buf = reinterpret_cast<unsigned short*>(screenBuf + yTable[y] + x);
				const unsigned char* data_line = data_ptr + px;

				for(int j = 0; j < psx; j ++){
					if(*data_line)
						*scr_buf = make_rgb555u(data_line[2], data_line[1], data_line[0]);
					scr_buf += dx;
					data_line += 3;
				}

				data_ptr += sx;
				y += dy;
			}
		}
	}
}

void grDispatcher::DrawSprContour_a(int x,int y,int sx,int sy,const unsigned char* p,int contour_color,int mode)
{
	int px = 0;
	int py = 0;

	int psx = sx;
	int psy = sy;

	if(!clip_rectangle(x,y,px,py,psx,psy)) return;

	if(bytes_per_pixel() == 3 || bytes_per_pixel() == 4){
		sx *= 4;
		px *= 4;

		int dpx,dpy;
		if(mode & GR_FLIP_HORIZONTAL){
			px = sx - px - 4;
			dpx = -4;
		}
		else
			dpx = 4;

		if(mode & GR_FLIP_VERTICAL){
			py = sy - py - 1;
			dpy = -sx;
		}
		else
			dpy = sx;

		const unsigned char* pic_buf = p + py * sx;
		for(int i = 0; i < psy; i ++){
			int jj = px;
			int empty_pixel = 1;
			for(int j = 0; j < psx; j ++){
				if(pic_buf[jj + 3] < 200){
					if(empty_pixel)
						SetPixelFast(x + j,y + i,contour_color);
					empty_pixel = 0;
				}
				else {
					if(!empty_pixel)
						SetPixelFast(x + j - 1,y + i,contour_color);
					empty_pixel = 1;
				}

				jj += dpx;
			}
			if(!empty_pixel)
				SetPixelFast(x + psx - 1,y + i,contour_color);
			pic_buf += dpy;
		}

		int jj = px;
		for(int j = 0; j < psx; j ++){
			int empty_pixel = 1;
			pic_buf = p + py * sx;
			for(int i = 0; i < psy; i ++){
				if(pic_buf[jj + 3] < 200){
					if(empty_pixel)
						SetPixelFast(x + j,y + i,contour_color);
					empty_pixel = 0;
				}
				else {
					if(!empty_pixel)
						SetPixelFast(x + j,y + i - 1,contour_color);
					empty_pixel = 1;
				}
				pic_buf += dpy;
			}
			if(!empty_pixel)
				SetPixelFast(x + j,y + psy - 1,contour_color);
			jj += dpx;
		}
		return;
	}
	if(bytes_per_pixel() == 2){
		int dpx,dpy;
		if(mode & GR_FLIP_HORIZONTAL){
			px = sx - px - 1;
			dpx = -1;
		}
		else
			dpx = 1;

		if(mode & GR_FLIP_VERTICAL){
			py = sy - py - 1;
			dpy = -sx;
		}
		else
			dpy = sx;

		sx <<= 1;
		dpy <<= 1;

		const unsigned short* pic_buf = reinterpret_cast<const unsigned short*>(p) + py * sx;
		for(int i = 0; i < psy; i ++){
			int jj = px;
			int empty_pixel = 1;
			for(int j = 0; j < psx; j ++){
				if(pic_buf[jj * 2 + 1] < 200){
					if(empty_pixel)
						SetPixelFast(x + j,y + i,contour_color);
					empty_pixel = 0;
				}
				else {
					if(!empty_pixel)
						SetPixelFast(x + j - 1,y + i,contour_color);
					empty_pixel = 1;
				}
				jj += dpx;
			}
			if(!empty_pixel)
				SetPixelFast(x + psx - 1,y + i,contour_color);
			pic_buf += dpy;
		}
		int jj = px;
		for(int j = 0; j < psx; j ++){
			int empty_pixel = 1;
			const unsigned short* pic_buf = reinterpret_cast<const unsigned short*>(p) + py * sx;
			for(int i = 0; i < psy; i ++){
				if(pic_buf[jj * 2 + 1] < 200){
					if(empty_pixel)
						SetPixelFast(x + j,y + i,contour_color);
					empty_pixel = 0;
				}
				else {
					if(!empty_pixel)
						SetPixelFast(x + j,y + i - 1,contour_color);
					empty_pixel = 1;
				}
				pic_buf += dpy;
			}
			if(!empty_pixel)
				SetPixelFast(x + j,y + psy - 1,contour_color);
			jj += dpx;
		}
		return;
	}
}

void grDispatcher::DrawSprContour(int x,int y,int sx,int sy,const unsigned char* p,int contour_color,int mode)
{
	int px = 0;
	int py = 0;

	int psx = sx;
	int psy = sy;

	if(!clip_rectangle(x,y,px,py,psx,psy)) return;

	if(bytes_per_pixel() == 3){
		sx *= 3;
		px *= 3;

		int dpx,dpy;
		if(mode & GR_FLIP_HORIZONTAL){
			px = sx - px - 3;
			dpx = -3;
		}
		else
			dpx = 3;

		if(mode & GR_FLIP_VERTICAL){
			py = sy - py - 1;
			dpy = -sx;
		}
		else
			dpy = sx;

		const unsigned char* pic_buf = p + py * sx;
		for(int i = 0; i < psy; i ++){
			int jj = px;
			int empty_pixel = 1;
			for(int j = 0; j < psx; j ++){
				if(pic_buf[jj + 2] || pic_buf[jj + 1] || pic_buf[jj + 0]){
					if(empty_pixel)
						SetPixelFast(x + j,y + i,contour_color);
					empty_pixel = 0;
				}
				else {
					if(!empty_pixel)
						SetPixelFast(x + j - 1,y + i,contour_color);
					empty_pixel = 1;
				}

				jj += dpx;
			}
			if(!empty_pixel)
				SetPixelFast(x + psx - 1,y + i,contour_color);
			pic_buf += dpy;
		}

		int jj = px;
		for(int j = 0; j < psx; j ++){
			int empty_pixel = 1;
			pic_buf = p + py * sx;
			for(int i = 0; i < psy; i ++){
				if(pic_buf[jj + 2] || pic_buf[jj + 1] || pic_buf[jj + 0]){
					if(empty_pixel)
						SetPixelFast(x + j,y + i,contour_color);
					empty_pixel = 0;
				}
				else {
					if(!empty_pixel)
						SetPixelFast(x + j,y + i - 1,contour_color);
					empty_pixel = 1;
				}
				pic_buf += dpy;
			}
			if(!empty_pixel)
				SetPixelFast(x + j,y + psy - 1,contour_color);
			jj += dpx;
		}
		return;
	}
	if(bytes_per_pixel() == 2){
		int dpx,dpy;
		if(mode & GR_FLIP_HORIZONTAL){
			px = sx - px - 1;
			dpx = -1;
		}
		else
			dpx = 1;

		if(mode & GR_FLIP_VERTICAL){
			py = sy - py - 1;
			dpy = -sx;
		}
		else
			dpy = sx;

		const unsigned short* pic_buf = reinterpret_cast<const unsigned short*>(p) + py * sx;
		for(int i = 0; i < psy; i ++){
			int jj = px;
			int empty_pixel = 1;
			for(int j = 0; j < psx; j ++){
				unsigned cl = pic_buf[jj];
				if(cl){
					if(empty_pixel)
						SetPixelFast(x + j,y + i,contour_color);
					empty_pixel = 0;
				}
				else {
					if(!empty_pixel)
						SetPixelFast(x + j - 1,y + i,contour_color);
					empty_pixel = 1;
				}
				jj += dpx;
			}
			if(!empty_pixel)
				SetPixelFast(x + psx - 1,y + i,contour_color);
			pic_buf += dpy;
		}
		int jj = px;
		for(int j = 0; j < psx; j ++){
			int empty_pixel = 1;
			const unsigned short* pic_buf = reinterpret_cast<const unsigned short*>(p) + py * sx;
			for(int i = 0; i < psy; i ++){
				unsigned cl = pic_buf[jj];
				if(cl){
					if(empty_pixel)
						SetPixelFast(x + j,y + i,contour_color);
					empty_pixel = 0;
				}
				else {
					if(!empty_pixel)
						SetPixelFast(x + j,y + i - 1,contour_color);
					empty_pixel = 1;
				}
				pic_buf += dpy;
			}
			if(!empty_pixel)
				SetPixelFast(x + j,y + psy - 1,contour_color);
			jj += dpx;
		}
		return;
	}
}

void grDispatcher::DrawSprContour(int x,int y,int sx,int sy,const unsigned char* p,int contour_color,int mode,float scale)
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
		const unsigned short* src = reinterpret_cast<const unsigned short*>(p);
		for(int i = y0; i != y1; i += iy){
			const unsigned short* line_src = src + ((fy >> 16) * sx);

			fy += dy;
			fx = (1 << 15);

			int empty_pixel = 1;

			for(int j = x0; j != x1; j += ix){
				if(line_src[fx >> 16]){
					if(empty_pixel)
						SetPixel(x + j,y + i,contour_color);
					empty_pixel = 0;
				}
				else {
					if(!empty_pixel)
						SetPixel(x + j - 1,y + i,contour_color);
					empty_pixel = 1;
				}
				fx += dx;
			}
			if(!empty_pixel)
				SetPixel(x + x1 - 1,y + i,contour_color);
		}
		fx = (1 << 15);
		for(int j = x0; j != x1; j += ix){
			fy = (1 << 15);
			int empty_pixel = 1;

			for(int i = y0; i != y1; i += iy){
				const unsigned short* line_src = src + ((fy >> 16) * sx);

				if(line_src[fx >> 16]){
					if(empty_pixel)
						SetPixel(x + j,y + i,contour_color);
					empty_pixel = 0;
				}
				else {
					if(!empty_pixel)
						SetPixel(x + j,y + i - 1,contour_color);
					empty_pixel = 1;
				}
				fy += dy;
			}
			if(!empty_pixel)
				SetPixel(x + j,y + y1 - 1,contour_color);

			fx += dx;
		}

		return;
	}

	if(bytes_per_pixel() == 3){
		int sx3 = sx * 3;

		for(int i = y0; i != y1; i += iy){
			const unsigned char* line_src = p + ((fy >> 16) * sx3);

			fy += dy;
			fx = (1 << 15);

			int empty_pixel = 1;

			for(int j = x0; j != x1; j += ix){
				int idx = (fx >> 16) * 3;
				if(line_src[idx + 2] || line_src[idx + 1] || line_src[idx + 0]){
					if(empty_pixel)
						SetPixel(x + j,y + i,contour_color);
					empty_pixel = 0;
				}
				else {
					if(!empty_pixel)
						SetPixel(x + j - 1,y + i,contour_color);
					empty_pixel = 1;
				}

				fx += dx;
			}
			if(!empty_pixel)
				SetPixel(x + x1 - 1,y + i,contour_color);
		}

		fx = (1 << 15);
		for(int j = x0; j != x1; j += ix){
			fy = (1 << 15);

			int empty_pixel = 1;
			int idx = (fx >> 16) * 3;

			for(int i = y0; i != y1; i += iy){
				const unsigned char* line_src = p + ((fy >> 16) * sx3);

				if(line_src[idx + 2] || line_src[idx + 1] || line_src[idx + 0]){
					if(empty_pixel)
						SetPixel(x + j,y + i,contour_color);
					empty_pixel = 0;
				}
				else {
					if(!empty_pixel)
						SetPixel(x + j,y + i - 1,contour_color);
					empty_pixel = 1;
				}

				fy += dy;
			}

			if(!empty_pixel)
				SetPixel(x + j,y + y1 - 1,contour_color);

			fx += dx;
		}
		return;
	}
}

void grDispatcher::DrawSprContour_a(int x,int y,int sx,int sy,const unsigned char* p,int contour_color,int mode,float scale)
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
		sx <<= 1;
		const unsigned short* src = reinterpret_cast<const unsigned short*>(p);

		for(int i = y0; i != y1; i += iy){
			const unsigned short* line_src = src + ((fy >> 16) * sx);

			fy += dy;
			fx = (1 << 15);

			int empty_pixel = 1;

			for(int j = x0; j != x1; j += ix){
				if(line_src[((fx >> 16) << 1) + 1] < 200){
					if(empty_pixel)
						SetPixel(x + j,y + i,contour_color);
					empty_pixel = 0;
				}
				else {
					if(!empty_pixel)
						SetPixel(x + j - 1,y + i,contour_color);
					empty_pixel = 1;
				}
				fx += dx;
			}
			if(!empty_pixel)
				SetPixel(x + x1 - 1,y + i,contour_color);
		}

		fx = (1 << 15);
		for(int j = x0; j != x1; j += ix){
			fy = (1 << 15);
			int empty_pixel = 1;

			for(int i = y0; i != y1; i += iy){
				const unsigned short* line_src = src + ((fy >> 16) * sx);

				if(line_src[((fx >> 16) << 1) + 1] < 200){
					if(empty_pixel)
						SetPixel(x + j,y + i,contour_color);
					empty_pixel = 0;
				}
				else {
					if(!empty_pixel)
						SetPixel(x + j,y + i - 1,contour_color);
					empty_pixel = 1;
				}
				fy += dy;
			}
			if(!empty_pixel)
				SetPixel(x + j,y + y1 - 1,contour_color);

			fx += dx;
		}

		return;
	}

	if(bytes_per_pixel() == 3 || bytes_per_pixel() == 4){
		int sx3 = sx * 4;

		for(int i = y0; i != y1; i += iy){
			const unsigned char* line_src = p + ((fy >> 16) * sx3);

			fy += dy;
			fx = (1 << 15);

			int empty_pixel = 1;

			for(int j = x0; j != x1; j += ix){
				int idx = (fx >> 16) << 2;
				if(line_src[idx + 3] < 200){
					if(empty_pixel)
						SetPixel(x + j,y + i,contour_color);
					empty_pixel = 0;
				}
				else {
					if(!empty_pixel)
						SetPixel(x + j - 1,y + i,contour_color);
					empty_pixel = 1;
				}

				fx += dx;
			}
			if(!empty_pixel)
				SetPixel(x + x1 - 1,y + i,contour_color);
		}

		fx = (1 << 15);
		for(int j = x0; j != x1; j += ix){
			fy = (1 << 15);

			int empty_pixel = 1;
			int idx = (fx >> 16) * 4;

			for(int i = y0; i != y1; i += iy){
				const unsigned char* line_src = p + ((fy >> 16) * sx3);

				if(line_src[idx + 3] < 200){
					if(empty_pixel)
						SetPixel(x + j,y + i,contour_color);
					empty_pixel = 0;
				}
				else {
					if(!empty_pixel)
						SetPixel(x + j,y + i - 1,contour_color);
					empty_pixel = 1;
				}

				fy += dy;
			}

			if(!empty_pixel)
				SetPixel(x + j,y + y1 - 1,contour_color);

			fx += dx;
		}

		return;
	}
}

void grDispatcher::PutChar(int x,int y,unsigned color,int font_sx,int font_sy,const unsigned char* font_alpha,const grScreenRegion& chr_region)
{
	int px = chr_region.x();
	int py = chr_region.y();

	int psx = chr_region.size_x();
	int psy = chr_region.size_y();

	if(!clip_rectangle(x,y,px,py,psx,psy)) return;

	const unsigned char* alpha_buf = font_alpha + px + py * font_sx;

	if(bytes_per_pixel() == 4){
		x *= 4;

		unsigned r = ((unsigned char*)(&color))[2];
		unsigned g = ((unsigned char*)(&color))[1];
		unsigned b = ((unsigned char*)(&color))[0];

		for(int i = 0; i < psy; i++,y++){
			unsigned char* scr_buf = reinterpret_cast<unsigned char*>(screenBuf + yTable[y] + x);
			for(int j = 0; j < psx; j++){
				unsigned a = alpha_buf[j];
				unsigned a1 = 255 - a;
				if(a){
					if(a != 255){
						scr_buf[0] = ((r * a) >> 8) + ((a1 * scr_buf[0]) >> 8);
						scr_buf[1] = ((g * a) >> 8) + ((a1 * scr_buf[1]) >> 8);
						scr_buf[2] = ((b * a) >> 8) + ((a1 * scr_buf[2]) >> 8);
					}
					else {
						scr_buf[0] = r;
						scr_buf[1] = g;
						scr_buf[2] = b;
					}
				}
				scr_buf += 4;
			}
			alpha_buf += font_sx;
		}
		return;
	}
	else if(bytes_per_pixel() == 3){
		x *= 3;

		unsigned r = ((unsigned char*)(&color))[2];
		unsigned g = ((unsigned char*)(&color))[1];
		unsigned b = ((unsigned char*)(&color))[0];

		for(int i = 0; i < psy; i++,y++){
			unsigned char* scr_buf = reinterpret_cast<unsigned char*>(screenBuf + yTable[y] + x);
			for(int j = 0; j < psx; j++){
				unsigned a = alpha_buf[j];
				unsigned a1 = 255 - a;
				if(a){
					if(a != 255){
						scr_buf[0] = ((r * a) >> 8) + ((a1 * scr_buf[0]) >> 8);
						scr_buf[1] = ((g * a) >> 8) + ((a1 * scr_buf[1]) >> 8);
						scr_buf[2] = ((b * a) >> 8) + ((a1 * scr_buf[2]) >> 8);
					}
					else {
						scr_buf[0] = r;
						scr_buf[1] = g;
						scr_buf[2] = b;
					}
				}
				scr_buf += 3;
			}
			alpha_buf += font_sx;
		}
		return;
	}
	else if(bytes_per_pixel() == 2){
		x *= 2;
		color = make_rgb(color);

		if(pixel_format_ == GR_RGB565){
			for(int i = 0; i < psy; i++,y++){
				unsigned short* scr_buf = reinterpret_cast<unsigned short*>(screenBuf + yTable[y] + x);
				for(int j = 0; j < psx; j++){
					unsigned a = alpha_buf[j];
					unsigned a1 = 255 - a;
					if(a){
						if(a != 255){
							unsigned scr_col = *scr_buf;
							*scr_buf++ = (((((color & mask_565_r) * a) >> 8) & mask_565_r) |
									((((color & mask_565_g) * a) >> 8) & mask_565_g) |
									((((color & mask_565_b) * a) >> 8) & mask_565_b)) +
									(((((scr_col & mask_565_r) * a1) >> 8) & mask_565_r) |
									((((scr_col & mask_565_g) * a1) >> 8) & mask_565_g) |
									((((scr_col & mask_565_b) * a1) >> 8) & mask_565_b));
						}
						else
							*scr_buf++ = color;
					}
					else scr_buf++;
				}
				alpha_buf += font_sx;
			}
		}
		else {
			for(int i = 0; i < psy; i++,y++){
				unsigned short* scr_buf = reinterpret_cast<unsigned short*>(screenBuf + yTable[y] + x);
				for(int j = 0; j < psx; j++){
					unsigned a = alpha_buf[j];
					unsigned a1 = 255 - a;
					if(a){
						if(a != 255){
							unsigned scr_col = *scr_buf;
							*scr_buf++ = (((((color & mask_555_r) * a) >> 8) & mask_555_r) |
									((((color & mask_555_g) * a) >> 8) & mask_555_g) |
									((((color & mask_555_b) * a) >> 8) & mask_555_b)) +
									(((((scr_col & mask_555_r) * a1) >> 8) & mask_555_r) |
									((((scr_col & mask_555_g) * a1) >> 8) & mask_555_g) |
									((((scr_col & mask_555_b) * a1) >> 8) & mask_555_b));
						}
						else
							*scr_buf++ = color;
					}
					else scr_buf++;
				}
				alpha_buf += font_sx;
			}
		}
		return;
	}
}

void grDispatcher::PutSprMask(int x,int y,int sx,int sy,const unsigned char* p,unsigned mask_color,int mask_alpha,int mode)
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
			x += psx * 4 - 4;
			px = sx - px - psx;
		}
		else 
			dx = 4;

		if(mode & GR_FLIP_VERTICAL){
			y += psy - 1;
			py = sy - py - psy;
		}
		else 
			dy = 1;

		int px3 = px * 3;
		int sx3 = sx * 3;

		const unsigned char* data_ptr = p + py * sx3;

		unsigned mr,mg,mb;
		split_rgb888(mask_color,mr,mg,mb);

		mr = (mr * (255 - mask_alpha)) >> 8;
		mg = (mg * (255 - mask_alpha)) >> 8;
		mb = (mb * (255 - mask_alpha)) >> 8;

		for(int i = 0; i < psy; i ++){
			unsigned char* scr_buf = reinterpret_cast<unsigned char*>(screenBuf + yTable[y] + x);
			const unsigned char* data_line = data_ptr + px3;

			for(int j = 0; j < psx; j ++){
				if(data_line[0] || data_line[1] || data_line[2]){
					scr_buf[2] = mr + ((mask_alpha * scr_buf[2]) >> 8);
					scr_buf[1] = mg + ((mask_alpha * scr_buf[1]) >> 8);
					scr_buf[0] = mb + ((mask_alpha * scr_buf[0]) >> 8);
				}
				scr_buf += dx;
				data_line += 3;
			}

			data_ptr += sx3;
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

		if(mode & GR_FLIP_VERTICAL){
			y += psy - 1;
			py = sy - py - psy;
		}
		else 
			dy = 1;

		int px3 = px * 3;
		int sx3 = sx * 3;

		const unsigned char* data_ptr = p + py * sx3;

		unsigned mr,mg,mb;
		split_rgb888(mask_color,mr,mg,mb);

		mr = (mr * (255 - mask_alpha)) >> 8;
		mg = (mg * (255 - mask_alpha)) >> 8;
		mb = (mb * (255 - mask_alpha)) >> 8;

		for(int i = 0; i < psy; i ++){
			unsigned char* scr_buf = reinterpret_cast<unsigned char*>(screenBuf + yTable[y] + x);
			const unsigned char* data_line = data_ptr + px3;

			for(int j = 0; j < psx; j ++){
				if(data_line[0] || data_line[1] || data_line[2]){
					scr_buf[2] = mr + ((mask_alpha * scr_buf[2]) >> 8);
					scr_buf[1] = mg + ((mask_alpha * scr_buf[1]) >> 8);
					scr_buf[0] = mb + ((mask_alpha * scr_buf[0]) >> 8);
				}
				scr_buf += dx;
				data_line += 3;
			}

			data_ptr += sx3;
			y += dy;
		}
	}
	else if(bytes_per_pixel() == 2){
		int dx = -1;
		int dy = -1;

		if(mode & GR_FLIP_HORIZONTAL){
			x += psx - 1;
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

		x <<= 1;

		px *= 3;
		sx *= 3;

		const unsigned char* data_ptr = p + py * sx + px;

		if(pixel_format_ == GR_RGB565){
			unsigned mr,mg,mb;
			split_rgb565u(mask_color,mr,mg,mb);

			mr = (mr * (255 - mask_alpha)) >> 8;
			mg = (mg * (255 - mask_alpha)) >> 8;
			mb = (mb * (255 - mask_alpha)) >> 8;

			unsigned mcl = make_rgb565u(mr,mg,mb);

			for(int i = 0; i < psy; i ++){
				unsigned short* scr_buf = (unsigned short*)(screenBuf + yTable[y] + x);
				const unsigned char* data_line = data_ptr;

				for(int j = 0; j < psx; j ++){
					if(data_line[0] || data_line[1] || data_line[2])
						*scr_buf = alpha_blend_565(mcl,*scr_buf,mask_alpha);
					scr_buf += dx;
					data_line += 3;
				}

				data_ptr += sx;
				y += dy;
			}
		}
		else {
			unsigned mr,mg,mb;
			split_rgb555u(mask_color,mr,mg,mb);

			mr = (mr * (255 - mask_alpha)) >> 8;
			mg = (mg * (255 - mask_alpha)) >> 8;
			mb = (mb * (255 - mask_alpha)) >> 8;

			unsigned mcl = make_rgb555u(mr,mg,mb);

			for(int i = 0; i < psy; i ++){
				unsigned short* scr_buf = (unsigned short*)(screenBuf + yTable[y] + x);
				const unsigned char* data_line = data_ptr;

				for(int j = 0; j < psx; j ++){
					if(data_line[0] || data_line[1] || data_line[2])
						*scr_buf = alpha_blend_555(mcl,*scr_buf,mask_alpha);
					scr_buf += dx;
					data_line += 3;
				}

				data_ptr += sx;
				y += dy;
			}
		}
	}
}

void grDispatcher::PutSprMask(int x,int y,int sx,int sy,const unsigned char* p,unsigned mask_color,int mask_alpha,int mode,float scale)
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
		if(pixel_format_ == GR_RGB565){
			unsigned mr,mg,mb;
			split_rgb565u(mask_color,mr,mg,mb);

			mr = (mr * (255 - mask_alpha)) >> 8;
			mg = (mg * (255 - mask_alpha)) >> 8;
			mb = (mb * (255 - mask_alpha)) >> 8;

			unsigned mcl = make_rgb565u(mr,mg,mb);

			sx *= 3;

			for(int i = y0; i != y1; i += iy){
				const unsigned char* line_src = p + ((fy >> 16) * sx);

				fy += dy;
				fx = (1 << 15);

				for(int j = x0; j != x1; j += ix){
					const unsigned char* src_data = line_src + (fx >> 16) * 3;
					if(src_data[0] || src_data[1] || src_data[2]){
						unsigned scl;
						GetPixel(x + j,y + i,scl);
						SetPixel(x + j,y + i,alpha_blend_565(mcl,scl,mask_alpha));
					}
					fx += dx;
				}
			}
		}
		else {
			unsigned mr,mg,mb;
			split_rgb555u(mask_color,mr,mg,mb);

			mr = (mr * (255 - mask_alpha)) >> 8;
			mg = (mg * (255 - mask_alpha)) >> 8;
			mb = (mb * (255 - mask_alpha)) >> 8;

			unsigned mcl = make_rgb555u(mr,mg,mb);

			for(int i = y0; i != y1; i += iy){
				const unsigned char* line_src = p + ((fy >> 16) * sx);

				fy += dy;
				fx = (1 << 15);

				for(int j = x0; j != x1; j += ix){
					const unsigned char* src_data = line_src + (fx >> 16) * 3;
					if(src_data[0] || src_data[1] || src_data[2]){
						unsigned scl;
						GetPixel(x + j,y + i,scl);
						SetPixel(x + j,y + i,alpha_blend_555(mcl,scl,mask_alpha));
					}
					fx += dx;
				}
			}
		}
	}
	else if(bytes_per_pixel() == 3 || bytes_per_pixel() == 4){
		unsigned mr,mg,mb;
		split_rgb888(mask_color,mr,mg,mb);

		mr = (mr * (255 - mask_alpha)) >> 8;
		mg = (mg * (255 - mask_alpha)) >> 8;
		mb = (mb * (255 - mask_alpha)) >> 8;

		int sx3 = sx * 3;
		for(int i = y0; i != y1; i += iy){
			const unsigned char* line_src = p + ((fy >> 16) * sx3);

			fy += dy;
			fx = (1 << 15);

			for(int j = x0; j != x1; j += ix){
				int idx = (fx >> 16) * 3;

				unsigned r = line_src[idx + 2];
				unsigned g = line_src[idx + 1];
				unsigned b = line_src[idx + 0];

				if(r || g || b){
					r = mr + ((mask_alpha * r) >> 8);
					g = mg + ((mask_alpha * g) >> 8);
					b = mb + ((mask_alpha * b) >> 8);

					SetPixel(x + j,y + i,r,g,b);
				}

				fx += dx;
			}
		}
	}
}

void grDispatcher::PutSprMask_a(int x,int y,int sx,int sy,const unsigned char* p,unsigned mask_color,int mask_alpha,int mode)
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

		if(mode & GR_FLIP_VERTICAL){
			y += psy - 1;
			py = sy - py - psy;
		}
		else 
			dy = 1;

		int px3 = px * 4;
		int sx3 = sx * 4;

		const unsigned char* data_ptr = p + py * sx3;

		unsigned mr,mg,mb;
		split_rgb888(mask_color,mr,mg,mb);

		for(int i = 0; i < psy; i ++){
			unsigned char* scr_buf = reinterpret_cast<unsigned char*>(screenBuf + yTable[y] + x);
			const unsigned char* data_line = data_ptr + px3;

			for(int j = 0; j < psx; j ++){
				unsigned a = data_line[3];
				if(a != 255){
					a = mask_alpha + ((a * (255 - mask_alpha)) >> 8);

					scr_buf[0] = ((mb * (255 - a)) >> 8) + ((a * scr_buf[0]) >> 8);
					scr_buf[1] = ((mg * (255 - a)) >> 8) + ((a * scr_buf[1]) >> 8);
					scr_buf[2] = ((mr * (255 - a)) >> 8) + ((a * scr_buf[2]) >> 8);
				}
				scr_buf += dx;
				data_line += 4;
			}
			data_ptr += sx3;
			y += dy;
		}
	}
	else if(bytes_per_pixel() == 3){
		int dx = -3;
		int dy = -1;

		x *= 3;

		if(mode & GR_FLIP_HORIZONTAL){
			x += (psx - 1) * 3;
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

		int px3 = px * 4;
		int sx3 = sx * 4;

		const unsigned char* data_ptr = p + py * sx3;

		unsigned mr,mg,mb;
		split_rgb888(mask_color,mr,mg,mb);

		for(int i = 0; i < psy; i ++){
			unsigned char* scr_buf = reinterpret_cast<unsigned char*>(screenBuf + yTable[y] + x);
			const unsigned char* data_line = data_ptr + px3;

			for(int j = 0; j < psx; j ++){
				unsigned a = data_line[3];
				if(a != 255){
					a = mask_alpha + ((a * (255 - mask_alpha)) >> 8);

					scr_buf[0] = ((mb * (255 - a)) >> 8) + ((a * scr_buf[0]) >> 8);
					scr_buf[1] = ((mg * (255 - a)) >> 8) + ((a * scr_buf[1]) >> 8);
					scr_buf[2] = ((mr * (255 - a)) >> 8) + ((a * scr_buf[2]) >> 8);
				}
				scr_buf += dx;
				data_line += 4;
			}
			data_ptr += sx3;
			y += dy;
		}
	}
	else if(bytes_per_pixel() == 2){
		int dx = -1;
		int dy = -1;

		if(mode & GR_FLIP_HORIZONTAL){
			x += psx - 1;
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

		x <<= 1;
		sx <<= 2;
		px <<= 2;

		const unsigned char* data_ptr = p + py * sx + px;

		if(pixel_format_ == GR_RGB565){
			unsigned mr,mg,mb;
			split_rgb565u(mask_color,mr,mg,mb);

			for(int i = 0; i < psy; i ++){
				unsigned short* scr_buf = (unsigned short*)(screenBuf + yTable[y] + x);
				const unsigned char* data_line = data_ptr;

				for(int j = 0; j < psx; j ++){
					unsigned a = data_line[3];

					if(a != 255){
						a = mask_alpha + ((a * (255 - mask_alpha)) >> 8);

						unsigned r = (mr * (255 - a)) >> 8;
						unsigned g = (mg * (255 - a)) >> 8;
						unsigned b = (mb * (255 - a)) >> 8;

						unsigned cl = make_rgb565u(r,g,b);

						*scr_buf = alpha_blend_565(cl,*scr_buf,a);
					}
					scr_buf += dx;
					data_line += 4;
				}
				data_ptr += sx;
				y += dy;
			}
		}
		else {
			unsigned mr,mg,mb;
			split_rgb555u(mask_color,mr,mg,mb);

			for(int i = 0; i < psy; i ++){
				unsigned short* scr_buf = (unsigned short*)(screenBuf + yTable[y] + x);
				const unsigned char* data_line = data_ptr;

				for(int j = 0; j < psx; j ++){
					unsigned a = data_line[3];

					if(a != 255){
						a = mask_alpha + ((a * (255 - mask_alpha)) >> 8);

						unsigned r = (mr * (255 - a)) >> 8;
						unsigned g = (mg * (255 - a)) >> 8;
						unsigned b = (mb * (255 - a)) >> 8;

						unsigned cl = make_rgb555u(r,g,b);

						*scr_buf = alpha_blend_555(cl,*scr_buf,a);
					}
					scr_buf += dx;
					data_line += 4;
				}
				data_ptr += sx;
				y += dy;
			}
		}
	}
}

void grDispatcher::PutSprMask_a(int x,int y,int sx,int sy,const unsigned char* p,unsigned mask_color,int mask_alpha,int mode,float scale)
{
	int i,j,sx_dest,sy_dest;

	sx_dest = round(float(sx) * scale);
	sy_dest = round(float(sy) * scale);

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
		const unsigned short* src = reinterpret_cast<const unsigned short*>(p);
		sx <<= 1;
		if(pixel_format_ == GR_ARGB1555){
			unsigned mr,mg,mb;
			split_rgb555u(mask_color,mr,mg,mb);
            
			for(i = y0; i != y1; i += iy){
				const unsigned char* line_src = p + ((fy >> 16) * sx);

				fy += dy;
				fx = (1 << 15);

				for(j = x0; j != x1; j += ix){
					const unsigned char* src_data = line_src + (fx >> 16) * 4;
					unsigned a = src_data[3];

					if(a != 255 && ClipCheck(x + j,y + i)){
						unsigned sc;
						GetPixel(x + j,y + i,sc);

						a = mask_alpha + ((a * (255 - mask_alpha)) >> 8);

						unsigned r = (mr * (255 - a)) >> 8;
						unsigned g = (mg * (255 - a)) >> 8;
						unsigned b = (mb * (255 - a)) >> 8;

						unsigned cl = make_rgb555u(r,g,b);

						SetPixel(x + j,y + i,alpha_blend_555(cl,sc,a));
					}
					fx += dx;
				}
			}
		}
		else {
			unsigned mr,mg,mb;
			split_rgb565u(mask_color,mr,mg,mb);

			for(i = y0; i != y1; i += iy){
				const unsigned char* line_src = p + ((fy >> 16) * sx);

				fy += dy;
				fx = (1 << 15);

				for(j = x0; j != x1; j += ix){
					const unsigned char* src_data = line_src + (fx >> 16) * 4;
					unsigned a = src_data[3];

					if(a != 255 && ClipCheck(x + j,y + i)){
						unsigned sc;
						GetPixel(x + j,y + i,sc);

						a = mask_alpha + ((a * (255 - mask_alpha)) >> 8);

						unsigned r = (mr * (255 - a)) >> 8;
						unsigned g = (mg * (255 - a)) >> 8;
						unsigned b = (mb * (255 - a)) >> 8;

						unsigned cl = make_rgb565u(r,g,b);

						SetPixel(x + j,y + i,alpha_blend_565(cl,sc,a));
					}
					fx += dx;
				}
			}
		}
	}
	else if(bytes_per_pixel() == 3 || bytes_per_pixel() == 4){
		int sx3 = sx * 4;

		unsigned mr,mg,mb;
		split_rgb888(mask_color,mr,mg,mb);

		for(i = y0; i != y1; i += iy){
			const unsigned char* line_src = p + ((fy >> 16) * sx3);

			fy += dy;
			fx = (1 << 15);

			for(j = x0; j != x1; j += ix){
				int idx = (fx >> 16) << 2;
				unsigned a = line_src[idx + 3];
				if(a != 255 && ClipCheck(x + j,y + i)){
					unsigned sr,sg,sb;
					GetPixel(x + j,y + i,sr,sg,sb);

					a = mask_alpha + ((a * (255 - mask_alpha)) >> 8);

					unsigned r = ((mr * (255 - a)) >> 8) + ((a * sr) >> 8);
					unsigned g = ((mg * (255 - a)) >> 8) + ((a * sg) >> 8);
					unsigned b = ((mb * (255 - a)) >> 8) + ((a * sb) >> 8);

					SetPixel(x + j,y + i,r,g,b);
				}

				fx += dx;
			}
		}
	}
}
