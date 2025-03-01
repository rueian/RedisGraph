//------------------------------------------------------------------------------
// GB_AxB_saxpy_generic: compute C=A*B, C<M>=A*B, or C<!M>=A*B in parallel
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// GB_AxB_saxpy_generic computes C=A*B, C<M>=A*B, or C<!M>=A*B in parallel,
// with arbitrary types and operators.  C can have any sparsity pattern:
// hyper, sparse, bitmap, or full.  For all cases, the four matrices C, M
// (if present), A, and B have the same format (by-row or by-column), or they
// represent implicitly transposed matrices with the same effect.  This method
// does not handle the dot-product methods, which compute C=A'*B if A and B
// are held by column, or equivalently A*B' if both are held by row.

// This method uses GB_AxB_saxpy_template.c to implement two meta-methods,
// each of which can contain further specialized methods (such as the fine/
// coarse x Gustavson/Hash, mask/no-mask methods in saxpy3):

// saxpy3: general purpose method, where C is sparse or hypersparse,
//          via GB_AxB_saxpy3_template.c.  SaxpyTasks holds the (fine/coarse x
//          Gustavson/Hash) tasks constructed by GB_AxB_saxpy3_slice*.

// bitmap_saxpy: general purpose method, where C is bitmap or full, via
//          GB_bitmap_AxB_saxpy_template.c.  The method constructs its own
//          tasks in workspace defined and freed in that template.

// C is not iso.

//------------------------------------------------------------------------------

#include "GB_mxm.h"
#include "GB_ek_slice.h"
#include "GB_binop.h"
#include "GB_bracket.h"
#include "GB_sort.h"
#include "GB_atomics.h"
#include "GB_ek_slice_search.c"
#include "GB_bitmap_assign_methods.h"

