/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on, or a modified version of code from TinyGL (C) 1997-1998 Fabrice Bellard,
 * which is licensed under the zlib-license (see LICENSE).
 * It also has modifications by the ResidualVM-team, which are covered under the GPLv2 (or later).
 */

#include "graphics/tinygl/zgl.h"

namespace TinyGL {

void glopMaterial(GLContext *c, GLParam *p) {
	int mode = p[1].i;
	int type = p[2].i;
	Vector4 v(p[3].f, p[4].f, p[5].f, p[6].f);
	GLMaterial *m;

	if (mode == TGL_FRONT_AND_BACK) {
		p[1].i = TGL_FRONT;
		glopMaterial(c, p);
		mode = TGL_BACK;
	}
	if (mode == TGL_FRONT)
		m = &c->materials[0];
	else
		m = &c->materials[1];

	switch (type) {
	case TGL_EMISSION:
		m->emission = v;
		break;
	case TGL_AMBIENT:
		m->ambient = v;
		break;
	case TGL_DIFFUSE:
		m->diffuse = v;
		break;
	case TGL_SPECULAR:
		m->specular = v;
		m->has_specular = v.X != 0 || v.Y != 0 || v.Z != 0 || v.W != 1;
		break;
	case TGL_SHININESS:
		m->shininess = v.X;
		m->shininess_i = (int)(v.X / 128.0f) * SPECULAR_BUFFER_RESOLUTION;
		break;
	case TGL_AMBIENT_AND_DIFFUSE:
		m->diffuse = v;
		m->ambient = v;
		break;
	default:
		assert(0);
	}
}

void glopColorMaterial(GLContext *c, GLParam *p) {
	int mode = p[1].i;
	int type = p[2].i;

	c->current_color_material_mode = mode;
	c->current_color_material_type = type;
}

void glopLight(GLContext *c, GLParam *p) {
	int light = p[1].i;
	int type = p[2].i;
	Vector4 v(p[3].f, p[4].f, p[5].f, p[6].f);
	GLLight *l;

	assert(light >= TGL_LIGHT0 && light < TGL_LIGHT0 + T_MAX_LIGHTS);

	l = &c->lights[light - TGL_LIGHT0];

	switch (type) {
	case TGL_AMBIENT:
		l->ambient = v;
		break;
	case TGL_DIFFUSE:
		l->diffuse = v;
		break;
	case TGL_SPECULAR:
		l->specular = v;
		l->has_specular = v.X != 0 || v.Y != 0 || v.Z != 0 || v.W != 1;
		break;
	case TGL_POSITION: {
		Vector4 pos;
		c->matrix_stack_ptr[0]->transform(v, pos);

		l->position = pos;

		if (l->position.W == 0) {
			l->norm_position.X = pos.X;
			l->norm_position.Y = pos.Y;
			l->norm_position.Z = pos.Z;
			l->norm_position.normalize();
		}
	}
	break;
	case TGL_SPOT_DIRECTION:
		l->spot_direction.X = v.X;
		l->spot_direction.Y = v.Y;
		l->spot_direction.Z = v.Z;
		c->matrix_stack_ptr[0]->transform3x3(l->spot_direction, l->norm_spot_direction);
		l->norm_spot_direction.normalize();
		break;
	case TGL_SPOT_EXPONENT:
		l->spot_exponent = v.X;
		break;
	case TGL_SPOT_CUTOFF: {
		float a = v.X;
		assert(a == 180 || (a >= 0 && a <= 90));
		l->spot_cutoff = a;
		if (a != 180)
			l->cos_spot_cutoff = (float)(cos(a * (float)M_PI / 180.0));
	}
	break;
	case TGL_CONSTANT_ATTENUATION:
		l->attenuation[0] = v.X;
		break;
	case TGL_LINEAR_ATTENUATION:
		l->attenuation[1] = v.X;
		break;
	case TGL_QUADRATIC_ATTENUATION:
		l->attenuation[2] = v.X;
		break;
	default:
		assert(0);
	}
}

void glopLightModel(GLContext *c, GLParam *p) {
	int pname = p[1].i;

	switch (pname) {
	case TGL_LIGHT_MODEL_AMBIENT:
		c->ambient_light_model = Vector4(p[2].f, p[3].f, p[4].f, p[5].f);
		break;
	case TGL_LIGHT_MODEL_LOCAL_VIEWER:
		c->local_light_model = (int)p[2].f;
		break;
	case TGL_LIGHT_MODEL_TWO_SIDE:
		c->light_model_two_side = (int)p[2].f;
		break;
	default:
		warning("glopLightModel: illegal pname: 0x%x", pname);
		break;
	}
}


static inline float clampf(float a, float min, float max) {
	if (a < min)
		return min;
	else if (a > max)
		return max;
	else
		return a;
}

void gl_enable_disable_light(GLContext *c, int light, int v) {
	GLLight *l = &c->lights[light];
	if (v && !l->enabled) {
		l->enabled = 1;
		if (c->first_light != l) {
			l->next = c->first_light;
			if (c->first_light)
				c->first_light->prev = l;
			c->first_light = l;
			l->prev = NULL;
		}
	} else if (!v && l->enabled) {
		l->enabled = 0;
		if (!l->prev)
			c->first_light = l->next;
		else
			l->prev->next = l->next;
		if (l->next)
			l->next->prev = l->prev;
	}
}

// non optimized lightening model
void gl_shade_vertex(GLContext *c, GLVertex *v) {
	float R, G, B, A;
	GLMaterial *m;
	GLLight *l;
	Vector3 n, s, d;
	float dist, tmp, att, dot, dot_spot, dot_spec;
	int twoside = c->light_model_two_side;

	m = &c->materials[0];

	n = v->normal;

	R = m->emission.X + m->ambient.X * c->ambient_light_model.X;
	G = m->emission.Y + m->ambient.Y * c->ambient_light_model.Y;
	B = m->emission.Z + m->ambient.Z * c->ambient_light_model.Z;
	A = clampf(m->diffuse.W, 0, 1);

	for (l = c->first_light; l != NULL; l = l->next) {
		float lR, lB, lG;

		// ambient
		lR = l->ambient.X * m->ambient.X;
		lG = l->ambient.Y * m->ambient.Y;
		lB = l->ambient.Z * m->ambient.Z;

		if (l->position.W == 0) {
			// light at infinity
			d.X = l->norm_position.X;
			d.Y = l->norm_position.Y;
			d.Z = l->norm_position.Z;
			dist = 1;
			att = 1;
		} else {
			// distance attenuation
			d.X = l->position.X - v->ec.X;
			d.Y = l->position.Y - v->ec.Y;
			d.Z = l->position.Z - v->ec.Z;
			dist = sqrt(d.X * d.X + d.Y * d.Y + d.Z * d.Z);
			att = 1.0f / (l->attenuation[0] + dist * (l->attenuation[1] +
			              dist * l->attenuation[2]));
		}
		dot = d.X * n.X + d.Y * n.Y + d.Z * n.Z;
		if (twoside && dot < 0)
			dot = -dot;
		if (dot > 0) {
			tmp = 1 / dist;
			d *= tmp;
			dot *= tmp;
			// diffuse light
			lR += dot * l->diffuse.X * m->diffuse.X;
			lG += dot * l->diffuse.Y * m->diffuse.Y;
			lB += dot * l->diffuse.Z * m->diffuse.Z;

			const bool is_spotlight = l->spot_cutoff != 180;
			const bool has_specular = l->has_specular && m->has_specular;
			if (is_spotlight || has_specular) {
				if (is_spotlight) {
					dot_spot = -(d.X * l->norm_spot_direction.X +
								 d.Y * l->norm_spot_direction.Y +
								 d.Z * l->norm_spot_direction.Z);
					if (twoside && dot_spot < 0)
						dot_spot = -dot_spot;
					if (dot_spot < l->cos_spot_cutoff) {
						// no contribution
						continue;
					} else {
						// TODO: optimize
						if (l->spot_exponent > 0) {
							att = att * pow(dot_spot, l->spot_exponent);
						}
					}
				}

				if (has_specular) {
					if (c->local_light_model) {
						Vector3 vcoord;
						vcoord.X = v->ec.X;
						vcoord.Y = v->ec.Y;
						vcoord.Z = v->ec.Z;
						vcoord.normalize();
						s.X = d.X - vcoord.X;
						s.Y = d.Y - vcoord.Y;
						s.Z = d.Z - vcoord.Z;
					} else {
						s.X = d.X;
						s.Y = d.Y;
						s.Z = (float)(d.Z + 1.0);
					}
					dot_spec = n.X * s.X + n.Y * s.Y + n.Z * s.Z;
					if (twoside && dot_spec < 0)
						dot_spec = -dot_spec;
					if (dot_spec > 0) {
						GLSpecBuf *specbuf;
						int idx;
						dot_spec = dot_spec / sqrt(s.X * s.X + s.Y * s.Y + s.Z * s.Z);
						// TODO: optimize
						// testing specular buffer code
						// dot_spec= pow(dot_spec,m->shininess)
						specbuf = specbuf_get_buffer(c, m->shininess_i, m->shininess);
						tmp = dot_spec * SPECULAR_BUFFER_SIZE;
						if (tmp > SPECULAR_BUFFER_SIZE)
							idx = SPECULAR_BUFFER_SIZE;
						else
							idx = (int)tmp;

						dot_spec = specbuf->buf[idx];
						lR += dot_spec * l->specular.X * m->specular.X;
						lG += dot_spec * l->specular.Y * m->specular.Y;
						lB += dot_spec * l->specular.Z * m->specular.Z;
					}
				}
			}
		}

		R += att * lR;
		G += att * lG;
		B += att * lB;
	}

	v->color.X = clampf(c->current_color.X * R, 0, 1);
	v->color.Y = clampf(c->current_color.Y * G, 0, 1);
	v->color.Z = clampf(c->current_color.Z * B, 0, 1);
	v->color.W = c->current_color.W * A;
}

} // end of namespace TinyGL
