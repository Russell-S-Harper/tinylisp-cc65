# tinylisp-cc65

tinylisp-cc65 is a port of Robert van Engelen’s tinylisp to compile under cc65 and run on 8-bit platforms. This port brings a working Lisp environment to these platforms where traditional Lisp implementations are too large.

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

One issue found during porting was that programs compiled with GNU `gcc` have their function arguments evaluated from left-to-right, whereas with `cc65` they are evaluated from right-to-left. Given the code has a lot of global variables, to be safe, the code was revised to ensure the same order of evaluation.

The build process is complex, so if you want to try it out beforehand you can copy the PRG and sample Lisp files from my [Google Drive](
https://drive.google.com/drive/folders/1QpG756L5m1HsCHO-QX4mNadew2sTPWxh?usp=sharing).

## Samples

Some sample source files are provided, note that the file names or contents may be in upper or lower case depending on the target:

- `CONSTANTS.LISP` – some numerical constants, e.g. `(* pi (* 10 10))` → `314.159`
- `MATH.LISP` – some mathematical functions, e.g. `(gcd 231 847)` → `77`
- `FACTOR.LISP` – factor a number, e.g. `(factor 39767)` → `(23 19 13 7)`
- `SORT.LISP` – sort a list, e.g. `(sort '(12 8.9 -1000 2.3 -9) <)` → `(-1000 -9 2.3 8.9 12)`
- `LIST.LISP` – various list functions, e.g. `(remove-dups '(1 2 3 4 1 1 2 2 5))` → `(1 2 3 4 5)`

Here’s a session for illustration where we’re loading some Lisp files and trying them out. Note that this implementation is definitely not a “speed demon”!

```
tinylisp
1852> (load 'constants)
constants
1849> pi
1839> e
1828> golden
1816> sqrt2
1805> gamma
1793> (print pi e golden sqrt2 gamma)
(3.142 2.718 1.618 1.414 0.577)
1793> (load 'factor)
factor
1792> mod
1744> lpf
1608> factor
1522> factor-x
1430> (factor 4101)
(1367 3)
1430> (factor 19695)
(101 13 5 3)
1430> (factor -49)
(7 7 -1)
1430> (factor 19697)
(19697)
1430> (bye)
bye!

```

## Building tinylisp-cc65

You’ll need [cc65](https://github.com/cc65/cc65), [FLT](https://github.com/Russell-S-Harper/FLT), and your preferred emulator (or actual hardware). Check the repositories if there are any other dependencies and be sure to adhere to the licensing terms to ensure proper usage and compliance.

Edit the `«flt-repo»/flt/build-cc65` script to point `XCC` to where the ***cc65*** repo is located and run the script. The script will build a library for each target.

After the libraries are created, edit the `«tinylisp-cc65-repo»/src/build-cc65` script to point `XCC` and `FLT` to where the ***cc65*** and ***FLT*** repositories are located, and in the specific function for your target, edit `DST` to point to a destination of your choice. Run the script to create the Lisp interpreter. The script will copy the interpreter and Lisp files to the destination, then (usually) run the emulator.

Current status for each target:

   | Target    | Tested with Emulator                                   | Notes
   |-----------|--------------------------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
   | apple2enh | [AppleWin](https://github.com/AppleWin/AppleWin)       | Still under investigation. The main issue is outdated, incomplete, and sometimes contradictory documentation, especially regarding emulator, ProDOS, and file-transfer workflows.
   | atari     | [atari800](https://atari800.github.io/)                | You’ll need to specify a device and number like `H1` to load, e.g. `(load 'h1:math)`, and because of the 8.3 filename format, for `CONSTANTS` use `(load 'h1:constnts)`.
   | c64       | [vice-jz.x64](https://vice-emu.sourceforge.io)         | If you have `c1541` installed, it will also create a `D64` image. You may need to set up a peripheral drive or a disk image in VICE.
   | cx16      | [x16emu](https://github.com/x16community/x16-emulator) | Straightforward integration and file handling during testing.

If you want to use a different emulator or actual hardware, ensure you revise the specific function for your target in `«tinylisp-cc65-repo»/src/build-cc65`.

## License

This repository is governed by the ***BSD-3-Clause license***. Your use of this code is subject to these license terms.

---

If you have any questions and/or suggestions, feel free to contact:

Russell Harper

russell.s.harper@gmail.com

2026-04-19
