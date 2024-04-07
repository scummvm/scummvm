/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "gr_dispatcher.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

#ifdef _GR_ENABLE_ZBUFFER
void grDispatcher::PutSpr_a_z(int x,int y,int z,int sx,int sy,const unsigned char* p,int mode,float scale)
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
			for(i = y0; i != y1; i += iy){
				const unsigned short* line_src = src + ((fy >> 16) * sx);

				fy += dy;
				fx = (1 << 15);

				for(j = x0; j != x1; j += ix){
					unsigned a = line_src[((fx >> 16) << 1) + 1];
					if(a != 255 && ClipCheck(x + j,y + i)){
						unsigned sc;
						GetPixel(x + j,y + i,sc);
						SetPixel(x + j,y + i,alpha_blend_555(line_src[(fx >> 16) << 1],sc,a));
						put_z(x + j,y + i,z);
					}
					fx += dx;
				}
			}
		}
		else {
			for(i = y0; i != y1; i += iy){
				const unsigned short* line_src = src + ((fy >> 16) * sx);

				fy += dy;
				fx = (1 << 15);

				for(j = x0; j != x1; j += ix){
					unsigned a = line_src[((fx >> 16) << 1) + 1];
					if(a != 255 && ClipCheck(x + j,y + i)){
						unsigned sc;
						GetPixel(x + j,y + i,sc);
						SetPixel(x + j,y + i,alpha_blend_565(line_src[(fx >> 16) << 1],sc,a));
						put_z(x + j,y + i,z);
					}
					fx += dx;
				}
			}
		}
		return;
	}
	if(bytes_per_pixel() == 3 || bytes_per_pixel() == 4){
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
					put_z(x + j,y + i,z);
				}

				fx += dx;
			}
		}
		return;
	}
}

void grDispatcher::PutSpr_z(int x,int y,int z,int sx,int sy,const unsigned char* p,int mode,float scale)
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
				if(cl){
					SetPixel(x + j,y + i,cl);
					put_z(x + j,y + i,z);
				}
				fx += dx;
			}
		}
		return;
	}

	if(bytes_per_pixel() == 3){
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
					SetPixel(x + j,y + i,r,g,b);
					put_z(x + j,y + i,z);
				}

				fx += dx;
			}
		}
		return;
	}

	if(bytes_per_pixel() == 4){
		const unsigned* src = reinterpret_cast<const unsigned*>(p);

		for(int i = y0; i != y1; i += iy){
			const unsigned* line_src = src + ((fy >> 16) * sx);

			fy += dy;
			fx = (1 << 15);

			for(int j = x0; j != x1; j += ix){
				unsigned cl = line_src[fx >> 16];
				if(cl){
					SetPixel(x + j,y + i,cl);
					put_z(x + j,y + i,z);
				}
				fx += dx;
			}
		}
		return;
	}
}

void grDispatcher::PutSpr_a_z(int x,int y,int z,int sx,int sy,const unsigned char* p,int mode)
{
	int px = 0;
	int py = 0;

	int psx = sx;
	int psy = sy;

	if(!clip_rectangle(x,y,px,py,psx,psy)) return;

	if(bytes_per_pixel() == 4){
		int dx = -4;
		int zdx = -1;
		int dy = -1;

		int x4 = x * 4;

		if(mode & GR_FLIP_HORIZONTAL){
			x4 += (psx - 1) * 4;
			x += psx - 1;
			px = sx - px - psx;
		}
		else {
			dx = 4;
			zdx = 1;
		}

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
			unsigned char* scr_buf = reinterpret_cast<unsigned char*>(screenBuf + yTable[y] + x4);
			zbuf_t* zbuf = zbuffer_ + y * SizeX + x;
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
					*zbuf = z;
				}
				scr_buf += dx;
				zbuf += zdx;
				data_line += 4;
			}
			data_ptr += sx3;
			y += dy;
		}
		return;
	}
	if(bytes_per_pixel() == 3){
		int dx = -3;
		int zdx = -1;
		int dy = -1;

		int x3 = x * 3;

		if(mode & GR_FLIP_HORIZONTAL){
			x3 += (psx - 1) * 3;
			x += psx - 1;
			px = sx - px - psx;
		}
		else {
			dx = 3;
			zdx = 1;
		}

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
			unsigned char* scr_buf = reinterpret_cast<unsigned char*>(screenBuf + yTable[y] + x3);
			zbuf_t* zbuf = zbuffer_ + y * SizeX + x;
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
					*zbuf = z;
				}
				scr_buf += dx;
				zbuf += zdx;
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
		sx <<= 1;
		px <<= 1;

		const unsigned short* data_ptr = reinterpret_cast<const unsigned short*>(p) + py * sx;

		if(pixel_format_ == GR_RGB565){
			for(int i = 0; i < psy; i ++){
				unsigned short* scr_buf = reinterpret_cast<unsigned short*>(screenBuf + yTable[y] + x);
				zbuf_t* zbuf = zbuffer_ + y * SizeX + (x >> 1);
				const unsigned short* data_line = data_ptr + px;

				for(int j = 0; j < psx; j ++){
					unsigned a = data_line[1];
					*scr_buf = alpha_blend_565(*data_line,*scr_buf,a);
					if(a != 255) *zbuf = z;
					scr_buf += dx;
					zbuf += dx;
					data_line += 2;
				}
				data_ptr += sx;
				y += dy;
			}
		}
		else {
			for(int i = 0; i < psy; i ++){
				unsigned short* scr_buf = reinterpret_cast<unsigned short*>(screenBuf + yTable[y] + x);
				zbuf_t* zbuf = zbuffer_ + y * SizeX + (x >> 1);
				const unsigned short* data_line = data_ptr + px;

				for(int j = 0; j < psx; j ++){
					unsigned a = data_line[1];
					*scr_buf = alpha_blend_555(*data_line,*scr_buf,a);
					if(a != 255) *zbuf = z;
					scr_buf += dx;
					zbuf += dx;
					data_line += 2;
				}
				data_ptr += sx;
				y += dy;
			}
		}
		return;
	}
}

