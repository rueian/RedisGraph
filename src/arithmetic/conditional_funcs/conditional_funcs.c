/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "conditional_funcs.h"
#include "../func_desc.h"
#include "../../util/arr.h"
#include "../../datatypes/set.h"

/* Case When
 * Case Value [When Option i Then Result i] Else Default end */
SIValue AR_CASEWHEN(SIValue *argv, int argc) {
	int alternatives = argc - 1;
	SIValue d = argv[argc - 1];

	if((argc % 2) == 0) {
		/* Simple form:
		 * argv[0] - Value
		 * argv[i] - Option i
		 * argv[i+1] - Result i
		 * argv[argc-1] - Default
		 *
		 * Evaluate alternatives in order, return first alternatives which
		 * is equals to Value. */
		SIValue v = argv[0];
		for(int i = 1; i < alternatives; i += 2) {
			SIValue a = argv[i];
			int disjointOrNull;
			if(SIValue_Compare(v, a, &disjointOrNull) == 0) {
				// Return Result i.
				// The value's ownership must be transferred to avoid a double free if it is an allocated value.
				SIValue retval = argv[i + 1];
				SIValue_MakeVolatile(&argv[i + 1]);
				return retval;
			}
		}
	} else {
		/* Generic form:
		 * argv[i] - Option i
		 * argv[i+1] - Result i
		 * arg[argc-1] - Default
		 *
		 * Evaluate alternatives in order, return first alternatives which
		 * is not NULL or false. */
		for(int i = 0; i < alternatives; i += 2) {
			SIValue a = argv[i];
			// Skip NULL and false options.
			if(SIValue_IsNull(a) || ((SI_TYPE(a) & T_BOOL) && SIValue_IsFalse(a))) continue;
			// The option was truthy, return the associated value.
			// The value's ownership must be transferred to avoid a double free if it is an allocated value.
			SIValue retval = argv[i + 1];
			SIValue_MakeVolatile(&argv[i + 1]);
			return retval;
		}
	}

	//Did not match against any Option return default.
	SIValue_MakeVolatile(&argv[argc - 1]);
	return d;
}

// Coalesce - return the first value which is not null. Defaults to null.
SIValue AR_COALESCE(SIValue *argv, int argc) {
	for(int i = 0; i < argc; i++)
		if(!SIValue_IsNull(argv[i])) {
			/* Avoid double free, since the value is propagated and will be free twice:
			 * 1. Argument array free.
			 * 2. Record free. */
			SIValue copy = argv[i];
			SIValue_MakeVolatile(argv + i);
			return copy;
		}
	return SI_NullVal();
}

// Distinct maintains a set of values,
// if value `X` already in the set return NULL,
// otherwise `X` is returned and added to to the set.
SIValue AR_DISTINCT(SIValue *argv, int argc) {
	set *set = argv[1].ptrval;
	if(Set_Add(set, argv[0])) return SI_ConstValue(argv);
	return SI_NullVal();
}

// Routine for freeing a Distinct function context.
void Distinct_Free(void *ctx_ptr) {
	set *set = ctx_ptr;
	if(set == NULL) return;
	Set_Free(set);
}

// Routine for cloning a Distinct function context.
void *Distinct_Clone(void *orig) {
	return Set_New();
}

void Register_ConditionalFuncs() {
	SIType *types;
	AR_FuncDesc *func_desc;

	types = array_new(SIType, 1);
	array_append(types, SI_ALL);
	func_desc = AR_FuncDescNew("case", AR_CASEWHEN, 2, VAR_ARG_LEN, types, true, false);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	array_append(types, SI_ALL);
	func_desc = AR_FuncDescNew("coalesce", AR_COALESCE, 1, VAR_ARG_LEN, types, true, false);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	array_append(types, SI_ALL);
	func_desc = AR_FuncDescNew("distinct", AR_DISTINCT, 2, 2, types, false, false);
	AR_SetPrivateDataRoutines(func_desc, Distinct_Free, Distinct_Clone);
	AR_RegFunc(func_desc);
}

