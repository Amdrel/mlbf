// Copyright (c) 2017 Walter Kuppens
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef BF_COMPILER_H
#define BF_COMPILER_H

#include <stdbool.h>

/**
 * Generates a compiled brainfuck progam from a brainfuck source string. The
 * string that's passed in doesn't have ownership transferred.
 */
struct bf_program *bf_compile(char *src);

/**
 * Performs an unoptimized compilation of source. An AST isn't passed in the
 * function arguments since brainfuck is a very simple language.
 */
bool bf_unoptimized_pass(struct bf_program *program, const char *src);

/**
 * Replaces clear loops with CLEAR instructions.
 */
bool bf_optimization_pass_1(struct bf_program *program);

/**
 * Utility function that finds a matching closing brace in the source code.
 * This is used by the compiler to determine the addresses of conditional jumps
 *
 * A negative return value indicates a matching brace wasn't found.
 */
int bf_find_closing_brace(int pos, const char *src);

/**
 * Utility function that finds a matching opening brace in the source code.
 * This is used by the compiler to determine the addresses of conditional jumps
 *
 * A negative return value indicates a matching brace wasn't found.
 */
int bf_find_opening_brace(int pos, const char *src);

/**
 * Returns true if the character passed is a valid brainfuck instruction.
 */
bool bf_is_valid_instruction(const char ch);

#endif
