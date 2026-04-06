
#define BIT(v, idx)       (((v) >> (idx)) & 1)
#define BITS(v, idx, n)   (((v) >> (idx)) & ((1<<(n))-1))
#define FETCH16(mem)      (((mem)[0] << 8) | (mem)[1])

#define FORCE_INLINE      __attribute__((always_inline))

