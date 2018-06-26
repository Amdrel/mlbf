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

#ifndef BF_PROGRAM_H
#define BF_PROGRAM_H

#include <stdbool.h>
#include <stdlib.h>

#include "instruction.h"
#include "patterns.h"

/**
 * A dynamic array of compiled program instructions that can be given to the
 * brainfuck virtual machine for execution.
 */
struct bf_program {
    size_t size;
    size_t capacity;
    struct bf_instruction *ir;
};

/**
 * Initializes a new brainfuck program with a minimum capacity that's specified
 * in INSTRUCTION_ALLOC_COUNT.
 */
struct bf_program *bf_program_create();

/**
 * Cleans up memory used to store the program code.
 */
void bf_program_destroy(struct bf_program *program);

/**
 * Allocates more space for the program if needed.
 */
bool bf_program_grow(struct bf_program *program);

/**
 * Appends an instruction to the end of the program.
 */
bool bf_program_append(struct bf_program *program, const struct bf_instruction instruction);

/**
 * Injects IR into an existing program at a specified location. This function
 * will return false if the IR won't fit at the position specified.
 */
bool bf_program_substitute(struct bf_program *program, const struct bf_instruction *ir, int pos, size_t size);

/**
 * Compares a sequence of instruction opcodes at the desired position to a
 * referenced list of instructions. This function is primarily used during
 * optimization of existing IR while looking for common optimizable patterns.
 */
bool bf_program_match_sequence(struct bf_program *program, const struct bf_pattern_rule *rules, int pos, size_t size);

/**
 * Dumps the program bytecode to stdout.
 */
void bf_program_dump(const struct bf_program *program);

/**
 * Returns a string representation of a given instruction. This is used
 * primarily for debugging purposes (dumping human-readable logs and IR code).
 */
const char *bf_program_map_ins_name(enum bf_opcode opcode);

#endif
