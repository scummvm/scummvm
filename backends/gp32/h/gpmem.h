
#ifndef __GPMEM_H__
#define	 __GPMEM_H__ 

typedef struct tagGM_HEAP_DEF{
	void * heapstart;
	void * heapend;
        } GM_HEAP_DEF;

struct tagGPMEMFUNC{
	void * (*malloc)(unsigned int size);
	void * (*zimalloc)(unsigned int size);
	void * (*calloc)(int count, unsigned int size);
	void (*free)(void * pt);
	unsigned int (*availablemem)(void);
	void * (*malloc_ex)(unsigned int size, int ex_flag, unsigned char init_val);
	void (*free_ex)(void);
	void * (*make_mem_partition)(struct tagGPMEMFUNC * p_mem_sub, unsigned int size, int * err_no);
        };
typedef struct tagGPMEMFUNC GPMEMFUNC;

typedef struct tagGPSTRFUNC{
	void (*memset)(void * ptr, unsigned char val, unsigned int size);
	void * (*memcpy)(void * s1, const void * s2, unsigned int size);
	char * (*strcpy)(char * s1, const char * s2);
	char * (*strncpy)(char * s1, const char * s2, unsigned int size); 
	char * (*strcat)(char * s1, const char * s2);
	char * (*strncat)(char * s1, const char * s2, unsigned int size);
	int (*gpstrlen)(const char * s);
	int (*sprintf)(char * buf, const char * fmt, ...);
	void (*uppercase)(char *ptr, int count);
	void (*lowercase)(char *ptr, int count);
	int (*compare)(const char *pt1, const char *pt2);
	void (*trim_right)(const char *ptr);
} GPSTRFUNC;

void gm_heap_init(GM_HEAP_DEF * p_def);

extern void * gm_malloc(unsigned int size);
extern void * gm_zi_malloc(unsigned int size);
#define MALLOC_EX_AUTOFREE		0x1
#define MALLOC_EX_MEMSET		0x2
extern void * gm_malloc_ex(unsigned int size, int ex_flag, unsigned char init_val);
extern void * gm_calloc(int count, unsigned int size);
extern void gm_free(void * pt);
extern void gm_free_ex(void);
extern unsigned int gm_availablesize(void);
extern void * gm_make_mem_part(GPMEMFUNC * p_mem_sub, unsigned int size, int * err_no);

extern void gm_memset(void * /*ptr*/, unsigned char /*val*/, unsigned int /*size*/);
extern void * gm_memcpy(void * /*s1*/, const void * /*s2*/, unsigned int /*size*/);
extern char * gm_strcpy(char * /*s1*/, const char * /*s2*/);
extern char * gm_strncpy(char * /*s1*/, const char * /*s2*/, unsigned int /*size*/);
extern char * gm_strcat(char * /*s1*/, const char * /*s2*/);
extern char * gm_strncat(char * /*s1*/, const char * /*s2*/, unsigned int /*size*/);
extern int gm_lstrlen(const char * /*s*/);
extern int gm_sprintf(char * buf, const char * fmt, ...);
extern void gm_lowercase(char * ptr, int count);
extern void gm_uppercase(char * ptr, int count);
extern int gm_compare(const char * pt1, const char * pt2);
extern void gm_trim_right(const char * ptr);

#endif /*__GPMEM_H__*/


