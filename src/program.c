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

#include <stdbool.h>
#include <stdio.h>

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

/**
 * Unconditionally increases the capacity of contiguous memory holding the
 * bytecode by INSTRUCTION_ALLOC_COUNT * sizeof(struct bf_instruction) bytes.
 */
bool bf_program_grow(struct bf_program *program)
{
    struct bf_instruction *resized_ir;
    size_t new_capacity;

    new_capacity = program->capacity + INSTRUCTION_ALLOC_COUNT;
    resized_ir = realloc(program->ir, sizeof(struct bf_instruction) * new_capacity);
    if (!resized_ir) {
        goto error1;
    }

    program->ir = resized_ir;
    program->capacity = new_capacity;

    return true;

error1:
    return false;
}

/**
 * Appends an instruction to the end of the program. This function will also
 * allocate more space by calling 'bf_program_grow' if there isn't enough room
 * for the new instruction.
 */
bool bf_program_append(struct bf_program *program, struct bf_instruction instruction)
{
    if (program->size >= program->capacity) {
        if (!bf_program_grow(program)) {
            goto error1;
        }
    }

    program->ir[program->size] = instruction;
    program->size++;

    return true;

error1:
    return false;
}

void bf_program_dump(struct bf_program *program)
{
    struct bf_instruction *instr;

    for (int i = 0; i < program->size; i++) {
        instr = &program->ir[i];
        printf("(0x%08x) %-9s -> 0x%08x\n", i, bf_program_map_ins_name(instr->opcode), instr->argument);
    }
}

char *bf_program_map_ins_name(enum bf_opcode opcode)
{
    switch (opcode) {
    case BF_INS_NOP:
        return "NOP";
    case BF_INS_IN:
        return "IN";
    case BF_INS_OUT:
        return "OUT";
    case BF_INS_INC_V:
        return "INC_V";
    case BF_INS_DEC_V:
        return "DEC_V";
    case BF_INS_ADD_V:
        return "ADD_V";
    case BF_INS_SUB_V:
        return "SUB_V";
    case BF_INS_INC_P:
        return "INC_P";
    case BF_INS_DEC_P:
        return "DEC_P";
    case BF_INS_ADD_P:
        return "ADD_P";
    case BF_INS_SUB_P:
        return "SUB_P";
    case BF_INS_BRANCH_Z:
        return "BRANCH_Z";
    case BF_INS_BRANCH_NZ:
        return "BRANCH_NZ";
    case BF_INS_JMP:
        return "JMP";
    case BF_INS_HALT:
        return "HALT";
    default:
        return "?";
    }
}
