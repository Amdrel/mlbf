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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Allocation size used when reading brainfuck from stdin.
#define STDIN_ALLOC_SIZE 64

// Amount of memory allocated by the brainfuck vm.
#define MEMORY_SIZE 30000

/**
 * The virtual machine does not need to hold very much state. Brainfuck uses a
 * pointer that points to a place in memory which is statically allocated.
 */
typedef struct bf_vm_t {
    size_t pc;
    size_t pointer;
    int8_t memory[MEMORY_SIZE];
    char *src;
} bf_vm;

/**
 * Initializes a brainfuck virtual machine. This function requires that
 * brainfuck source code be passed which will be interpreted later. The src
 * parameter is owned and managed by the virtual machine and should not be used
 * directly.
 */
bf_vm *create_bf_vm(char *src)
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

/**
 * Frees resources contained in a brainfuck virtual machine such as the main
 * memory and brainfuck source code.
 */
void destroy_bf_vm(bf_vm *vm)
{
    free(vm->src);
    free(vm);
}

/**
 * Prevent buffer over-read after pointer increments.
 */
void bf_check_overread(bf_vm *vm)
{
    if (vm->pointer >= MEMORY_SIZE - 1) {
        vm->pointer = 0;
    }
    if (vm->pc >= MEMORY_SIZE - 1) {
        vm->pc = 0;
    }
}

/**
 * Scans for the last "[" and sets the pc to the opcode after.
 */
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
            if (depth <= 0) {
                vm->pc = i;
                break;
            } else {
                depth--;
            }
        }

        // Give up if we reach the start of the tape without finding a valid
        // matching bracket (prevent over-read).
        if (i == 0) {
            break;
        }
        i--;
    }
}

/**
 * Scans for the next "]" and sets the pc to the opcode after.
 */
void bf_goto_closing(bf_vm *vm)
{
    // Exit early if the current opcode is a null terminator. Code that follows
    // this check assumes the next opcode exists.
    if (vm->src[vm->pc] == '\0') return;

    int ch;        // Current opcode being read from program memory.
    int depth = 0; // Brackets need to match in brainfuck, no simple searches.
    size_t i = vm->pc + 1;

    while (1) {
        ch = vm->src[i];

        // Give up and set an arbitrary pc if we reach the end of the tape
        // before finding a valid maching bracket (prevent over-read).
        if (ch == '\0') {
            vm->pc = 0;
            break;
        }

        if (ch == '[') {
            depth++;
        } else if (ch == ']') {
            if (depth <= 0) {
                vm->pc = ++i;
                break;
            } else {
                depth--;
            }
        }
        i++;
    }
}

/**
 * Starts the execution loop to execute code on the passed virtual machine and
 * returns once there is no more input (EOF).
 */
void bf_run(bf_vm *vm)
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
}

/**
 * Reads a file and returns a string containing the contents. A size must be
 * specified which allows the caller to control allocation size.
 */
char *read_file(FILE *fp, size_t size)
{
    char *str;      // Will contain file contents when the function completes.
    int ch;         // Holder for last character read from the file.
    size_t len = 0; // Current length of the string (not allocated size).

    str = calloc(1, sizeof(char) * size);
    if (!str) goto fail;

    // Read bytes from the file copying them into the string. The string will
    // be dynamically reallocated as needed.
    while ((ch = getc(fp)) != EOF) {
        str[len++] = ch;

        if (len >= size) {
            size += STDIN_ALLOC_SIZE;
            char *nstr = realloc(str, sizeof(char) * size);
            if (!nstr) goto fail; // Failed realloc, cleanup.
            str = nstr;
        }
    }
    str[len++] = '\0';

    return str;

fail:
    free(str);
    return NULL;
}

int main(void)
{
    // Read brainfuck source code from stdin and initialize the virtual machine.
    char *src = read_file(stdin, STDIN_ALLOC_SIZE);
    bf_vm *vm = create_bf_vm(src);
    if (!vm) return 1;

    // Start executing brainfuck in the virtual machine.
    bf_run(vm);

    // Cleanup resources used by the virtual machine before quitting.
    destroy_bf_vm(vm);

    return 0;
}
