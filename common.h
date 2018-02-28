#ifndef CLIP_COMMON_H
#define CLIP_COMMON_H

#include <glad/glad.h>
#include "math_3d.h"

typedef vec3_t t_pt;

typedef struct {
	GLfloat r;
	GLfloat g;
	GLfloat b;
	GLfloat a;
} t_clr;

typedef struct {
	GLfloat x;
	GLfloat y;
	GLfloat z;
	GLfloat r;
	GLfloat g;
	GLfloat b;
	GLfloat a;
	GLfloat nx;
	GLfloat ny;
	GLfloat nz;
} t_cpt;

typedef struct {
	t_pt p[3];
} t_tri;

typedef struct {
	t_cpt p[3];
} t_ctri;

typedef struct {
	t_pt p[4];
} t_quad;

typedef struct {
	GLuint e[6];
} t_quad_el;

typedef struct {
	GLfloat x;
	GLfloat y;
} t_uv_pt;

typedef struct {
	t_uv_pt p[4];
} t_uv;


#endif //CLIP_COMMON_H
