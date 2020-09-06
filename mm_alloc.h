/*
 * mm_alloc.h
 *
 * A clone of the interface documented in "man 3 malloc".
 */

#pragma once

#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

// Hafizaya erisimi duzenleyen mutex kilidi
pthread_mutex_t global_malloc_lock;
// mm_print_mem fonskiyonu icin gerekli adres turu
typedef long unsigned int addrtype;

// Blok yapisi
struct header_t {
	size_t size;
	unsigned is_free;
	struct header_t *next;
};

struct header_t *head, *tail;

void *mm_malloc(size_t size);
void *mm_realloc(void *ptr, size_t size);
void mm_free(void *ptr);
void mm_print_mem();
