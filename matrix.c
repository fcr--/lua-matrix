#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "lua.h"
#include "lauxlib.h"

#include "matrix_config.h"

/**
 * m = matrix.new{3, 2, value=2}  or  matrix.new(3, 2)
 * m = matrix.id(3)
 * m = matrix.random(3, 2)
 * m.rows
 * m.cols
 * #m == m.rows * m.cols
 */

#define XSTR(x) #x
#define CAT_MATRIX_STR(x) XSTR(x) " matrix"
#define MATRIX_MT CAT_MATRIX_STR(MATRIX_TYPE)

#define MATRIX_MAX_TOSTRING 200

struct Matrix {
    int rows, cols;
    MATRIX_TYPE d[]; // column major order
};

static const char * interned_rows, * interned_cols;

static struct Matrix * push_matrix(lua_State * L, int rows, int cols) {
    struct Matrix * m = (struct Matrix *) lua_newuserdata(L, sizeof (struct Matrix) + sizeof (MATRIX_TYPE[rows*cols]));
    m->rows = rows;
    m->cols = cols;
    luaL_setmetatable(L, MATRIX_MT);
    return m;
}

static int matrix_new(lua_State * L) {
    struct Matrix * m;
    int rows, cols, i;
    MATRIX_TYPE value = 0;
    if (lua_isnumber(L, 1)) {
        // rows, cols = checkinteger(arg1), checkinteger(arg2):
        rows = luaL_checkinteger(L, 1);
        cols = luaL_checkinteger(L, 2);
    } else {
        // rows = checkinteger(arg1[1]):
        lua_pushinteger(L, 1);
        lua_gettable(L, 1);
        rows = luaL_checkinteger(L, -1);
        // cols = checkinteger(arg1[2]):
        lua_pushinteger(L, 2);
        lua_gettable(L, 1);
        cols = luaL_checkinteger(L, -1);
        // value = arg1.value or 0
        lua_getfield(L, 1, "value");
        value = luaL_optnumber(L, -1, value);
    }
    if (rows < 1 || cols < 1) return luaL_error(L, "invalid size %d*%d", rows, cols);
    m = push_matrix(L, rows, cols);
    for (i = 0; i < rows*cols; i++) m->d[i] = value;
    return 1;
}

static int matrix_id(lua_State * L) {
    struct Matrix * m;
    int side, size, i;
    side = luaL_checkinteger(L, 1);
    if (side < 1) return luaL_error(L, "invalid size %d*%d", side);
    size = side * side;
    m = push_matrix(L, side, side);
    for (i = 0; i < size; i++) m->d[i] = 0;
    for (i = 0; i < size; i += side + 1) m->d[i] = 1;
    return 1;
}

static int matrix_random(lua_State * L) {
    struct Matrix * m;
    int rows, cols, i;
    MATRIX_TYPE factor = 1.0 / ((MATRIX_TYPE)RAND_MAX + 1.0);
    rows = luaL_checkinteger(L, 1);
    cols = luaL_checkinteger(L, 2);
    if (rows < 1 || cols < 1) return luaL_error(L, "invalid size %d*%d", rows, cols);
    m = push_matrix(L, rows, cols);
#ifdef LUA_USE_POSIX
    for (i = 0; i < rows*cols; i++) m->d[i] = factor * (MATRIX_TYPE)random();
#else
    for (i = 0; i < rows*cols; i++) m->d[i] = factor * (MATRIX_TYPE)rand();
#endif
    return 1;
}

static int matrix_fromtable(lua_State * L) {
    struct Matrix * m;
    int rows, cols, i;
    if (!lua_istable(L, 1)) return luaL_error(L, "fromtable requires a table");
    lua_getfield(L, 1, interned_rows);
    rows = luaL_optinteger(L, -1, 1);
    lua_getfield(L, 1, interned_cols);
    cols = luaL_optinteger(L, -1, 1);
    lua_len(L, 1);
    if (luaL_checkinteger(L, -1) != rows * cols)
        return luaL_error(L, "#t must be equal to t.cols*t.rows");
    m = push_matrix(L, rows, cols);
    for (i = 0; i < rows*cols; i++) {
        lua_geti(L, 1, i+1);
        m->d[i] = lua_tonumber(L, -1);
        lua_pop(L, 1);
    }
    return 1;
}

