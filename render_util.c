#include "render_util.h"
#include "misc_util.h"
#include "common.h"
#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

GLuint create_shader_program(const char *vert_file_name, const char *frag_file_name) {
	const GLchar* vertex_shader = load_file(vert_file_name);
	const GLchar* fragment_shader = load_file(frag_file_name);

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertex_shader, NULL);
	glCompileShader(vertexShader);
	GLint status;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		char buffer[512];
		glGetShaderInfoLog(vertexShader, 512, NULL, buffer);
		printf("ERROR compiling the vertex shader: %s\n",buffer);
	}

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragment_shader, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		char buffer[512];
		glGetShaderInfoLog(fragmentShader, 512, NULL, buffer);
		printf("ERROR compiling the fragment shader: %s\n",buffer);
	}

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "outColor");
	glLinkProgram(shaderProgram);

	GLint isLinked = 0;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &isLinked);
	if(isLinked == GL_FALSE) {
		printf("ERROR: shader program failed to link\n");
		GLint maxLength = 0;
		glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &maxLength);
		char buffer[maxLength];
		glGetProgramInfoLog(shaderProgram, maxLength, &maxLength, buffer);
		printf("%s\n", buffer);
	}

	return shaderProgram;
}

GLint load_texture_to_uniform(const char *filename, const char *unif_name, GLuint shaderProgram, GLuint *tex, GLenum tex_num, GLint tex_idx) {
	GLenum err;
	//glUseProgram(shaderProgram);
	int tw,th,tn;
	unsigned char *image_data = stbi_load(filename, &tw, &th, &tn, 0);
	printf("image %s is %d x %d with %d components\n",filename, tw, th, tn);
	glGenTextures(1, tex);
	err = glGetError();
	if (err != GL_NO_ERROR) {
		printf("-=-= ERROR 1: %d\n", err);
	}

	glActiveTexture(tex_num);
	err = glGetError();
	if (err != GL_NO_ERROR) {
		printf("-=-= ERROR 2: %d\n", err);
	}
	glBindTexture(GL_TEXTURE_2D, *tex);
	err = glGetError();
	if (err != GL_NO_ERROR) {
		printf("-=-= ERROR 3: %d\n", err);
	}

	printf("TEXTURE ID is %d, tex_num is %d, tex_idx is %d\n", *tex, tex_num, tex_idx);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tw, th, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
	err = glGetError();
	if (err != GL_NO_ERROR) {
		printf("-=-= ERROR 4: %d\n", err);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	err = glGetError();
	if (err != GL_NO_ERROR) {
		printf("-=-= ERROR 5.1: %d\n", err);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	err = glGetError();
	if (err != GL_NO_ERROR) {
		printf("-=-= ERROR 5.2: %d\n", err);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	err = glGetError();
	if (err != GL_NO_ERROR) {
		printf("-=-= ERROR 5.2: %d\n", err);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	err = glGetError();
	if (err != GL_NO_ERROR) {
		printf("-=-= ERROR 5.3: %d\n", err);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	err = glGetError();
	if (err != GL_NO_ERROR) {
		printf("-=-= ERROR 5.4: %d\n", err);
	}
	glGenerateMipmap(GL_TEXTURE_2D);
	err = glGetError();
	if (err != GL_NO_ERROR) {
		printf("-=-= ERROR 5.5: %d\n", err);
	}
	stbi_image_free(image_data);
	err = glGetError();
	if (err != GL_NO_ERROR) {
		printf("-=-= ERROR 6: %d\n", err);
	}

	GLint texUnif;
	texUnif = glGetUniformLocation(shaderProgram, unif_name);
	glUniform1i(texUnif, tex_idx);
	err = glGetError();
	if (err != GL_NO_ERROR) {
		printf("-=-= ERROR 7: %d\n", err);
	}
	return texUnif;
}

void alloc_buffers(render_def *rd) {
	rd->tris = (t_ctri *)malloc(sizeof(t_ctri) * rd->num_items * rd->num_bufs);
}

void free_render_def(render_def *rd) {
	glDeleteBuffers(1, &rd->vbo);
	glDeleteVertexArrays(1, &rd->vao);
	if (rd->fences) free(rd->fences);
	glDeleteProgram(rd->shader);
	if (rd->num_bufs == 1) {
		free(rd->tris);
	}
}

void setup_render_def(render_def *rd, const char *vertex_shader, const char *fragment_shader, GLfloat *vp_mat) {
	GLenum err;
	rd->buf_idx = 0;
	rd->item_idx = 0;
	if (rd->num_bufs > 1) {
		rd->fences = (GLsync *)malloc(rd->num_bufs * sizeof(GLsync));
		for (int i=0; i<rd->num_bufs; i++) rd->fences[i] = NULL;
		rd->tris = NULL;
	}
	rd->shader = create_shader_program(vertex_shader, fragment_shader);
	glUseProgram(rd->shader);
	rd->vp_unif = glGetUniformLocation(rd->shader, "vp");
	glUniformMatrix4fv(rd->vp_unif, 1, GL_FALSE, vp_mat);

	glGenVertexArrays(1, &rd->vao);
	glGenBuffers(1, &rd->vbo);
	glBindVertexArray(rd->vao);

	// buffer for vertices
	glBindBuffer(GL_ARRAY_BUFFER, rd->vbo);
	glBufferData(GL_ARRAY_BUFFER, rd->num_items * rd->num_bufs * sizeof(t_ctri), NULL, GL_STREAM_DRAW);
	err = glGetError();
	if (err != GL_NO_ERROR) {
		printf("tri buffer data: %d\n", err);
	}

	// attribute for vertex position
	glBindBuffer(GL_ARRAY_BUFFER, rd->vbo);
	rd->pos_attrib = (GLuint)glGetAttribLocation(rd->shader, "position");
	glEnableVertexAttribArray(rd->pos_attrib);
	glVertexAttribPointer(rd->pos_attrib, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(GLfloat), 0);
	err = glGetError();
	if (err != GL_NO_ERROR) {
		printf("pos_attrib is %d: %d\n", rd->pos_attrib, err);
	}
	// attribute for color
	rd->clr_attrib = (GLuint)glGetAttribLocation(rd->shader, "color");
	glEnableVertexAttribArray(rd->clr_attrib);
	glVertexAttribPointer(rd->clr_attrib, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(GLfloat), (void*)(3* sizeof(GLfloat)));
	err = glGetError();
	if (err != GL_NO_ERROR) {
		printf("clr_attrib is %d: %d\n", rd->clr_attrib, err);
	}
	// attribute for normal
	rd->nrm_attrib = (GLuint)glGetAttribLocation(rd->shader, "normal");
	glEnableVertexAttribArray(rd->nrm_attrib);
	glVertexAttribPointer(rd->nrm_attrib, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(GLfloat), (void*)(7* sizeof(GLfloat)));
	err = glGetError();
	if (err != GL_NO_ERROR) {
		printf("nrm_attrib is %d: %d\n", rd->nrm_attrib, err);
	}
}

int init_render(render_def *rd) {
	// just starting a buffer. we need to wait and map on that shit
	if (rd->item_idx == 0) {
		glBindVertexArray(rd->vao);
		if (rd->fences[rd->buf_idx] != NULL) {
			GLenum state = glClientWaitSync(rd->fences[rd->buf_idx], GL_SYNC_FLUSH_COMMANDS_BIT, 1000000000);
			if (state == GL_TIMEOUT_EXPIRED || state == GL_WAIT_FAILED) {
				printf("wait for fence on buf_idx %d failed with error %d\n", rd->buf_idx, state);
			}
			glDeleteSync(rd->fences[rd->buf_idx]);
			rd->fences[rd->buf_idx] = NULL;
		}
		glBindBuffer(GL_ARRAY_BUFFER, rd->vbo);
		int buf_len = rd->num_items * sizeof(t_ctri);
		int map_start = rd->buf_idx * buf_len;
		rd->tris = (t_ctri *)glMapBufferRange(GL_ARRAY_BUFFER, map_start, buf_len, GL_MAP_WRITE_BIT|GL_MAP_UNSYNCHRONIZED_BIT);
		if (rd->tris == NULL) printf("failed to map tri buffer for buf_idx %d\n", rd->buf_idx);
	}
	if (rd->item_idx >= rd->num_items) {
		printf("can't render to buf_idx %d: overflow\n", rd->buf_idx);
		return -1;
	}
	return 1;
}

void render_advance(render_def *rd) {
	glBindVertexArray(rd->vao);
	glBindBuffer(GL_ARRAY_BUFFER, rd->vbo);
	glUnmapBuffer(GL_ARRAY_BUFFER);
	rd->fences[rd->buf_idx] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	rd->buf_idx = ((rd->buf_idx + 1) % rd->num_bufs);
	rd->item_idx = 0;
};

void render_tri(render_def *rd, t_tri *tri, t_clr *clr) {
	if (init_render(rd) < 0) return;
	t_pt nrm = v3_norm(v3_cross(v3_sub(tri->p[0], tri->p[1]), v3_sub(tri->p[2], tri->p[1])));
	int idx = rd->item_idx;
	rd->tris[idx].p[0].x = tri->p[0].x;
	rd->tris[idx].p[0].y = tri->p[0].y;
	rd->tris[idx].p[0].z = tri->p[0].z;
	rd->tris[idx].p[0].r = clr->r;
	rd->tris[idx].p[0].g = clr->g;
	rd->tris[idx].p[0].b = clr->b;
	rd->tris[idx].p[0].a = clr->a;
	rd->tris[idx].p[0].nx = nrm.x;
	rd->tris[idx].p[0].ny = nrm.y;
	rd->tris[idx].p[0].nz = nrm.z;

	rd->tris[idx].p[1].x = tri->p[1].x;
	rd->tris[idx].p[1].y = tri->p[1].y;
	rd->tris[idx].p[1].z = tri->p[1].z;
	rd->tris[idx].p[1].r = clr->r;
	rd->tris[idx].p[1].g = clr->g;
	rd->tris[idx].p[1].b = clr->b;
	rd->tris[idx].p[1].a = clr->a;
	rd->tris[idx].p[1].nx = nrm.x;
	rd->tris[idx].p[1].ny = nrm.y;
	rd->tris[idx].p[1].nz = nrm.z;

	rd->tris[idx].p[2].x = tri->p[2].x;
	rd->tris[idx].p[2].y = tri->p[2].y;
	rd->tris[idx].p[2].z = tri->p[2].z;
	rd->tris[idx].p[2].r = clr->r;
	rd->tris[idx].p[2].g = clr->g;
	rd->tris[idx].p[2].b = clr->b;
	rd->tris[idx].p[2].a = clr->a;
	rd->tris[idx].p[2].nx = nrm.x;
	rd->tris[idx].p[2].ny = nrm.y;
	rd->tris[idx].p[2].nz = nrm.z;
	rd->item_idx++;
}

void render_buffer(render_def *rd) {
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(rd->shader);
	glBindVertexArray(rd->vao);
	//glBindBuffer(GL_ARRAY_BUFFER, rd->vbo);
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, rd->tex);
	if (rd->item_idx > 0) {
		//printf("drawing %d items of %d from buffer %d\n", rd->item_idx, rd->num_items, rd->buf_idx);
		glDrawArrays(GL_TRIANGLES, rd->buf_idx * rd->num_items * rd->verts_per_item, rd->item_idx * rd->verts_per_item);
	}
}
