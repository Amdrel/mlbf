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

#include "compiler.h"
#include "interpreter.h"
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

    return program;

error2:
    bf_program_destroy(program);
error1:
    return NULL;
}

struct bf_program *bf_unoptimized_pass(struct bf_program *program, char *src)
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

    return program;

error1:
    return NULL;
}

int bf_find_closing_brace(int pos, char *src)
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

error1:
    return result + offset;
}

int bf_find_opening_brace(int pos, char *src)
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

bool bf_is_valid_instruction(char ch)
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
