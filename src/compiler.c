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

#include <stdio.h>

#include "assert.h"
#include "compiler.h"
#include "interpreter.h"
#include "patterns.h"
#include "program.h"

struct bf_program *bf_compile(char *src)
{
    struct bf_program *program;

    program = bf_program_create();
    if (!program) {
        goto error1;
    }

    if (!bf_unoptimized_pass(program, src)) {
        goto error2;
    }

    if (!bf_optimization_pass_1(program)) {
        goto error2;
    }
    if (!bf_optimization_pass_2(program)) {
        goto error2;
    }
    if (!bf_optimization_pass_3(program)) {
        goto error2;
    }

    return program;

error2:
    bf_program_destroy(program);
error1:
    return NULL;
}

bool bf_unoptimized_pass(struct bf_program *program, const char *src)
{
    char ch;
    int i = 0;
    int offset = 0;
    int address;

    while ((ch = src[i]) != '\0') {
        switch (ch) {
        case '>':
            bf_program_append(program,
                (struct bf_instruction){
                    .opcode = BF_INS_INC_P,
                    .argument = 0,
                });
            break;
        case '<':
            bf_program_append(program,
                (struct bf_instruction){
                    .opcode = BF_INS_DEC_P,
                    .argument = 0,
                });
            break;
        case '+':
            bf_program_append(program,
                (struct bf_instruction){
                    .opcode = BF_INS_INC_V,
                    .argument = 0,
                });
            break;
        case '-':
            bf_program_append(program,
                (struct bf_instruction){
                    .opcode = BF_INS_DEC_V,
                    .argument = 0,
                });
            break;
        case '.':
            bf_program_append(program,
                (struct bf_instruction){
                    .opcode = BF_INS_OUT,
                    .argument = 0,
                });
            break;
        case ',':
            bf_program_append(program,
                (struct bf_instruction){
                    .opcode = BF_INS_IN,
                    .argument = 0,
                });
            break;
        case '[':
            address = bf_find_closing_brace(i, src);
            if (address < 0) {
                goto error1;
            }

            bf_program_append(program,
                (struct bf_instruction){
                    .opcode = BF_INS_BRANCH_Z,
                    .argument = address + 1 - offset,
                });

            break;
        case ']':
            address = bf_find_opening_brace(i, src);
            if (address < 0) {
                goto error1;
            }

            bf_program_append(program,
                (struct bf_instruction){
                    .opcode = BF_INS_BRANCH_NZ,
                    .argument = address + 1 - offset,
                });
            break;
        default:
            offset++;
            break;
        }

        i++;
    }

    // Ensure there's a halt at the end so the interpreter stops when execution
    // reaches the end of the program.
    bf_program_append(program,
        (struct bf_instruction){
            .opcode = BF_INS_HALT,
            .argument = 0,
        });

    return true;

error1:
    return false;
}

/**
 * Peeks at IR at and ahead of the cursor and injects a clear instruction
 * in-place of a clear loop if a one is detected.
 */
int bf_try_optimization_clear_loop(struct bf_program *program, int pos)
{
    const size_t clear_pattern_length = sizeof(bf_pattern_clear) / sizeof(bf_pattern_clear[0]);

    if (!bf_program_match_sequence(program, bf_pattern_clear, pos, clear_pattern_length)) {
        return 0;
    }

    const struct bf_instruction new_ir[] = {
        { BF_INS_CLEAR, 0 },
        { BF_INS_NOP, 0 },
        { BF_INS_NOP, 0 },
    };
    const size_t new_ir_length = sizeof(new_ir) / sizeof(new_ir[0]);
    assert(new_ir_length == clear_pattern_length);

    bf_program_substitute(program, new_ir, pos, new_ir_length);

    return new_ir_length;
}

/**
 * Peeks at IR and looks for a multiplication loop. A variable amount of MUL
 * instructions and a CLEAR will be added if one is found.
 */
