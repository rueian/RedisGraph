//------------------------------------------------------------------------------
// GB_mex_about5: still more basic tests
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Test lots of random stuff.  The function otherwise serves no purpose.

#include "GB_mex.h"
#include "GB_mex_errors.h"
#include "GB_dynamic.h"
#include "GB_serialize.h"

#define USAGE "GB_mex_about5"
#define FREE_ALL ;
#define GET_DEEP_COPY ;
#define FREE_DEEP_COPY ;

void banded_idx
(
    bool *z,
    const int64_t *x,   // unused
    int64_t i,
    int64_t j,
    const int64_t *thunk
) ;

void banded_idx
(
    bool *z,
    const int64_t *x,   // unused
    int64_t i,
    int64_t j,
    const int64_t *thunk
)
{
    int64_t d = GB_IABS (j-i) ;
    (*z) = (d <= *thunk) ;
}

void upperbanded_idx
(
    bool *z,
    const int64_t *x,   // unused
    int64_t i,
    int64_t j,
    const int64_t *thunk
) ;

void upperbanded_idx
(
    bool *z,
    const int64_t *x,   // unused
    int64_t i,
    int64_t j,
    const int64_t *thunk
)
{
    int64_t d = j-i ;
    (*z) = (d >= 0 && d <= *thunk) ;
}

void upperbanded_idx_int64
(
    int64_t *z,
    const int64_t *x,   // unused
    int64_t i,
    int64_t j,
    const int64_t *thunk
) ;

void upperbanded_idx_int64
(
    int64_t *z,
    const int64_t *x,   // unused
    int64_t i,
    int64_t j,
    const int64_t *thunk
)
{
    int64_t d = j-i ;
    (*z) = (int64_t) (d >= 0 && d <= *thunk) ;
}

void add_int64
(
    int64_t *z,
    const int64_t *x,
    const int64_t *y
) ;

void add_int64
(
    int64_t *z,
    const int64_t *x,
    const int64_t *y
)
{
    (*z) = 2 * (*x) + (*y) ;
}

typedef struct
{
    float x ;
    int y ;
}
mytype ;

void donothing
(
    void *z,
    const void *x,
    int64_t i,
    int64_t j,
    const void *thunk
) ;

