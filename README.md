# lua-matrix
Yet another matrix library for lua without external dependencies

## Usage

Importing the library:
```lua
matrix = require 'matrix'
```

#### Creation

| method | description |
|--------|-------------|
| `m = matrix.new(h, w)` or<br> `m = matrix.new{h, w}` | creates a new h\*w matrix with zeros |
| `m = matrix.new{h, w, value=v}` | creates a new h\*w matrix with value v in every cell |
| `m = matrix.random(h, w)` | creates a random h\*w matrix (values between 0 and 1) |
| `m = matrix.id(n)` | creates an identity n\*n matrix |
| `m = matrix.fromtable{v1, ..., vn, rows=h, cols=w}` | creates a matrix from a table |

#### Conversion

To convert a matrix to a table: `m:totable()` which returns a lua table with indexes 1 to m.rows\*m.cols
containing the values for each cell plus the dimensions with the keys rows and cols.
Matrices are stored in column major order, which means that any h\*w matrix will be stored linearly
with the first *1* .. *h* entries containing the first column; *h+1* to *2\*h* the second one, and so on.

For instance, the following table, will be converted to `{1,2, 3,4, 5,6, cols=3, rows=2}`:

| 1 | 3 | 5 |
|---|---|---|
| 2 | 4 | 6 |

A simple way to initialize a matrix (when the values are known) is to convert a table back to a matrix
with `matrix.fromtable(t)`, where the argument must have the same format as returned by `m:totable()`.
Even though the `cols` and `rows` keys may be optional (default 1), the number of numeric items in the
table `#t` must be equal to `(t.cols or 1)*(t.rows or 1)`.

#### Slicing

In matrix jargon, slicing refers to the process of extracting a subset made of contiguous rows and
contiguous columns.  In this library both reading and writing slices is supported.

Let m be a h\*w matrix:
* `m[{}]` ← returns a copy of the array.
* `m[{1}]` ← returns a 1\*w matrix with the contents of m's first row
* `m[{{2,4}}]` ← returns a 3\*w matrix with the contents of m's second to fourth rows
* `m[{nil,2}]` ← returns a h\*1 matrix with a copy of m's second column.
* `m[{{10,19},{20,39}]` ← returns a 10\*20 matrix with the rows 10 to 19 of the columns 20 to 39 of m.

Note that returned slices are not "views" but full fledged matrices, copying the content of the
slice instead of sharing memory.

Slices can also be used for writing operations using a similar syntax. Let m be a h\*w matrix:
* `m[{1}] = 42` ← fills the matrix first row with fourty-twos
* `m[{nil,{2,3}}] = p` ← copies p into m's second and third columns, p must be a h\*2 matrix.

#### Operations

TO DOCUMENT

Transpose: `m:t()`
> Returns the transposed matrix.

LUp decomposition: `lu = m:lup()`
> Returns a table containing the arrays for the LU decomposition, where:
> * `lu.L` is a lower triangular matrix with ones on its diagonal,
> * `lu.U` is an upper triangular matrix, and
> * `lu.P` is a permutation matrix (also saved as a table in lu.p, such that all lu.P's non-zero elements are located at `{i, lu.p[i]}`).
> * `lu.det` m's determinant.
>
> Note that lu.P:t():dot(lu.L:dot(lu.U)) should be approximately equal to lu ± computation errors.

Reduced Row Echelon Form: `rref, inv = m:rref()`
> This function returns the RREF of m. If m is square it also returns m's inverse. At each column this implementation chooses the row with biggest absolute value as the pivot (swapping the rows if needed) for best stability.

Matrix inversion: `inv = m:inv()`
> The m matrix must be square or an error is returned. Since the implementation uses `select(2, m:rref())` a result is returned even if it's not invertible, so (in that case) don't expect m:inv():inv() ≈ m.
> To check for invertibility use `m.cols == m.rows and m:rref()[m.cols*m.rows] == 1`.

#### Mutable Operations

The operations documented in this section change in some or other way the content of the matrices
used, and are only provided for performance reasons. Be careful if you choose to use them.

Row swap: `m:rswap(i1, i2)`
> Interchanges the contents of rows *i1* and *i2*. The arguments must be integers between 1 and `m.rows`.

Column swap: `m:cswap(j1, j2)`
> Similar to row swap, but this time the contents of the columns *j1* and *j2* get swapped. The arguments must be integers between 1 and `m.cols`.

Reshape: `m:reshape(h, w)`
> This changes the `rows` and `cols` attribute of the underlaying matrix without changing the content.
> However, the total size of the matrix must be kept the same, thus h\*w must be equal to m.rows\*m.cols.
> This can be used as a fast way to transpose a row vector to a column vector and vice versa.
