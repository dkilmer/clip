#ifndef RENDER_UTIL_H
#define RENDER_UTIL_H

#include <glad/glad.h>
#include "common.h"


typedef struct {
	GLuint shader;
	GLint vp_unif;
	GLint light_unif;
	//GLuint tex;
	//GLint tex_unif;
	GLuint pos_attrib;
	GLuint clr_attrib;
	GLuint nrm_attrib;
	GLuint vao;
	GLuint vbo;
	//GLenum draw_type;
	int num_bufs;
	int num_items;
	int verts_per_item;
	int buf_idx;
	int item_idx;
	GLsync *fences;
	t_ctri *tris;
} render_def;

GLuint create_shader_program(const char *vert_file_name, const char *frag_file_name);
GLint load_texture_to_uniform(const char *filename, const char *unif_name, GLuint shaderProgram, GLuint *tex, GLenum tex_num, GLint tex_idx);
void alloc_buffers(render_def *rd);
void free_render_def(render_def *rd);
void setup_render_def(render_def *rd, const char *vertex_shader, const char *fragment_shader, GLfloat *vp_mat);
void render_advance(render_def *rd);
void render_tri(render_def *rd, t_tri *tri, t_clr *clr);
void render_buffer(render_def *rd);

#endif //RENDER_UTIL_H
