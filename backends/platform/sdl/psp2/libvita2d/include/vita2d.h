#ifndef VITA2D_H
#define VITA2D_H

#include <psp2/gxm.h>
#include <psp2/types.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/pgf.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RGBA8(r,g,b,a) ((((a)&0xFF)<<24) | (((b)&0xFF)<<16) | (((g)&0xFF)<<8) | (((r)&0xFF)<<0))

typedef struct vita2d_clear_vertex {
	float x;
	float y;
} vita2d_clear_vertex;

typedef struct vita2d_color_vertex {
	float x;
	float y;
	float z;
	unsigned int color;
} vita2d_color_vertex;

typedef struct vita2d_texture_vertex {
	float x;
	float y;
	float z;
	float w;
	float u;
	float v;
} vita2d_texture_vertex;

typedef struct vita2d_texture {
	SceGxmTexture gxm_tex;
	SceUID data_UID;
	SceUID palette_UID;
  SceGxmRenderTarget *gxm_rtgt;
  SceGxmColorSurface gxm_sfc;
	SceGxmDepthStencilSurface gxm_sfd;
	SceUID depth_UID;
} vita2d_texture;

typedef struct vita2d_shader_input{
	SceGxmProgramParameter * video_size;
	SceGxmProgramParameter * texture_size;
	SceGxmProgramParameter * output_size;
	SceGxmProgramParameter * frame_count;
	SceGxmProgramParameter * frame_direction;
	SceGxmProgramParameter * frame_rotation;
} vita2d_shader_input;

typedef struct vita2d_shader {
  SceGxmShaderPatcherId vertexProgramId;
  SceGxmShaderPatcherId fragmentProgramId;
  SceGxmVertexProgram *vertexProgram;
  SceGxmFragmentProgram *fragmentProgram;
  SceGxmProgramParameter * paramPositionAttribute;
  SceGxmProgramParameter * paramTexcoordAttribute;
  SceGxmProgramParameter * wvpParam;
	vita2d_shader_input	vertexInput;
	vita2d_shader_input fragmentInput;
} vita2d_shader;

typedef struct vita2d_system_pgf_config {
	SceFontLanguageCode code;
	int (*in_font_group)(unsigned int c);
} vita2d_system_pgf_config;

typedef struct vita2d_font vita2d_font;
typedef struct vita2d_pgf vita2d_pgf;

typedef enum vita2d_start_drawing_flags {
	VITA_2D_SCENE_FRAGMENT_SET_DEPENDENCY		= 0x00000001u,
	VITA_2D_SCENE_VERTEX_WAIT_FOR_DEPENDENCY	= 0x00000002u,
	VITA_2D_RESET_POOL
	= 0x00000010u
} vita2d_start_drawing_flags;

int vita2d_init();
int vita2d_init_advanced(unsigned int temp_pool_size);
void vita2d_wait_rendering_done();
int vita2d_fini();

void vita2d_clear_screen();
void vita2d_swap_buffers();

void vita2d_start_drawing();
void vita2d_start_drawing_advanced(vita2d_texture *target, vita2d_start_drawing_flags flags);
void vita2d_end_drawing();

int vita2d_common_dialog_update();

void vita2d_set_clear_color(unsigned int color);
void vita2d_set_vblank_wait(int enable);
void *vita2d_get_current_fb();

void vita2d_set_region_clip(SceGxmRegionClipMode mode, unsigned int x_min, unsigned int y_min, unsigned int x_max, unsigned int y_max);

void *vita2d_pool_malloc(unsigned int size);
void *vita2d_pool_memalign(unsigned int size, unsigned int alignment);
unsigned int vita2d_pool_free_space();
void vita2d_pool_reset();

void vita2d_draw_pixel(float x, float y, unsigned int color);
void vita2d_draw_line(float x0, float y0, float x1, float y1, unsigned int color);
void vita2d_draw_rectangle(float x, float y, float w, float h, unsigned int color);
void vita2d_draw_fill_circle(float x, float y, float radius, unsigned int color);

void vita2d_texture_set_alloc_memblock_type(SceKernelMemBlockType type);
SceKernelMemBlockType vita2d_texture_get_alloc_memblock_type();
vita2d_texture *vita2d_create_empty_texture(unsigned int w, unsigned int h);
vita2d_texture *vita2d_create_empty_texture_format(unsigned int w, unsigned int h, SceGxmTextureFormat format);
void vita2d_free_texture(vita2d_texture *texture);

unsigned int vita2d_texture_get_width(const vita2d_texture *texture);
unsigned int vita2d_texture_get_height(const vita2d_texture *texture);
unsigned int vita2d_texture_get_stride(const vita2d_texture *texture);
SceGxmTextureFormat vita2d_texture_get_format(const vita2d_texture *texture);
void *vita2d_texture_get_datap(const vita2d_texture *texture);
void *vita2d_texture_get_palette(const vita2d_texture *texture);
SceGxmTextureFilter vita2d_texture_get_min_filter(const vita2d_texture *texture);
SceGxmTextureFilter vita2d_texture_get_mag_filter(const vita2d_texture *texture);
void vita2d_texture_set_filters(vita2d_texture *texture, SceGxmTextureFilter min_filter, SceGxmTextureFilter mag_filter);

