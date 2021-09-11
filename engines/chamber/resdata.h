#ifndef _RESDATA_H_
#define _RESDATA_H_

typedef struct ResEntry_t {
char	name[5 + 1 + 3 + 1];
void    *buffer;
} ResEntry_t;

/* Max resource file size among all languages */
#define RES_ALEAT_MAX 256
#define RES_ANICO_MAX 667
#define RES_ANIMA_MAX 2046
#define RES_ARPLA_MAX 7910
#define RES_CARPC_MAX 384
#define RES_GAUSS_MAX 1449
#define RES_ICONE_MAX 2756
#define RES_LUTIN_MAX 2800
#define RES_MURSM_MAX 76
#define RES_SOUCO_MAX 10838
#define RES_SOURI_MAX 1152
#define RES_TEMPL_MAX 27300
#define RES_ZONES_MAX 9014
#define RES_PUZZL_MAX 45671
#define RES_SPRIT_MAX 23811
#define RES_PERS1_MAX 14294
#define RES_PERS2_MAX 10489
#define RES_DESCI_MAX 10491
#define RES_DIALI_MAX 10070
#define RES_MOTSI_MAX 1082
#define RES_VEPCI_MAX 1548

extern unsigned char vepci_data[];
extern unsigned char motsi_data[];

extern unsigned char puzzl_data[];
extern unsigned char sprit_data[];

extern unsigned char pers1_data[];
extern unsigned char pers2_data[];

extern unsigned char desci_data[];
extern unsigned char diali_data[];

extern unsigned char arpla_data[];
extern unsigned char aleat_data[];
extern unsigned char carpc_data[];
extern unsigned char icone_data[];
extern unsigned char souco_data[];
extern unsigned char souri_data[];
extern unsigned char templ_data[];
extern unsigned char mursm_data[];
extern unsigned char gauss_data[];
extern unsigned char lutin_data[];
extern unsigned char anima_data[];
extern unsigned char anico_data[];
extern unsigned char zones_data[];

unsigned char * SeekToEntry(unsigned char *bank, unsigned int num, unsigned char **end);
unsigned char * SeekToEntryW(unsigned char *bank, unsigned int num, unsigned char **end);

unsigned int LoadFile(char *filename, unsigned char *buffer);
unsigned int SaveFile(char *filename, unsigned char *buffer, unsigned int size);
int LoadFilesList(ResEntry_t *entries);

int LoadStaticData(void);
int LoadFond(void);
int LoadSpritesData(void);
int LoadPersData(void);

extern ResEntry_t res_texts[];
int LoadVepciData(void);

extern ResEntry_t res_desci[];
int LoadDesciData(void);

extern ResEntry_t res_diali[];
int LoadDialiData(void);

#endif
