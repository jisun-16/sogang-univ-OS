#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "bitmap.h"
#include "debug.h"
#include "hash.h"
#include "hex_dump.h"
#include "limits.h"
#include "list.h"
#include "round.h"

#define LIST 0
#define BITMAP 1
#define HASH 2

bool _command();

void _create();
void _delete();
void _dumpdata();
int find_idx(char name[], int *ds);

void _list();
bool less_func(const struct list_elem* a, const struct list_elem* b, void* aux);

void _bitmap();

void _hash();
hash_hash_func hash_func;
hash_action_func hash_action;
hash_action_func square;
hash_action_func triple;
unsigned hash_func(const struct hash_elem* e, void* aux);
bool hash_less(const struct hash_elem* a, const struct hash_elem* b, void* aux);
void hash_action(struct hash_elem* e, void *aux);
void square(struct hash_elem* e, void* aux);
void triple(struct hash_elem* e, void* aux);

char command[100], *Cmd[6];
struct list List[10];
struct hash Hash[10];
struct bitmap* Bitmap[10];
char list_name[10][100], hash_name[10][100], bitmap_name[10][100];
int list_cnt, hash_cnt, bitmap_cnt;

int main(void) {
	bool fin = false;

	while (!fin) {
		fgets(command, sizeof(command), stdin);
		command[strlen(command)-1] = '\0';
		fin = _command();
	}

	return 0;
}

bool _command() {
	bool fin = false;

	int i = 0;
	Cmd[i++] = strtok(command, " ");
	while (true) {
		Cmd[i] = strtok(NULL, " ");
		if (Cmd[i] == NULL) break;
		i++;
	}

	//common ground (list, bitmap, hash)
	if (!strcmp(Cmd[0], "create"))
		_create();
	else if (!strcmp(Cmd[0], "delete")) 
		_delete();
	else if (!strcmp(Cmd[0], "dumpdata"))
		_dumpdata();
	else if (!strcmp(Cmd[0], "quit"))
		fin = true;

	else if (!strncmp(Cmd[0], "list", 4))
		_list();
	else if (!strncmp(Cmd[0], "bitmap", 6))
		_bitmap();
	else if (!strncmp(Cmd[0], "hash", 4))
		_hash();
	else
		printf("wrong command!\n");

	return fin;
}

//common func
void _create() {	
	if (!strcmp(Cmd[1], "list")) {
		list_init(&List[list_cnt]);
		strcpy(list_name[list_cnt++], Cmd[2]);
	}
	else if (!strcmp(Cmd[1], "hashtable")) {
		hash_init(&Hash[hash_cnt], hash_func, hash_less, NULL);
		strcpy(hash_name[hash_cnt++], Cmd[2]);
	}
	else if (!strcmp(Cmd[1], "bitmap")) {
		Bitmap[bitmap_cnt] = bitmap_create(atoi(Cmd[3]));
		strcpy(bitmap_name[bitmap_cnt++], Cmd[2]);
	}

	return;
}

void _delete() {
	int ds=-1;
	int idx = find_idx(Cmd[1], &ds);

	switch (ds) {
	case LIST: {
		struct list_elem* cur = list_begin(&List[idx]), * next;
		next = cur;
		struct list_item* d_item = list_entry(cur, struct list_item, elem);
		int size = list_size(&List[idx]);
		while (size--) {
			next = list_next(cur);
			list_remove(cur);
			free(d_item);
			cur = next;
			d_item = list_entry(cur, struct list_item, elem);
		}
		break;
	}
	case BITMAP: {
		bitmap_destroy(Bitmap[idx]);
		break;
	}
	case HASH: {
		hash_destroy(&Hash[idx], hash_action);
	}
	}

	return;
}

