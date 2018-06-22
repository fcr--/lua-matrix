#!/usr/bin/env lua5.3

local matrix = require 'matrix'

local m = matrix.id(3)
assert(m[1] == 1 and m[2] == 0 and m[3] == 0)
assert(m[4] == 0 and m[5] == 1 and m[6] == 0)
assert(m[7] == 0 and m[8] == 0 and m[9] == 1)

math.randomseed(os.time())
local m = matrix.random(30, 20)
assert(m.rows == 30 and m.cols == 20)

assert(matrix.fromtable{2,20,cols=2}:dot(matrix.fromtable{3,30, 1,10,rows=2,cols=2}):dot(matrix.fromtable{1,1, rows=2})[1] == 808)

local m = matrix.fromtable{1,2,3,4,5,6, rows=3,cols=2}:t()
assert(m.rows==2 and m.cols==3)
assert(m[1] == 1 and m[3] == 2 and m[5] == 3)
assert(m[2] == 4 and m[4] == 5 and m[6] == 6)
