#include <stdint.h>
#include <string.h>

#ifndef NDEBUG
#include <stdio.h>
#define STBI_ZERROR(x) fprintf(stderr, "%s:%d: error: %s\n", __FILE__, __LINE__, x), 0
#else
#define STBI_ZERROR(x) 0
#endif

// fast-way is faster to check than jpeg huffman, but slow way is slower
#define STBI__ZFAST_BITS  9 // accelerate all cases in default tables
#define STBI__ZFAST_MASK  ((1 << STBI__ZFAST_BITS) - 1)

// zlib-style huffman encoding
// (jpegs packs from left, zlib from right, so can't share code)
struct stbi__zhuffman {
   uint16_t fast[1 << STBI__ZFAST_BITS];
   uint16_t firstcode[16];
   int maxcode[17];
   uint16_t firstsymbol[16];
   uint8_t  size[288];
   uint16_t value[288];
};

static inline int stbi__bitreverse16(int n)
{
  n = ((n & 0xAAAA) >>  1) | ((n & 0x5555) << 1);
  n = ((n & 0xCCCC) >>  2) | ((n & 0x3333) << 2);
  n = ((n & 0xF0F0) >>  4) | ((n & 0x0F0F) << 4);
  n = ((n & 0xFF00) >>  8) | ((n & 0x00FF) << 8);
  return n;
}

static inline int stbi__bit_reverse(int v, int bits)
{
   // to bit reverse n bits, reverse 16 and shift
   // e.g. 11 bits, bit reverse and shift away 5
   return stbi__bitreverse16(v) >> (16-bits);
}

static int stbi__zbuild_huffman(struct stbi__zhuffman *z, uint8_t *sizelist, int num)
{
   int i,k=0;
   int code, next_code[16], sizes[17];

   // DEFLATE spec for generating codes
   memset(sizes, 0, sizeof(sizes));
   memset(z->fast, 0, sizeof(z->fast));
   for (i=0; i < num; ++i)
      ++sizes[sizelist[i]];
   sizes[0] = 0;
   for (i=1; i < 16; ++i)
      if (sizes[i] > (1 << i))
         return STBI_ZERROR("bad sizes");
   code = 0;
   for (i=1; i < 16; ++i) {
      next_code[i] = code;
      z->firstcode[i] = (uint16_t) code;
      z->firstsymbol[i] = (uint16_t) k;
      code = (code + sizes[i]);
      if (sizes[i])
         if (code-1 >= (1 << i)) return STBI_ZERROR("bad codelengths");
      z->maxcode[i] = code << (16-i); // preshift for inner loop
      code <<= 1;
      k += sizes[i];
   }
   z->maxcode[16] = 0x10000; // sentinel
   for (i=0; i < num; ++i) {
      int s = sizelist[i];
      if (s) {
         int c = next_code[s] - z->firstcode[s] + z->firstsymbol[s];
         uint16_t fastv = (uint16_t) ((s << 9) | i);
         z->size [c] = (uint8_t     ) s;
         z->value[c] = (uint16_t) i;
         if (s <= STBI__ZFAST_BITS) {
            int j = stbi__bit_reverse(next_code[s],s);
            while (j < (1 << STBI__ZFAST_BITS)) {
               z->fast[j] = fastv;
               j += (1 << s);
            }
         }
         ++next_code[s];
      }
   }
   return 1;
}

// zlib-from-memory implementation for PNG reading
//    because PNG allows splitting the zlib stream arbitrarily,
//    and it's annoying structurally to have PNG call ZLIB call PNG,
//    we require PNG read all the IDATs and combine them into a single
//    memory buffer

typedef struct stbi__stream
{
    const uint8_t *start_in;
    const uint8_t *next_in;
    const uint8_t *end_in;

    uint8_t *start_out;
    uint8_t *next_out;
    uint8_t *end_out;

    void *cookie_in;
    void *cookie_out;

    size_t total_in;
    size_t total_out;

    int (*refill)(struct stbi__stream *);
    int (*flush)(struct stbi__stream *);

   int num_bits;
   uint32_t code_buffer;

   struct stbi__zhuffman z_length, z_distance;
} stbi__zbuf;

int refill_zeros(struct stbi__stream *stream) {
    static const uint8_t zeros[64] = {0};
    stream->start_in = stream->next_in = zeros;
    stream->end_in = zeros + sizeof(zeros);
    return 0;
}

int refill_stdio(struct stbi__stream *stream) {
    size_t n = fread((void *)stream->start_in, 1, stream->end_in - stream->start_in, (FILE *)stream->cookie_in);
    if (n) {
        stream->next_in = stream->start_in;
        stream->end_in = stream->start_in + n;
        return 0;
    }
    return refill_zeros(stream);
}

int flush_stdio(struct stbi__stream *stream) {
    size_t n = fwrite(stream->start_out, 1, stream->next_out - stream->start_out, (FILE *)stream->cookie_out);
    if (n) {
        stream->next_out = stream->start_out;
        return 0;
    }
    return -1;
}

