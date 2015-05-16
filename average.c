#include "average.h"
#include <stdlib.h>

void average_init(struct s_average *s, int len) {
	s->max_len = len;
	s->ok = false;
	s->ptr = 0;
	s->v = (int*)malloc(sizeof(int) * len);
}

void average_destroy(struct s_average *s) {
	free(s->v);
}

void average_add(struct s_average *s, int val) {
	s->v[s->ptr++] = val;
	if (s->ptr==s->max_len) {
		s->ok = true;
		s->ptr = 0;
	}
}

float average_get(struct s_average *s) {
	float ret = 0.f;
	if (!s->ok) return 0.f;

	for (int i=0;i<s->max_len;i++)
		ret += s->v[i];

	return (ret / (float)s->max_len);
}

