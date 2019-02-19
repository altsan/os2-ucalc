Useful Calculator for OS/2
==========================

_Useful Calculator_ is a simple desktop calculator for OS/2, written
using the native Presentation Manager API.  It has no external
dependencies and should work on all OS/2 Warp systems.  

(This program has been successfully tested on OS/2 Warp version 3.  
However, Warp 4 or later is recommended so that the `MMPMDigital` font
is available.)


Functionality
-------------

The calculator functions are arranged in three groups:

 * Numbers and basic arithmetic operations (addition, subtraction,
   multiplication, division, and modulo/remainder), as well as simple
   memory operations.

 * Algebraic functions (pi, square, exponent-n, square root, n-root, 
   reciprocal, base-10 logarithm, and natural logarithm).  

 * Bitwise (binary logic) operations: OR, AND, eXclusive OR, NOT (i.e.
   ones-complement negation), left bit-shift, and right bit-shift.

Only decimal input is supported, but the hexadecimal equivalent of the
currently-displayed value (integer portion only) is shown under the 
input area.  (Negative numbers are shown in 32-bit twos-complement form
for values up to negative 0xFFFFFFFF, and 64-bit twos-complement form
for higher-magnitude negative values.)

Mathematical order of operations is observed.  For example, entering 
`5 [+] 2 [×] 3 [xⁿ] 3` will be evaluated as `5 + (2 × 3³)`, yielding a 
result of `59`.

Values requiring more than 10 significant digits will be displayed in
exponential notation.

Values beyond the range of ±9223372036854775807 cannot be used as 
operands; this is a limitation of the integer types used.  Such values
are supported as final results in some cases, but will (as above) be 
shown in exponential notation.


Notices
-------

    Useful Calculator for OS/2
    Copyright (C) 2018-2019 Alexander Taylor.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

URL: https://github.com/altsan/os2-ucalc

Contact: http://altsan.org/contact.html