static int matrix_mt__index(lua_State * L) {
    struct Matrix * m = (struct Matrix*)luaL_checkudata(L, 1, MATRIX_MT);
    if (lua_type(L, 2) == LUA_TSTRING && lua_getmetatable(L, 1)) {
        const char * key = lua_tostring(L, 2);
        if (key == interned_cols) {
            lua_pushinteger(L, m->cols);
            return 1;
        } else if (key == interned_rows) {
            lua_pushinteger(L, m->rows);
            return 1;
        }
        lua_pushvalue(L, 2); // stack: {m, key, ..., mt} -> {m, key, ..., mt, key}
        lua_rawget(L, -2); // stack -> {m, key, ..., rawget(mt, key)}
        if (!lua_isnil(L, -1)) return 1;
        return luaL_error(L, "method %s not available in metatable", key);
    }
    if (lua_isnumber(L, 2)) {
        int idx = luaL_checkinteger(L, 2);
        if (idx < 1 || idx > m->rows * m->cols) return luaL_error(L, "index out of bounds: %d", idx);
        lua_pushnumber(L, m->d[idx-1]);
    } else if (lua_istable(L, 2)) { // stride support {rows, cols}, where they can be nil, a number or {from, to}
        int dim, row1, rown, col1, coln;
        int point = 0;
        for (dim = 1; dim <= 2; dim++) {
            int * v1, * vn, n;
            if (dim == 1) { v1=&row1; vn=&rown; n=m->rows; } else { v1=&col1; vn=&coln; n=m->cols; }
            lua_pushinteger(L, dim);
            lua_gettable(L, 2);
            switch (lua_type(L, -1)) {
                case LUA_TNIL:
                    *v1 = 1;
                    *vn = n;
                    break;
                case LUA_TNUMBER:
                    *v1 = *vn = luaL_checkinteger(L, -1);
                    point++;
                    break;
                case LUA_TTABLE:
                    lua_pushinteger(L, 1);
                    lua_gettable(L, -2);
                    *v1 = luaL_checkinteger(L, -1);
                    lua_pushinteger(L, 2);
                    lua_gettable(L, -3); // stack: table, row1, 2
                    *vn = luaL_checkinteger(L, -1);
                    break;
                default:
                    return luaL_error(L, "invalid stride index type");
            }
        }
        if (row1 < 1 || col1 < 1 || row1 > rown || col1 > coln || rown > m->rows || coln > m->cols) {
            return luaL_error(L, "invalid stride index {%d..%d, %d..%d}", row1, rown, col1, coln);
        }
        if (point == 2) {
            lua_pushnumber(L, m->d[(col1-1) * m->rows + row1-1]);
        } else {
            int stride = m->rows;
            int limit = coln * stride;
            int height = rown - row1 + 1;
            struct Matrix * dest = push_matrix(L, height, coln - col1 + 1);
            int i, j, k = 0;
            for (j = (col1 - 1)*stride + row1 - 1; j < limit; j += stride) {
                for (i = j; i < j + height; i++) dest->d[k++] = m->d[i];
            }
        }
    } else return luaL_error(L, "invalid index type");
    return 1;
}

/**
 * m[n] = value  -- where n is an integer between 1 and m.rows*m.cols
 * m[slice] = matrix or value -- if src is matrix, then its sice must be slice conformant
 */
