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
#include <stdint.h>
#include <stdlib.h>

#define INSTRUCTION_ALLOC_COUNT 1024

/**
 * README: Any opcodes that are added here should have a string representation
 * defined in the implementation of 'bf_program_map_ins_name'.
 */
enum bf_opcode {
    BF_INS_NOP,
    BF_INS_IN, // ,
    BF_INS_OUT, // .
    BF_INS_INC_V, // +
    BF_INS_DEC_V, // -
    BF_INS_ADD_V, // (BF_INS_ADD_V, 3) = +++
    BF_INS_SUB_V, // (BF_INS_SUB_V, 3) = ---
    BF_INS_INC_P, // >
    BF_INS_DEC_P, // <
    BF_INS_ADD_P, // (BF_INS_ADD_P, 3) = >>>
    BF_INS_SUB_P, // (BF_INS_SUB_P, 3) = <<<
    BF_INS_BRANCH_Z, // (BF_INS_BRANCH_Z, address) = [
    BF_INS_BRANCH_NZ, // (BF_INS_BRANCH_NZ, address) = ]
    BF_INS_JMP,
    BF_INS_HALT,
    BF_INS_CLEAR, // [-]
    BF_INS_COPY, // (BF_INS_COPY, 1), (BF_INS_COPY, 2), (BF_INS_CLEAR) = [->+>+<<]
};

/**
 * Contains an opcode and an optional argument paired with the instruction.
 * This argument is almost always an address or handle.
 */
struct __attribute__((aligned)) bf_instruction {
    enum bf_opcode opcode;
    uint32_t argument;
};

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
bool bf_program_match_sequence(struct bf_program *program, const struct bf_instruction *ir, int pos, size_t size);

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