static inline uint8_t stbi__zget8(struct stbi__stream *stream)
{
    if (stream->next_in == stream->end_in)
        stream->refill(stream);
    return *stream->next_in++;
}

static void stbi__fill_bits(stbi__zbuf *z)
{
   do {
      z->code_buffer |= (unsigned int) stbi__zget8(z) << z->num_bits;
      z->num_bits += 8;
   } while (z->num_bits <= 24);
}

static inline unsigned int stbi__zreceive(stbi__zbuf *z, int n)
{
   unsigned int k;
   if (z->num_bits < n) stbi__fill_bits(z);
   k = z->code_buffer & ((1 << n) - 1);
   z->code_buffer >>= n;
   z->num_bits -= n;
   return k;
}

static int stbi__zhuffman_decode_slowpath(stbi__zbuf *a, struct stbi__zhuffman *z)
{
   int b,s,k;
   // not resolved by fast table, so compute it the slow way
   // use jpeg approach, which requires MSbits at top
   k = stbi__bit_reverse(a->code_buffer, 16);
   for (s=STBI__ZFAST_BITS+1; ; ++s)
      if (k < z->maxcode[s])
         break;
   if (s == 16) return -1; // invalid code!
   // code size is s, so:
   b = (k >> (16-s)) - z->firstcode[s] + z->firstsymbol[s];
   a->code_buffer >>= s;
   a->num_bits -= s;
   return z->value[b];
}

static inline int stbi__zhuffman_decode(stbi__zbuf *a, struct stbi__zhuffman *z)
{
   int b,s;
   if (a->num_bits < 16) stbi__fill_bits(a);
   b = z->fast[a->code_buffer & STBI__ZFAST_MASK];
   if (b) {
      s = b >> 9;
      a->code_buffer >>= s;
      a->num_bits -= s;
      return b & 511;
   }
   return stbi__zhuffman_decode_slowpath(a, z);
}

static int stbi__zlength_base[31] = {
   3,4,5,6,7,8,9,10,11,13,
   15,17,19,23,27,31,35,43,51,59,
   67,83,99,115,131,163,195,227,258,0,0 };

static int stbi__zlength_extra[31]=
{ 0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0,0,0 };

static int stbi__zdist_base[32] = { 1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,
257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577,0,0};

static int stbi__zdist_extra[32] =
{ 0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};

static int stbi__parse_huffman_block(stbi__zbuf *a)
{
   uint8_t *zout = a->next_out;
   for(;;) {
      int z = stbi__zhuffman_decode(a, &a->z_length);
      if (z < 256) {
         if (z < 0) return STBI_ZERROR("bad huffman code"); // error in huffman codes
         if (zout == a->end_out) {
             a->next_out = zout;
             if (!a->flush || a->flush(a))
                 return 0;
             zout = a->next_out;
         }
         *zout++ = (char) z;
         //a->window[a->total_out++ % (1 << 15)] = (char)z;
         //if (a->total_out % (1 << 15) == 0)
         //    fwrite(a->window, sizeof(a->window), 1, stdout);
      } else {
         //uint8_t *p;
         int len,dist;
         if (z == 256) {
            a->next_out = zout;
            return 1;
         }
         z -= 257;
         len = stbi__zlength_base[z];
         if (stbi__zlength_extra[z]) len += stbi__zreceive(a, stbi__zlength_extra[z]);
         z = stbi__zhuffman_decode(a, &a->z_distance);
         if (z < 0) return STBI_ZERROR("bad huffman code");
         dist = stbi__zdist_base[z];
         if (stbi__zdist_extra[z]) dist += stbi__zreceive(a, stbi__zdist_extra[z]);
         //if (zout - a->zout_start < dist) return STBI_ZERROR("bad dist");
         //if (zout + len > a->zout_end) {
         //   if (!stbi__zexpand(a, zout, len)) return 0;
         //   zout = a->zout;
         //}
         //p = (uint8_t *) (zout - dist);
         //if (dist == 1) { // run of one byte; common in images.
         //   uint8_t v = *p;
         //   if (len) { do *zout++ = v; while (--len); }
         //} else {
         //   if (len) { do *zout++ = *p++; while (--len); }
         //}
         if (len) {
            uint8_t *src = zout - dist;
            if (src < a->start_out)
                src += a->end_out - a->start_out; 
            do {
                if (src == a->end_out)
                    src = a->start_out;
                if (zout == a->end_out){
                    a->next_out = zout;
                    if (!a->flush(a))
						return 0;
                    zout = a->next_out;
                }
                *zout++ = *src++;
            } while (--len);

            ////zout += len;
            //size_t x = a->total_out - dist;
            //do {
            //    a->window[a->total_out++ % (1 << 15)] = a->window[x++ % (1 << 15)];
            //    //a->window[a->total_out % (1 << 15)] = a->window[(a->total_out - dist) % (1 << 15)];
            //    if (a->total_out % (1 << 15) == 0)
            //        fwrite(a->window, sizeof(a->window), 1, stdout);
            //} while (--len);
         }
      }
   }
}