void donothing
(
    void *z,
    const void *x,
    int64_t i,
    int64_t j,
    const void *thunk
)
{
    ;  // do nothing
}

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    GrB_Info info ;
    GrB_Matrix C = NULL, A = NULL, M = NULL, S = NULL, E = NULL ;
    GrB_Descriptor desc = NULL ;
    GrB_Vector w = NULL ;
    GrB_Scalar scalar = NULL ;
    GrB_IndexUnaryOp Banded = NULL, UpperBanded = NULL, 
        UpperBanded_int64 = NULL, Gunk = NULL ;
    GrB_Type type = NULL, MyType = NULL, MyInt64 = NULL ;
    char *err ;
    mytype scalar1 ;
    scalar1.x = 4 ;
    scalar1.y = 3 ;

    //--------------------------------------------------------------------------
    // startup GraphBLAS
    //--------------------------------------------------------------------------

    bool malloc_debug = GB_mx_get_global (true) ;
    int expected = GrB_SUCCESS ;

    //--------------------------------------------------------------------------
    // type_name
    //--------------------------------------------------------------------------

    char type_name [GxB_MAX_NAME_LEN] ;

    OK (GxB_UnaryOp_xtype_name (type_name, GrB_ABS_INT32)) ;
    CHECK (MATCH (type_name, "int32_t")) ;
    OK (GxB_Type_from_name (&type, type_name)) ;
    CHECK (type == GrB_INT32) ;
    OK (GxB_UnaryOp_ztype_name (type_name, GrB_IDENTITY_UINT8)) ;
    CHECK (MATCH (type_name, "uint8_t")) ;
    OK (GxB_Type_from_name (&type, type_name)) ;
    CHECK (type == GrB_UINT8) ;

    OK (GxB_UnaryOp_xtype_name (type_name, GrB_ABS_UINT64)) ;
    CHECK (MATCH (type_name, "uint64_t")) ;
    OK (GxB_Type_from_name (&type, type_name)) ;
    CHECK (type == GrB_UINT64) ;
    OK (GxB_UnaryOp_ztype_name (type_name, GrB_IDENTITY_INT8)) ;
    CHECK (MATCH (type_name, "int8_t")) ;
    OK (GxB_Type_from_name (&type, type_name)) ;
    CHECK (type == GrB_INT8) ;

    OK (GxB_BinaryOp_xtype_name (type_name, GrB_PLUS_FP32)) ;
    CHECK (MATCH (type_name, "float")) ;
    OK (GxB_Type_from_name (&type, type_name)) ;
    CHECK (type == GrB_FP32) ;
    OK (GxB_BinaryOp_ytype_name (type_name, GrB_PLUS_FP64)) ;
    CHECK (MATCH (type_name, "double")) ;
    OK (GxB_Type_from_name (&type, type_name)) ;
    CHECK (type == GrB_FP64) ;
    OK (GxB_BinaryOp_ztype_name (type_name, GrB_LT_FP64)) ;
    CHECK (MATCH (type_name, "bool")) ;
    OK (GxB_Type_from_name (&type, type_name)) ;
    CHECK (type == GrB_BOOL) ;

    OK (GxB_BinaryOp_xtype_name (type_name, GxB_PLUS_FC32)) ;
    CHECK (MATCH (type_name, "float complex")) ;
    OK (GxB_Type_from_name (&type, type_name)) ;
    CHECK (type == GxB_FC32) ;
    OK (GxB_BinaryOp_ytype_name (type_name, GxB_PLUS_FC64)) ;
    CHECK (MATCH (type_name, "double complex")) ;
    OK (GxB_Type_from_name (&type, type_name)) ;
    CHECK (type == GxB_FC64) ;
    OK (GxB_BinaryOp_ztype_name (type_name, GxB_PLUS_FC32)) ;
    CHECK (MATCH (type_name, "float complex")) ;
    OK (GxB_Type_from_name (&type, type_name)) ;
    CHECK (type == GxB_FC32) ;

    OK (GxB_BinaryOp_xtype_name (type_name, GrB_PLUS_INT16)) ;
    CHECK (MATCH (type_name, "int16_t")) ;
    OK (GxB_Type_from_name (&type, type_name)) ;
    CHECK (type == GrB_INT16) ;
    OK (GxB_BinaryOp_ytype_name (type_name, GrB_PLUS_UINT16)) ;
    CHECK (MATCH (type_name, "uint16_t")) ;
    OK (GxB_Type_from_name (&type, type_name)) ;
    CHECK (type == GrB_UINT16) ;
    OK (GxB_BinaryOp_ztype_name (type_name, GrB_PLUS_UINT32)) ;
    CHECK (MATCH (type_name, "uint32_t")) ;
    OK (GxB_Type_from_name (&type, type_name)) ;
    CHECK (type == GrB_UINT32) ;

    OK (GxB_IndexUnaryOp_xtype_name (type_name, GrB_TRIL)) ;
    CHECK (MATCH (type_name, "")) ;
    OK (GxB_Type_from_name (&type, type_name)) ;
    CHECK (type == NULL) ;

    OK (GxB_IndexUnaryOp_ytype_name (type_name, GrB_TRIL)) ;
    CHECK (MATCH (type_name, "int64_t")) ;
    OK (GxB_Type_from_name (&type, type_name)) ;
    CHECK (type == GrB_INT64) ;
    OK (GxB_IndexUnaryOp_ztype_name (type_name, GrB_VALUELT_INT16)) ;
    CHECK (MATCH (type_name, "bool")) ;
    OK (GxB_Type_from_name (&type, type_name)) ;
    CHECK (type == GrB_BOOL) ;
    OK (GxB_IndexUnaryOp_xtype_name (type_name, GrB_VALUELT_INT16)) ;
    CHECK (MATCH (type_name, "int16_t")) ;
    OK (GxB_Type_from_name (&type, type_name)) ;
    CHECK (type == GrB_INT16) ;

    expected = GrB_NULL_POINTER ;
    ERR (GxB_UnaryOp_xtype_name (NULL, GrB_ABS_INT32)) ;
    ERR (GxB_UnaryOp_ztype_name (NULL, GrB_IDENTITY_INT8)) ;
    ERR (GxB_BinaryOp_xtype_name (NULL, GrB_PLUS_FP32)) ;
    ERR (GxB_BinaryOp_ytype_name (NULL, GrB_PLUS_FP32)) ;
    ERR (GxB_BinaryOp_ztype_name (NULL, GrB_PLUS_FP32)) ;
    ERR (GxB_IndexUnaryOp_xtype_name (NULL, GrB_TRIL)) ;
    ERR (GxB_IndexUnaryOp_ytype_name (NULL, GrB_TRIL)) ;
    ERR (GxB_IndexUnaryOp_ztype_name (NULL, GrB_VALUELT_INT16)) ;

    ERR (GxB_UnaryOp_xtype_name (type_name, NULL)) ;
    ERR (GxB_UnaryOp_ztype_name (type_name, NULL)) ;
    ERR (GxB_BinaryOp_xtype_name (type_name, NULL)) ;
    ERR (GxB_BinaryOp_ytype_name (type_name, NULL)) ;
    ERR (GxB_BinaryOp_ztype_name (type_name, NULL)) ;
    ERR (GxB_IndexUnaryOp_xtype_name (type_name, NULL)) ;
    ERR (GxB_IndexUnaryOp_ytype_name (type_name, NULL)) ;
    ERR (GxB_IndexUnaryOp_ztype_name (type_name, NULL)) ;

    OK (GxB_Type_name (type_name, GrB_BOOL)) ;
    CHECK (MATCH (type_name, "bool")) ;

    OK (GxB_Type_name (type_name, GrB_INT8)) ;
    CHECK (MATCH (type_name, "int8_t")) ;
    OK (GxB_Type_name (type_name, GrB_INT16)) ;
    CHECK (MATCH (type_name, "int16_t")) ;
    OK (GxB_Type_name (type_name, GrB_INT32)) ;
    CHECK (MATCH (type_name, "int32_t")) ;
    OK (GxB_Type_name (type_name, GrB_INT64)) ;
    CHECK (MATCH (type_name, "int64_t")) ;

    OK (GxB_Type_name (type_name, GrB_UINT8)) ;
    CHECK (MATCH (type_name, "uint8_t")) ;
    OK (GxB_Type_name (type_name, GrB_UINT16)) ;
    CHECK (MATCH (type_name, "uint16_t")) ;
    OK (GxB_Type_name (type_name, GrB_UINT32)) ;
    CHECK (MATCH (type_name, "uint32_t")) ;
    OK (GxB_Type_name (type_name, GrB_UINT64)) ;
    CHECK (MATCH (type_name, "uint64_t")) ;

    OK (GxB_Type_name (type_name, GrB_FP32)) ;
    CHECK (MATCH (type_name, "float")) ;
    OK (GxB_Type_name (type_name, GrB_FP64)) ;
    CHECK (MATCH (type_name, "double")) ;

    OK (GxB_Type_name (type_name, GxB_FC32)) ;
    CHECK (MATCH (type_name, "float complex")) ;
    OK (GxB_Type_name (type_name, GxB_FC64)) ;
    CHECK (MATCH (type_name, "double complex")) ;

    OK (GrB_Matrix_new (&A, GrB_FP32, 5, 5)) ;
    ERR (GxB_Matrix_type_name (NULL, A)) ;
    ERR (GxB_Matrix_type_name (type_name, NULL)) ;
    OK (GxB_Matrix_type_name (type_name, A)) ;
    CHECK (MATCH (type_name, "float")) ;
    OK (GrB_Matrix_free (&A)) ;

    OK (GrB_Vector_new (&w, GrB_INT16, 5)) ;
    ERR (GxB_Vector_type_name (NULL, w)) ;
    ERR (GxB_Vector_type_name (type_name, NULL)) ;
    OK (GxB_Vector_type_name (type_name, w)) ;
    CHECK (MATCH (type_name, "int16_t")) ;
    OK (GrB_Vector_free (&w)) ;

    OK (GrB_Scalar_new (&scalar, GrB_BOOL)) ;
    ERR (GxB_Scalar_type_name (NULL, scalar)) ;
    ERR (GxB_Scalar_type_name (type_name, NULL)) ;
    OK (GxB_Scalar_type_name (type_name, scalar)) ;
    CHECK (MATCH (type_name, "bool")) ;
    OK (GrB_Scalar_free (&scalar)) ;

    //--------------------------------------------------------------------------
    // fprint for GrB_IndexUnaryOp
    //--------------------------------------------------------------------------

    OK (GxB_IndexUnaryOp_fprint (GrB_ROWINDEX_INT32,  "rowindex32",  3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_ROWINDEX_INT64,  "rowindex64",  3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_COLINDEX_INT32,  "colindex32",  3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_COLINDEX_INT64,  "colindex64",  3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_DIAGINDEX_INT32, "diagindex32", 3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_DIAGINDEX_INT64, "diagindex64", 3, NULL)) ;

    OK (GxB_IndexUnaryOp_fprint (GrB_TRIL,    "tril",    3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_TRIU,    "triu",    3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_DIAG,    "diag",    3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_OFFDIAG, "offdiag", 3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_COLLE,   "colle",   3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_COLGT,   "colgt",   3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_ROWLE,   "rowle",   3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_ROWGT,   "rowgt",   3, NULL)) ;

    OK (GxB_IndexUnaryOp_fprint (GrB_VALUEEQ_BOOL,   "valueeq_bool",   3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUEEQ_INT8,   "valueeq_int8",   3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUEEQ_INT16,  "valueeq_int16",  3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUEEQ_INT32,  "valueeq_int32",  3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUEEQ_INT64,  "valueeq_int64",  3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUEEQ_UINT8,  "valueeq_uint8",  3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUEEQ_UINT16, "valueeq_uint16", 3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUEEQ_UINT32, "valueeq_uint32", 3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUEEQ_UINT64, "valueeq_uint64", 3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUEEQ_FP32,   "valueeq_fp32",   3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUEEQ_FP64,   "valueeq_fp64",   3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GxB_VALUEEQ_FC32,   "valueeq_fc32",   3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GxB_VALUEEQ_FC64,   "valueeq_fc64",   3, NULL)) ;

    OK (GxB_IndexUnaryOp_fprint (GrB_VALUENE_BOOL,   "valuene_bool",   3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUENE_INT8,   "valuene_int8",   3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUENE_INT16,  "valuene_int16",  3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUENE_INT32,  "valuene_int32",  3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUENE_INT64,  "valuene_int64",  3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUENE_UINT8,  "valuene_uint8",  3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUENE_UINT16, "valuene_uint16", 3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUENE_UINT32, "valuene_uint32", 3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUENE_UINT64, "valuene_uint64", 3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUENE_FP32,   "valuene_fp32",   3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUENE_FP64,   "valuene_fp64",   3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GxB_VALUENE_FC32,   "valuene_fc32",   3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GxB_VALUENE_FC64,   "valuene_fc64",   3, NULL)) ;

    OK (GxB_IndexUnaryOp_fprint (GrB_VALUELT_BOOL,   "valuelt_bool",   3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUELT_INT8,   "valuelt_int8",   3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUELT_INT16,  "valuelt_int16",  3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUELT_INT32,  "valuelt_int32",  3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUELT_INT64,  "valuelt_int64",  3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUELT_UINT8,  "valuelt_uint8",  3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUELT_UINT16, "valuelt_uint16", 3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUELT_UINT32, "valuelt_uint32", 3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUELT_UINT64, "valuelt_uint64", 3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUELT_FP32,   "valuelt_fp32",   3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUELT_FP64,   "valuelt_fp64",   3, NULL)) ;

    OK (GxB_IndexUnaryOp_fprint (GrB_VALUELE_BOOL,   "valuele_bool",   3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUELE_INT8,   "valuele_int8",   3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUELE_INT16,  "valuele_int16",  3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUELE_INT32,  "valuele_int32",  3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUELE_INT64,  "valuele_int64",  3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUELE_UINT8,  "valuele_uint8",  3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUELE_UINT16, "valuele_uint16", 3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUELE_UINT32, "valuele_uint32", 3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUELE_UINT64, "valuele_uint64", 3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUELE_FP32,   "valuele_fp32",   3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUELE_FP64,   "valuele_fp64",   3, NULL)) ;

    OK (GxB_IndexUnaryOp_fprint (GrB_VALUEGT_BOOL,   "valuegt_bool",   3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUEGT_INT8,   "valuegt_int8",   3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUEGT_INT16,  "valuegt_int16",  3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUEGT_INT32,  "valuegt_int32",  3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUEGT_INT64,  "valuegt_int64",  3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUEGT_UINT8,  "valuegt_uint8",  3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUEGT_UINT16, "valuegt_uint16", 3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUEGT_UINT32, "valuegt_uint32", 3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUEGT_UINT64, "valuegt_uint64", 3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUEGT_FP32,   "valuegt_fp32",   3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUEGT_FP64,   "valuegt_fp64",   3, NULL)) ;

    OK (GxB_IndexUnaryOp_fprint (GrB_VALUEGE_BOOL,   "valuege_bool",   3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUEGE_INT8,   "valuege_int8",   3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUEGE_INT16,  "valuege_int16",  3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUEGE_INT32,  "valuege_int32",  3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUEGE_INT64,  "valuege_int64",  3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUEGE_UINT8,  "valuege_uint8",  3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUEGE_UINT16, "valuege_uint16", 3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUEGE_UINT32, "valuege_uint32", 3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUEGE_UINT64, "valuege_uint64", 3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUEGE_FP32,   "valuege_fp32",   3, NULL)) ;
    OK (GxB_IndexUnaryOp_fprint (GrB_VALUEGE_FP64,   "valuege_fp64",   3, NULL)) ;

    expected = GrB_NULL_POINTER ;
    ERR (GxB_IndexUnaryOp_fprint (NULL, "nothing", 3, NULL)) ;
    expected = GrB_INVALID_OBJECT ;
    ERR (GxB_IndexUnaryOp_fprint (GrB_PLUS_FP32, "plus", 3, NULL)) ;

    //--------------------------------------------------------------------------
    // IndexUnaryOp
    //--------------------------------------------------------------------------

    expected = GrB_NULL_POINTER ;
    #if (GxB_IMPLEMENTATION_MAJOR <= 5)
    ERR (GrB_IndexUnaryOp_wait (NULL)) ;
    ERR (GrB_IndexUnaryOp_wait_ (&Banded)) ;
    #else
    ERR (GrB_IndexUnaryOp_wait (NULL, GrB_MATERIALIZE)) ;
    #endif

    OK (GrB_IndexUnaryOp_new (&Banded, banded_idx,
        GrB_BOOL, GrB_INT64, GrB_INT64)) ;
    #if (GxB_IMPLEMENTATION_MAJOR <= 5)
    OK (GrB_IndexUnaryOp_wait_ (&Banded)) ;
    #else
    OK (GrB_IndexUnaryOp_wait_ (Banded, GrB_MATERIALIZE)) ;
    #endif
    OK (GxB_IndexUnaryOp_fprint (Banded, "banded", 3, NULL)) ;

    #undef GrB_IndexUnaryOp_new
    #undef GrM_IndexUnaryOp_new
    OK (GRB (IndexUnaryOp_new) (&UpperBanded, upperbanded_idx,
        GrB_BOOL, GrB_INT64, GrB_INT64)) ;
    OK (GxB_IndexUnaryOp_fprint (UpperBanded, "upperbanded", 3, NULL)) ;

    OK (GRB (IndexUnaryOp_new) (&UpperBanded_int64, upperbanded_idx_int64,
        GrB_INT64, GrB_INT64, GrB_INT64)) ;
    OK (GxB_IndexUnaryOp_fprint (UpperBanded_int64, "upperbanded64", 3, NULL)) ;

    OK (GrB_Matrix_new (&A, GrB_INT64, 5, 6)) ;
    for (int i = 0 ; i < 5 ; i++)
    {
        for (int j = 0 ; j < 6 ; j++)
        {
            OK (GrB_Matrix_setElement_INT64 (A, i*100 + j, i, j)) ;
        }
    }
    #if (GxB_IMPLEMENTATION_MAJOR <= 5)
    OK (GrB_Matrix_wait (&A)) ;
    #else
    OK (GrB_Matrix_wait (A, GrB_MATERIALIZE)) ;
    #endif
    OK (GxB_Matrix_fprint (A, "A", 3, NULL)) ;

    OK (GrB_Matrix_new (&C, GrB_INT64, 5, 6)) ;
    int64_t cnvals ;

    OK (GxB_Matrix_fprint (A, "A for select:banded", 3, NULL)) ;
    OK (GrB_Matrix_select_INT64 (C, NULL, NULL, Banded, A, 1, NULL)) ;
    OK (GxB_Matrix_fprint (C, "C = select:banded (A)", 3, NULL)) ;
    OK (GrB_Matrix_nvals (&cnvals, C)) ;
    CHECK (cnvals == 14) ;
    for (int i = 0 ; i < 5 ; i++)
    {
        for (int j = i-1 ; j <= i+1 ; j++)
        {
            if (j >= 0 && j < 6)
            {
                int64_t cij = -999 ;
                OK (GrB_Matrix_extractElement_INT64 (&cij, C, i, j)) ;
                CHECK (cij == i*100 + j) ;
                cnvals -- ;
            }
        }
    }
    CHECK (cnvals == 0) ;

    OK (GrB_Matrix_apply_IndexOp_INT32 (C, NULL, NULL, Banded, A, 1, NULL)) ;
    OK (GxB_Matrix_fprint (C, "C = apply:banded (A)", 3, NULL)) ;
    OK (GrB_Matrix_nvals (&cnvals, C)) ;
    CHECK (cnvals == 30) ;
    for (int i = 0 ; i < 5 ; i++)
    {
        for (int j = 0 ; j < 6 ; j++)
        {
            int64_t cij = -999 ;
            int d = GB_IABS (j-i) ;
            OK (GrB_Matrix_extractElement_INT64 (&cij, C, i, j)) ;
            CHECK (cij == (d <= 1)) ;
        }
    }

    OK (GrB_Matrix_select_INT64 (C, NULL, NULL, UpperBanded, A, 1, NULL)) ;
    OK (GxB_Matrix_fprint (C, "C = upper_banded (A)", 3, NULL)) ;
    OK (GrB_Matrix_nvals (&cnvals, C)) ;
    CHECK (cnvals == 10) ;
    for (int i = 0 ; i < 5 ; i++)
    {
        for (int j = i ; j <= i+1 ; j++)
        {
            if (j >= 0 && j < 6)
            {
                int64_t cij = -999 ;
                OK (GrB_Matrix_extractElement_INT64 (&cij, C, i, j)) ;
                CHECK (cij == i*100 + j) ;
                cnvals -- ;
            }
        }
    }
    CHECK (cnvals == 0) ;

    OK (GrB_Matrix_new (&E, GrB_INT64, 6, 5)) ;
    OK (GrB_Matrix_select_INT64 (E, NULL, NULL, UpperBanded, A, 1,
        GrB_DESC_T0)) ;
    OK (GxB_Matrix_fprint (E, "E = upper_banded (A')", 3, NULL)) ;
    int64_t envals ;
    OK (GrB_Matrix_nvals (&envals, E)) ;
    CHECK (envals == 9) ;
    for (int i = 0 ; i < 6 ; i++)
    {
        for (int j = i ; j <= i+1 ; j++)
        {
            if (j >= 0 && j < 5)
            {
                int64_t eij = -999 ;
                OK (GrB_Matrix_extractElement_INT64 (&eij, E, i, j)) ;
                CHECK (eij == j*100 + i) ;
                envals -- ;
            }
        }
    }
    CHECK (envals == 0) ;

    OK (GrB_Matrix_apply_IndexOp_INT64 (E, NULL, NULL, UpperBanded, A, 1,
        GrB_DESC_T0)) ;
    OK (GxB_Matrix_fprint (E, "E = apply:upper_banded (A')", 3, NULL)) ;
    OK (GrB_Matrix_nvals (&envals, E)) ;
    CHECK (envals == 30) ;
    for (int i = 0 ; i < 6 ; i++)
    {
        for (int j = 0 ; j < 5 ; j++)
        {
            int64_t eij = -999 ;
            OK (GrB_Matrix_extractElement_INT64 (&eij, E, i, j)) ;
            CHECK (eij == (j == i || j == i+1)) ;
        }
    }

    OK (GrB_Matrix_free (&E)) ;

    OK (GrB_Matrix_new (&E, GrB_BOOL, 6, 5)) ;
    malloc_debug = true ;
    METHOD (GrB_Matrix_apply_IndexOp_INT64 (E, NULL, NULL, UpperBanded, A, 1,
        GrB_DESC_T0)) ;
    malloc_debug = false ;
    OK (GxB_Matrix_fprint (E, "E = apply:upper_banded (A')", 3, NULL)) ;
    OK (GrB_Matrix_nvals (&envals, E)) ;
    CHECK (envals == 30) ;
    for (int i = 0 ; i < 6 ; i++)
    {
        for (int j = 0 ; j < 5 ; j++)
        {
            bool eij = true ;
            OK (GrB_Matrix_extractElement_BOOL (&eij, E, i, j)) ;
            CHECK (eij == (j == i || j == i+1)) ;
        }
    }
    OK (GrB_Matrix_free (&E)) ;

    OK (GrB_Matrix_new (&E, GrB_INT64, 6, 5)) ;
    OK (GrB_Matrix_apply_IndexOp_INT64 (E, NULL, NULL, UpperBanded_int64, A, 1,
        GrB_DESC_T0)) ;
    OK (GxB_Matrix_fprint (E, "E = apply:upper_banded64 (A')", 3, NULL)) ;
    OK (GrB_Matrix_nvals (&envals, E)) ;
    CHECK (envals == 30) ;
    for (int i = 0 ; i < 6 ; i++)
    {
        for (int j = 0 ; j < 5 ; j++)
        {
            int64_t eij = true ;
            OK (GrB_Matrix_extractElement_INT64 (&eij, E, i, j)) ;
            CHECK (eij == (j == i || j == i+1)) ;
        }
    }

    // change A to iso
    OK (GrB_Matrix_assign_INT64 (A, NULL, NULL, 42, GrB_ALL, 5, GrB_ALL, 6,
        NULL)) ;
    OK (GrB_Matrix_apply_IndexOp_INT64 (E, NULL, NULL, UpperBanded_int64, A, 1,
        GrB_DESC_T0)) ;
    OK (GxB_Matrix_fprint (E, "E = apply:upper_banded64 (A')", 3, NULL)) ;
    OK (GrB_Matrix_nvals (&envals, E)) ;
    CHECK (envals == 30) ;
    for (int i = 0 ; i < 6 ; i++)
    {
        for (int j = 0 ; j < 5 ; j++)
        {
            int64_t eij = true ;
            OK (GrB_Matrix_extractElement_INT64 (&eij, E, i, j)) ;
            CHECK (eij == (j == i || j == i+1)) ;
        }
    }

    // make A sparse
    OK (GrB_Matrix_clear (A)) ;
    for (int i = 0 ; i < 5 ; i++)
    {
        for (int j = 0 ; j < 6 ; j++)
        {
            OK (GrB_Matrix_setElement_INT64 (A, i*100 + j, i, j)) ;
        }
    }
    OK (GrB_Matrix_removeElement (A, 0, 0)) ;
    #if (GxB_IMPLEMENTATION_MAJOR <= 5)
    OK (GrB_Matrix_wait (&A)) ;
    #else
    OK (GrB_Matrix_wait (A, GrB_MATERIALIZE)) ;
    #endif
    OK (GxB_Matrix_Option_set_ (A, GxB_SPARSITY_CONTROL, GxB_SPARSE)) ;
    for (int k = 0 ; k <= 1 ; k++)
    {
        printf ("\n %d ##########################################\n", k) ;
        if (k == 1)
        {
            // make A iso
            OK (GrB_Matrix_assign_INT64 (A, NULL, NULL, 99,
                GrB_ALL, 5, GrB_ALL, 6, NULL)) ;
            OK (GrB_Matrix_removeElement (A, 0, 0)) ;
        }
        OK (GxB_Matrix_fprint (A, "A for select:banded", 3, NULL)) ;
        OK (GrB_Matrix_select_INT64 (C, NULL, NULL, Banded, A, 1, NULL)) ;
        OK (GxB_Matrix_fprint (C, "C = select:banded (A)", 3, NULL)) ;
        OK (GrB_Matrix_nvals (&cnvals, C)) ;
        CHECK (cnvals == 13) ;
        for (int i = 0 ; i < 5 ; i++)
        {
            for (int j = i-1 ; j <= i+1 ; j++)
            {
                if (i == 0 && j == 0) continue ;
                if (j >= 0 && j < 6)
                {
                    int64_t cij = -999 ;
                    OK (GrB_Matrix_extractElement_INT64 (&cij, C, i, j)) ;
                    CHECK (cij == ((k == 0) ? (i*100 + j) : 99)) ;
                    cnvals -- ;
                }
            }
        }
        CHECK (cnvals == 0) ;
    }

    OK (GrB_Matrix_free (&A)) ;
    OK (GrB_Matrix_free (&C)) ;
    OK (GrB_Matrix_free (&E)) ;

    // mangle the user-defined operators
    expected = GrB_INVALID_OBJECT ;

    Banded->ztype = NULL ;
    ERR (GxB_IndexUnaryOp_fprint (Banded, "banded", 3, NULL)) ;
    Banded->ztype = GrB_BOOL ;

    Banded->ytype = NULL ;
    ERR (GxB_IndexUnaryOp_fprint (Banded, "banded", 3, NULL)) ;
    Banded->ytype = GrB_INT64 ;

    Banded->xtype = GrB_PLUS_FP32 ;
    ERR (GxB_IndexUnaryOp_fprint (Banded, "banded", 3, NULL)) ;
    Banded->xtype = GrB_INT64 ;

    Banded->idxunop_function = NULL ;
    ERR (GxB_IndexUnaryOp_fprint (Banded, "banded", 3, NULL)) ;
    Banded->idxunop_function = banded_idx ;

    Banded->opcode = 0 ;
    ERR (GB_Operator_check (Banded, "banded", 3, NULL)) ;
    Banded->opcode = GB_USER_idxunop_code ;

    OK (GB_Operator_check (Banded, "banded", 3, NULL)) ;

    OK (GrB_IndexUnaryOp_error_ (&err, Banded)) ;
    CHECK (MATCH (err, "")) ;
    expected = GrB_NULL_POINTER ;
    ERR (GrB_IndexUnaryOp_error (NULL, Banded)) ;

    OK (GrB_IndexUnaryOp_free_ (&Banded)) ;
    OK (GrB_IndexUnaryOp_free_ (&UpperBanded)) ;
    OK (GrB_IndexUnaryOp_free_ (&UpperBanded_int64)) ;

    //--------------------------------------------------------------------------
    // operator check
    //--------------------------------------------------------------------------

    OK (GB_Operator_check (GrB_PLUS_FP32, "plus", 3, NULL)) ;
    OK (GB_Operator_check (GrB_ABS_FP32, "abs", 3, NULL)) ;
    OK (GB_Operator_check (GrB_TRIL, "tril_idx", 3, NULL)) ;
    OK (GB_Operator_check (GxB_TRIL, "tril_selectop", 3, NULL)) ;
    expected = GrB_NULL_POINTER ;
    ERR (GB_Operator_check (NULL, "null", 3, NULL)) ;

    //--------------------------------------------------------------------------
    // ignore_dup
    //--------------------------------------------------------------------------

    OK (GxB_BinaryOp_fprint (GxB_IGNORE_DUP, "ignore_dup", 3, NULL)) ;
    OK (GrB_Matrix_new (&A, GrB_FP32, 5, 5)) ;
    expected = GrB_INVALID_OBJECT ;
    ERR (GrB_Matrix_eWiseAdd_BinaryOp (A, NULL, NULL, GxB_IGNORE_DUP, A, A,
        NULL)) ;
    OK (GrB_Matrix_free_ (&A)) ;

    //--------------------------------------------------------------------------
    // dynamic header
    //--------------------------------------------------------------------------

    OK (GrB_Matrix_new (&A, GrB_FP32, 5, 5)) ;
    OK (GrB_Matrix_setElement_INT64 (A, 2, 0, 0)) ;
    OK (GrB_Matrix_setElement_INT64 (A, 1, 1, 1)) ;
    #if (GxB_IMPLEMENTATION_MAJOR <= 5)
    OK (GrB_Matrix_wait_ (&A)) ;
    #else
    OK (GrB_Matrix_wait_ (A, GrB_MATERIALIZE)) ;
    #endif
    OK (GxB_Matrix_fprint (A, "A for static/dynamic header", 3, NULL)) ;
    struct GB_Matrix_opaque A2_header ;
    GrB_Matrix A2 = GB_clear_static_header (&A2_header) ;
    GB_undo_dynamic_header (NULL, NULL, NULL) ;
    GB_undo_dynamic_header (&A, A, NULL) ;
    CHECK (A != NULL) ;
    OK (GxB_Matrix_fprint (A, "A unchanged", 3, NULL)) ;
    GB_undo_dynamic_header (&A, A2, NULL) ;
    OK (GxB_Matrix_fprint (A2, "A2 with static header", 3, NULL)) ;
    CHECK (A == NULL) ;

    OK (GB_do_dynamic_header (&A, NULL, NULL)) ;
    CHECK (A == NULL) ;

    OK (GB_do_dynamic_header (&A, A2, NULL)) ;
    CHECK (A != NULL) ;

    OK (GB_do_dynamic_header (&A, A, NULL)) ;
    CHECK (A != NULL) ;

    GB_undo_dynamic_header (&A, A2, NULL) ;
    CHECK (A == NULL) ;

    GB_Global_malloc_debug_count_set (0) ;
    GB_Global_malloc_debug_set (true) ;
    expected = GrB_OUT_OF_MEMORY ;
    ERR (GB_do_dynamic_header (&A, A2, NULL)) ;
    CHECK (A == NULL) ;
    GB_Global_malloc_debug_set (false) ;

    OK (GrB_Matrix_free_ (&A2)) ;

    //--------------------------------------------------------------------------
    // apply with user idxunop
    //--------------------------------------------------------------------------

    OK (GxB_Type_new (&MyType, sizeof (mytype), "mytype", "")) ;
    OK (GrB_Matrix_new (&A, MyType, 4, 4)) ;
    OK (GrB_Matrix_setElement_UDT (A, &scalar1, 2, 3)) ;
    #if (GxB_IMPLEMENTATION_MAJOR <= 5)
    OK (GrB_Matrix_wait_ (&A)) ;
    #else
    OK (GrB_Matrix_wait_ (A, GrB_MATERIALIZE)) ;
    #endif
    OK (GxB_Matrix_fprint (A, "A of MyType", 3, NULL)) ;

    GB_Global_malloc_debug_count_set (0) ;
    GB_Global_malloc_debug_set (true) ;
    expected = GrB_OUT_OF_MEMORY ;
    ERR (GxB_IndexUnaryOp_new (&Banded, banded_idx,
        GrB_BOOL, GrB_INT64, GrB_INT64, "banded_index", "")) ;
    CHECK (Banded == NULL) ;
    GB_Global_malloc_debug_set (false) ;

    OK (GxB_IndexUnaryOp_new (&Banded, banded_idx,
        GrB_BOOL, GrB_INT64, GrB_INT64, "banded_index", "")) ;

    expected = GrB_DOMAIN_MISMATCH ;
    OK (GrB_Matrix_new (&C, GrB_BOOL, 4, 4)) ;
    ERR (GrB_Matrix_apply_IndexOp_INT32 (C, NULL, NULL, Banded, A, 1, NULL)) ;
    OK (GrB_Matrix_error_ (&err, C)) ;
    printf ("error expected: %s\n", err) ;

    expected = GrB_DOMAIN_MISMATCH ;
    OK (GrB_Scalar_new (&scalar, MyType)) ;
    ERR (GrB_Matrix_apply_IndexOp_Scalar (C, NULL, NULL, Banded, C, scalar,
        NULL)) ;
    OK (GrB_Matrix_error_ (&err, C)) ;
    printf ("error expected: %s\n", err) ;
    OK (GrB_Scalar_free_ (&scalar)) ;
    OK (GrB_Matrix_free_ (&C)) ;
    OK (GrB_IndexUnaryOp_free_ (&Banded)) ;

    //--------------------------------------------------------------------------
    // serialize
    //--------------------------------------------------------------------------

    void *blob = NULL ;
    GrB_Index blob_size = 0, blob_size2 = 0 ;
    OK (GxB_Matrix_serialize (&blob, &blob_size, A, NULL)) ;
    OK (GxB_Matrix_deserialize (&C, MyType, blob, blob_size, NULL)) ;
    OK (GxB_Matrix_fprint (C, "C of MyType", 3, NULL)) ;

    OK (GxB_deserialize_type_name (type_name, blob, blob_size)) ;
    CHECK (MATCH (type_name, "mytype")) ;

    // mangle the blob
    expected = GrB_INVALID_OBJECT ;
    ERR (GxB_deserialize_type_name (type_name, blob, 2)) ;
    ERR (GxB_Matrix_deserialize (&E, MyType, blob, 2, NULL)) ;
    CHECK (E == NULL) ;

    ERR (GxB_deserialize_type_name (type_name, blob, 200000)) ;
    int64_t *blob64 = (int64_t *) blob ;
    blob_size2 = GB_BLOB_HEADER_SIZE + 2 ;
    blob64 [0] = blob_size2 ;
    ERR (GxB_deserialize_type_name (type_name, blob, blob_size2)) ;
    ERR (GxB_Matrix_deserialize (&E, MyType, blob, blob_size2, NULL)) ;
    CHECK (E == NULL) ;
    blob64 [0] = blob_size ;

    OK (GxB_deserialize_type_name (type_name, blob, blob_size)) ;
    CHECK (MATCH (type_name, "mytype")) ;

    int32_t *blob32 = (int32_t *) blob ;
    blob32 [2] = -1 ;
    ERR (GxB_deserialize_type_name (type_name, blob, blob_size)) ;
    blob32 [2] = GB_UDT_code ;

    OK (GxB_deserialize_type_name (type_name, blob, blob_size)) ;
    CHECK (MATCH (type_name, "mytype")) ;

    expected = GrB_DOMAIN_MISMATCH ;
    ERR (GxB_Matrix_deserialize (&E, NULL, blob, blob_size, NULL)) ;
    ERR (GxB_Matrix_deserialize (&E, GrB_BOOL, blob, blob_size, NULL)) ;
    ERR (GxB_Matrix_deserialize (&E, GrB_FP64, blob, blob_size, NULL)) ;
    printf ("size of mytype: %d\n", sizeof (mytype)) ;

    OK (GrB_Matrix_free_ (&A)) ;
    OK (GrB_Matrix_free_ (&C)) ;
    OK (GrB_Type_free_ (&MyType)) ;
    mxFree (blob) ;

    OK (GrB_Matrix_new (&A, GrB_FP32, 3, 4)) ;
    OK (GrB_Matrix_setElement_FP32 (A, (float) 1.1, 2, 2)) ;
    OK (GrB_Matrix_setElement_FP32 (A, (float) 9.1, 1, 1)) ;
    #if (GxB_IMPLEMENTATION_MAJOR <= 5)
    OK (GrB_Matrix_wait_ (&A)) ;
    #else
    OK (GrB_Matrix_wait_ (A, GrB_MATERIALIZE)) ;
    #endif
    OK (GxB_Matrix_fprint (A, "A for serialize", 3, NULL)) ;
    OK (GxB_Matrix_serialize (&blob, &blob_size, A, NULL)) ;
    expected = GrB_DOMAIN_MISMATCH ;
    ERR (GxB_Matrix_deserialize (&C, GrB_INT32, blob, blob_size, NULL)) ;
    OK (GxB_Matrix_deserialize (&C, GrB_FP32, blob, blob_size, NULL)) ;
    OK (GxB_Matrix_fprint (C, "C from deserialize", 3, NULL)) ;

    mxFree (blob) ;

    blob_size = 2 ;
    blob = mxMalloc (2) ;
    expected = GrB_INSUFFICIENT_SPACE ;
    ERR (GrB_Matrix_serialize (blob, &blob_size, A)) ;

    OK (GrB_Matrix_free_ (&A)) ;
    OK (GrB_Matrix_free_ (&C)) ;
    mxFree (blob) ;

    //--------------------------------------------------------------------------
    // descriptor
    //--------------------------------------------------------------------------

    OK (GrB_Descriptor_new (&desc)) ;
    OK (GxB_Desc_set (desc, GxB_IMPORT, GxB_SECURE_IMPORT)) ;
    OK (GxB_Descriptor_fprint (desc, "desc with secure import", 3, NULL)) ;

    int method = -999 ;
    OK (GxB_Desc_get (desc, GxB_IMPORT, &method)) ;
    CHECK (method == GxB_SECURE_IMPORT) ;

    OK (GxB_Desc_set (desc, GxB_COMPRESSION, GxB_COMPRESSION_LZ4HC + 4)) ;
    OK (GxB_Desc_get (desc, GxB_COMPRESSION, &method)) ;
    CHECK (method == GxB_COMPRESSION_LZ4HC + 4)

    OK (GxB_Descriptor_fprint (desc, "desc with secure & lz4hc+4", 3, NULL)) ;
    OK (GrB_Descriptor_free_ (&desc)) ;

    //--------------------------------------------------------------------------
    // export hint
    //--------------------------------------------------------------------------

    OK (GrB_Matrix_new (&A, GrB_FP32, 3, 4)) ;
    OK (GrB_Matrix_assign_FP32 (A, NULL, NULL, (float) 1, GrB_ALL, 3,
        GrB_ALL, 4, NULL)) ;
    OK (GrB_Matrix_setElement_FP32 (A, (float) 32, 0, 0)) ;

    GrB_Format fmt ;

    OK (GxB_Matrix_Option_set (A, GxB_FORMAT, GxB_BY_ROW)) ;
    OK (GxB_Matrix_Option_set (A, GxB_SPARSITY_CONTROL, GxB_HYPERSPARSE)) ;
    OK (GrB_Matrix_exportHint (&fmt, A)) ;
    CHECK (fmt == GrB_COO_FORMAT) ;
    OK (GxB_Matrix_Option_set (A, GxB_SPARSITY_CONTROL, GxB_SPARSE)) ;
    OK (GrB_Matrix_exportHint (&fmt, A)) ;
    CHECK (fmt == GrB_CSR_FORMAT) ;
    OK (GxB_Matrix_Option_set (A, GxB_SPARSITY_CONTROL, GxB_BITMAP)) ;
    OK (GrB_Matrix_exportHint (&fmt, A)) ;
    CHECK (fmt == GrB_CSR_FORMAT) ;
    OK (GxB_Matrix_Option_set (A, GxB_SPARSITY_CONTROL, GxB_FULL)) ;
    OK (GrB_Matrix_exportHint (&fmt, A)) ;
    CHECK (fmt == GrB_CSR_FORMAT) ;