static int matrix_mt__newindex(lua_State * L) {
    struct Matrix * m = (struct Matrix*)luaL_checkudata(L, 1, MATRIX_MT);
    if (lua_isnumber(L, 2)) {
        int idx = luaL_checkinteger(L, 2);
        if (idx < 1 || idx > m->rows * m->cols) return luaL_error(L, "index out of bounds: %d", idx);
        m->d[idx-1] = luaL_checknumber(L, 3);
    } else if (lua_istable(L, 2)) { // stride support {rows, cols}, where they can be nil, a number or {from, to}
        int dim, row1, rown, col1, coln;
        for (dim = 1; dim <= 2; dim++) {
            int * v1, * vn, n;
            if (dim == 1) { v1=&row1; vn=&rown; n=m->rows; } else { v1=&col1; vn=&coln; n=m->cols; }
            lua_pushinteger(L, dim);
            lua_gettable(L, 2);
            switch (lua_type(L, -1)) {
                case LUA_TNIL:
                    *v1 = 1;
                    *vn = n;
                    break;
                case LUA_TNUMBER:
                    *v1 = *vn = luaL_checkinteger(L, -1);
                    break;
                case LUA_TTABLE:
                    lua_pushinteger(L, 1);
                    lua_gettable(L, -2);
                    *v1 = luaL_checkinteger(L, -1);
                    lua_pushinteger(L, 2);
                    lua_gettable(L, -3); // stack: table, row1, 2
                    *vn = luaL_checkinteger(L, -1);
                    break;
                default:
                    return luaL_error(L, "invalid stride index type");
            }
        }
        if (row1 < 1 || col1 < 1 || row1 > rown || col1 > coln || rown > m->rows || coln > m->cols) {
            return luaL_error(L, "invalid stride index {%d..%d, %d..%d}", row1, rown, col1, coln);
        }
        int stride = m->rows;
        int limit = coln * stride;
        int height = rown - row1 + 1;
        int i, j, k = 0;
        if (lua_isnumber(L, 3)) {
            MATRIX_TYPE src = lua_tonumber(L, 3);
            for (j = (col1 - 1)*stride; j < limit; j += stride) {
                for (i = j; i < j + height; i++) m->d[i] = src;
            }
        } else {
            struct Matrix * src = (struct Matrix*)luaL_checkudata(L, 3, MATRIX_MT);
            if (src->rows != height || src->cols != coln - col1 + 1)
                return luaL_error(L, "non-conforming source %d*%d matrix, expecting %d*%d",
                        src->rows, src->cols, height, coln - col1  + 1);
            for (j = (col1 - 1)*stride + row1 - 1; j < limit; j += stride) {
                for (i = j; i < j + height; i++) m->d[i] = src->d[k++];
            }
        }
    } else return luaL_error(L, "invalid index type");
    return 1;
}

#ifdef MATRIX_ENABLE__TOSTRING
static int matrix_mt__tostring(lua_State * L) {
    struct Matrix * m = (struct Matrix*)luaL_checkudata(L, 1, MATRIX_MT);
    int i, limit = m->rows * m->cols;
    lua_checkstack(L, 3 + 2 * MATRIX_MAX_TOSTRING);
    lua_pushstring(L, "[ ");
    if (limit > MATRIX_MAX_TOSTRING) limit = MATRIX_MAX_TOSTRING + 1;
    for (i = 0; i < limit; i++) {
        MATRIX_TYPE v = m->d[i];
        if (i == MATRIX_MAX_TOSTRING) {
            lua_pushstring(L, "...");
        } else if (v == (int)v) {
            lua_pushinteger(L, (int)v);
        } else {
            lua_pushnumber(L, v);
        }
        lua_pushstring(L, i >= limit - 1 ? " " : ", ");
    }
    lua_pushstring(L, "]");
    lua_concat(L, 2 + 2*limit);
    return 1;
}
#endif

#ifdef MATRIX_ENABLE_TOTABLE
static int matrix_mt_totable(lua_State * L) {
    int i = 0;
    struct Matrix * m = (struct Matrix*)luaL_checkudata(L, 1, MATRIX_MT);
    int size = m->rows * m->cols;
    lua_createtable(L, size, 2);
    while (i < size) {
        MATRIX_TYPE v = m->d[i];
        if (v == (int)v) lua_pushinteger(L, (int)v);
        else lua_pushnumber(L, v);
        lua_rawseti(L, -2, ++i);
    }
    lua_pushinteger(L, m->rows);
    lua_setfield(L, -2, interned_rows);
    lua_pushinteger(L, m->cols);
    lua_setfield(L, -2, interned_cols);
    return 1;
}
#endif