int bf_try_optimization_mul_loop(struct bf_program *program, int pos)
{
    const size_t mul_pattern_length = sizeof(bf_pattern_mul) / sizeof(bf_pattern_mul[0]);
    const size_t mul_pattern_op_length = sizeof(bf_pattern_mul_op) / sizeof(bf_pattern_mul_op[0]);

    int read_cursor = pos;
    int write_cursor = pos;
    int pattern_length = 0;
    int opcount = 0;
    int offset = 0;
    int seq_offset = 0;

    // We're -potentially- dealing with a mul loop if this pattern is found.
    if (!bf_program_match_sequence(program, bf_pattern_mul, read_cursor, mul_pattern_length)) {
        return 0;
    }
    read_cursor += 2; // Jump over the branch and clear sections of the loop.

    // Look for sequences of pointer increments (offset) and additions (mul
    // operand). NOPs are skipped over when trying to extract the mul result
    // offset as they're very likely to be present.
    while ((seq_offset = bf_program_match_sequence(program, bf_pattern_mul_op, read_cursor, mul_pattern_op_length)) != 0) {
        int nop_offset = read_cursor;
        while (program->ir[nop_offset].opcode == BF_INS_NOP) {
            nop_offset++;
        }

        opcount++;
        offset += program->ir[nop_offset].argument;
        read_cursor += seq_offset;
    }
    if (opcount <= 0) {
        return 0;
    }

    // Note that the offset is passed to the pattern. The number of pointer
    // decrements must match the amount of 'operations' in the multiplication
    // loop. If this fails that means the multiplication loop is either an
    // uncommon variation or it's simply not a mul loop.
    //
    // This is where the pointer is reset to the previous value so the next
    // iterations operate on the same section of memory.
    const struct bf_pattern_rule end_pattern[] = {
        { { BF_INS_SUB_P, offset }, BF_PATTERN_STRICT },
        { { BF_INS_BRANCH_NZ, 0 }, 0 },
    };
    const size_t end_pattern_length = sizeof(end_pattern) / sizeof(end_pattern[0]);
    if ((seq_offset = bf_program_match_sequence(program, end_pattern, read_cursor, end_pattern_length)) == 0) {
        return 0;
    }

    pattern_length = (read_cursor + seq_offset) - pos;

    read_cursor = pos + 2;
    offset = 0;
    seq_offset = 0;

    // At this point we're looking at a mul loop, start doing destructive
    // mutations on the IR now that we're confident in our assumptions.
    //
    // This bit of code will overwrite the IR as it reads all the mul
    // operations. Since the resulting optimized code is always smaller, there
    // shouldn't be an issue with doing this.
    while ((seq_offset = bf_program_match_sequence(program, bf_pattern_mul_op, read_cursor, mul_pattern_op_length)) != 0) {
        int nop_offset = read_cursor;
        int argument = 0;

        // Get the offset for the MUL operation with the pointer shift value.
        while (program->ir[nop_offset].opcode == BF_INS_NOP) {
            nop_offset++;
        }
        offset += program->ir[nop_offset].argument;
        nop_offset++;

        // Get the argument for the MUL operation with the increment value.
        while (program->ir[nop_offset].opcode == BF_INS_NOP) {
            nop_offset++;
        }
        argument = program->ir[nop_offset].argument;

        program->ir[write_cursor].opcode = BF_INS_MUL;
        program->ir[write_cursor].argument = argument;
        program->ir[write_cursor].offset = offset;

        read_cursor += seq_offset;
        write_cursor++;
    }

    // Add a last CLEAR instruction since multiplication loops end up clearing
    // the cell they're using as an argument for the operation.
    program->ir[write_cursor].opcode = BF_INS_CLEAR;
    program->ir[write_cursor].argument = 0;
    program->ir[write_cursor].offset = 0;
    write_cursor++;

    // Replace remaining instructions from the old mul loop with NOPs.
    while (write_cursor < pos + pattern_length) {
        program->ir[write_cursor].opcode = BF_INS_NOP;
        program->ir[write_cursor].argument = 0;
        program->ir[write_cursor].offset = 0;
        write_cursor++;
    }

    return pattern_length;
}

int bf_try_optimization_combine_inc_v(struct bf_program *program, int pos)
{
    int i = pos;
    int accumulator = 0;
    int end;

    // Figure out how many sequential instructions there are.
    while (i < program->size) {
        if (program->ir[i].opcode == BF_INS_INC_V) {
            accumulator++;
        } else {
            break;
        }
        i++;
    }

    if (accumulator > 0) {
        i = pos;
        end = pos + accumulator;

        // Inject the new add instruction.
        program->ir[i].opcode = BF_INS_ADD_V;
        program->ir[i].argument = accumulator;

        i++;

        // Replace the remaining instructions that were previously increments
        // with NOPs. These will be stripped out later.
        while (i < end) {
            program->ir[i].opcode = BF_INS_NOP;
            program->ir[i].argument = 0;
            i++;
        }
    }

    return accumulator;
}

