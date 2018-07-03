#ifndef MATRIX_TYPE
#define MATRIX_TYPE float
// enable one and only one of the following macros (the supported ones):
#define MATRIX_TYPE_FLOAT
//#define MATRIX_TYPE_DOUBLE
#endif

struct Matrix {
    int rows, cols;
    MATRIX_TYPE d[]; // column major order
};

// support for tostring(m) via __tostring metamethod
#define MATRIX_ENABLE__TOSTRING

// convert the matrix to a standard lua table with keys 1 to m.cols*m.rows
#define MATRIX_ENABLE_TOTABLE

// support for matrix addition: m+n, n+m, m+m, rv+m, m+rw, cv+m, m+cv
#define MATRIX_ENABLE__ADD

// support for matrix substraction: m-n, n-m, m-m, rv-m, m-rw, cv-m, m-cv
#define MATRIX_ENABLE__SUB

// support for matrix element-wise multiplication: m*n, n*m, m*m, rv*m, m*rw, cv*m, m*cv
#define MATRIX_ENABLE__MUL

// support for matrix element-wise division: m/n, n/m, m/m, rv/m, m/rw, cv/m, m/cv
#define MATRIX_ENABLE__DIV

// support for matrix element-wise modulo operation: m%n, n%m, m%m, rv%m, m%rw, cv%m, m%cv
#define MATRIX_ENABLE__MOD

// support for matrix element-wise modulo operation: m^n, n^m, m^m, rv^m, m^rw, cv^m, m^cv
#define MATRIX_ENABLE__POW

// support for matrix unary minus: -m
#define MATRIX_ENABLE__UNM

// support for MUTABLE matrix reshaping: m:reshape(rows, cols) rows*cols must be equal to m.rows*m.cols
#define MATRIX_ENABLE_RESHAPE

// support for matrix transposition: m:t()
#define MATRIX_ENABLE_T

// support for matrix multiplication: m:dot(p) where sizes must be i*k and k*j
#define MATRIX_ENABLE_DOT

// support for transposition composed with matrix multiplication: m:tdot(p), equivalent to m:t():dot(p)
#define MATRIX_ENABLE_TDOT

// support for MUTABLE matrix row swapping: m:rswap(i1, i2)
#define MATRIX_ENABLE_RSWAP

// support for MUTABLE matrix column swapping: m:cswap(j1, j2)
#define MATRIX_ENABLE_CSWAP

// support for LU factorization: lu = m:lup()
#define MATRIX_ENABLE_LUP

// support for Reduced Row Echelon Form: rref, inv = m:rref()
#define MATRIX_ENABLE_RREF

// support for matrix inversion (requires MATRIX_ENABLE_RREF)
#define MATRIX_ENABLE_INV

// some unary function, where each element is applied to the corresponding C function of the same name:
#define MATRIX_ENABLE_FLOOR
#define MATRIX_ENABLE_CEIL
#define MATRIX_ENABLE_ACOS
#define MATRIX_ENABLE_ASIN
#define MATRIX_ENABLE_ATAN
#define MATRIX_ENABLE_COS
#define MATRIX_ENABLE_SIN
#define MATRIX_ENABLE_TAN
#define MATRIX_ENABLE_COSH
#define MATRIX_ENABLE_SINH
#define MATRIX_ENABLE_TANH
#define MATRIX_ENABLE_EXP
#define MATRIX_ENABLE_LOG
#define MATRIX_ENABLE_LOG10
#define MATRIX_ENABLE_SQRT
#define MATRIX_ENABLE_ABS
#define MATRIX_ENABLE_ISINF
#define MATRIX_ENABLE_FINITE
#define MATRIX_ENABLE_ISNAN