void _dumpdata() {
	int ds = -1;
	int idx = find_idx(Cmd[1], &ds);
	
	if(idx==-1){
		printf("wrong name!\n");
		return;
	}

	switch (ds) {
	case LIST: {
		if (list_empty(&List[idx])) return;

		struct list_elem* e;
		for (e = list_begin(&List[idx]); e != list_end(&List[idx]); e = list_next(e)) {
			struct list_item* temp = list_entry(e, struct list_item, elem);
			printf("%d ", temp->data);
		}
		printf("\n");

		break;
	}
	case BITMAP: {
		int size = bitmap_size(Bitmap[idx]);
		if (size == 0) return;

		for (int i = 0; i < size; i++)
			printf("%d", bitmap_test(Bitmap[idx], i));
		printf("\n");

		break;
	}
	case HASH: {
		if (hash_empty(&Hash[idx])) return;

		struct hash_iterator i;
		hash_first(&i, &Hash[idx]);
		while (hash_next(&i)) {
			struct hash_item* temp = hash_entry(hash_cur(&i), struct hash_item, elem);
			printf("%d ", temp->data);
		}
		printf("\n");
	}
	}
	return;
}

int find_idx(char name[], int* ds) {
	int idx = -1;

	for (idx = 0; idx < list_cnt; idx++)
		if (!strcmp(name, list_name[idx])) {
			*ds = LIST;
			return idx;
		}
	for (idx = 0; idx < bitmap_cnt; idx++)
		if (!strcmp(name, bitmap_name[idx])) {
			*ds = BITMAP;
			return idx;
		}
	for (idx = 0; idx < hash_cnt; idx++)
		if (!strcmp(name, hash_name[idx])) {
			*ds = HASH;
			return idx;
		}

	return -1;
}

//list
void _list() {
	int ds = -1;
	int idx = find_idx(Cmd[1], &ds);

	if (ds != LIST)
		return;

	if (!strcmp("list_push_back", Cmd[0])) {
		struct list_item* node = (struct list_item*)malloc(sizeof(struct list_item));
		node->data = atoi(Cmd[2]);
		list_push_back(&List[idx], &(node->elem));
	}
	else if (!strcmp("list_front", Cmd[0])) {
		struct list_elem* e = list_front(&List[idx]);
		struct list_item* i = list_entry(e, struct list_item, elem);
		printf("%d\n", i->data);
	}
	else if (!strcmp("list_back", Cmd[0])) {
		struct list_elem* e = list_back(&List[idx]);
		struct list_item* i = list_entry(e, struct list_item, elem);
		printf("%d\n", i->data);
	}
	else if (!strcmp("list_pop_back", Cmd[0])) {
		list_pop_back(&List[idx]);
	}
	else if (!strcmp("list_pop_front", Cmd[0])) {
		list_pop_front(&List[idx]);
	}
	else if (!strcmp("list_insert", Cmd[0])) {
		struct list_item* node = (struct list_item*)malloc(sizeof(struct list_item));
		node->data = atoi(Cmd[3]);
		struct list_elem* e = list_head(&List[idx]);
		for (int i = 0; i <= (atoi(Cmd[2]));i++)
			e=list_next(e);
		list_insert(e,&(node->elem));
	}
	else if (!strcmp("list_remove", Cmd[0])) {
		struct list_elem* e=list_head(&List[idx]);
		for (int i = 0; i <= (atoi(Cmd[2])); i++)
			e = list_next(e);
		list_remove(e);
	}
	else if (!strcmp("list_insert_ordered", Cmd[0])) {
		struct list_item* node = (struct list_item*)malloc(sizeof(struct list_item));
		node->data = atoi(Cmd[2]);
		list_insert_ordered(&List[idx], &(node->elem), less_func, NULL);
	}
	else if (!strcmp("list_empty", Cmd[0])) {
		bool is_empty = list_empty(&List[idx]);
		if (is_empty) printf("true\n");
		else printf("false\n");
	}
	else if (!strcmp("list_size", Cmd[0])) {
		printf("%zd\n", list_size(&List[idx]));
	}
	else if (!strcmp("list_max", Cmd[0])) {
		struct list_elem* e = list_max(&List[idx], less_func, NULL);
		struct list_item* i = list_entry(e, struct list_item, elem);
		printf("%d\n", i->data);
	}
	else if (!strcmp("list_min", Cmd[0])) {
		struct list_elem* e = list_min(&List[idx], less_func, NULL);
		struct list_item* i = list_entry(e, struct list_item, elem);
		printf("%d\n", i->data);
	}
	else if (!strcmp("list_sort", Cmd[0])) {
		list_sort(&List[idx], less_func, NULL);
	}
	else if (!strcmp("list_swap", Cmd[0])) {
		struct list_elem* a = list_head(&List[idx]);
		struct list_elem* b = list_head(&List[idx]);
		for (int i = 0; i <= atoi(Cmd[2]); i++)
			a = a->next;
		for (int i = 0; i < atoi(Cmd[3]); i++)
			b = b->next;
		list_swap(a, b);
	}
	else if (!strcmp("list_shuffle", Cmd[0])) {
		list_shuffle(&List[idx]);
	}
	else if (!strcmp("list_reverse", Cmd[0])) {
		list_reverse(&List[idx]);
	}
	else if (!strcmp("list_splice", Cmd[0])) {
		int idx2 = find_idx(Cmd[3], &ds);
		if (ds != LIST)
			return;
		struct list_elem* before = list_head(&List[idx]);
		struct list_elem* first = list_head(&List[idx2]);
		struct list_elem* last = list_head(&List[idx2]);
		for (int i = 0; i <= atoi(Cmd[2]); i++)
			before = before->next;
		for (int i = 0; i <= atoi(Cmd[4]); i++)
			first = first->next;
		for (int i = 0; i < atoi(Cmd[5]); i++)
			last = last->next;
		list_splice(before, first, last);
	}

	return;
}

