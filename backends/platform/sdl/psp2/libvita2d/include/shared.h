#ifndef SHARED_H
#define SHARED_H

/* Shared with other .c */
extern float _vita2d_ortho_matrix[4*4];
extern SceGxmContext *_vita2d_context;
extern SceGxmVertexProgram *_vita2d_colorVertexProgram;
extern SceGxmFragmentProgram *_vita2d_colorFragmentProgram;
extern SceGxmVertexProgram *_vita2d_textureVertexProgram;
extern SceGxmFragmentProgram *_vita2d_textureFragmentProgram;
extern SceGxmFragmentProgram *_vita2d_textureTintFragmentProgram;
extern const SceGxmProgramParameter *_vita2d_colorWvpParam;
extern const SceGxmProgramParameter *_vita2d_textureWvpParam;
extern SceGxmProgramParameter *_vita2d_textureTintColorParam;

extern SceGxmVertexProgram * _vita2d_selectedTexVertexProgram;
extern SceGxmFragmentProgram * _vita2d_selectedTexFragmentProgram;
extern SceGxmProgramParameter *_vita2d_selectedTexWvpParam;
extern vita2d_shader_input *_vita2d_selectedVertexInput;
extern vita2d_shader_input *_vita2d_selectedFragmentInput;

#endif
