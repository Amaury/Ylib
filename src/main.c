#include "y.h"

#if 0
typedef struct s_s {
	int i;
	char c;
} s_t;
ystatus_t func(size_t index, void *data, void *user_data) {
	yarray_t *a = user_data;
	s_t *d = data;
	printf("%ld/%ld '%d' '%c'\n", index, yarray_length(*a), d->i, d->c);
	return (YENOERR);
}
int main(int argc, char **argv) {
	s_t data = {.i = 0, .c = 'a'};
	yarray_t a = yarray_new();
	for (int i = 0; i < 10; ++i) {
		data.i++;
		data.c++;
		s_t *data2 = malloc0(sizeof(s_t));
		data2->i = data.i;
		data2->c = data.c;
		yarray_push(&a, data2);
	}
	yarray_foreach(a, func, &a);
}
#endif
#if 0
void func(size_t index, void *data, void *user_data) {
	yvector_t *v = user_data;
	printf("%ld/%ld: '%s'\n", index, v->count, (char*)data);
}

int main(int argc, char **argv) {
	if (argc < 2) {
		printf("ERROR\n");
		return (1);
	}
	yvector_t v;
	yvector_init(&v);
	for (int i = 1; i < argc; ++i) {
		yvector_push(&v, argv[i]);
	}
	yvector_foreach(&v, func, &v);
}
#endif
#if 1
int main(int argc, char **argv) {
	yjson_parser_t *json;
	yvar_t val;

	if (argc != 2 && argc != 3) {
		printf("ERROR bad entry\n");
		return (1);
	}
	json = yjson_new(argv[1]);
	val = yjson_parse(json);
	if (json->status != YENOERR) {
		printf("JSON error line '%d'\n", json->line);
		return (2);
	}
	printf("JSON type value : '%s'\n",
	       yvar_isnull(&val) ? "null" :
	       yvar_isbool(&val) ? "bool" :
	       yvar_isint(&val) ? "int" :
	       yvar_isfloat(&val) ? "float" :
	       yvar_isstring(&val) ? "string" :
	       yvar_isarray(&val) ? "array" :
	       yvar_ishashmap(&val) ? "object" : "undef");
	yjson_print(&val);

	if (argc == 3) {
		const char *path = argv[2];
		yvar_t *result = yvar_get_from_path(&val, path);
		yjson_print(result);
	}

	yjson_free(json);
}
#endif
#if 0
void show_attribute(size_t index, void *elem, void *data);

void main(int argc, char **argv) {
	ydom_t *dom;
	ydom_node_t *data, *elem;

	ylog_init(YLOG_STDERR, argv[0]);
	if (argc > 1 && !strcmp(argv[1], "-d"))
		ylog_threshold(YLOG_DEBUG);
	dom = ydom_new();
#if 1
	ydom_read_file(dom, "test.xml");
#else
	{
		ydom_node_t *root, *listen, *threads;

		root = ydom_add_elem(dom, "finedb-server");
		listen = ydom_node_add_elem(root, "listen");
		ydom_node_add_attr(listen, "port", "11137");
		ydom_node_add_attr(listen, "interface", "*");
		threads = ydom_node_add_elem(root, "threads");
		ydom_node_add_attr(threads, "start", "5");
		ydom_node_add_attr(threads, "max", "50");
	}
#endif
	ydom_write(dom, stdout);
	printf("===========================\n");
	{
		int port;
		char *interface;
		yvect_t list;

		list = ydom_xpath(dom, "/finedb-server/listen/@port");
		if (yv_len(list))
			port = atoi(((ydom_node_t*)list[0])->value);
		list = ydom_xpath(dom, "/finedb-server/listen/@interface");
		if (yv_len(list))
			interface = strdup(((ydom_node_t*)list[0])->value);
		printf("PORT='%d'\nIFACE='%s'\n", port, interface);
		//yv_foreach(list, show_attribute, NULL);
	}
	ydom_free(dom);
}

void show_attribute(size_t index, void *elem, void *data) {
	ydom_node_t *node = elem;
	printf("> '%s'\n", node->value);
}
#endif
