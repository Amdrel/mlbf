# My Little Brainfuck

A simple brainfuck interpreter written in C. Brainfuck is fed through stdin and
output is written to stdout. If you ever wanted to use brainfuck in your
scripting environment this is the right tool for you.

mlbf can also convert brainfuck to C, which can then be passed to a compiler
and turned into native code. Depending on the optimization level used,
compilation may take a while; however when used with programs like
mandlebrot.b, performance increases as much as 6x can be seen at runtime.

## Why?

I plan on using this on a future project and it was a excellent excuse to get
comfortable with C.

## Security

Contact me at reshurum@gmail.com first for bugs that you believe are critical
security issues (buffer over-reads and the like) before reporting them in the
issue trackers.

## Changelog

### Jul 01, 2018

* Added brainfuck bytecode to C transpiler (4-6x performance increase on
average). This feature isn't compiled in at the moment as mlbf isn't set up
to accept command-line flags.
* Added copy loop optimization.

### Jun 30, 2018

* Added multiplication loop optimization.
* Added clear loop optimization.
* Added instruction contraction optimization.

### Jun 20, 2018

* Released a new bytecode compiler and interpreter for faster execution.

### May 29, 2018

* Added automated testing script.
* Migrated from GNU Make to Meson.

### May 20, 2018

* Implemented the ',' instruction.

### Apr 18, 2017

* First limited working version finished, albeit very slow.

## License

Copyright (c) 2017 Walter Kuppens

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
