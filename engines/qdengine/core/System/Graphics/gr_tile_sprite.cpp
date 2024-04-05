#include "qd_precomp.h"

#include "gr_dispatcher.h"
#include "gr_tile_sprite.h"

#include "LZ77.h"

unsigned grTileSprite::comprasionTolerance_ = 2;

namespace tile_compress
{

const unsigned RLE_SEQUENCE_MASK = 1 << (GR_TILE_SPRITE_SIZE_SHIFT * 2 + 1); 

unsigned encodeRLE(const unsigned* in_data, unsigned* out_data)
{
	unsigned size = 0;

	int count = 0;
	while(count < GR_TILE_SPRITE_SIZE){
		int index = count;
		unsigned pixel = in_data[index ++];

		while(index < GR_TILE_SPRITE_SIZE && in_data[index] == pixel)
			index ++;

		if(index - count == 1){
			while(index < GR_TILE_SPRITE_SIZE && (in_data[index] != in_data[index - 1] || index > 1 && in_data[index] != in_data[index - 2]))
				index ++;

			while(index < GR_TILE_SPRITE_SIZE && in_data[index] == in_data[index - 1])
				index --;

			out_data[size] = index - count;
			out_data[size] |= RLE_SEQUENCE_MASK;
			size++;

			for(int i = count; i < index; i ++)
				out_data[size++] = in_data[i];
		} 
		else {
			out_data[size++] = index - count;
			out_data[size++] = pixel;
		}

		count = index;
		xassert(index < GR_TILE_SPRITE_SIZE * 4);
	}

	return size;
}

bool decodeRLE(const unsigned* in_data, unsigned* out_data)
{
	const unsigned* in_buf = in_data;
	unsigned* out_buf = out_data;

	int out_size = 0;
	while(out_size < GR_TILE_SPRITE_SIZE){
		unsigned count = *in_buf++;
		if(count & RLE_SEQUENCE_MASK){
			count ^= RLE_SEQUENCE_MASK;
			for(int i = 0; i < count; i++)
				*out_buf++ = *in_buf++;
		}
		else {
			unsigned color = *in_buf++;
			for(int i = 0; i < count; i++)
				*out_buf++ = color;
		}

		out_size += count;
	}

	return true;
}

}; // namespace tile_compress