//  CHECK (fmt == GrB_DENSE_ROW_FORMAT) ;

    OK (GxB_Matrix_Option_set (A, GxB_FORMAT, GxB_BY_COL)) ;
    OK (GxB_Matrix_Option_set (A, GxB_SPARSITY_CONTROL, GxB_HYPERSPARSE)) ;
    OK (GrB_Matrix_exportHint (&fmt, A)) ;
    CHECK (fmt == GrB_COO_FORMAT) ;
    OK (GxB_Matrix_Option_set (A, GxB_SPARSITY_CONTROL, GxB_SPARSE)) ;
    OK (GrB_Matrix_exportHint (&fmt, A)) ;
    CHECK (fmt == GrB_CSC_FORMAT) ;
    OK (GxB_Matrix_Option_set (A, GxB_SPARSITY_CONTROL, GxB_BITMAP)) ;
    OK (GrB_Matrix_exportHint (&fmt, A)) ;
    CHECK (fmt == GrB_CSC_FORMAT) ;
    OK (GxB_Matrix_Option_set (A, GxB_SPARSITY_CONTROL, GxB_FULL)) ;
    OK (GrB_Matrix_exportHint (&fmt, A)) ;
    CHECK (fmt == GrB_CSC_FORMAT) ;
//  CHECK (fmt == GrB_DENSE_COL_FORMAT) ;

    expected = GrB_NULL_POINTER ;
    ERR (GrB_Matrix_exportHint (NULL, A)) ;
    ERR (GrB_Matrix_exportHint (&fmt, NULL)) ;
    ERR (GrB_Matrix_exportHint (NULL, NULL)) ;

    OK (GrB_Matrix_free_ (&A)) ;

    //--------------------------------------------------------------------------
    // conform_hyper
    //--------------------------------------------------------------------------

    OK (GrB_Matrix_new (&A, GrB_FP32, 100, 100)) ;
    OK (GrB_Matrix_setElement_FP32 (A, 1, 0, 0)) ;
    #if (GxB_IMPLEMENTATION_MAJOR <= 5)
    OK (GrB_Matrix_wait_ (&A)) ;
    #else
    OK (GrB_Matrix_wait_ (A, GrB_MATERIALIZE)) ;
    #endif
    A->nvec_nonempty = -1 ;
    OK (GB_conform_hyper (A, NULL)) ;
    OK (GxB_Matrix_fprint (A, "A conformed", 3, NULL)) ;
    OK (GrB_Matrix_free_ (&A)) ;

    //--------------------------------------------------------------------------
    // import/export
    //--------------------------------------------------------------------------

    GrB_Index Ap_len = 5 ;
    GrB_Index Ai_len = 16 ;
    GrB_Index Ax_len = 16 ;
    GrB_Index *Ap = mxCalloc (Ap_len , sizeof (GrB_Index)) ;
    GrB_Index *Ai = mxCalloc (Ax_len, sizeof (GrB_Index)) ;
    float *Ax = mxCalloc (Ax_len, sizeof (float))  ;
    OK (GrB_Matrix_new (&A, GrB_FP32, 4, 4)) ;
    OK (GrB_Matrix_setElement_FP32 (A, 1, 0, 0)) ;
