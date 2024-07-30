#include "y.h"

#if 0

#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

typedef struct {
	_Atomic int atomic_counter;
	int non_atomic_counter;
} toto_t;
toto_t toto;
void* main_thread(void *arg);
void action(toto_t *t);

void action(toto_t *t) {
	++t->non_atomic_counter;
	++t->atomic_counter;
}

void* main_thread(void *arg) {
	for (int i = 0; i < 1000; ++i) {
		//++toto.non_atomic_counter;
		//++toto.atomic_counter;
		action(&toto);
	}
	return NULL;
}

int main(int argc, char **argv) {
	pthread_t threads[10];
	for (int i = 0; i < 10; ++i)
		pthread_create(&threads[i], NULL, main_thread, NULL);
	for (int i = 0; i < 10; ++i)
		pthread_join(threads[i], NULL);
	printf("atomic     %d\n", toto.atomic_counter);
	printf("non-atomic %d\n", toto.non_atomic_counter);
	return (0);
}

#elif 0

#include <stdio.h>
#include <threads.h>
#include <stdatomic.h>

typedef struct {
	atomic_int atomic_counter;
	int non_atomic_counter;
} toto_t;
toto_t toto;
int mythread(void *thr_data);

int mythread(void *thr_data) {
    (void)thr_data;
    for(int n = 0; n < 1000; ++n) {
        ++toto.non_atomic_counter;
        ++toto.atomic_counter;
        // for this example, relaxed memory order is sufficient, e.g.
        // atomic_fetch_add_explicit(&atomic_counter, 1, memory_order_relaxed);
    }
    return 0;
}

int main(void) {
    thrd_t thr[10];
    for(int n = 0; n < 10; ++n)
        thrd_create(&thr[n], mythread, NULL);
    for(int n = 0; n < 10; ++n)
        thrd_join(thr[n], NULL);
    printf("atomic     %d\n", toto.atomic_counter);
    printf("non-atomic %d\n", toto.non_atomic_counter);
}

#elif 0
ystatus_t callback(uint64_t hash, char *key, void *data, void *user_data) {
	//printf("in callback [%lu] (key: %lx) (data: %lx)\n", hash, (long unsigned int)key, (long unsigned int)data);
	printf("[%lu/%s] : '%s'\n", hash, key, (char*)data);
	return (YENOERR);
}
int main() {
	ytable_t static_table;
	ytable_init(&static_table, NULL, NULL);
	ytable_t *yt = &static_table;//ytable_new();
	ytable_add(yt, "aaa");
	ytable_set_index(yt, 8, "bbb");
	ytable_add(yt, "ccc");
	ytable_push(yt, "zzz");
	ytable_push(yt, "yyy");
	printf("length: %u\n", ytable_length(yt));
	ystatus_t st = ytable_foreach(yt, callback, NULL);
	if (st == YENOERR) {
		printf("OK\n");
	} else {
		printf("KO\n");
	}

	ytable_set_index(yt, 123, "xxx");
	ytable_mpush(yt, 2, "www", "vvv");
	//ytable_add(yt, "www");
	//ytable_add(yt, "vvv");
	ytable_set_index(yt, 124, "uuu");
	ytable_set_key(yt, "bbb", "111");
	printf("length: %u\n", ytable_length(yt));
	st = ytable_foreach(yt, callback, NULL);
	if (st == YENOERR) {
		printf("OK\n");
	} else {
		printf("KO\n");
	}
}
#elif 0
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
#elif 0
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
#elif 1
int main(int argc, char **argv) {
	yjson_parser_t json;

	if (argc != 2 && argc != 3) {
		printf("ERROR bad entry\n");
		return (1);
	}
	yres_var_t res = yjson_parse(&json, argv[1]);
	YASSERT(res, "JSON error line '%d'\n", json.line);
	yvar_t val = YRES_VAL(res);
	printf("JSON type value : '%s'\n",
	       yvar_is_undef(&val) ? "undef" :
	       yvar_is_null(&val) ? "null" :
	       yvar_is_bool(&val) ? "bool" :
	       yvar_is_int(&val) ? "int" :
	       yvar_is_float(&val) ? "float" :
	       yvar_is_string(&val) ? "string" :
	       yvar_is_array(&val) ? "array" :
	       yvar_is_table(&val) ? "object" : "unknown");
	yjson_print(&val);

	if (argc == 3) {
		const char *path = argv[2];
		yvar_t *result = yvar_get_from_path(&val, path);
		yjson_print(result);
		result = yvar_release(result);
		yjson_print(result);
	}

printf("AA\n");
	yvar_release(&val);
printf("BB\n");
	yjson_print(&val);
	//yjson_free(json);
}
#elif 0
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
