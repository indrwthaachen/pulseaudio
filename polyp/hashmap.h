#ifndef foohashmaphfoo
#define foohashmaphfoo

/* $Id$ */

/***
  This file is part of polypaudio.
 
  polypaudio is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation; either version 2 of the License,
  or (at your option) any later version.
 
  polypaudio is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.
 
  You should have received a copy of the GNU Lesser General Public License
  along with polypaudio; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.
***/

/* Simple Implementation of a hash table. Memory management is the
 * user's job. It's a good idea to have the key pointer point to a
 * string in the value data. */

struct pa_hashmap;

/* Create a new hashmap. Use the specified functions for hashing and comparing objects in the map */
struct pa_hashmap *pa_hashmap_new(unsigned (*hash_func) (const void *p), int (*compare_func) (const void*a, const void*b));

/* Free the hash table. Calls the specified function for every value in the table. The function may be NULL */
void pa_hashmap_free(struct pa_hashmap*, void (*free_func)(void *p, void *userdata), void *userdata);

/* Returns non-zero when the entry already exists */
int pa_hashmap_put(struct pa_hashmap *h, const void *key, void *value);
void* pa_hashmap_get(struct pa_hashmap *h, const void *key);

/* Returns the data of the entry while removing */
void* pa_hashmap_remove(struct pa_hashmap *h, const void *key);

unsigned pa_hashmap_ncontents(struct pa_hashmap *h);

/* May be used to iterate through the hashmap. Initially the opaque
   pointer *state has to be set to NULL. The hashmap may not be
   modified during iteration. The key of the entry is returned in
   *key, if key is non-NULL. After the last entry in the hashmap NULL
   is returned. */
void *pa_hashmap_iterate(struct pa_hashmap *h, void **state, const void**key);

#endif
