/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "decode_previous.h"
#include "prev/decoders.h"

GraphContext *Decode_Previous(RedisModuleIO *rdb, int encver) {
	switch(encver) {
	case 6:
		return RdbLoadGraphContext_v6(rdb);
	case 7:
		return RdbLoadGraphContext_v7(rdb);
	case 8:
		return RdbLoadGraphContext_v8(rdb);
	case 9:
		return RdbLoadGraphContext_v9(rdb);
	case 10:
		return RdbLoadGraphContext_v10(rdb);
	default:
		ASSERT(false && "attempted to read unsupported RedisGraph version from RDB file.");
		return NULL;
	}
}