void vita2d_draw_texture(const vita2d_texture *texture, float x, float y);
void vita2d_draw_texture_rotate(const vita2d_texture *texture, float x, float y, float rad);
void vita2d_draw_texture_rotate_hotspot(const vita2d_texture *texture, float x, float y, float rad, float center_x, float center_y);
void vita2d_draw_texture_scale(const vita2d_texture *texture, float x, float y, float x_scale, float y_scale);
void vita2d_draw_texture_part(const vita2d_texture *texture, float x, float y, float tex_x, float tex_y, float tex_w, float tex_h);
void vita2d_draw_texture_part_scale(const vita2d_texture *texture, float x, float y, float tex_x, float tex_y, float tex_w, float tex_h, float x_scale, float y_scale);
void vita2d_draw_texture_scale_rotate_hotspot(const vita2d_texture *texture, float x, float y, float x_scale, float y_scale, float rad, float center_x, float center_y);
void vita2d_draw_texture_scale_rotate(const vita2d_texture *texture, float x, float y, float x_scale, float y_scale, float rad);
void vita2d_draw_texture_part_scale_rotate(const vita2d_texture *texture, float x, float y, float tex_x, float tex_y, float tex_w, float tex_h, float x_scale, float y_scale, float rad);

void vita2d_draw_texture_tint(const vita2d_texture *texture, float x, float y, unsigned int color);
void vita2d_draw_texture_tint_rotate(const vita2d_texture *texture, float x, float y, float rad, unsigned int color);
void vita2d_draw_texture_tint_rotate_hotspot(const vita2d_texture *texture, float x, float y, float rad, float center_x, float center_y, unsigned int color);
void vita2d_draw_texture_tint_scale(const vita2d_texture *texture, float x, float y, float x_scale, float y_scale, unsigned int color);
void vita2d_draw_texture_tint_part(const vita2d_texture *texture, float x, float y, float tex_x, float tex_y, float tex_w, float tex_h, unsigned int color);
void vita2d_draw_texture_tint_part_scale(const vita2d_texture *texture, float x, float y, float tex_x, float tex_y, float tex_w, float tex_h, float x_scale, float y_scale, unsigned int color);
void vita2d_draw_texture_tint_scale_rotate_hotspot(const vita2d_texture *texture, float x, float y, float x_scale, float y_scale, float rad, float center_x, float center_y, unsigned int color);
void vita2d_draw_texture_tint_scale_rotate(const vita2d_texture *texture, float x, float y, float x_scale, float y_scale, float rad, unsigned int color);
void vita2d_draw_texture_part_tint_scale_rotate(const vita2d_texture *texture, float x, float y, float tex_x, float tex_y, float tex_w, float tex_h, float x_scale, float y_scale, float rad, unsigned int color);

vita2d_texture *vita2d_load_PNG_file(const char *filename);
vita2d_texture *vita2d_load_PNG_buffer(const void *buffer);

vita2d_texture *vita2d_load_JPEG_file(const char *filename);
vita2d_texture *vita2d_load_JPEG_buffer(const void *buffer, unsigned long buffer_size);

vita2d_texture *vita2d_load_BMP_file(const char *filename);
vita2d_texture *vita2d_load_BMP_buffer(const void *buffer);

vita2d_font *vita2d_load_font_file(const char *filename);
vita2d_font *vita2d_load_font_mem(const void *buffer, unsigned int size);
void vita2d_free_font(vita2d_font *font);
int vita2d_font_draw_text(vita2d_font *font, int x, int y, unsigned int color, unsigned int size, const char *text);
int vita2d_font_draw_textf(vita2d_font *font, int x, int y, unsigned int color, unsigned int size, const char *text, ...);
void vita2d_font_text_dimensions(vita2d_font *font, unsigned int size, const char *text, int *width, int *height);
int vita2d_font_text_width(vita2d_font *font, unsigned int size, const char *text);
int vita2d_font_text_height(vita2d_font *font, unsigned int size, const char *text);

/* PGF functions are weak imports at the moment, they have to be resolved manually */
vita2d_pgf *vita2d_load_system_pgf(int numFonts, const vita2d_system_pgf_config *configs);
vita2d_pgf *vita2d_load_default_pgf();
vita2d_pgf *vita2d_load_custom_pgf(const char *path);
void vita2d_free_pgf(vita2d_pgf *font);
int vita2d_pgf_draw_text(vita2d_pgf *font, int x, int y, unsigned int color, float scale, const char *text);
int vita2d_pgf_draw_textf(vita2d_pgf *font, int x, int y, unsigned int color, float scale, const char *text, ...);
void vita2d_pgf_text_dimensions(vita2d_pgf *font, float scale, const char *text, int *width, int *height);
int vita2d_pgf_text_width(vita2d_pgf *font, float scale, const char *text);
int vita2d_pgf_text_height(vita2d_pgf *font, float scale, const char *text);

vita2d_shader *vita2d_create_shader(const SceGxmProgram* vertexProgramGxp, const SceGxmProgram* fragmentProgramGxp);
void vita2d_texture_set_program(SceGxmVertexProgram *vertexProgram, SceGxmFragmentProgram *fragmentProgram);
void vita2d_texture_set_wvp(SceGxmProgramParameter *wvpParam);
void vita2d_texture_set_vertexInput(vita2d_shader_input *input);
void vita2d_texture_set_fragmentInput(vita2d_shader_input *input);
void vita2d_free_shader(vita2d_shader *shader);

#ifdef __cplusplus
}
#endif

#endif
