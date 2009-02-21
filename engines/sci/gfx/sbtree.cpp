/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

// Static binary lookup tree lookup


#include "sci/include/sci_memory.h"
#include "sci/gfx/sbtree.h"

namespace Sci {

#define NOT_A_KEY -1

typedef struct {
	int key;
	void *value;
} sbcell_t;

int int_compar(const void *a, const void *b) {
	return (*((int *)a)) - (*((int *)b));
}

void insert_interval(sbcell_t *data, int start, int stop, int *keys, int plus) {
	int center = start + ((stop - start) >> 1);

	data->key = keys[center];

	if (start == stop)
		return;

	if (center > start)
		insert_interval(data + plus, start, center - 1, keys, plus << 1);

	if (center < stop)
		insert_interval(data + plus + 1, center + 1, stop, keys, ((plus << 1) + 1));
}

sbtree_t *sbtree_new(int size, int *keys) {
	int table_size = 2;
	int levels = 0;
	sbcell_t *table;
	sbtree_t *tree;
	int i;

	if (size < 0)
		return NULL;

	while (table_size <= size) {
		table_size <<= 1;
		++levels;
	}

	if (table_size > 1)
		--table_size;

	table = (sbcell_t *)sci_calloc(sizeof(sbcell_t), table_size);
	for (i = 0; i < table_size; i++)
		table[i].key = NOT_A_KEY;

	if (!table) {
		fprintf(stderr, "SBTree: Out of memory: Could not allocate %d cells\n", table_size);
		return NULL;
	}

	tree = (sbtree_t *)sci_malloc(sizeof(sbtree_t));

	if (!tree) {
		fprintf(stderr, "SBTree: Could not allocate tree structure\n");
		free(table);
		return NULL;
	}

	qsort(keys, size, sizeof(int), int_compar);

	insert_interval(table, 0, size - 1, keys, 1);

	tree->levels = levels;
	tree->entries_nr = size;
	if ((tree->min_entry = keys[0]) < 0) {
		fprintf(stderr, "SBTree: Error: Using negative keys\n");
		free(table);
		free(tree);
		return NULL;
	}
	tree->max_entry = keys[size - 1];
	tree->data = (void *) table;
	tree->alloced_entries = table_size;
	return tree;
}

void sbtree_free(sbtree_t *tree) {
	if (!tree) {
		fprintf(stderr, "SBTree: Attempt to free NULL sbtree\n");
		return;
	}

	free(tree->data);
	free(tree);
}

void sbtree_foreach(sbtree_t *tree, void *args, void *(*operation)(sbtree_t *, const int, const void *, void *)) {
	int i;
	sbcell_t *cell = (sbcell_t *) tree->data;

	for (i = 0; i < tree->alloced_entries; i++) {
		if (cell->key != NOT_A_KEY)
			cell->value = operation(tree, cell->key, cell->value, args);
		cell = cell + 1;
	}
}

sbcell_t *locate(sbcell_t *start, int key, int level, int levels, int plus) {
	int comparison;

	if (level >= levels && (level != levels || start->key == NOT_A_KEY))
		// For large tables, the speed improvement caused by this comparison
		// scheme is almost (cough) measurable...
		return NULL;

	comparison = key - start->key;

	if (!comparison)
		return start;

	return locate(start + plus + (comparison > 0), key, level + 1, levels, (plus << 1) + (comparison > 0));
}

int sbtree_set(sbtree_t *tree, int key, void *value) {
	sbcell_t *cell = locate((sbcell_t *) tree->data, key, 0, tree->levels, 1);

	if (cell)
		cell->value = value;
	else
		return -1;

	return 0;
}

void *sbtree_get(sbtree_t *tree, int key) {
	sbcell_t *cell = locate((sbcell_t *) tree->data, key, 0, tree->levels, 1);

	if (cell)
		return cell->value;
	else
		return NULL;
}

#if 0
static void sbtree_print(sbtree_t *tree) {
	int l, i;
	sbcell_t *cells = (sbcell_t *)tree->data;

	fprintf(stderr, "\tTree:\n");
	for (l = 0; l <= tree->levels; l++) {
		fprintf(stderr, "\t  ");
		for (i = 0; i < (1 << l); i++) {
			if (cells->key == NOT_A_KEY)
				fprintf(stderr, "-- ");
			else {
				if (cells->value)
					fprintf(stderr, "%d+ ", cells->key);
				else
					fprintf(stderr, "%d  ", cells->key);
			}

			cells = cells + 1;
		}
		fprintf(stderr, "\n");
	}
	fprintf(stderr, "\n");
}
#endif

//**************************** TEST CODE *******************************


#ifdef SBTREE_DEBUG

static int any_error;

void *foreach_double_func(sbtree_t *tree, const int key, const void *value, void *args) {
	int *real_value = (int *) value;

	if (!real_value)
		fprintf(stderr, "foreach_double_func(): key %d mapped to non-value!\n", key);
	else
		*real_value *= 2;

	return real_value;
}

int *generate_linear_forward(int numbers) {
	int i;
	int *data = sci_malloc(sizeof(int) * numbers);

	for (i = 0; i < numbers; i++)
		data[i] = i + 1;

	return data;
}

int *generate_linear_backward(int numbers) {
	int i;
	int *data = sci_malloc(sizeof(int) * numbers);

	for (i = 0; i < numbers; i++)
		data[i] = numbers - i;

	return data;
}

int *generate_random(int numbers, int max) {
	int i;
	int *data = sci_malloc(sizeof(int) * numbers);

	for (i = 0; i < numbers; i++)
		data[i] = 1 + (int)((rand() * 1.0 * max) / (RAND_MAX + 1.0));

	return data;
}

void insert_values(sbtree_t *tree, int nr, int *data) {
	int i;

	for (i = 0; i < nr; i++)
		if (sbtree_set(tree, data[i], (void *)(data + i))) {
			fprintf(stderr, "While inserting: %d incorrectly deemed invalid\n", data[i]);
			any_error = 1;
		}
}

#define MODE_LINEAR 0
#define MODE_LINEAR_MAP 1
#define MODE_RANDOM 2
#define MODE_LINEAR_DOUBLE 3

void test_value(sbtree_t *tree, int times, int max, int numbers, int *data, int mode) {
	int i;
	int failed = 0;

	for (i = 0; i < times; i++) {
		int key = (mode == MODE_LINEAR || mode == MODE_LINEAR_DOUBLE) ? i :
		          (mode == MODE_LINEAR_MAP) ? data[i % numbers] :
		          (int)((rand() * 1.0 * max) / (RAND_MAX + 1.0));
		int *value = (int *) sbtree_get(tree, (mode == MODE_LINEAR_DOUBLE) ? key >> 1 : key);
		int found = 0;
		int j;

		for (j = 0; j < numbers && !found; j++)
			if (data[j] == key)
				found = 1;

		if (found && !value) {
			fprintf(stderr, "!%d ", key);
			++failed;
		} else if (!found && found) {
			fprintf(stderr, "?[%d]=%d ", key, *value);
			++failed;
		}
	}

	if (failed)
		fprintf(stderr, "(%d/%d errors)\n", any_error = failed, times);
	else
		fprintf(stderr, "OK\n");
}

void test_boundary(sbtree_t *tree, int max, int random) {
	int *value_too_low = sbtree_get(tree, 0);
	int *value_too_high = sbtree_get(tree, max + 1);
	int *value_low = sbtree_get(tree, 1);
	int *value_high = sbtree_get(tree, max);
	int failure = (value_too_low || value_too_high || (!random && (!value_low || !value_high)));

	if (!failure)
		fprintf(stderr, "OK\n");
	else {
		any_error = 1;

		fprintf(stderr, "Errors: ");
		if (value_too_low)
			fprintf(stderr, "too-low=%d ", *value_too_low);
		if (value_too_high)
			fprintf(stderr, "too-high=%d ", *value_too_high);

		if (!random) {
			if (!value_low)
				fprintf(stderr, "!low ");
			if (!value_high)
				fprintf(stderr, "!high ");
		}
		fprintf(stderr, "\n");
	}
}

void test_empty(sbtree_t *tree, int count, int max) {
	int i;
	int errors = 0;

	for (i = 0; i < count; i++) {
		int key = 1 + (int)((rand() * 1.0 * max) / (RAND_MAX + 1.0));
		int *value;

		if ((value = (int *) sbtree_get(tree, key))) {
			fprintf(stderr, "?[%d]=%d\n", key, *value);
			++errors;
		}
	}

	if (errors)
		fprintf(stderr, " (%d/%d errors)\n", any_error = errors, count);
	else
		fprintf(stderr, "OK\n");
}

void run_test(sbtree_t *tree, int entries, int *data, int random, int max_value) {
	char *tests[] = {"\tLinear reference test: \t\t", "\tKey map reference test: \t", "\tRandom access test: \t\t"};
	int i;

	any_error = 0;

	fprintf(stderr, "\tEmpty test: \t\t\t");
	test_empty(tree, entries * 2, entries + 1);
	insert_values(tree, entries, data);
	fprintf(stderr, "\tBoundary test: \t\t\t");
	test_boundary(tree, max_value, random);

	for (i = 0; i < 3; i++) {
		fprintf(stderr, tests[i]);
		test_value(tree, entries * 2, entries * 2, entries, data, i);
	}

	if (!random) {
		i = data[0];
		sbtree_foreach(tree, NULL, foreach_double_func);
		fprintf(stderr, "\tForeach test: \t\t\t");
		if (i * 2 != data[0]) {
			fprintf(stderr, "Error: No effect: %d * 2 != %d\n", i, data[0]);
			any_error = 1;
		} else
			test_value(tree, entries * 2, entries * 2, entries, data, MODE_LINEAR_DOUBLE);
	}

	if (any_error)
		sbtree_print(tree);

	free(data);
	sbtree_free(tree);
}

#define TESTS_NR 11

int main(int argc, char **argv) {
	int tests_nr = TESTS_NR;
	int test_sizes[TESTS_NR] = {1, 2, 3, 7, 8, 9, 1000, 16383, 16384, 16385, 1000000};
	int i;
	fprintf(stderr, "sbtree.c Copyright (C) 2000 Christoph Reichenbach <jameson@linuxgames.com>\n"
	        "This program is provided WITHOUT WARRANTY of any kind\n"
	        "Please refer to the file COPYING that should have come with this program\n");
	fprintf(stderr, "Static Binary Tree testing facility\n");

	free(malloc(42)); // Make sure libefence's Copyright message is print here if we're using it

	fprintf(stderr, "\nsbtree.c: Running %d tests.\n", tests_nr);

	for (i = 0; i < tests_nr; i++) {
		int entries = test_sizes[i];
		sbtree_t *tree;
		int *data;

		fprintf(stderr, "Test #%d: %d entries\n", i + 1, entries);

		fprintf(stderr, "\t%da: Linear values\n", i + 1);
		data = generate_linear_forward(entries);
		tree = sbtree_new(entries, data);
		run_test(tree, entries, data, 0, entries);

		fprintf(stderr, "\t%db: Reverse linear values\n", i + 1);
		data = generate_linear_backward(entries);
		tree = sbtree_new(entries, data);
		run_test(tree, entries, data, 0, entries);

		fprintf(stderr, "\t%dc: Dense random values\n", i + 1);
		data = generate_random(entries, 1 + (entries >> 2));
		tree = sbtree_new(entries, data);
		run_test(tree, entries, data, 1, 1 + (entries >> 2));

		fprintf(stderr, "\t%dc: Sparse random values\n", i + 1);
		data = generate_random(entries, (entries << 2));
		tree = sbtree_new(entries, data);
		run_test(tree, entries, data, 1, entries << 2);

		fprintf(stderr, "Test #%d completed.\n\n", i + 1);
	}

	fprintf(stderr, "Test suite completed.\n");
	return 0;
}

#endif // SBTREE_DEBUG

} // End of namespace Sci
