/*
 * Copyright 2017 by Philip N. Garner
 *
 * ...although the copyright of *this file* is probably that of the two
 * sources:
 *  http://www.netlib.org/clapack/f2c.h
 *  http://www.netlib.org/clapack/clapack.h
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, April 2017
 */

#ifndef CLAPACK_H
#define CLAPACK_H

/*
 * We could in principle just include clapack.h from netlib.  The problem is
 * that it's f2c'd, meaning the functions that return floats are prototyped as
 * returning doubles.  We want to link with the fortran library that normally
 * returns float in those cases.  However, there are implementations (MacOS
 * Accelerate being one) that do return double.  So just copy the necessary
 * lines and edit.
 */

// From: http://www.netlib.org/clapack/f2c.h
#define VOID void
typedef long int integer;
typedef long int logical;
typedef long int ftnlen;
typedef float real;
typedef double doublereal;
typedef std::complex<float> complex;
typedef std::complex<double> doublecomplex;
typedef logical (*L_fp)(...);

// From: http://www.netlib.org/clapack/clapack.h
#ifdef __cplusplus 	
extern "C" {	
#endif		

#ifdef HAVE_F2C_BLAS
#define FLOATRET doublereal
#else
#define FLOATRET real
#endif
    
/* Subroutine */ int cswap_(integer *n, complex *cx, integer *incx, complex *
	cy, integer *incy);
/* Subroutine */ int dswap_(integer *n, doublereal *dx, integer *incx, 
	doublereal *dy, integer *incy);
/* Subroutine */ int sswap_(integer *n, real *sx, integer *incx, real *sy, 
	integer *incy);
/* Subroutine */ int zswap_(integer *n, doublecomplex *zx, integer *incx, 
	doublecomplex *zy, integer *incy);
/* Subroutine */ int ccopy_(integer *n, complex *cx, integer *incx, complex *
	cy, integer *incy);
/* Subroutine */ int dcopy_(integer *n, doublereal *dx, integer *incx, 
	doublereal *dy, integer *incy);
/* Subroutine */ int scopy_(integer *n, real *sx, integer *incx, real *sy, 
	integer *incy);
/* Subroutine */ int zcopy_(integer *n, doublecomplex *zx, integer *incx, 
	doublecomplex *zy, integer *incy);
/* Subroutine */ int caxpy_(integer *n, complex *ca, complex *cx, integer *
	incx, complex *cy, integer *incy);
/* Subroutine */ int daxpy_(integer *n, doublereal *da, doublereal *dx, 
	integer *incx, doublereal *dy, integer *incy);
/* Subroutine */ int saxpy_(integer *n, real *sa, real *sx, integer *incx, 
	real *sy, integer *incy);
/* Subroutine */ int zaxpy_(integer *n, doublecomplex *za, doublecomplex *zx, 
	integer *incx, doublecomplex *zy, integer *incy);
/* Subroutine */ int cscal_(integer *n, complex *ca, complex *cx, integer *
	incx);
/* Subroutine */ int csscal_(integer *n, real *sa, complex *cx, integer *incx);
/* Subroutine */ int dscal_(integer *n, doublereal *da, doublereal *dx, 
	integer *incx);
/* Subroutine */ int sscal_(integer *n, real *sa, real *sx, integer *incx);
/* Subroutine */ int zscal_(integer *n, doublecomplex *za, doublecomplex *zx, 
	integer *incx);
/* Complex */ VOID cdotc_(complex * ret_val, integer *n, complex *cx, integer 
	*incx, complex *cy, integer *incy);
doublereal ddot_(integer *n, doublereal *dx, integer *incx, doublereal *dy, 
	integer *incy);
FLOATRET sdot_(integer *n, real *sx, integer *incx, real *sy, integer *incy);
/* Double Complex */ VOID zdotc_(doublecomplex * ret_val, integer *n, 
	doublecomplex *zx, integer *incx, doublecomplex *zy, integer *incy);
integer icamax_(integer *n, complex *cx, integer *incx);
integer idamax_(integer *n, doublereal *dx, integer *incx);
integer isamax_(integer *n, real *sx, integer *incx);
integer izamax_(integer *n, doublecomplex *zx, integer *incx);
doublereal dasum_(integer *n, doublereal *dx, integer *incx);
FLOATRET sasum_(integer *n, real *sx, integer *incx);
/* Subroutine */ int dtbmv_(char *uplo, char *trans, char *diag, integer *n, 
	integer *k, doublereal *a, integer *lda, doublereal *x, integer *incx);
/* Subroutine */ int stbmv_(char *uplo, char *trans, char *diag, integer *n, 
	integer *k, real *a, integer *lda, real *x, integer *incx);
/* Subroutine */ int dsbmv_(char *uplo, integer *n, integer *k, doublereal *
	alpha, doublereal *a, integer *lda, doublereal *x, integer *incx, 
	doublereal *beta, doublereal *y, integer *incy);
/* Subroutine */ int ssbmv_(char *uplo, integer *n, integer *k, real *alpha, 
	real *a, integer *lda, real *x, integer *incx, real *beta, real *y, 
	integer *incy);
/* Subroutine */ int dgemm_(char *transa, char *transb, integer *m, integer *
	n, integer *k, doublereal *alpha, doublereal *a, integer *lda, 
	doublereal *b, integer *ldb, doublereal *beta, doublereal *c__, 
	integer *ldc);
/* Subroutine */ int sgemm_(char *transa, char *transb, integer *m, integer *
	n, integer *k, real *alpha, real *a, integer *lda, real *b, integer *
	ldb, real *beta, real *c__, integer *ldc);
/* Subroutine */ int sgees_(char *jobvs, char *sort, L_fp select, integer *n, 
	real *a, integer *lda, integer *sdim, real *wr, real *wi, real *vs, 
	integer *ldvs, real *work, integer *lwork, logical *bwork, integer *
	info);
/* Subroutine */ int dgeev_(char *jobvl, char *jobvr, integer *n, doublereal *
	a, integer *lda, doublereal *wr, doublereal *wi, doublereal *vl, 
	integer *ldvl, doublereal *vr, integer *ldvr, doublereal *work, 
	integer *lwork, integer *info);
/* Subroutine */ int sgeev_(char *jobvl, char *jobvr, integer *n, real *a, 
	integer *lda, real *wr, real *wi, real *vl, integer *ldvl, real *vr, 
	integer *ldvr, real *work, integer *lwork, integer *info);

#ifdef __cplusplus
}
#endif

#endif
