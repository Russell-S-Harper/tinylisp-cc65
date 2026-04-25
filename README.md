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

There are still some issues, especially within the `lambda` context, not sure why, researching if it’s my error or something already existing. One issue was that programs compiled with GNU `gcc` have their function arguments evaluated from left-to-right, whereas with `cc65` they are evaluated from right-to-left. Given the code has a lot of globals, to be safe, the code was revised to ensure the same order of evaluation.

The build process is complex, so if you want to try it out beforehand you can copy the PRG and sample Lisp files from my [Google Drive](
https://drive.google.com/drive/folders/1QpG756L5m1HsCHO-QX4mNadew2sTPWxh?usp=sharing).

## Samples

Some sample source files are provided, note that the file names and contents are in uppercase:

- `CONSTANTS.LISP` – some numerical constants, e.g. `(* pi (* 10 10))` → `314.159`
- `MATH.LISP` – some mathematical functions, e.g. `(gcd 231 847)` → `77`
- `FACTOR.LISP` – factor a number, e.g. `(factor 39767)` → `(23 19 13 7 1)`
- `SORT.LISP` – sort a list, e.g. `(sort '(12 89 -1000 2.3 -9) <)` → `(-1000 -9 2.3 12 89)`

Here’s a session for illustration where we’re loading some Lisp files and trying them out. Note that this implementation is definitely not a “speed demon” on the CX16!

```
tinylisp
1857> (load 'constants)
constants
1855> pi
1844> e
1834> golden
1822> sqrt2
1810> gamma
1799> (print pi e golden sqrt2 gamma)
(3.142 2.718 1.618 1.414 0.577)
1799> (load 'factor)
factor
1797> mod
1749> abs
1706> lpf
1609> lpf-x
1512> factor
1452> factor-x
1373> (factor 4101)
(1367 3 1)
1373> (factor 19695)
(101 13 5 3 1)
1373> (factor -49)
(7 7 -1 1)
1373> (factor 19697)
(19697 1)
1373> (bye)
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
