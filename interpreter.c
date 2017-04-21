// Copyright (c) 2017 Walter Kuppens
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <stdio.h>

#include "interpreter.h"

bf_vm *bf_create_vm(char *src)
{
    bf_vm *vm = calloc(1, sizeof(bf_vm));
    if (!vm) goto fail;

    if (src) {
        vm->src = src;
    } else {
        goto fail;
    }

    return vm;

    fail:
        free(vm);
        free(src);
        return NULL;
}

void bf_destroy_vm(bf_vm *vm)
{
    free(vm->src);
    free(vm);
}

void bf_check_overread(bf_vm *vm)
{
    if (vm->pointer >= MEMORY_SIZE - 1) {
        vm->pointer = 0;
    }
    if (vm->pc >= MEMORY_SIZE - 1) {
        vm->pc = 0;
    }
}

void bf_goto_opening(bf_vm *vm)
{
    // Exit early if we are at the beginning of the tape (can't go back).
    if (vm->pc == 0) return;

    int ch;        // Current opcode being read from program memory.
    int depth = 0; // Brackets need to match in brainfuck, no simple searches.
    size_t i = vm->pc - 1;

    while (1) {
        ch = vm->src[i];
        if (ch == ']') {
            depth++;
        } else if (ch == '[') {
            if (depth == 0) {
                vm->pc = i;
                break;
            } else {
                depth--;
            }
        }

        if (i == 0) break; // Exit if at tape start to avoid overflow.
        i--;
    }
}

void bf_goto_closing(bf_vm *vm)
{
    int ch;        // Current opcode being read from program memory.
    int depth = 0; // Brackets need to match in brainfuck, no simple searches.
    size_t i = vm->pc + 1;

    while (1) {
        ch = vm->src[i];
        if (ch == '\0') {
            vm->pc = i;
            break;
        }
        if (ch == '[') {
            depth++;
        } else if (ch == ']') {
            if (depth == 0) {
                vm->pc = ++i;
                break;
            } else {
                depth--;
            }
        }
        i++;
    }
}

int bf_run(bf_vm *vm)
{
    int ch; // Holder for opcodes being read from the brainfuck.

    while ((ch = vm->src[vm->pc]) != '\0') {
        switch (ch) {
        case '>':
            vm->pointer++;
            vm->pc++;
            break;
        case '<':
            vm->pointer--;
            vm->pc++;
            break;
        case '+':
            vm->memory[vm->pointer]++;
            vm->pc++;
            break;
        case '-':
            vm->memory[vm->pointer]--;
            vm->pc++;
            break;
        case '.':
            putchar(vm->memory[vm->pointer]);
            vm->pc++;
            break;
        case ',':
            vm->memory[vm->pointer] = 0; // I don't believe in input.
            vm->pc++;
            break;
        case '[':
            if (!vm->memory[vm->pointer]) {
                bf_goto_closing(vm);
            } else {
                vm->pc++;
            }
            break;
        case ']':
            if (vm->memory[vm->pointer]) {
                bf_goto_opening(vm);
            } else {
                vm->pc++;
            }
            break;
        default:
            vm->pc++;
            break;
        }

        bf_check_overread(vm);
    }

    return 0; // TODO: Return operation count.
}
