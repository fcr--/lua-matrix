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

#### Operations

TODO