void grDispatcher::PutSpr_z(int x,int y,int z,int sx,int sy,const unsigned char* p,int mode)
{
	int px = 0;
	int py = 0;

	int psx = sx;
	int psy = sy;

	if(!clip_rectangle(x,y,px,py,psx,psy)) return;

	if(bytes_per_pixel() == 4){
		int dx = -4;
		int zdx = -1;
		int dy = -1;

		int x4 = x * 4;

		if(mode & GR_FLIP_HORIZONTAL){
			x4 += psx * 4 - 4;
			x += psx - 1;
			px = sx - px - psx;
		}
		else {
			dx = 4;
			zdx = 1;
		}

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
			unsigned char* scr_buf = reinterpret_cast<unsigned char*>(screenBuf + yTable[y] + x4);
			zbuf_t* zbuf = zbuffer_ + y * SizeX + x;
			const unsigned char* data_line = data_ptr + px3;

			for(int j = 0; j < psx; j ++){
				if(data_line[0] || data_line[1] || data_line[2]){
					scr_buf[0] = data_line[0];
					scr_buf[1] = data_line[1];
					scr_buf[2] = data_line[2];
					*zbuf = z;
				}
				scr_buf += dx;
				zbuf += zdx;
				data_line += 3;
			}

			data_ptr += sx3;
			y += dy;
		}
		return;
	}
	if(bytes_per_pixel() == 3){
		int dx = -3;
		int zdx = -1;
		int dy = -1;

		int x3 = x * 3;

		if(mode & GR_FLIP_HORIZONTAL){
			x3 += psx * 3 - 3;
			x += psx - 1;
			px = sx - px - psx;
		}
		else {
			dx = 3;
			zdx = 1;
		}

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
			unsigned char* scr_buf = reinterpret_cast<unsigned char*>(screenBuf + yTable[y] + x3);
			zbuf_t* zbuf = zbuffer_ + y * SizeX + x;
			const unsigned char* data_line = data_ptr + px3;

			for(int j = 0; j < psx; j ++){
				if(data_line[0] || data_line[1] || data_line[2]){
					scr_buf[0] = data_line[0];
					scr_buf[1] = data_line[1];
					scr_buf[2] = data_line[2];
					*zbuf = z;
				}
				scr_buf += dx;
				zbuf += zdx;
				data_line += 3;
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

		const unsigned short* data_ptr = reinterpret_cast<const unsigned short*>(p) + py * sx;

		for(int i = 0; i < psy; i ++){
			unsigned short* scr_buf = reinterpret_cast<unsigned short*>(screenBuf + yTable[y] + x);
			zbuf_t* zbuf = zbuffer_ + y * SizeX + x;
			const unsigned short* data_line = data_ptr + px;

			for(int j = 0; j < psx; j ++){
				if(*data_line){
					*scr_buf = *data_line;
					*zbuf = z;
				}
				zbuf += dx;
				scr_buf += dx;
				data_line++;
			}

			data_ptr += sx;
			y += dy;
		}
		return;
	}
}
#endif