int bf_try_optimization_combine_dec_v(struct bf_program *program, int pos)
{
    int i = pos;
    int accumulator = 0;
    int end;

    // Figure out how many sequential instructions there are.
    while (i < program->size) {
        if (program->ir[i].opcode == BF_INS_DEC_V) {
            accumulator++;
        } else {
            break;
        }
        i++;
    }

    if (accumulator > 0) {
        i = pos;
        end = pos + accumulator;

        // Inject the new add instruction.
        program->ir[i].opcode = BF_INS_SUB_V;
        program->ir[i].argument = accumulator;

        i++;

        // Replace the remaining instructions that were previously increments
        // with NOPs. These will be stripped out later.
        while (i < end) {
            program->ir[i].opcode = BF_INS_NOP;
            program->ir[i].argument = 0;
            i++;
        }
    }

    return accumulator;
}

int bf_try_optimization_combine_inc_p(struct bf_program *program, int pos)
{
    int i = pos;
    int accumulator = 0;
    int end;

    // Figure out how many sequential instructions there are.
    while (i < program->size) {
        if (program->ir[i].opcode == BF_INS_INC_P) {
            accumulator++;
        } else {
            break;
        }
        i++;
    }

    if (accumulator > 0) {
        i = pos;
        end = pos + accumulator;

        // Inject the new add instruction.
        program->ir[i].opcode = BF_INS_ADD_P;
        program->ir[i].argument = accumulator;

        i++;

        // Replace the remaining instructions that were previously increments
        // with NOPs. These will be stripped out later.
        while (i < end) {
            program->ir[i].opcode = BF_INS_NOP;
            program->ir[i].argument = 0;
            i++;
        }
    }

    return accumulator;
}

int bf_try_optimization_combine_dec_p(struct bf_program *program, int pos)
{
    int i = pos;
    int accumulator = 0;
    int end;

    // Figure out how many sequential instructions there are.
    while (i < program->size) {
        if (program->ir[i].opcode == BF_INS_DEC_P) {
            accumulator++;
        } else {
            break;
        }
        i++;
    }

    if (accumulator > 0) {
        i = pos;
        end = pos + accumulator;

        // Inject the new add instruction.
        program->ir[i].opcode = BF_INS_SUB_P;
        program->ir[i].argument = accumulator;

        i++;

        // Replace the remaining instructions that were previously increments
        // with NOPs. These will be stripped out later.
        while (i < end) {
            program->ir[i].opcode = BF_INS_NOP;
            program->ir[i].argument = 0;
            i++;
        }
    }

    return accumulator;
}

/*
 * Replaces increments and decrements with ADDs and SUBs. This is done for two
 * reasons:
 *
 * 1. ADDs / SUBs are easier to check when looking for optimization patterns.
 * 2. It's not very efficient to increment and decrement in a loop.
 *
 * Once all complex optimizations are done, ADDs and SUBs with '1' in them can
 * be turned back into INC and DEC instructions (this happens in a later pass).
 */
bool bf_optimization_pass_1(struct bf_program *program)
{
    int i = 0;
    int offset;

    while (i < program->size) {
        if (program->ir[i].opcode == BF_INS_NOP) {
            i++;
            continue;
        }

        if ((offset = bf_try_optimization_combine_inc_v(program, i))) {
            i += offset;
            continue;
        }
        if ((offset = bf_try_optimization_combine_dec_v(program, i))) {
            i += offset;
            continue;
        }
        if ((offset = bf_try_optimization_combine_inc_p(program, i))) {
            i += offset;
            continue;
        }
        if ((offset = bf_try_optimization_combine_dec_p(program, i))) {
            i += offset;
            continue;
        }

        i++;
    }

    return true;
}

/**
 * Pass 2 applys optimizations for the following constructs:
 *
 * - Clear Loops
 * - Multiplication Loops
 * - Copy Loops
 * - Scan Loops (uses memchr)
 */