bool less_func(const struct list_elem* a, const struct list_elem* b, void* aux) {
	struct list_item* node1 = list_entry(a, struct list_item, elem);
	struct list_item* node2 = list_entry(b, struct list_item, elem);

	if (node1->data < node2->data)
		return true;
	else
		return false;
}

//bitmap
void _bitmap() {
	int ds = -1;
	int idx = find_idx(Cmd[1], &ds);

	if (ds != BITMAP) return;

	if (!strcmp("bitmap_mark", Cmd[0])) {
		bitmap_mark(Bitmap[idx], atoi(Cmd[2]));
	}
	else if (!strcmp("bitmap_any", Cmd[0])) {
		if (bitmap_any(Bitmap[idx], atoi(Cmd[2]), atoi(Cmd[3])))
			printf("true\n");
		else
			printf("false\n");
	}
	else if (!strcmp("bitmap_expand", Cmd[0])) {
		bitmap_expand(Bitmap[idx], atoi(Cmd[2]));
	}
	else if (!strcmp("bitmap_set", Cmd[0])) {
		bitmap_set(Bitmap[idx], atoi(Cmd[2]), strcmp("true", Cmd[3]));
	}
	else if (!strcmp("bitmap_set_all", Cmd[0])) {
		bitmap_set_all(Bitmap[idx], strcmp("true", Cmd[2])==0);
	}
	else if (!strcmp("bitmap_set_multiple", Cmd[0])) {
		bool value = (strcmp("true", Cmd[4])==0);
		bitmap_set_multiple(Bitmap[idx], atoi(Cmd[2]), atoi(Cmd[3]), value);
	}
	else if (!strcmp("bitmap_flip", Cmd[0])) {
		bitmap_flip(Bitmap[idx], atoi(Cmd[2]));
	}
	else if (!strcmp("bitmap_none", Cmd[0])) {
		int start = atoi(Cmd[2]), cnt = atoi(Cmd[3]);
		bitmap_none(Bitmap[idx], start, cnt);
	}
	else if (!strcmp("bitmap_reset", Cmd[0])) {
		bitmap_reset(Bitmap[idx], atoi(Cmd[2]));
	}
	else if (!strcmp("bitmap_scan", Cmd[0])) {
		int start = atoi(Cmd[2]), cnt = atoi(Cmd[3]);
		bool value = (strcmp("true", Cmd[4])==0);
		printf("%zd\n", bitmap_scan(Bitmap[idx], start, cnt, value));
	}
	else if (!strcmp("bitmap_scan_and_flip", Cmd[0])) {
		int start = atoi(Cmd[2]), cnt = atoi(Cmd[3]);
		bool value = (strcmp("true", Cmd[4])==0);
		printf("%zd\n", bitmap_scan_and_flip(Bitmap[idx], start, cnt, value));
	}
	else if (!strcmp("bitmap_test", Cmd[0])) {
		if(bitmap_test(Bitmap[idx],atoi(Cmd[2])))
			printf("true\n");
		else
			printf("false\n");
	}
	else if (!strcmp("bitmap_all", Cmd[0])) {
		if (bitmap_all(Bitmap[idx], atoi(Cmd[2]), atoi(Cmd[3])))
			printf("true\n");
		else
			printf("false\n");
	}
	else if (!strcmp("bitmap_dump", Cmd[0])) {
		bitmap_dump(Bitmap[idx]);
	}
	else if (!strcmp("bitmap_contains", Cmd[0])) {
		int start = atoi(Cmd[2]), cnt = atoi(Cmd[3]);
		bool value = (strcmp("true", Cmd[4])==0);
		bool is_contained = bitmap_contains(Bitmap[idx], start, cnt, value);
		if (is_contained)
			printf("true\n");
		else
			printf("false\n");
	}
	else if (!strcmp("bitmap_size", Cmd[0])) {
		printf("%zd\n", bitmap_size(Bitmap[idx]));
	}
	else if (!strcmp("bitmap_count", Cmd[0])) {
		int start = atoi(Cmd[2]), cnt = atoi(Cmd[3]);
		bool value = (strcmp("true", Cmd[4])==0);
		printf("%zd\n", bitmap_count(Bitmap[idx], start, cnt, value));
	}

	return;
}

