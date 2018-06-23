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
#include <string.h>

#include "interpreter.h"

struct bf_vm *bf_vm_create(struct bf_program *program, uint32_t vm_flags)
{
    struct bf_vm *vm = calloc(1, sizeof(struct bf_vm));
    if (!vm) {
        goto error1; // Failed allocation, cannot continue.
    }

    if (program && program->size > 0) {
        vm->program = program;
    } else {
        goto error2;
    }
    vm->pc = 0;
    vm->pointer = 0;
    vm->vm_flags = vm_flags;

    return vm;

error2:
    free(vm);
error1:
    bf_program_destroy(program);

    return NULL;
}

void bf_vm_destroy(struct bf_vm *vm)
{
    bf_program_destroy(vm->program);
    free(vm);
}

struct bf_result bf_vm_run(struct bf_vm *vm)
{
    struct bf_instruction *instr; // Owned and managed by vm.
    int input; // Buffered input from stdin.
    int pointer_holder;

    for (;;) {
        instr = &vm->program->ir[vm->pc];

        switch (instr->opcode) {
        case BF_INS_NOP:
            vm->pc++;
            break;
        case BF_INS_IN:
            if ((input = getchar()) != EOF) {
                vm->memory[vm->pointer] = input;
            }
            vm->pc++;
            break;
        case BF_INS_OUT:
            putchar(vm->memory[vm->pointer]);
            vm->pc++;
            break;
        case BF_INS_INC_V:
            vm->memory[vm->pointer]++;
            vm->pc++;
            break;
        case BF_INS_DEC_V:
            vm->memory[vm->pointer]--;
            vm->pc++;
            break;
        case BF_INS_ADD_V:
            vm->memory[vm->pointer] += instr->argument;
            vm->pc++;
            break;
        case BF_INS_SUB_V:
            vm->memory[vm->pointer] -= instr->argument;
            vm->pc++;
            break;
        case BF_INS_INC_P:
            // Bounds check to prevent buffer over-reads.
            if (vm->pointer < BF_MEMORY_SIZE) {
                vm->pointer++;
            }
            vm->pc++;
            break;
        case BF_INS_DEC_P:
            // Bounds check to prevent buffer under-reads.
            if (vm->pointer > 0) {
                vm->pointer--;
            }
            vm->pc++;
            break;
        case BF_INS_ADD_P:
            pointer_holder = vm->pointer + instr->argument;
            if (pointer_holder < BF_MEMORY_SIZE) {
                vm->pointer = pointer_holder;
            }
            vm->pc++;
            break;
        case BF_INS_SUB_P:
            pointer_holder = vm->pointer - instr->argument;
            if (pointer_holder >= 0) {
                vm->pointer = pointer_holder;
            }
            vm->pc++;
            break;
        case BF_INS_BRANCH_Z:
            if (vm->memory[vm->pointer] == 0) {
                vm->pc = instr->argument;
            } else {
                vm->pc++;
            }
            break;
        case BF_INS_BRANCH_NZ:
            if (vm->memory[vm->pointer] != 0) {
                vm->pc = instr->argument;
            } else {
                vm->pc++;
            }
            break;
        case BF_INS_JMP:
            vm->pc = instr->argument;
            break;
        case BF_INS_HALT:
            goto halt;
        case BF_INS_CLEAR:
            vm->memory[vm->pointer] = 0;
            vm->pc++;
            break;
        default:
            goto halt; // Failsafe for unrecognized opcodes.
        }
    }

halt:

    return (struct bf_result){
        .code = BF_RESULT_SUCCESS,
        .message = NULL,
    };
}