#define matrix_mt__declare_binop(name, op) \
    static int matrix_mt##name(lua_State * L) { \
        if (lua_isnumber(L, 1)) { \
            struct Matrix * m = (struct Matrix*)luaL_checkudata(L, 2, MATRIX_MT); \
            MATRIX_TYPE param = lua_tonumber(L, 1); \
            struct Matrix * dest = push_matrix(L, m->rows, m->cols); \
            int i, size = m->rows * m->cols; \
            for (i = 0; i < size; i++) dest->d[i] = op(param, m->d[i]); \
            return 1; \
        } \
        struct Matrix * m = (struct Matrix*)luaL_checkudata(L, 1, MATRIX_MT); \
        switch (lua_type(L, 2)) { \
            case LUA_TNUMBER: { \
                MATRIX_TYPE param = lua_tonumber(L, 2); \
                struct Matrix * dest = push_matrix(L, m->rows, m->cols); \
                int i, size = m->rows * m->cols; \
                for (i = 0; i < size; i++) dest->d[i] = op(m->d[i], param); \
                return 1; \
            } \
            case LUA_TUSERDATA: { \
                struct Matrix * param = (struct Matrix*)luaL_checkudata(L, 2, MATRIX_MT); \
                if (m->rows == param->rows) { \
                    if (m->cols == param->cols) { \
                        struct Matrix * dest = push_matrix(L, m->rows, m->cols); \
                        int i, size = m->rows * m->cols; \
                        for (i = 0; i < size; i++) dest->d[i] = op(m->d[i], param->d[i]); \
                        return 1; \
                    } else if (param->cols == 1) { /* param is a vertical vector */ \
                        struct Matrix * dest = push_matrix(L, m->rows, m->cols); \
                        int i, j, size = m->rows * m->cols; \
                        for (j = 0; j < size; j += m->rows) \
                            for (i = 0; i < m->rows; i++) \
                                dest->d[j + i] = op(m->d[j + i], param->d[i]); \
                        return 1; \
                    } else if (param->cols == 1) { /* m is the vertical vector */ \
                        struct Matrix * dest = push_matrix(L, m->rows, param->cols); \
                        int i, j, size = m->rows * param->cols; \
                        for (j = 0; j < size; j += m->rows) \
                            for (i = 0; i < m->rows; i++) \
                                dest->d[j + i] = op(m->d[i], param->d[j + i]); \
                        return 1; \
                    } \
                } else if (m->cols == param->cols) { \
                    if (param->rows == 1) { /* param is the row vector */ \
                        struct Matrix * dest = push_matrix(L, m->rows, m->cols); \
                        int i = 0, i2, j; \
                        for (j = 0; j < m->cols; j += m->cols) { \
                            MATRIX_TYPE p = param->d[j]; \
                            for (i2 = m->rows; i2 >= 0; i2--, i++) \
                                dest->d[i] = op(m->d[i], p); \
                        } \
                        return 1; \
                    } else if (m->rows == 1) { /* m is the row vector*/ \
                        struct Matrix * dest = push_matrix(L, param->rows, m->cols); \
                        int i = 0, i2, j; \
                        for (j = 0; j < m->cols; j += m->cols) { \
                            MATRIX_TYPE v = m->d[j]; \
                            for (i2 = param->rows; i2 >= 0; i2--, i++) \
                                dest->d[i] = op(v, param->d[i]); \
                        } \
                        return 1; \
                    } \
                } \
                return luaL_error(L, "non conformat matrices %d*%d, %d*%d", m->rows, m->cols, param->rows, param->cols); \
            } \
        } \
        return luaL_error(L, "invalid parameters for operand \"" # name "\""); \
    }

#ifdef MATRIX_ENABLE__ADD
#define op(x,y) (x)+(y)
matrix_mt__declare_binop(__add, op)
#undef op
#endif
#ifdef MATRIX_ENABLE__SUB
#define op(x,y) (x)-(y)
matrix_mt__declare_binop(__sub, op)
#undef op
#endif
#ifdef MATRIX_ENABLE__MUL
#define op(x,y) (x)*(y)
matrix_mt__declare_binop(__mul, op)
#undef op
#endif
#ifdef MATRIX_ENABLE__DIV
#define op(x,y) (x)/(y)
matrix_mt__declare_binop(__div, op)
#undef op
#endif
#ifdef MATRIX_ENABLE__MOD
#  if defined(MATRIX_TYPE_FLOAT)
#    define op(x,y) fmodf(x, y)
#  elif defined(MATRIX_TYPE_DOUBLE)
#    define op(x,y) fmod(x, y)
#  else
#    error "MATRIX_ENABLE__MOD is only supported for float and double"
#  endif
matrix_mt__declare_binop(__mod, op)
#undef op
#endif
#ifdef MATRIX_ENABLE__POW
#  if defined(MATRIX_TYPE_FLOAT)
#    define op(x,y) powf(x, y)
#  elif defined(MATRIX_TYPE_DOUBLE)
#    define op(x,y) pow(x, y)
#  else
#    error "MATRIX_ENABLE__POW is only supported for float and double"
#  endif
matrix_mt__declare_binop(__pow, op)
#undef op
#endif

