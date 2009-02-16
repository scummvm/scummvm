/***************************************************************************
 hashmap.c Copyright (C) 2001 Christoph Reichenbach


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Christoph Reichenbach (CR) <jameson@linuxgames.com>

***************************************************************************/
/* Defines a universal hash map.
** Preprocessor parameters:
** TYPE: The type to hash
** HASH_MAX: Maximum hash value
** HASH(x): Hashes a value of type TYPE to an int from 0 to HASH_MAX
*/
#ifdef MUST_FREE
#  define CLEAR_NODE(n) free(n->name); n->name = NULL
#  define FREE_PARAM(n) free(n)
#else
#  define CLEAR_NODE(n)
#  define FREE_PARAM(n)
#endif


#ifdef DUPLICATOR
#  define DUP_VALUE(x) DUPLICATOR((x))
#else
#  define DUP_VALUE(x) (x)
#endif


#define DEFINE_FUNCTIONS(TYPE) \
										\
										\
TYPE##_hash_map_t *								\
new_##TYPE##_hash_map(void)							\
{										\
	TYPE##_hash_map_t *map = (TYPE##_hash_map_t*)calloc(1, sizeof(TYPE##_hash_map_t));\
										\
	return map;								\
}										\
                                                                \
                                                                \
static void							\
print_##TYPE##_nodes(TYPE##_hash_map_node_t *node)		\
{								\
	while (node) {						\
		fprintf(stderr,"%p  ", (void *)node);		\
		node = node->next;				\
	}							\
}								\
								\
void								\
print_##TYPE##_hash_map(TYPE##_hash_map_t *map)			\
{								\
	int bucket;						\
	fprintf(stderr, #TYPE " map %p: base value=%d\n", (void *)map,  \
		map->base_value);		                \
	for (bucket = 0; bucket <= HASH_MAX; bucket++) {        \
		fprintf(stderr,"bucket %d: ", bucket);		\
		print_##TYPE##_nodes(map->nodes[bucket]);	\
		fprintf(stderr,"\n");				\
	}							\
	fprintf(stderr,"holes: ");				\
	print_##TYPE##_nodes(map->holes);			\
	fprintf(stderr,"\n");					\
}                                                               \
                                                                \
void														 \
apply_to_##TYPE##_hash_map(TYPE##_hash_map_t *map, void *param, void (*note)(void *param, TYPE name, int value)) \
{														 \
	int i;													 \
	for (i = 0; i < HASH_MAX; i++) {									 \
		TYPE##_hash_map_node_t *node = map->nodes[i];							 \
		while (node) {											 \
			note(param, node->name, node->value);							 \
			node = node->next;									 \
		}												 \
	}													 \
}														 \
														 \
										\
static void									\
free_##TYPE##_hash_map_node_t##_recursive(TYPE##_hash_map_node_t *node)		\
{										\
	if (node) {								\
		CLEAR_NODE(node);				                \
		free_##TYPE##_hash_map_node_t##_recursive(node->next);		\
		free(node);							\
	}									\
}										\
										\
										\
void										\
free_##TYPE##_hash_map(TYPE##_hash_map_t *map)					\
{										\
	int i;									\
										\
	for (i = 0; i <= HASH_MAX; i++)						\
		free_##TYPE##_hash_map_node_t##_recursive(map->nodes[i]);	\
										\
	free_##TYPE##_hash_map_node_t##_recursive(map->holes);			\
										\
	map->base_value = -42000; /* Trigger problems for people who		\
			     ** forget to loose the reference  */		\
	free(map);								\
}										\
										\
int										\
TYPE##_hash_map_check_value(TYPE##_hash_map_t *map, TYPE value,                 \
			    char add, char *was_added)                  	\
{										\
	TYPE##_hash_map_node_t **node = &(map->nodes[HASH(value)]);		\
										\
	while (*node && COMP(value, (*node)->name))				\
		node = &((*node)->next);					\
										\
        if (was_added)								\
                *was_added = 0;							\
										\
	if (*node) {								\
		FREE_PARAM(value);						\
		return (*node)->value;						\
	}									\
	/* Not found */								\
										\
	if (!add)								\
		return -1;							\
										\
        if (was_added)								\
                *was_added = 1;							\
										\
	if (map->holes) { /* Re-use old node */					\
		(*node) = map->holes;						\
		map->holes = (*node)->next;					\
                (*node)->next = NULL;                                           \
		(*node)->name = DUP_VALUE(value);				\
	} else {								\
		*node = (TYPE##_hash_map_node_t*)malloc(sizeof(TYPE##_hash_map_node_t));\
		(*node)->name = DUP_VALUE(value);				\
		(*node)->value = map->base_value++;				\
                (*node)->next = NULL;                                           \
	}									\
										\
	return (*node)->value;							\
}										\
										\
										\
int										\
TYPE##_hash_map_remove_value(TYPE##_hash_map_t *map, TYPE value)		\
{										\
	TYPE##_hash_map_node_t **node = &(map->nodes[HASH(value)]);		\
										\
	while (*node && COMP(value, (*node)->name))				\
		node = &((*node)->next);					\
										\
	if (*node) {								\
		TYPE##_hash_map_node_t *oldnode = *node;			\
		*node = (*node)->next;						\
										\
		oldnode->next = map->holes; /* Old node is now a 'hole' */	\
		map->holes = oldnode;						\
		return oldnode->value;						\
	} else return -1; /* Not found */					\
}										\
 
