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
    return 0;
}

bool bf_optimization_pass_1(struct bf_program *program)
{
    int i;
    int offset;

    while (i < program->size) {
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