//  expected = GrB_INVALID_VALUE ;
//  ERR (GrB_Matrix_export_FP32 (Ap, Ai, Ax, &Ap_len, &Ai_len, &Ax_len, GrB_DENSE_ROW_FORMAT, A)) ;
//  ERR (GrB_Matrix_export_FP32 (Ap, Ai, Ax, &Ap_len, &Ai_len, &Ax_len, GrB_DENSE_COL_FORMAT, A)) ;
    OK (GrB_Matrix_assign_FP32 (A, NULL, NULL, (float) 2.0, GrB_ALL, 4,
        GrB_ALL, 4, NULL)) ;
    OK (GxB_Matrix_fprint (A, "A iso to export", 3, NULL)) ;
    OK (GrB_Matrix_export_FP32 (Ap, Ai, Ax, &Ap_len, &Ai_len, &Ax_len, GrB_CSC_FORMAT, A)) ;
//  OK (GrB_Matrix_export_FP32 (Ap, Ai, Ax, &Ap_len, &Ai_len, &Ax_len, GrB_DENSE_COL_FORMAT, A)) ;
    for (int i = 0 ; i < 16 ; i++)
    {
        CHECK (Ax [i] == 2.0) ;
    }

    expected = GrB_INSUFFICIENT_SPACE ;
    Ap_len = 1 ;
    ERR (GrB_Matrix_export_FP32 (Ap, Ai, Ax, &Ap_len, &Ai_len, &Ax_len, GrB_CSC_FORMAT, A)) ;
    Ap_len = 5 ;
    Ai_len = 1 ;
    ERR (GrB_Matrix_export_FP32 (Ap, Ai, Ax, &Ap_len, &Ai_len, &Ax_len, GrB_CSC_FORMAT, A)) ;
    Ai_len = 16 ;
    Ax_len = 1 ;
    ERR (GrB_Matrix_export_FP32 (Ap, Ai, Ax, &Ap_len, &Ai_len, &Ax_len, GrB_CSC_FORMAT, A)) ;
    ERR (GrB_Matrix_export_FP32 (Ap, Ai, Ax, &Ap_len, &Ai_len, &Ax_len, GrB_COO_FORMAT, A)) ;
    ERR (GrB_Matrix_export_FP32 (Ap, Ai, Ax, &Ap_len, &Ai_len, &Ax_len, GrB_COO_FORMAT, A)) ;
    Ax_len = 16 ;

    expected = GrB_INVALID_VALUE ;
    ERR (GrB_Matrix_export_FP32 (Ap, Ai, Ax, &Ap_len, &Ai_len, &Ax_len, -1, A)) ;

    OK (GrB_Matrix_free_ (&A)) ;

    expected = GrB_INVALID_VALUE ;
    ERR (GrB_Matrix_import_FP32 (&A, GrB_FP32, 2 * GB_NMAX, 1, Ap, Ai, Ax,
        5, 16, 16, GrB_CSR_FORMAT)) ;
    CHECK (A == NULL) ;

    ERR (GrB_Matrix_import_FP32 (&A, GrB_FP32, 100, 100, Ap, Ai, Ax,
        5, 16, 16, GrB_CSC_FORMAT)) ;
    CHECK (A == NULL) ;

    for (int j = 0 ; j <= 4 ; j++)
    { 
        Ap [j] = j ;
    }
    ERR (GrB_Matrix_import_FP32 (&A, GrB_FP32, 4, 4, Ap, Ai, Ax,
        5, 3, 3, GrB_CSC_FORMAT)) ;
    CHECK (A == NULL) ;