bool bf_optimization_pass_2(struct bf_program *program)
{
    int i = 0;
    int offset;

    while (i < program->size) {
        if (program->ir[i].opcode == BF_INS_NOP) {
            i++;
            continue;
        }

        // Replaces clear loops with singular clear instructions.
        if ((offset = bf_try_optimization_clear_loop(program, i))) {
            i += offset;
            continue;
        }
        // Replaces multiplication loops with multiply instructions.
        if ((offset = bf_try_optimization_mul_loop(program, i))) {
            i += offset;
            continue;
        }

        i++;
    }

    return true;
}

/**
 * Replaces occurences of ADD(1) and SUB(1) with INC and DEC respectively.
 *
 * Optimizations that happen here assume that branches always appear in certain
 * orders and always have a matching branch 'brace'.
 */
bool bf_optimization_pass_3(struct bf_program *program)
{
    int i = 0;
    int offset = 0;
    int addr = 0;

    while (i < program->size) {
        enum bf_opcode opcode = program->ir[i].opcode;
        enum bf_opcode argument = program->ir[i].argument;

        if (opcode == BF_INS_NOP) {
            offset++;
        } else if (opcode == BF_INS_BRANCH_Z) {
            addr = program->ir[i].argument;
            program->ir[addr - 1].offset = offset;
            program->ir[i - offset] = program->ir[i];
        } else if (opcode == BF_INS_BRANCH_NZ) {
            addr = program->ir[i].argument - program->ir[i].offset;
            program->ir[addr - 1].offset = offset;
            program->ir[i - offset] = program->ir[i];
        } else if (opcode == BF_INS_ADD_V && argument == 1) {
            program->ir[i].opcode = BF_INS_INC_V;
            program->ir[i].argument = 0;
            program->ir[i - offset] = program->ir[i];
        } else if (opcode == BF_INS_SUB_V && argument == 1) {
            program->ir[i].opcode = BF_INS_DEC_V;
            program->ir[i].argument = 0;
            program->ir[i - offset] = program->ir[i];
        } else if (opcode == BF_INS_ADD_P && argument == 1) {
            program->ir[i].opcode = BF_INS_INC_P;
            program->ir[i].argument = 0;
            program->ir[i - offset] = program->ir[i];
        } else if (opcode == BF_INS_SUB_P && argument == 1) {
            program->ir[i].opcode = BF_INS_DEC_P;
            program->ir[i].argument = 0;
            program->ir[i - offset] = program->ir[i];
        } else {
            program->ir[i - offset] = program->ir[i];
        }

        i++;
    }

    program->size -= offset;
    i = 0;

    while (i < program->size) {
        enum bf_opcode opcode = program->ir[i].opcode;

        if (opcode == BF_INS_BRANCH_NZ) {
            program->ir[i].argument -= program->ir[i].offset;
        } else if (opcode == BF_INS_BRANCH_Z) {
            program->ir[i].argument -= program->ir[i].offset;
        }

        i++;
    }

    return true;
}

int bf_find_closing_brace(int pos, const char *src)
{
    char ch;
    int i = pos + 1;
    int depth = 0;
    int offset = 0;
    int result = -1;

    // Failsafe in-case the position is at the end for some reason.
    if (src[pos] == '\0') {
        goto error1;
    }

    while ((ch = src[i]) != '\0') {
        if (!bf_is_valid_instruction(ch)) {
            offset--;
            i++;
            continue;
        }

        if (ch == '[') {
            depth++;
        } else if (ch == ']') {
            if (depth == 0) {
                result = i;
                break;
            } else {
                depth--;
            }
        }

        i++;
    }

    return result + offset;

error1:
    return -1;
}

int bf_find_opening_brace(int pos, const char *src)
{
    char ch;
    int i = pos - 1;
    int depth = 0;
    int offset = 0;
    int result = -1;

    while (i >= 0) {
        ch = src[i];

        if (!bf_is_valid_instruction(ch)) {
            offset++;
            i--;
            continue;
        }

        if (ch == ']') {
            depth++;
        } else if (ch == '[') {
            if (depth == 0) {
                result = i;
                break;
            } else {
                depth--;
            }
        }

        i--;
    }

    return result + offset;
}

bool bf_is_valid_instruction(const char ch)
{
    switch (ch) {
    case '>':
        return true;
    case '<':
        return true;
    case '+':
        return true;
    case '-':
        return true;
    case '.':
        return true;
    case ',':
        return true;
    case '[':
        return true;
    case ']':
        return true;
    default:
        return false;
    }
}
