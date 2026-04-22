# tinylisp-cc65

tinylisp-cc65 is a port of Robert van Engelen’s tinylisp to compile under cc65 and run on 8-bit platforms. This port brings a working Lisp environment to resource-constrained 8-bit systems, like the Commodore 64 and the Commander X16, where traditional Lisp implementations are too large.

## About

The source is `tinylisp-float-opt.c` in [Robert van Engelen’s tinylisp](https://github.com/Robert-van-Engelen/tinylisp) to which I added some additional features to accommodate 8-bit environments:

- optional tracing for debugging purposes (compile with -DTRACE)
- `(print arg1 arg2 ...)` print arguments
- `(load file)` load and evaluate `file.lisp`, e.g. `(load 'common)` to load `common.lisp`
- `(incr var1 var2 ...)` increment variables, mutates in place
- `(decr var1 var2 ...)` decrement variables, mutates in place
- `(while condition code1 code2 ...)` non-recursive loop to run code
- `(bye)` end session gracefully as opposed to exiting upon reading `EOF`

I didn’t realize until later that the “extras” in Robert van Engelen’s tinylisp also included a `print` and a `load`, ours are quite similar!

There are still some issues, especially within the `lambda` context with `define` and `let*`, not sure why, researching if it’s my error or something already existing. One issue was that programs compiled with GNU `gcc` have their function arguments evaluated from left-to-right, whereas with `cc65` they are evaluated from right-to-left. Given the code has a lot of globals, to be safe, the code was revised to ensure the same order of evaluation.

The build process is complex, so if you want to try it out beforehand you can copy the PRG and sample Lisp files from my [Google Drive](
https://drive.google.com/drive/folders/1QpG756L5m1HsCHO-QX4mNadew2sTPWxh?usp=sharing). Note the Lisp file contents are in uppercase.

## Samples

Some sample source files are provided:

- `CONSTANTS.LISP` – some numerical constants
- `MATH.LISP` – some mathematical functions
- `FACTOR.LISP` – factor a number, e.g. `(factor 39767)` → `(23 19 13 7 1)`

Here's a session for illustration: loading `math` and finding 29 as a factor of 899. Note that this implementation is definitely not a “speed demon” on the CX16!

```
tinylisp
1857> (load 'math)
math
1856> abs
1813> frac
1779> truncate
1767> floor
1720> ceiling
1680> round
1646> mod
1599> gcd
1548> lcm
1505> even?
1469> odd?
1434> (define n 899)
n
1424> (define i 2)
i
1413> (while (< 0 (mod n i)) (incr i))
29
1413> (bye)
bye!
```

## Build for C64

[in progress - likely won’t have a lot of room!]

## Build for CX16

Should you want to build `LISP.PRG` yourself, you’ll need [cc65](https://github.com/cc65/cc65), [FLT](https://github.com/Russell-S-Harper/FLT), and [x16emu](https://github.com/x16community/x16-emulator). Check these repositories if there are any other dependencies. Be sure to adhere to the licensing terms provided in these repositories to ensure proper usage and compliance.

Edit the `«flt-repo»/flt/build-cc65` script to point `XCC` to where the ***cc65*** repo is located, revise `TGT` as required, and run the script to build the `flt.lib` library.

Then edit the `«tinylisp-cc65-repo»/src/build-cc65-cx16` script to point `XCC`, `FLT`, and `EMU` to where the ***cc65***, ***FLT*** and ***x16emu*** repositories are located, and run the script to create `LISP.PRG`.

## License

This repository is governed by the ***BSD-3-Clause license***. Your use of this code is subject to these license terms.

---

If you have any questions and/or suggestions, feel free to contact:

Russell Harper

russell.s.harper@gmail.com

2026-04-19
