#include "tinygl/zgl.h"
#include <stdio.h>
/* glVertex */

void tglVertex4f(float x,float y,float z,float w)
{
  TGLParam p[5];

  p[0].op=OP_Vertex;
  p[1].f=x;
  p[2].f=y;
  p[3].f=z;
  p[4].f=w;

  gl_add_op(p);
}

void tglVertex2f(float x,float y) 
{
  tglVertex4f(x,y,0,1);
}

void tglVertex3f(float x,float y,float z) 
{
  tglVertex4f(x,y,z,1);
}

void tglVertex3fv(float *v) 
{
  tglVertex4f(v[0],v[1],v[2],1);
}

/* glNormal */

void tglNormal3f(float x,float y,float z)
{
  TGLParam p[4];

  p[0].op=OP_Normal;
  p[1].f=x;
  p[2].f=y;
  p[3].f=z;

  gl_add_op(p);
}

void tglNormal3fv(float *v) 
{
  tglNormal3f(v[0],v[1],v[2]);
}

/* glColor */

void tglColor4f(float r,float g,float b,float a)
{
  TGLParam p[8];

  p[0].op=OP_Color;
  p[1].f=r;
  p[2].f=g;
  p[3].f=b;
  p[4].f=a;
  /* direct convertion to integer to go faster if no shading */
  p[5].ui = (unsigned int) (r * (ZB_POINT_RED_MAX - ZB_POINT_RED_MIN) + 
                            ZB_POINT_RED_MIN);
  p[6].ui = (unsigned int) (g * (ZB_POINT_GREEN_MAX - ZB_POINT_GREEN_MIN) + 
                            ZB_POINT_GREEN_MIN);
  p[7].ui = (unsigned int) (b * (ZB_POINT_BLUE_MAX - ZB_POINT_BLUE_MIN) + 
                            ZB_POINT_BLUE_MIN);
  gl_add_op(p);
}

void tglColor4fv(float *v)
{
  TGLParam p[8];

  p[0].op=OP_Color;
  p[1].f=v[0];
  p[2].f=v[1];
  p[3].f=v[2];
  p[4].f=v[3];
  /* direct convertion to integer to go faster if no shading */
  p[5].ui = (unsigned int) (v[0] * (ZB_POINT_RED_MAX - ZB_POINT_RED_MIN) + 
                            ZB_POINT_RED_MIN);
  p[6].ui = (unsigned int) (v[1] * (ZB_POINT_GREEN_MAX - ZB_POINT_GREEN_MIN) + 
                            ZB_POINT_GREEN_MIN);
  p[7].ui = (unsigned int) (v[2] * (ZB_POINT_BLUE_MAX - ZB_POINT_BLUE_MIN) + 
                            ZB_POINT_BLUE_MIN);
  gl_add_op(p);
}

void tglColor3f(float x,float y,float z) 
{
  tglColor4f(x,y,z,1);
}

void glColor3fv(float *v) 
{
  tglColor4f(v[0],v[1],v[2],1);
}


/* TexCoord */

void tglTexCoord4f(float s,float t,float r,float q)
{
  TGLParam p[5];

  p[0].op=OP_TexCoord;
  p[1].f=s;
  p[2].f=t;
  p[3].f=r;
  p[4].f=q;

  gl_add_op(p);
}

void tglTexCoord2f(float s,float t)
{
  tglTexCoord4f(s,t,0,1);
}

void tglTexCoord2fv(float *v)
{
  tglTexCoord4f(v[0],v[1],0,1);
}

void tglEdgeFlag(int flag)
{
  TGLParam p[2];

  p[0].op=OP_EdgeFlag;
  p[1].i=flag;

  gl_add_op(p);
}

/* misc */

void tglShadeModel(int mode)
{
  TGLParam p[2];

  assert(mode == TGL_FLAT || mode == TGL_SMOOTH);

  p[0].op=OP_ShadeModel;
  p[1].i=mode;

  gl_add_op(p);
}

void tglCullFace(int mode)
{
  TGLParam p[2];

  assert(mode == TGL_BACK || 
         mode == TGL_FRONT || 
         mode == TGL_FRONT_AND_BACK);

  p[0].op=OP_CullFace;
  p[1].i=mode;

  gl_add_op(p);
}

void tglFrontFace(int mode)
{
  TGLParam p[2];

  assert(mode == TGL_CCW || mode == TGL_CW);

  mode = (mode != TGL_CCW);

  p[0].op=OP_FrontFace;
  p[1].i=mode;

  gl_add_op(p);
}