//hash
void _hash() {
	int ds = -1;
	int idx = find_idx(Cmd[1], &ds);

	if (ds != HASH) return;

	if (!strcmp("hash_insert", Cmd[0])) {
		struct hash_item* e = (struct hash_item*)malloc(sizeof(struct hash_item));
		e->data = atoi(Cmd[2]);
		hash_insert(&Hash[idx], &(e->elem));
	}
	else if (!strcmp("hash_apply", Cmd[0])) {
		if (!strcmp("square", Cmd[2]))
			hash_apply(&Hash[idx], square);
		else if (!strcmp("triple", Cmd[2]))
			hash_apply(&Hash[idx], triple);
	}
	else if (!strcmp("hash_delete", Cmd[0])) {
		struct hash_item* temp = (struct hash_item*)malloc(sizeof(struct hash_item));
		temp->data = atoi(Cmd[2]);
		hash_delete(&Hash[idx], &(temp->elem));
		free(temp);
		temp = NULL;
	}
	else if (!strcmp("hash_replace", Cmd[0])) {
		struct hash_item* temp = (struct hash_item*)malloc(sizeof(struct hash_item));
		temp->data = atoi(Cmd[2]);
		hash_replace(&Hash[idx], &(temp->elem));
	}
	else if (!strcmp("hash_find", Cmd[0])) {
		struct hash_item* temp = (struct hash_item*)malloc(sizeof(struct hash_item));
		temp->data = atoi(Cmd[2]);
		struct hash_elem* found = hash_find(&Hash[idx], &(temp->elem));
		if (found)
			printf("%d\n", temp->data);
	}
	else if (!strcmp("hash_empty", Cmd[0])) {
		if (hash_empty(&Hash[idx]))
			printf("true\n");
		else
			printf("false\n");
	}
	else if (!strcmp("hash_size", Cmd[0])) {
		printf("%zd\n", hash_size(&Hash[idx]));
	}
	else if (!strcmp("hash_clear", Cmd[0])) {
		hash_clear(&Hash[idx], hash_action);
	}

	return;
}

unsigned hash_func(const struct hash_elem* e, void* aux) {
	struct hash_item* item_h = hash_entry(e, struct hash_item, elem);
	int val = item_h->data;
	return hash_int(val);
}

bool hash_less(const struct hash_elem* a, const struct hash_elem* b, void* aux) {
	struct hash_item* item_a = hash_entry(a, struct hash_item, elem);
	int val_a = item_a->data;

	struct hash_item* item_b = hash_entry(b, struct hash_item, elem);
	int val_b = item_b->data;

	return val_a < val_b;
}

void hash_action(struct hash_elem* e, void* aux){
	free(e);
}

void square(struct hash_elem* e, void* aux) {
	struct hash_item* temp = hash_entry(e, struct hash_item, elem);
	temp->data = temp->data * temp->data;
}

void triple(struct hash_elem* e, void* aux) {
	struct hash_item* temp = hash_entry(e, struct hash_item, elem);
	temp->data = temp->data * temp->data * temp->data;
}
