/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "rax.h"
#include "group.h"
#include "../../deps/xxHash/xxhash.h"

typedef rax CacheGroup;
typedef raxIterator CacheGroupIterator;

CacheGroup *CacheGroupNew(void);

void CacheGroupAdd(CacheGroup *groups, XXH64_hash_t key, Group *group);

// retrives a group, sets group to NULL if key is missing
Group *CacheGroupGet(CacheGroup *groups, XXH64_hash_t key);

void FreeGroupCache(CacheGroup *groups);

// populates an iterator to scan group cache
CacheGroupIterator *CacheGroupIter(CacheGroup *groups);

// advance iterator and returns value in current position
int CacheGroupIterNext(CacheGroupIterator *iter, Group **group);

void CacheGroupIterator_Free(CacheGroupIterator *iter);