void tglPolygonMode(int face,int mode)
{
  TGLParam p[3];

  assert(face == TGL_BACK || 
         face == TGL_FRONT || 
         face == TGL_FRONT_AND_BACK);
  assert(mode == TGL_POINT || mode == TGL_LINE || mode==TGL_FILL);

  p[0].op=OP_PolygonMode;
  p[1].i=face;
  p[2].i=mode;

  gl_add_op(p);
}


/* glEnable / glDisable */

void tglEnable(int cap)
{
  TGLParam p[3];

  p[0].op=OP_EnableDisable;
  p[1].i=cap;
  p[2].i=1;

  gl_add_op(p);
}

void tglDisable(int cap)
{
  TGLParam p[3];

  p[0].op=OP_EnableDisable;
  p[1].i=cap;
  p[2].i=0;

  gl_add_op(p);
}

/* glBegin / glEnd */

void tglBegin(int mode)
{
  TGLParam p[2];

  p[0].op=OP_Begin;
  p[1].i=mode;

  gl_add_op(p);
}

void tglEnd(void)
{
  TGLParam p[1];

  p[0].op=OP_End;

  gl_add_op(p);
}

/* matrix */

void tglMatrixMode(int mode)
{
  TGLParam p[2];

  p[0].op=OP_MatrixMode;
  p[1].i=mode;

  gl_add_op(p);
}

void tglLoadMatrixf(const float *m)
{
  TGLParam p[17];
  int i;

  p[0].op=OP_LoadMatrix;
  for(i=0;i<16;i++) p[i+1].f=m[i];

  gl_add_op(p);
}

void tglLoadIdentity(void)
{
  TGLParam p[1];

  p[0].op=OP_LoadIdentity;

  gl_add_op(p);
}

void tglMultMatrixf(const float *m)
{
  TGLParam p[17];
  int i;

  p[0].op=OP_MultMatrix;
  for(i=0;i<16;i++) p[i+1].f=m[i];

  gl_add_op(p);
}

void tglPushMatrix(void)
{
  TGLParam p[1];

  p[0].op=OP_PushMatrix;

  gl_add_op(p);
}

void tglPopMatrix(void)
{
  TGLParam p[1];

  p[0].op=OP_PopMatrix;

  gl_add_op(p);
}

void tglRotatef(float angle,float x,float y,float z)
{
  TGLParam p[5];

  p[0].op=OP_Rotate;
  p[1].f=angle;
  p[2].f=x;
  p[3].f=y;
  p[4].f=z;

  gl_add_op(p);
}

void tglTranslatef(float x,float y,float z)
{
  TGLParam p[4];

  p[0].op=OP_Translate;
  p[1].f=x;
  p[2].f=y;
  p[3].f=z;

  gl_add_op(p);
}

void tglScalef(float x,float y,float z)
{
  TGLParam p[4];

  p[0].op=OP_Scale;
  p[1].f=x;
  p[2].f=y;
  p[3].f=z;

  gl_add_op(p);
}


void tglViewport(int x,int y,int width,int height)
{
  TGLParam p[5];

  p[0].op=OP_Viewport;
  p[1].i=x;
  p[2].i=y;
  p[3].i=width;
  p[4].i=height;

  gl_add_op(p);
}

void tglFrustum(double left,double right,double bottom,double top,
               double nearv,double farv)
{
  TGLParam p[7];

  p[0].op=OP_Frustum;
  p[1].f=(float)left;
  p[2].f=(float)right;
  p[3].f=(float)bottom;
  p[4].f=(float)top;
  p[5].f=(float)nearv;
  p[6].f=(float)farv;

  gl_add_op(p);
}

/* lightening */

void tglMaterialfv(int mode,int type,float *v)
{
  TGLParam p[7];
  int i,n;

  assert(mode == TGL_FRONT  || mode == TGL_BACK || mode==TGL_FRONT_AND_BACK);

  p[0].op=OP_Material;
  p[1].i=mode;
  p[2].i=type;
  n=4;
  if (type == TGL_SHININESS) n=1;
  for(i=0;i<4;i++) p[3+i].f=v[i];
  for(i=n;i<4;i++) p[3+i].f=0;

  gl_add_op(p);
}

void tglMaterialf(int mode,int type,float v)
{
  TGLParam p[7];
  int i;

  p[0].op=OP_Material;
  p[1].i=mode;
  p[2].i=type;
  p[3].f=v;
  for(i=0;i<3;i++) p[4+i].f=0;

  gl_add_op(p);
}

void tglColorMaterial(int mode,int type)
{
  TGLParam p[3];

  p[0].op=OP_ColorMaterial;
  p[1].i=mode;
  p[2].i=type;

  gl_add_op(p);
}

