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

#ifndef BF_PATTERNS_H
#define BF_PATTERNS_H

#include "program.h"

/**
 * === README ===
 *
 * This file contains many instruction patterns that may indicate an
 * optimization can be made. Just because these tests pass doesn't mean an
 * optimization can be made; they merely serves as a hint to the compiler. This
 * is due to the fact that many occurrences of the same brainfuck design
 * patterns can be of varying length depending on the 'operands', with a good
 * example of this being multiplication loops and copy loops.
 *
 * Every pattern is checked at different compilation passes, and because of
 * this certain instructions may not be in the IR yet. If you edit / add
 * patterns, be mindful of when they'll be check or they might not work as
 * expected. Adding new optimizations can also break existing optimizations, so
 * check for performance regressions when implementing them.
 */

/**
 * Clear loop [-]
 */
const struct bf_instruction bf_pattern_clear[] = {
    { BF_INS_BRANCH_Z, 0 },
    { BF_INS_DEC_V, 0 },
    { BF_INS_BRANCH_NZ, 0 },
};

/**
 * Copy loop [->+>+<<]
 */
const struct bf_instruction bf_pattern_copy[] = {
    { BF_INS_BRANCH_Z, 0 },
    { BF_INS_DEC_V, 0 },
    { BF_INS_INC_P, 0 },
    { BF_INS_INC_V, 0 },
};

/**
 * Multiplication loop [->+++>+++++++<<]
 *
 * The pattern assumes an add opcode so having a mul againt '1' will not be
 * optimized. It's very unlikely people will write code like that anyway, so
 * it's a little low on the priority list.
 */
const struct bf_instruction bf_pattern_mul[] = {
    { BF_INS_BRANCH_Z, 0 },
    { BF_INS_DEC_V, 0 },
    { BF_INS_INC_P, 0 },
    { BF_INS_ADD_V, 0 },
};

#endif