void grDispatcher::PutTileSpr(int x, int y, const grTileSprite& sprite, bool has_alpha, int mode)
{
	int px = 0;
	int py = 0;

	int psx = GR_TILE_SPRITE_SIZE_X;
	int psy = GR_TILE_SPRITE_SIZE_Y;

	if(!clip_rectangle(x,y,px,py,psx,psy)) return;

	if(bytes_per_pixel() == 4){
		int dx = -4;
		int dy = -1;

		x *= 4;

		if(mode & GR_FLIP_HORIZONTAL){
			x += (psx - 1) * 4;
			px = GR_TILE_SPRITE_SIZE_X - px - psx;
		}
		else 
			dx = 4;

		if(mode & GR_FLIP_VERTICAL){
			y += psy - 1;
			py = GR_TILE_SPRITE_SIZE_Y - py - psy;
		}
		else 
			dy = 1;

		const unsigned char* data_ptr = (const unsigned char*)sprite.data() + px*4 + py * GR_TILE_SPRITE_SIZE_X*4;

		for(int i = 0; i < psy; i ++){
			unsigned char* scr_buf = (unsigned char*)(screenBuf + yTable[y] + x);
			const unsigned char* data_line = data_ptr;

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
			data_ptr += GR_TILE_SPRITE_SIZE_X*4;
			y += dy;
		}
		return;
	}
	else if(bytes_per_pixel() == 3){
		int dx = -3;
		int dy = -1;

		x *= 3;

		if(mode & GR_FLIP_HORIZONTAL){
			x += (psx - 1) * 3;
			px = GR_TILE_SPRITE_SIZE_X - px - psx;
		}
		else 
			dx = 3;

		if(mode & GR_FLIP_VERTICAL){
			y += psy - 1;
			py = GR_TILE_SPRITE_SIZE_Y - py - psy;
		}
		else 
			dy = 1;

		const unsigned char* data_ptr = (const unsigned char*)sprite.data() + px*4 + py * GR_TILE_SPRITE_SIZE_X*4;

		for(int i = 0; i < psy; i ++){
			unsigned char* scr_buf = (unsigned char*)(screenBuf + yTable[y] + x);
			const unsigned char* data_line = data_ptr;

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
			data_ptr += GR_TILE_SPRITE_SIZE_X*4;
			y += dy;
		}
		return;
	}
	else if(bytes_per_pixel() == 2){
		int dx = -1;
		int dy = -1;

		if(mode & GR_FLIP_HORIZONTAL){
			x += psx - 1;
			px = GR_TILE_SPRITE_SIZE_X - px - psx;
		}
		else 
			dx = 1;

		if(mode & GR_FLIP_VERTICAL){
			y += psy - 1;
			py = GR_TILE_SPRITE_SIZE_Y - py - psy;
		}
		else 
			dy = 1;

		x *= 2;

		const unsigned char* data_ptr = (unsigned char*)(sprite.data() + px + py * GR_TILE_SPRITE_SIZE_X);

		if(pixel_format_ == GR_RGB565){
			for(int i = 0; i < psy; i ++){
				unsigned short* scr_buf = reinterpret_cast<unsigned short*>(screenBuf + yTable[y] + x);
				const unsigned char* data_line = data_ptr;

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
				data_ptr += GR_TILE_SPRITE_SIZE_X*4;
				y += dy;
			}
		}
		else {
			for(int i = 0; i < psy; i ++){
				unsigned short* scr_buf = reinterpret_cast<unsigned short*>(screenBuf + yTable[y] + x);
				const unsigned char* data_line = data_ptr;

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
				data_ptr += GR_TILE_SPRITE_SIZE_X*4;
				y += dy;
			}
		}
	}
}

grTileSprite::grTileSprite(const unsigned* data_ptr) : data_(data_ptr)
{
}

bool grTileSprite::operator == (const grTileSprite& sprite) const
{
	if(isEmpty() || sprite.isEmpty())
		return (isEmpty() && sprite.isEmpty());

	const unsigned char* ptr0 = (const unsigned char*)data_;
	const unsigned char* ptr1 = (const unsigned char*)sprite.data_;

	for(int i = 0; i < GR_TILE_SPRITE_SIZE_BYTES; i++, ptr0++, ptr1++){
		if(abs(*ptr0 - *ptr1) > comprasionTolerance_)
			return false;
	}

	return true;
}

unsigned grTileSprite::compress(const unsigned* in_data, unsigned* out_data, grTileCompressionMethod compress_method)
{
	if(compress_method == TILE_COMPRESS_RLE){
		return tile_compress::encodeRLE(in_data, out_data);
	}
	else if(compress_method == TILE_COMPRESS_LZ77){
		CLZ77 encoder;
		long len = 0;
		encoder.Encode((unsigned char*)(out_data + 1), len, (const unsigned char*)in_data, GR_TILE_SPRITE_SIZE_BYTES);
		xassert(len);
		out_data[0] = len;
		return len / 4 + 2;
	}

	return 0;
}

bool grTileSprite::uncompress(const unsigned* in_data, unsigned in_data_length, unsigned* out_data, grTileCompressionMethod compress_method)
{
	if(compress_method == TILE_COMPRESS_RLE){
		return tile_compress::decodeRLE(in_data, out_data);
	}
	else if(compress_method == TILE_COMPRESS_LZ77){
		CLZ77 decoder;
		long len = 0;
		in_data_length = in_data[0];
		decoder.Decode((unsigned char*)out_data, len, (const unsigned char*)(in_data + 1), in_data_length);
		return true;
	}

	return false;
}