#ifdef MATRIX_ENABLE__UNM
static int matrix_mt__unm(lua_State * L) {
    struct Matrix * m = (struct Matrix*)luaL_checkudata(L, 1, MATRIX_MT);
    struct Matrix * dest = push_matrix(L, m->rows, m->cols);
    int i;
    for (i = 0; i < m->rows * m->cols; i++) {
        dest->d[i] = -m->d[i];
    }
    return 1;
}
#endif

#if defined(MATRIX_TYPE_FLOAT)
#  define MATRIX_ABS_OP(x) fabsf(x)
#elif defined(MATRIX_TYPE_DOUBLE)
#  define MATRIX_ABS_OP(x) fabs(x)
#else
#  define MATRIX_ABS_OP(x) ((x)<0 ? -(x) : (x))
#endif

#ifdef MATRIX_ENABLE_RESHAPE
static int matrix_mt_reshape(lua_State * L) {
    struct Matrix * m = (struct Matrix*)luaL_checkudata(L, 1, MATRIX_MT);
    int rows = luaL_checkinteger(L, 2);
    int cols = luaL_checkinteger(L, 3);
    if (rows < 1 || cols < 1 || rows*cols != m->rows*m->cols)
        return luaL_error(L, "invalid reshape size %d*%d", rows, cols);
    m->rows = rows;
    m->cols = cols;
    return 0;
}
#endif

#ifdef MATRIX_ENABLE_T
static int matrix_mt_t(lua_State * L) {
    struct Matrix * m = (struct Matrix*)luaL_checkudata(L, 1, MATRIX_MT);
    struct Matrix * dest = push_matrix(L, m->cols, m->rows);
    int i, j;
    int m_cursor = 0;
    for (j = 0; j < m->cols; j++) {
        int dest_cursor = j;
        for (i = 0; i < m->rows; i++, dest_cursor += m->cols)
            dest->d[dest_cursor] = m->d[m_cursor++];
    }
    return 1;
}
#endif

#ifdef MATRIX_ENABLE_DOT
static int matrix_mt_dot(lua_State * L) {
    struct Matrix * m = (struct Matrix*)luaL_checkudata(L, 1, MATRIX_MT);
    struct Matrix * p = (struct Matrix*)luaL_checkudata(L, 2, MATRIX_MT);
    if (m->cols != p->rows)
        return luaL_error(L, "non-conformant matrix multiplication %d*%d by %d*%d", m->rows, m->cols, p->rows, p->cols);
    struct Matrix * dest = push_matrix(L, m->rows, p->cols);
    int i, j;
    for (j = 0; j < dest->cols; j++) {
        for (i = 0; i < dest->rows; i++) {
            MATRIX_TYPE res = 0;
            int m_cursor = i;
            int p_cursor = j * p->rows;
            int p_limit = p_cursor + p->rows;
            for (; p_cursor < p_limit; p_cursor++, m_cursor += m->rows)
                res += m->d[m_cursor] * p->d[p_cursor];
            dest->d[i + j * dest->rows] = res;
        }
    }
    return 1;
}
#endif

#ifdef MATRIX_ENABLE_TDOT
static int matrix_mt_tdot(lua_State * L) {
    struct Matrix * m = (struct Matrix*)luaL_checkudata(L, 1, MATRIX_MT);
    struct Matrix * p = (struct Matrix*)luaL_checkudata(L, 2, MATRIX_MT);
    if (m->rows != p->rows)
        return luaL_error(L, "non-conformant operands for tdot %d*%d by %d*%d", m->rows, m->cols, p->rows, p->cols);
    struct Matrix * dest = push_matrix(L, m->cols, p->cols);
    int i, j;
    for (j = 0; j < dest->cols; j++) {
        for (i = 0; i < dest->rows; i++) {
            MATRIX_TYPE res = 0;
            int m_cursor = i * m->rows;
            int p_cursor = j * p->rows;
            int p_limit = p_cursor + p->rows;
            for (; p_cursor < p_limit; p_cursor++, m_cursor++)
                res += m->d[m_cursor] * p->d[p_cursor];
            dest->d[i + j * dest->rows] = res;
        }
    }
    return 1;
}
#endif