void tglLightfv(int light,int type,float *v)
{
  TGLParam p[7];
  int i;

  p[0].op=OP_Light;
  p[1].i=light;
  p[2].i=type;
  /* TODO: 3 composants ? */
  for(i=0;i<4;i++) p[3+i].f=v[i];

  gl_add_op(p);
}


void tglLightf(int light,int type,float v)
{
  TGLParam p[7];
  int i;

  p[0].op=OP_Light;
  p[1].i=light;
  p[2].i=type;
  p[3].f=v;
  for(i=0;i<3;i++) p[4+i].f=0;

  gl_add_op(p);
}

void tglLightModeli(int pname,int param)
{
  TGLParam p[6];
  int i;

  p[0].op=OP_LightModel;
  p[1].i=pname;
  p[2].f=(float)param;
  for(i=0;i<4;i++) p[3+i].f=0;

  gl_add_op(p);
}

void tglLightModelfv(int pname,float *param)
{
  TGLParam p[6];
  int i;

  p[0].op=OP_LightModel;
  p[1].i=pname;
  for(i=0;i<4;i++) p[2+i].f=param[i];

  gl_add_op(p);
}

/* clear */

void tglClear(int mask)
{
  TGLParam p[2];

  p[0].op=OP_Clear;
  p[1].i=mask;

  gl_add_op(p);
}

void tglClearColor(float r,float g,float b,float a)
{
  TGLParam p[5];

  p[0].op=OP_ClearColor;
  p[1].f=r;
  p[2].f=g;
  p[3].f=b;
  p[4].f=a;

  gl_add_op(p);
}

void tglClearDepth(double depth)
{
  TGLParam p[2];

  p[0].op=OP_ClearDepth;
  p[1].f=(float)depth;

  gl_add_op(p);
}


/* textures */

void tglTexImage2D( int target, int level, int components,
                   int width, int height, int border,
                   int format, int type, void *pixels)
{
  TGLParam p[10];

  p[0].op=OP_TexImage2D;
  p[1].i=target;
  p[2].i=level;
  p[3].i=components;
  p[4].i=width;
  p[5].i=height;
  p[6].i=border;
  p[7].i=format;
  p[8].i=type;
  p[9].p=pixels;

  gl_add_op(p);
}


void tglBindTexture(int target,int texture)
{
  TGLParam p[3];

  p[0].op=OP_BindTexture;
  p[1].i=target;
  p[2].i=texture;

  gl_add_op(p);
}

void tglTexEnvi(int target,int pname,int param)
{
  TGLParam p[8];
  
  p[0].op=OP_TexEnv;
  p[1].i=target;
  p[2].i=pname;
  p[3].i=param;
  p[4].f=0;
  p[5].f=0;
  p[6].f=0;
  p[7].f=0;

  gl_add_op(p);
}

void tglTexParameteri(int target,int pname,int param)
{
  TGLParam p[8];
  
  p[0].op=OP_TexParameter;
  p[1].i=target;
  p[2].i=pname;
  p[3].i=param;
  p[4].f=0;
  p[5].f=0;
  p[6].f=0;
  p[7].f=0;

  gl_add_op(p);
}

void tglPixelStorei(int pname,int param)
{
  TGLParam p[3];

  p[0].op=OP_PixelStore;
  p[1].i=pname;
  p[2].i=param;

  gl_add_op(p);
}

/* selection */

void tglInitNames(void)
{
  TGLParam p[1];

  p[0].op=OP_InitNames;

  gl_add_op(p);
}

void tglPushName(unsigned int name)
{
  TGLParam p[2];

  p[0].op=OP_PushName;
  p[1].i=name;

  gl_add_op(p);
}

void tglPopName(void)
{
  TGLParam p[1];

  p[0].op=OP_PopName;

  gl_add_op(p);
}

void tglLoadName(unsigned int name)
{
  TGLParam p[2];

  p[0].op=OP_LoadName;
  p[1].i=name;

  gl_add_op(p);
}

void 
tglPolygonOffset(TGLfloat factor, TGLfloat units)
{
  TGLParam p[3];
  p[0].op = OP_PolygonOffset;
  p[1].f = factor;
  p[2].f = units;
}

/* Special Functions */

void tglCallList(unsigned int list)
{
  TGLParam p[2];

  p[0].op=OP_CallList;
  p[1].i=list;

  gl_add_op(p);
}

void tglFlush(void)
{
  /* nothing to do */
}

void tglHint(int target,int mode)
{
  TGLParam p[3];

  p[0].op=OP_Hint;
  p[1].i=target;
  p[2].i=mode;

  gl_add_op(p);
}

/* Non standard functions */

void tglDebug(int mode)
{
  GLContext *c=gl_get_context();
  c->print_flag=mode;
}

