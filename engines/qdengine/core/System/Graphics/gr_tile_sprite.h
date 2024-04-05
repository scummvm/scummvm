#ifndef __GR_TILE_SPRITE_H__
#define __GR_TILE_SPRITE_H__

const GR_TILE_SPRITE_SIZE_SHIFT	= 4;
const GR_TILE_SPRITE_SIZE_X		= 1 << GR_TILE_SPRITE_SIZE_SHIFT;
const GR_TILE_SPRITE_SIZE_Y		= 1 << GR_TILE_SPRITE_SIZE_SHIFT;

const GR_TILE_SPRITE_SIZE		= GR_TILE_SPRITE_SIZE_X * GR_TILE_SPRITE_SIZE_Y;
const GR_TILE_SPRITE_SIZE_BYTES = GR_TILE_SPRITE_SIZE * 4;

enum grTileCompressionMethod
{
	TILE_UNCOMPRESSED,
	TILE_COMPRESS_RLE,
	TILE_COMPRESS_LZ77
};

/// Тайл-спрайт

/// Квадратный 32х битный спрайт фиксированного размера.
/// Данные внешние.
class grTileSprite
{
public:
	grTileSprite(const unsigned* data_ptr = 0);

	bool operator == (const grTileSprite& sprite) const;

	bool isEmpty() const { return !data_; }

	const unsigned* data() const { return data_; }

	static unsigned comprasionTolerance() { return comprasionTolerance_; }
	static void setComprasionTolerance(unsigned value){ comprasionTolerance_ = value; }

	static unsigned compress(const unsigned* in_data, unsigned* out_data, grTileCompressionMethod compress_method);
	static bool uncompress(const unsigned* in_data, unsigned in_data_length, unsigned* out_data, grTileCompressionMethod compress_method);

private:

	const unsigned* data_;

	/// толерантность побайтового сравнения данных, [0, 255]
	static unsigned comprasionTolerance_; 
};

#endif /*__GR_TILE_SPRITE_H__ */