GrB_Info GB_AxB_saxpy_generic
(
    GrB_Matrix C,                   // any sparsity
    const GrB_Matrix M,
    bool Mask_comp,
    const bool Mask_struct,
    const bool M_in_place,          // ignored if C is bitmap
    const GrB_Matrix A,
    bool A_is_pattern,
    const GrB_Matrix B,
    bool B_is_pattern,
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    const int saxpy_method,         // saxpy3 or bitmap method
    // for saxpy3 only:
    GB_saxpy3task_struct *restrict SaxpyTasks, // NULL if C is bitmap
    int ntasks,
    int nfine,
    int nthreads,
    const int do_sort,              // if true, sort in saxpy3
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // get operators, functions, workspace, contents of A, B, and C
    //--------------------------------------------------------------------------

    GrB_BinaryOp mult = semiring->multiply ;
    GrB_Monoid add = semiring->add ;
    GB_void *identity = (GB_void *) add->identity ;
    ASSERT (mult->ztype == add->op->ztype) ;
    ASSERT (mult->ztype == C->type) ;

    GxB_binary_function fmult = mult->binop_function ;    // NULL if positional
    GxB_binary_function fadd  = add->op->binop_function ;
    GB_Opcode opcode = mult->opcode ;
    bool op_is_positional = GB_OPCODE_IS_POSITIONAL (opcode) ;

    size_t csize = C->type->size ;
    size_t asize = A_is_pattern ? 0 : A->type->size ;
    size_t bsize = B_is_pattern ? 0 : B->type->size ;

    size_t xsize = mult->xtype->size ;
    size_t ysize = mult->ytype->size ;

    // scalar workspace: because of typecasting, the x/y types need not
    // be the same as the size of the A and B types.
    // flipxy false: aik = (xtype) A(i,k) and bkj = (ytype) B(k,j)
    // flipxy true:  aik = (ytype) A(i,k) and bkj = (xtype) B(k,j)
    size_t aik_size = flipxy ? ysize : xsize ;
    size_t bkj_size = flipxy ? xsize : ysize ;

    GB_cast_function cast_A, cast_B ;
    if (flipxy)
    { 
        // A is typecasted to y, and B is typecasted to x
        cast_A = A_is_pattern ? NULL : 
                 GB_cast_factory (mult->ytype->code, A->type->code) ;
        cast_B = B_is_pattern ? NULL : 
                 GB_cast_factory (mult->xtype->code, B->type->code) ;
    }
    else
    { 
        // A is typecasted to x, and B is typecasted to y
        cast_A = A_is_pattern ? NULL :
                 GB_cast_factory (mult->xtype->code, A->type->code) ;
        cast_B = B_is_pattern ? NULL :
                 GB_cast_factory (mult->ytype->code, B->type->code) ;
    }

    //--------------------------------------------------------------------------
    // C = A*B via saxpy3 or bitmap method, function pointers, and typecasting
    //--------------------------------------------------------------------------

    // memcpy (&(Cx [pC]), &(Hx [i]), len*csize)
    #define GB_CIJ_MEMCPY(pC,i,len) memcpy (GB_CX (pC), GB_HX (i), (len)*csize)

    // atomic update not available for function pointers
    #define GB_HAS_ATOMIC 0

    // user-defined monoid update cannot be done with an OpenMP atomic
    #define GB_HAS_OMP_ATOMIC 0

    // no special cases (C is not iso)
    #define GB_IS_ANY_MONOID 0
    #define GB_IS_ANY_FC32_MONOID 0
    #define GB_IS_ANY_FC64_MONOID 0
    #define GB_IS_PLUS_FC32_MONOID 0
    #define GB_IS_PLUS_FC64_MONOID 0
    #define GB_IS_ANY_PAIR_SEMIRING 0
    #define GB_IS_PAIR_MULTIPLIER 0

    #define GB_ATYPE GB_void
    #define GB_BTYPE GB_void
    #define GB_ASIZE asize
    #define GB_BSIZE bsize

    // no vectorization
    #define GB_PRAGMA_SIMD_VECTORIZE ;

    // The monoid identity byte value is not used in saxpy3
    #define GB_GENERIC
    #define GB_HAS_IDENTITY_BYTE 0
    #define GB_IDENTITY_BYTE (none)

    // definitions for GB_AxB_saxpy_template.c
    #include "GB_AxB_saxpy3_template.h"

    if (op_is_positional)
    { 

        //----------------------------------------------------------------------
        // generic semirings with positional mulitiply operators
        //----------------------------------------------------------------------

        GB_BURBLE_MATRIX (C, "(generic positional C=A*B) ") ;

        if (flipxy)
        { 
            // flip a positional multiplicative operator
            bool handled ;
            opcode = GB_flip_binop_code (opcode, &handled) ;
            ASSERT (handled) ;      // all positional ops can be flipped
        }

        // C always has type int64_t or int32_t.  The monoid must be used via
        // its function pointer.  The positional multiply operator must be
        // hard-coded since it has no function pointer.  The numerical values
        // and types of A and B are not accessed.

        ASSERT (A_is_pattern) ;
        ASSERT (B_is_pattern) ;

        // aik = A(i,k), located in Ax [A_iso ? 0:pA], value not used
        #define GB_A_IS_PATTERN 1
        #define GB_GETA(aik,Ax,pA,A_iso) ;

        // bkj = B(k,j), located in Bx [B_iso ? 0:pB], value not used
        #define GB_B_IS_PATTERN 1
        #define GB_GETB(bkj,Bx,pB,B_iso) ;

        // Gx [pG] = A(i,k), located in Ax [A_iso ? 0:pA], value not used
        #define GB_LOADA(Gx,pG,Ax,pA,A_iso) ;

        // Gx [pG] = B(k,j), located in Bx [B_iso ? 0:pB], value not used
        #define GB_LOADB(Gx,pG,Bx,pB,B_iso) ;

        // define t for each task
        #define GB_CIJ_DECLARE(t) GB_CTYPE t

        // address of Cx [p]
        #define GB_CX(p) (&Cx [p])

        // Cx [p] = t
        #define GB_CIJ_WRITE(p,t) Cx [p] = t

        // address of Hx [i]
        #define GB_HX(i) (&Hx [i])

        // Hx [i] = t
        #define GB_HX_WRITE(i,t) Hx [i] = t

        // Cx [p] = Hx [i]
        #define GB_CIJ_GATHER(p,i) Cx [p] = Hx [i]

        // Cx [p] += Hx [i]
        #define GB_CIJ_GATHER_UPDATE(p,i) fadd (GB_CX (p), GB_CX (p), GB_HX (i))

        // Cx [p] += t
        #define GB_CIJ_UPDATE(p,t) fadd (GB_CX (p), GB_CX (p), &t)

        // Hx [i] += t
        #define GB_HX_UPDATE(i,t) fadd (GB_HX (i), GB_HX (i), &t)

        int64_t offset = GB_positional_offset (opcode, NULL) ;

        if (mult->ztype == GrB_INT64)
        {
            #undef  GB_CTYPE
            #define GB_CTYPE int64_t
            #undef  GB_CSIZE
            #define GB_CSIZE (sizeof (int64_t))
            ASSERT (C->type == GrB_INT64) ;
            ASSERT (csize == sizeof (int64_t)) ;
            switch (opcode)
            {
                case GB_FIRSTI_binop_code   :   // z = first_i(A(i,k),y) == i
                case GB_FIRSTI1_binop_code  :   // z = first_i1(A(i,k),y) == i+1
                    #undef  GB_MULT
                    #define GB_MULT(t, aik, bkj, i, k, j) t = i + offset
                    #include "GB_AxB_saxpy_template.c"
                    break ;
                case GB_FIRSTJ_binop_code   :   // z = first_j(A(i,k),y) == k
                case GB_FIRSTJ1_binop_code  :   // z = first_j1(A(i,k),y) == k+1
                case GB_SECONDI_binop_code  :   // z = second_i(x,B(k,j)) == k
                case GB_SECONDI1_binop_code :   // z = second_i1(x,B(k,j))== k+1
                    #undef  GB_MULT
                    #define GB_MULT(t, aik, bkj, i, k, j) t = k + offset
                    #include "GB_AxB_saxpy_template.c"
                    break ;
                case GB_SECONDJ_binop_code  :   // z = second_j(x,B(k,j)) == j
                case GB_SECONDJ1_binop_code :   // z = second_j1(x,B(k,j))== j+1
                    #undef  GB_MULT
                    #define GB_MULT(t, aik, bkj, i, k, j) t = j + offset
                    #include "GB_AxB_saxpy_template.c"
                    break ;
                default: ;
            }
        }
        else
        {
            #undef  GB_CTYPE
            #define GB_CTYPE int32_t
            #undef  GB_CSIZE
            #define GB_CSIZE (sizeof (int32_t))
            ASSERT (C->type == GrB_INT32) ;
            ASSERT (csize == sizeof (int32_t)) ;
            switch (opcode)
            {
                case GB_FIRSTI_binop_code   :   // z = first_i(A(i,k),y) == i
                case GB_FIRSTI1_binop_code  :   // z = first_i1(A(i,k),y) == i+1
                    #undef  GB_MULT
                    #define GB_MULT(t,aik,bkj,i,k,j) t = (int32_t) (i + offset)
                    #include "GB_AxB_saxpy_template.c"
                    break ;
                case GB_FIRSTJ_binop_code   :   // z = first_j(A(i,k),y) == k
                case GB_FIRSTJ1_binop_code  :   // z = first_j1(A(i,k),y) == k+1
                case GB_SECONDI_binop_code  :   // z = second_i(x,B(k,j)) == k
                case GB_SECONDI1_binop_code :   // z = second_i1(x,B(k,j))== k+1
                    #undef  GB_MULT
                    #define GB_MULT(t,aik,bkj,i,k,j) t = (int32_t) (k + offset)
                    #include "GB_AxB_saxpy_template.c"
                    break ;
                case GB_SECONDJ_binop_code  :   // z = second_j(x,B(k,j)) == j
                case GB_SECONDJ1_binop_code :   // z = second_j1(x,B(k,j))== j+1
                    #undef  GB_MULT
                    #define GB_MULT(t,aik,bkj,i,k,j) t = (int32_t) (j + offset)
                    #include "GB_AxB_saxpy_template.c"
                    break ;
                default: ;
            }
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // generic semirings with standard multiply operators
        //----------------------------------------------------------------------

        GB_BURBLE_MATRIX (C, "(generic C=A*B) ") ;

        // aik = A(i,k), located in Ax [A_iso ? 0:pA]
        #undef  GB_A_IS_PATTERN
        #define GB_A_IS_PATTERN 0
        #undef  GB_GETA
        #define GB_GETA(aik,Ax,pA,A_iso)                                    \
            GB_void aik [GB_VLA(aik_size)] ;                                \
            if (!A_is_pattern)                                              \
            {                                                               \
                cast_A (aik, Ax +((A_iso) ? 0:((pA)*asize)), asize) ;       \
            }

        // bkj = B(k,j), located in Bx [B_iso ? 0:pB]
        #undef  GB_B_IS_PATTERN
        #define GB_B_IS_PATTERN 0
        #undef  GB_GETB
        #define GB_GETB(bkj,Bx,pB,B_iso)                                    \
            GB_void bkj [GB_VLA(bkj_size)] ;                                \
            if (!B_is_pattern)                                              \
            {                                                               \
                cast_B (bkj, Bx +((B_iso) ? 0:((pB)*bsize)), bsize) ;       \
            }

        // Gx [pG] = A(i,k), located in Ax [A_iso ? 0:pA], no typecasting
        #undef  GB_LOADA
        #define GB_LOADA(Gx,pG,Ax,pA,A_iso)                                 \
            memcpy (Gx + ((pG)*asize), Ax +((A_iso) ? 0:((pA)*asize)), asize)

        // Gx [pG] = B(k,j), located in Bx [B_iso ? 0:pB], no typecasting
        #undef  GB_LOADB
        #define GB_LOADB(Gx,pG,Bx,pB,B_iso)                                 \
            memcpy (Gx + ((pG)*bsize), Bx +((B_iso) ? 0:((pB)*bsize)), bsize)

        // define t for each task
        #undef  GB_CIJ_DECLARE
        #define GB_CIJ_DECLARE(t) GB_void t [GB_VLA(csize)]

        // address of Cx [p]
        #undef  GB_CX
        #define GB_CX(p) (Cx +((p)*csize))

        // Cx [p] = t
        #undef  GB_CIJ_WRITE
        #define GB_CIJ_WRITE(p,t) memcpy (GB_CX (p), t, csize)

        // address of Hx [i]
        #undef  GB_HX
        #define GB_HX(i) (Hx +((i)*csize))

        // Hx [i] = t
        #undef  GB_HX_WRITE
        #define GB_HX_WRITE(i,t) memcpy (GB_HX (i), t, csize)

        // Cx [p] = Hx [i]
        #undef  GB_CIJ_GATHER
        #define GB_CIJ_GATHER(p,i) memcpy (GB_CX (p), GB_HX(i), csize)

        // Cx [p] += Hx [i]
        #undef  GB_CIJ_GATHER_UPDATE
        #define GB_CIJ_GATHER_UPDATE(p,i) fadd (GB_CX (p), GB_CX (p), GB_HX (i))

        // Cx [p] += t
        #undef  GB_CIJ_UPDATE
        #define GB_CIJ_UPDATE(p,t) fadd (GB_CX (p), GB_CX (p), t)

        // Hx [i] += t
        #undef  GB_HX_UPDATE
        #define GB_HX_UPDATE(i,t) fadd (GB_HX (i), GB_HX (i), t)

        #undef  GB_CTYPE
        #define GB_CTYPE GB_void

        #undef  GB_CSIZE
        #define GB_CSIZE csize

        if (opcode == GB_FIRST_binop_code || opcode == GB_SECOND_binop_code)
        {
            // fmult is not used and can be NULL (for user-defined types)
            if (flipxy)
            { 
                // flip first and second
                bool handled ;
                opcode = GB_flip_binop_code (opcode, &handled) ;
                ASSERT (handled) ;      // FIRST and SECOND can be flipped
            }
            if (opcode == GB_FIRST_binop_code)
            { 
                // t = A(i,k)
                ASSERT (B_is_pattern) ;
                #undef  GB_MULT
                #define GB_MULT(t, aik, bkj, i, k, j) memcpy (t, aik, csize)
                #include "GB_AxB_saxpy_template.c"
            }
            else // opcode == GB_SECOND_binop_code
            { 
                // t = B(i,k)
                ASSERT (A_is_pattern) ;
                #undef  GB_MULT
                #define GB_MULT(t, aik, bkj, i, k, j) memcpy (t, bkj, csize)
                #include "GB_AxB_saxpy_template.c"
            }
        }
        else
        {
            ASSERT (fmult != NULL) ;
            if (flipxy)
            { 
                // t = B(k,j) * A(i,k)
                #undef  GB_MULT
                #define GB_MULT(t, aik, bkj, i, k, j) fmult (t, bkj, aik)
                #include "GB_AxB_saxpy_template.c"
            }
            else
            { 
                // t = A(i,k) * B(k,j)
                #undef  GB_MULT
                #define GB_MULT(t, aik, bkj, i, k, j) fmult (t, aik, bkj)
                #include "GB_AxB_saxpy_template.c"
            }
        }
    }

    return (GrB_SUCCESS) ;
}

