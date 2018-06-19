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

#include "program.h"

struct bf_program *bf_program_create()
{
    struct bf_program *program = calloc(1, sizeof(struct bf_program));
    if (!program) {
        goto error1;
    }

    struct bf_instruction *ir = calloc(1, sizeof(struct bf_instruction) * INSTRUCTION_ALLOC_COUNT);
    if (!ir) {
        goto error2;
    }

    program->size = 0;
    program->capacity = INSTRUCTION_ALLOC_COUNT;
    program->ir = ir;

    return program;

error2:
    free(program);
error1:
    return NULL;
}

void bf_program_destroy(struct bf_program *program)
{
    free(program->ir);
    free(program);
}

bool bf_program_grow(struct bf_program *program)
{
}

bool bf_program_append(struct bf_program *program, struct bf_instruction instruction)
{
}
