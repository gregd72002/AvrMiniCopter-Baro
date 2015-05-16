#ifndef AVERAGE_H
#define AVERAGE_H

struct s_average {
	int max_len;
	int ptr;
	bool ok;
	int *v;
};

void average_init(struct s_average *s, int len);

void average_destroy(struct s_average *s);

void average_add(struct s_average *s, int val);

float average_get(struct s_average *s);

#endif
