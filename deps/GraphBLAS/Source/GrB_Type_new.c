//------------------------------------------------------------------------------
// GrB_Type_new: create a new user-defined type
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// GrB_Type_new is implemented both as a macro and a function.  Both are
// user-callable.  The default is to use the macro, since this allows the name
// of the type to be saved as a string, for subsequent error reporting by
// GrB_error and for the return value of GxB_Matrix_type_name.  It is also
// provided as a function so that applications that require a function instead
// of macro can access it.  User code can simply do #undef GrB_Type_new before
// using the function.  This approach also places the function GrB_Type_new in
// the linkable SuiteSparse:GraphBLAS library so that it is visible for linking
// with applications in languages other than ANSI C.  The function version does
// not allow the name of the ctype to be saved in the new GraphBLAS type,
// however.  It is given a generic name, "user_type_of_size_%d" where "%d" is
// the size of the type.

#include "GB.h"

// the macro version of this function must first be #undefined
#undef GrB_Type_new
#undef GrM_Type_new

GrB_Info GRB (Type_new)         // create a new GraphBLAS type
(
    GrB_Type *type,             // handle of user type to create
    size_t sizeof_ctype         // size = sizeof (ctype) of the C type
)
{ 
    return (GxB_Type_new (type, sizeof_ctype, NULL, NULL)) ;
}

