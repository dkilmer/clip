#ifndef CLIP_CLIP_H
#define CLIP_CLIP_H

#include "common.h"

typedef struct {
	t_tri *out;
	int ocnt;
	int oidx;
	t_pt *opts;
	int opcnt;
	int opidx;
} tri_buf;

int clip(t_tri t, t_pt pp, t_pt pnorm, tri_buf *tb);

#endif //CLIP_CLIP_H