#if defined(MATRIX_ENABLE_RSWAP) || defined(MATRIX_ENABLE_LUP)
static void matrix_rswap(struct Matrix * m, int r1, int r2) {
    int limit = m->rows * m->cols;
    for (; r1 < limit; r1 += m->rows, r2 += m->rows) {
        MATRIX_TYPE t = m->d[r1];
        m->d[r1] = m->d[r2];
        m->d[r2] = t;
    }
}
#endif

#ifdef MATRIX_ENABLE_RSWAP
static int matrix_mt_rswap(lua_State * L) {
    struct Matrix * m = (struct Matrix*)luaL_checkudata(L, 1, MATRIX_MT);
    int r1 = luaL_checkinteger(L, 2);
    int r2 = luaL_checkinteger(L, 3);
    if (r1 < 1 || r2 < 1 || r1 > m->rows || r2 > m->rows)
        return luaL_error(L, "invalid row indices for rswap: %d, %d", r1, r2);
    if (r1 == r2) return 0;
    matrix_rswap(m, r1, r2);
    return 0;
}
#endif

#ifdef MATRIX_ENABLE_CSWAP
static void matrix_cswap(struct Matrix * m, int c1, int c2) {
    int cursor1 = c1 * m->rows, cursor2 = c2 * m->rows;
    int i;
    for (i = m->rows; i >= 0; i--, cursor1++, cursor2++) {
        MATRIX_TYPE t = m->d[cursor1];
        m->d[cursor1] = m->d[cursor2];
        m->d[cursor2] = t;
    }
}

static int matrix_mt_cswap(lua_State * L) {
    struct Matrix * m = (struct Matrix*)luaL_checkudata(L, 1, MATRIX_MT);
    int c1 = luaL_checkinteger(L, 2);
    int c2 = luaL_checkinteger(L, 3);
    if (c1 < 1 || c2 < 1 || c1 > m->cols || c2 > m->cols)
        return luaL_error(L, "invalid column indices for cswap: %d, %d", c1, c2);
    if (c1 == c2) return 0;
    matrix_cswap(m, c1, c2);
    return 0;
}
#endif

#ifdef MATRIX_ENABLE_LUP
static int matrix_mt_lup(lua_State * L) {
    struct Matrix * m = (struct Matrix*)luaL_checkudata(L, 1, MATRIX_MT);
    MATRIX_TYPE tolerance = luaL_optnumber(L, 2, 1e-7);
    int i, j, k, swaps = 0, n = m->rows;
    int size = n*n;
    if (n != m->cols) return luaL_error(L, "square matrix required");
    int *p = malloc(sizeof (int) * n);
    for (i = 0; i < n; i++) p[i] = i;
    lua_createtable(L, 0, 4); //{L=..., U=..., p=ptable, swaps=integer}
    // u starts as a copy of m:
    struct Matrix * upper = push_matrix(L, n, n);
    memcpy(upper->d, m->d, sizeof (MATRIX_TYPE[n*n]));
    lua_setfield(L, -2, "U");
    int i_as_column_offset = 0;
    for (i = 0; i < n; i++, i_as_column_offset += n) {
        int maxi = i;
        MATRIX_TYPE maxabs = MATRIX_ABS_OP(upper->d[i_as_column_offset + i]);
        for (k = i + 1; k < n; k++) {
            MATRIX_TYPE abs_k = MATRIX_ABS_OP(upper->d[i_as_column_offset + k]);
            if (abs_k > maxabs) {
                maxabs = abs_k;
                maxi = k;
            }
        }
        if (maxabs < tolerance) {
            free(p);
            lua_pushnil(L);
            lua_pushliteral(L, "degenerate matrix");
            return 2;
        }
        if (i != maxi) {
            int t = p[i];
            p[i] = p[maxi];
            p[maxi] = t;
            matrix_rswap(upper, i, maxi);
            swaps++;
        }
        for (j = i + 1; j < n; j++) {
            MATRIX_TYPE pivot = (upper->d[j + i_as_column_offset] /= upper->d[i + i_as_column_offset]);
            for (k = i_as_column_offset + n; k < size; k += n) {
                upper->d[j + k] -= pivot * upper->d[i + k];
            }
        }
    }

    struct Matrix * lower = push_matrix(L, n, n);
    // build L as copy of U's lower part, setting zeros in U's lower triangle, L's upper triangle
    //       and ones into L's diagonal
    for (i = 0, i_as_column_offset = 0; i < n; i++, i_as_column_offset += n) {
        for (j = 0; j < i; j++) { // upper triangle
            lower->d[i_as_column_offset + j] = 0;
        }
        lower->d[i_as_column_offset + i] = 1; // diagonal
        for (j = i + 1; j < n; j++) { // lower triangle
            lower->d[i_as_column_offset + j] = upper->d[i_as_column_offset + j];
            upper->d[i_as_column_offset + j] = 0;
        }
    }
    lua_setfield(L, -2, "L");
    // setting p table as copy of p:
    lua_createtable(L, n, 0);
    for (i = 0; i < n; i++) {
        lua_pushinteger(L, p[i]);
        lua_rawseti(L, -2, i+1);
    }
    lua_setfield(L, -2, "p");
    struct Matrix * perm = push_matrix(L, n, n);
    // and P matrix as the permutation matrix corresponding to p:
    for (i = 0; i < size; i++) perm->d[i] = 0;
    for (i = 0; i < n; i++) perm->d[i + p[i]*n] = 1;
    lua_setfield(L, -2, "P");
    // setting swaps into returned table:
    lua_pushinteger(L, swaps);
    lua_setfield(L, -2, "swaps");
    free(p);
    return 1;
}
#endif

