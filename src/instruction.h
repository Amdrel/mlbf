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

#ifndef BF_INSTRUCTION_H
#define BF_INSTRUCTION_H

#include <stdint.h>

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

#endif
