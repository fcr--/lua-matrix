# lua-matrix
Yet another matrix library for lua without external dependencies

## Usage

Importing the library:
```lua
matrix = require 'matrix'
```

#### Creation

* Creating a new h*w matrix with zeros: `matrix.new(h, w)` or `matrix.new{h, w}`.
* Creating a new h*w matrix with some value in every cell: `matrix.new{h, w, value=42}`.
* Creating a random h*w matrix (values between 0 and 1): `matrix.random(h, w)`.
* Creating an identity n*n matrix: `matrix.id(n)`.
* Creating a matrix from a table: `matrix.fromtable{v1, ..., vn, rows=h, cols=w}`.

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

TO DOCUMENT

#### Operations

TO DOCUMENT

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