//  ERR (GrB_Matrix_import_FP32 (&A, GrB_FP32, 4, 4, Ap, Ai, Ax,
//      0, 0, 3, GrB_DENSE_COL_FORMAT)) ;
//  CHECK (A == NULL) ;

    ERR (GrB_Matrix_import_FP32 (&A, GrB_FP32, 4, 4, Ap, Ai, Ax,
        5, 6, 7, GrB_COO_FORMAT)) ;
    CHECK (A == NULL) ;

    mxFree (Ap) ;
    mxFree (Ai) ;
    mxFree (Ax) ;

    //--------------------------------------------------------------------------
    // build with duplicates
    //--------------------------------------------------------------------------

    GrB_Index *I = mxCalloc (4, sizeof (GrB_Index)) ;
    GrB_Index *J = mxCalloc (4, sizeof (GrB_Index)) ;
    double *X    = mxCalloc (4, sizeof (double)) ;
    expected = GrB_INVALID_VALUE ;
    OK (GrB_Matrix_new (&A, GrB_FP64, 5, 5)) ;
    ERR (GrB_Matrix_build (A, I, J, X, 4, NULL)) ;
    mxFree (I) ;
    mxFree (J) ;
    mxFree (X) ;
    OK (GrB_Matrix_free_ (&A)) ;

    //--------------------------------------------------------------------------
    // select with idxunop
    //--------------------------------------------------------------------------

    OK (GrB_Matrix_new (&A, GrB_FP64, 5, 5)) ;
    for (int i = 0 ; i < 5 ; i++)
    {
        OK (GrB_Matrix_setElement_FP64 (A, (double) i, i, i)) ;
    }
    OK (GxB_Matrix_Option_set_ (A, GxB_SPARSITY_CONTROL, GxB_SPARSE)) ;
    OK (GrB_Scalar_new (&scalar, GrB_FP64)) ;
    expected = GrB_EMPTY_OBJECT ;
    ERR (GrB_Matrix_select_Scalar (A, NULL, NULL, GrB_VALUEEQ_FP64, A, scalar,
        NULL)) ;

    OK (GrB_Scalar_setElement_FP64 (scalar, 3)) ;
    OK (GxB_Type_new (&MyType, sizeof (mytype), "mytype", "")) ;

    expected = GrB_DOMAIN_MISMATCH ;

    printf ("(1)------------------------------------------------\n") ;
    OK (GRB (IndexUnaryOp_new) (&Gunk, donothing, MyType, MyType, MyType)) ;
    ERR (GrB_Matrix_select_Scalar (A, NULL, NULL, Gunk, A, scalar, NULL)) ;
    OK (GrB_Matrix_error_ (&err, A)) ;
    printf ("\nexpected error: %s\n", err) ;
    OK (GrB_IndexUnaryOp_free_ (&Gunk)) ;

    printf ("(2)------------------------------------------------\n") ;
    OK (GRB (IndexUnaryOp_new) (&Gunk, donothing, GrB_BOOL, MyType, MyType)) ;
    ERR (GrB_Matrix_select_Scalar (A, NULL, NULL, Gunk, A, scalar, NULL)) ;
    OK (GrB_Matrix_error_ (&err, A)) ;
    printf ("\nexpected error: %s\n", err) ;
    OK (GrB_IndexUnaryOp_free_ (&Gunk)) ;

    printf ("(3)------------------------------------------------\n") ;
    OK (GRB (IndexUnaryOp_new) (&Gunk, donothing, GrB_BOOL, GrB_FP64, MyType)) ;
    ERR (GrB_Matrix_select_Scalar (A, NULL, NULL, Gunk, A, scalar, NULL)) ;
    OK (GrB_Matrix_error_ (&err, A)) ;
    printf ("\nexpected error: %s\n", err) ;
    OK (GrB_IndexUnaryOp_free_ (&Gunk)) ;

    printf ("(4)------------------------------------------------\n") ;
    OK (GRB (IndexUnaryOp_new) (&Gunk, donothing, MyType, GrB_FP64, GrB_FP64)) ;
    ERR (GrB_Matrix_select_Scalar (A, NULL, NULL, Gunk, A, scalar, NULL)) ;
    OK (GrB_Matrix_error_ (&err, A)) ;
    printf ("\nexpected error: %s\n", err) ;
    OK (GrB_IndexUnaryOp_free_ (&Gunk)) ;

    OK (GrB_Type_free_ (&MyType)) ;

    // change A to iso
    OK (GrB_Matrix_assign_FP64 (A, A, NULL, (double) 3, GrB_ALL, 5,
        GrB_ALL, 5, GrB_DESC_S)) ;
    OK (GrB_Matrix_select_Scalar (A, NULL, NULL, GrB_VALUEEQ_FP32,
        A, scalar, NULL)) ;
    OK (GxB_Matrix_fprint (A, "A iso select output", 3, NULL)) ;
    int64_t anvals ;
    OK (GrB_Matrix_nvals (&anvals, A)) ;
    CHECK (anvals == 5) ;

    OK (GrB_Matrix_select_INT64 (A, NULL, NULL, GrB_COLLE,
        A, (int64_t) 2, NULL)) ;
    OK (GxB_Matrix_fprint (A, "A iso select COLLE output", 3, NULL)) ;
    OK (GrB_Matrix_nvals (&anvals, A)) ;
    CHECK (anvals == 3) ;

    OK (GrB_Matrix_assign_FP64 (A, NULL, NULL, (double) 3, GrB_ALL, 5,
        GrB_ALL, 5, NULL)) ;
    OK (GrB_Matrix_select_INT64 (A, NULL, NULL, GrB_COLGT,
        A, (int64_t) 2, NULL)) ;
    OK (GxB_Matrix_fprint (A, "A iso select COLGT output", 3, NULL)) ;
    OK (GrB_Matrix_nvals (&anvals, A)) ;
    CHECK (anvals == 10) ;

    OK (GrB_Matrix_assign_FP64 (A, NULL, NULL, (double) 3, GrB_ALL, 5,
        GrB_ALL, 5, NULL)) ;
    OK (GrB_Matrix_select_INT64 (A, NULL, NULL, GrB_ROWGT,
        A, (int64_t) 2, NULL)) ;
    OK (GxB_Matrix_fprint (A, "A iso select ROWGT output", 3, NULL)) ;
    OK (GrB_Matrix_nvals (&anvals, A)) ;
    CHECK (anvals == 10) ;

    OK (GrB_Matrix_assign_FP64 (A, NULL, NULL, (double) 3, GrB_ALL, 5,
        GrB_ALL, 5, NULL)) ;
    OK (GrB_Matrix_select_INT64 (A, NULL, NULL, GrB_COLINDEX_INT64,
        A, (int64_t) -2, NULL)) ;
    OK (GxB_Matrix_fprint (A, "A iso select COLINDEX output", 3, NULL)) ;
    OK (GrB_Matrix_nvals (&anvals, A)) ;
    CHECK (anvals == 20) ;

    OK (GxB_Type_new (&MyInt64, sizeof (int64_t), "myint64", "")) ;
    OK (GxB_IndexUnaryOp_new (&Banded, banded_idx,
        GrB_BOOL, GrB_INT64, MyInt64, "banded_index", "")) ;
    OK (GrB_Matrix_assign_FP64 (A, NULL, NULL, (double) 3, GrB_ALL, 5,
        GrB_ALL, 5, NULL)) ;
    int64_t one = 1 ;
    OK (GrB_Matrix_select_UDT (A, NULL, NULL, Banded, A, &one, NULL)) ;
    OK (GxB_Matrix_fprint (A, "A iso select Banded output", 3, NULL)) ;
    OK (GrB_Matrix_nvals (&anvals, A)) ;
    CHECK (anvals == 13) ;

    OK (GrB_Vector_new (&w, GrB_INT64, 5)) ;
    for (int i = 0 ; i < 5 ; i++)
    { 
        OK (GrB_Vector_setElement_INT64 (w, (int64_t) i, i)) ;
    }
    OK (GxB_Vector_fprint (w, "w for select Banded", 3, NULL)) ;
    OK (GrB_Vector_select_UDT (w, NULL, NULL, Banded, w, &one, NULL)) ;
    OK (GxB_Matrix_fprint (w, "w from select Banded output", 3, NULL)) ;
    OK (GrB_Vector_nvals (&anvals, w)) ;
    CHECK (anvals == 2) ;

    OK (GrB_Vector_assign_INT64 (w, NULL, NULL, (int64_t) 3, GrB_ALL, 5,
        NULL)) ;
    OK (GxB_Vector_fprint (w, "w for apply Banded ", 3, NULL)) ;
    OK (GrB_Vector_apply_IndexOp_UDT (w, NULL, NULL, Banded, w, &one, NULL)) ;
    OK (GxB_Vector_fprint (w, "w from apply Banded output", 3, NULL)) ;
    for (int i = 0 ; i < 5 ; i++)
    { 
        int64_t wi = 3 ;
        OK (GrB_Vector_extractElement_INT64 (&wi, w, i)) ;
        CHECK (wi == (i <= 1)) ;
    }

    OK (GrB_Matrix_free_ (&A)) ;
    OK (GrB_Matrix_new (&A, GrB_INT64, 5, 5)) ;
    OK (GrB_Matrix_assign_INT64 (A, NULL, NULL, (int64_t) 3, GrB_ALL, 5,
        GrB_ALL, 5, NULL)) ;
    OK (GxB_Matrix_fprint (A, "A for apply Banded ", 3, NULL)) ;
    OK (GrB_Matrix_apply_IndexOp_UDT (A, NULL, NULL, Banded, A, &one, NULL)) ;
    OK (GxB_Matrix_fprint (A, "A from apply Banded output", 3, NULL)) ;
    for (int i = 0 ; i < 5 ; i++)
    { 
        for (int j = 0 ; j < 5 ; j++)
        {
            int64_t aij = 3 ;
            OK (GrB_Matrix_extractElement_INT64 (&aij, A, i, j)) ;
            CHECK (aij == (GB_IABS (j-i) <= 1)) ;
        }
    }

    OK (GrB_IndexUnaryOp_free_ (&Banded)) ;
    OK (GrB_Scalar_free_ (&scalar)) ;
    OK (GrB_Matrix_free_ (&A)) ;
    OK (GrB_Vector_free_ (&w)) ;

    //--------------------------------------------------------------------------
    // apply with UDT
    //--------------------------------------------------------------------------

    GrB_BinaryOp Add = NULL ;
    OK (GrB_BinaryOp_new (&Add, add_int64, MyInt64, MyInt64, MyInt64)) ;
    int64_t four = 4 ;
    OK (GrB_Matrix_new (&A, MyInt64, 4, 4)) ;
    for (int i = 0 ; i < 4 ; i++)
    {
        for (int j = 0 ; j < 4 ; j++)
        {
            int64_t aij = i * 1000 + j ;
            OK (GrB_Matrix_setElement_UDT (A, &aij, i, j)) ;
        }
    }

    OK (GrB_Matrix_apply_BinaryOp2nd_UDT (A, NULL, NULL, Add, A, &four,
        NULL)) ;
    for (int i = 0 ; i < 4 ; i++)
    {
        for (int j = 0 ; j < 4 ; j++)
        {
            int64_t aij = -1 ;
            OK (GrB_Matrix_extractElement_UDT (&aij, A, i, j)) ;
            // printf ("i %d j %d aij %ld\n", i, j, aij) ;
            CHECK (aij == 2 * (i *1000 + j) + 4) ;
        }
    }

    OK (GrB_Matrix_apply_BinaryOp1st_UDT (A, NULL, NULL, Add, &four, A,
        NULL)) ;
    for (int i = 0 ; i < 4 ; i++)
    {
        for (int j = 0 ; j < 4 ; j++)
        {
            int64_t aij = -1 ;
            OK (GrB_Matrix_extractElement_UDT (&aij, A, i, j)) ;
            // printf ("i %d j %d aij %ld\n", i, j, aij) ;
            CHECK (aij == 8 + (2 * (i *1000 + j) + 4)) ;
        }
    }

    OK (GrB_Vector_new (&w, MyInt64, 4)) ;
    for (int i = 0 ; i < 4 ; i++)
    {
        int64_t wi = i ;
        OK (GrB_Vector_setElement_UDT (w, &wi, i)) ;
    }

    OK (GrB_Vector_apply_BinaryOp2nd_UDT (w, NULL, NULL, Add, w, &four,
        NULL)) ;
    for (int i = 0 ; i < 4 ; i++)
    {
        int64_t wi = -1 ;
        OK (GrB_Vector_extractElement_UDT (&wi, w, i)) ;
        CHECK (wi == 2 * (i) + 4) ;
    }

    OK (GrB_Vector_apply_BinaryOp1st_UDT (w, NULL, NULL, Add, &four, w,
        NULL)) ;
    for (int i = 0 ; i < 4 ; i++)
    {
        int64_t wi = -1 ;
        OK (GrB_Vector_extractElement_UDT (&wi, w, i)) ;
        CHECK (wi == 8 + (2 * (i) + 4)) ;
    }

    OK (GrB_Type_free_ (&MyInt64)) ;
    OK (GrB_Matrix_free_ (&A)) ;
    OK (GrB_Vector_free_ (&w)) ;
    OK (GrB_BinaryOp_free_ (&Add)) ;

    //--------------------------------------------------------------------------
    // iso in-place apply
    //--------------------------------------------------------------------------

    OK (GrB_Matrix_new (&A, GrB_INT64, 5, 4)) ;
    OK (GrB_Matrix_assign_INT64_ (A, NULL, NULL, (int64_t) 1, GrB_ALL, 5,
        GrB_ALL, 4, NULL)) ;
    OK (GxB_Matrix_fprint (A, "A iso", 3, NULL)) ;
    OK (GxB_Global_Option_set (GxB_BURBLE, true)) ;
    OK (GrB_Matrix_apply_IndexOp_INT64_ (A, NULL, NULL, GrB_ROWINDEX_INT64, A,
        (int64_t) 0, NULL)) ;
    OK (GxB_Global_Option_set (GxB_BURBLE, false)) ;
    OK (GxB_Matrix_fprint (A, "A after apply rowindex", 3, NULL)) ;
    for (int i = 0 ; i < 5 ; i++)
    {
        for (int j = 0 ; j < 4 ; j++)
        {
            int64_t aij = -1 ;
            OK (GrB_Matrix_extractElement (&aij, A, i, j)) ;
            CHECK (aij == i) ;
        }
    }
    OK (GrB_Matrix_free_ (&A)) ;

    //--------------------------------------------------------------------------
    // wrapup
    //--------------------------------------------------------------------------

    GB_mx_put_global (true) ;   
    printf ("\nGB_mex_about5: all tests passed\n\n") ;
}