static int stbi__compute_huffman_codes(stbi__zbuf *a)
{
   static uint8_t length_dezigzag[19] = { 16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15 };
   struct stbi__zhuffman z_codelength;
   uint8_t lencodes[286+32+137];//padding for maximum single op
   uint8_t codelength_sizes[19];
   int i,n;

   int hlit  = stbi__zreceive(a,5) + 257;
   int hdist = stbi__zreceive(a,5) + 1;
   int hclen = stbi__zreceive(a,4) + 4;

   memset(codelength_sizes, 0, sizeof(codelength_sizes));
   for (i=0; i < hclen; ++i) {
      int s = stbi__zreceive(a,3);
      codelength_sizes[length_dezigzag[i]] = (uint8_t) s;
   }
   if (!stbi__zbuild_huffman(&z_codelength, codelength_sizes, 19)) return 0;

   n = 0;
   while (n < hlit + hdist) {
      int c = stbi__zhuffman_decode(a, &z_codelength);
      if (c < 0 || c >= 19) return STBI_ZERROR("bad codelengths");
      if (c < 16)
         lencodes[n++] = (uint8_t) c;
      else if (c == 16) {
         c = stbi__zreceive(a,2)+3;
         memset(lencodes+n, lencodes[n-1], c);
         n += c;
      } else if (c == 17) {
         c = stbi__zreceive(a,3)+3;
         memset(lencodes+n, 0, c);
         n += c;
      } else {
         c = stbi__zreceive(a,7)+11;
         memset(lencodes+n, 0, c);
         n += c;
      }
   }
   if (n != hlit+hdist) return STBI_ZERROR("bad codelengths");
   if (!stbi__zbuild_huffman(&a->z_length, lencodes, hlit)) return 0;
   if (!stbi__zbuild_huffman(&a->z_distance, lencodes+hlit, hdist)) return 0;
   return 1;
}

static int stbi__parse_uncompressed_block(stbi__zbuf *a)
{
   uint8_t header[4];
   int len,nlen,k;
   if (a->num_bits & 7)
      stbi__zreceive(a, a->num_bits & 7); // discard
   // drain the bit-packed data into header
   k = 0;
   while (a->num_bits > 0) {
      header[k++] = (uint8_t) (a->code_buffer & 255); // suppress MSVC run-time check
      a->code_buffer >>= 8;
      a->num_bits -= 8;
   }
   // now fill header the normal way
   while (k < 4)
      header[k++] = stbi__zget8(a);
   len  = header[1] * 256 + header[0];
   nlen = header[3] * 256 + header[2];
   if (nlen != (len ^ 0xffff)) return STBI_ZERROR("zlib corrupt");
#if 1
   do {
       size_t avail_out = a->end_out - a->next_out;
       while (avail_out-- && len--)
           *a->next_out++ = stbi__zget8(a);
       if (len > 0) {
		   if (!a->flush || a->flush(a))
			   return 0;
	   }
   } while (len > 0);

   //while (len--) {
   //    a->window[a->total_out++ % (1 << 15)] = stbi__zget8(a);
   //    if (a->total_out % (1 << 15) == 0)
   //        fwrite(a->window, sizeof(a->window), 1, stdout);
   //}
#else
   memcpy(a->zout, a->zbuffer, len);
   a->zbuffer += len;
   a->zout += len;
#endif
   return 1;
}

// @TODO: should statically initialize these for optimal thread safety
static uint8_t stbi__zdefault_length[288], stbi__zdefault_distance[32];
static void stbi__init_zdefaults(void)
{
   int i;   // use <= to match clearly with spec
   for (i=0; i <= 143; ++i)     stbi__zdefault_length[i]   = 8;
   for (   ; i <= 255; ++i)     stbi__zdefault_length[i]   = 9;
   for (   ; i <= 279; ++i)     stbi__zdefault_length[i]   = 7;
   for (   ; i <= 287; ++i)     stbi__zdefault_length[i]   = 8;

   for (i=0; i <=  31; ++i)     stbi__zdefault_distance[i] = 5;
}

int stb_inflate(struct stbi__stream *a)
{
   int final, type;
   a->num_bits = 0;
   a->code_buffer = 0;
   a->total_out = 0;
   do {
      final = stbi__zreceive(a,1);
      type = stbi__zreceive(a,2);
      if (type == 0) {
         if (!stbi__parse_uncompressed_block(a)) return 0;
      } else if (type == 3) {
         return 0;
      } else {
         if (type == 1) {
            // use fixed code lengths
            if (!stbi__zdefault_distance[31]) stbi__init_zdefaults();
            if (!stbi__zbuild_huffman(&a->z_length  , stbi__zdefault_length  , 288)) return 0;
            if (!stbi__zbuild_huffman(&a->z_distance, stbi__zdefault_distance,  32)) return 0;
         } else {
            if (!stbi__compute_huffman_codes(a)) return 0;
         }
         if (!stbi__parse_huffman_block(a)) return 0;
      }
   } while (!final);
   if (a->flush)
	   a->flush(a);
   return 1;
}