#ifndef EXPORT_C
#define EXPORT_C
#endif

EXPORT_C int luaopen_matrix(lua_State * L) {
    if (luaL_newmetatable(L, MATRIX_MT)) {
        luaL_getmetatable(L, MATRIX_MT);
        lua_pushstring(L, "cols");
        interned_cols = lua_tostring(L, -1);
        lua_pushinteger(L, -1);
        lua_settable(L, -3);
        lua_pushstring(L, "rows");
        interned_rows = lua_tostring(L, -1);
        lua_pushinteger(L, -1);
        lua_settable(L, -3);
        luaL_setfuncs(L, (struct luaL_Reg[]){
            {"__index", &matrix_mt__index},
            {"__newindex", &matrix_mt__newindex},
#ifdef MATRIX_ENABLE__TOSTRING
            {"__tostring", &matrix_mt__tostring},
#endif
#ifdef MATRIX_ENABLE_TOTABLE
            {"totable", &matrix_mt_totable},
#endif
#ifdef MATRIX_ENABLE__ADD
            {"__add", &matrix_mt__add},
#endif
#ifdef MATRIX_ENABLE__SUB
            {"__sub", &matrix_mt__sub},
#endif
#ifdef MATRIX_ENABLE__MUL
            {"__mul", &matrix_mt__mul},
#endif
#ifdef MATRIX_ENABLE__DIV
            {"__div", &matrix_mt__div},
#endif
#ifdef MATRIX_ENABLE__MOD
            {"__mod", &matrix_mt__mod},
#endif
#ifdef MATRIX_ENABLE__MOD
            {"__pow", &matrix_mt__pow},
#endif
#ifdef MATRIX_ENABLE__UNM
            {"__unm", &matrix_mt__unm},
#endif
#ifdef MATRIX_ENABLE_RESHAPE
            {"reshape", &matrix_mt_reshape},
#endif
#ifdef MATRIX_ENABLE_T
            {"t", &matrix_mt_t},
#endif
#ifdef MATRIX_ENABLE_DOT
            {"dot", &matrix_mt_dot},
#endif
#ifdef MATRIX_ENABLE_TDOT
            {"tdot", &matrix_mt_tdot},
#endif
#ifdef MATRIX_ENABLE_RSWAP
            {"rswap", &matrix_mt_rswap},
#endif
#ifdef MATRIX_ENABLE_CSWAP
            {"cswap", &matrix_mt_cswap},
#endif
#ifdef MATRIX_ENABLE_LUP
            {"lup", &matrix_mt_lup},
#endif
            {NULL, NULL}
        }, 0);
    }
    lua_pop(L, 1); // discard metatable
    // main table:
    lua_newtable(L);
    luaL_setfuncs(L, (struct luaL_Reg[]){
        {"new",    &matrix_new},
        {"id",     &matrix_id},
        {"random", &matrix_random},
        {"fromtable", &matrix_fromtable},
        {NULL,     NULL}
    }, 0);
    return 1;
}
// vi: et sw=4
