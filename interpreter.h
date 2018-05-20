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

#ifndef BF_INTERPRETER_H
#define BF_INTERPRETER_H

#include <stdint.h>
#include <stdlib.h>

/** Amount of memory allocated by the brainfuck vm. */
#define BF_MEMORY_SIZE 30000

/** The interpreter will output to a buffer rather than stdout if set. */
#define BF_OUTPUT_BUFFER 0x1

/**
 * The virtual machine does not need to hold very much state. Brainfuck uses a
 * pointer that points to a place in memory which is statically allocated.
 */
typedef struct bf_vm {
    uint32_t flags;
    size_t pc;
    size_t pointer;
    int8_t memory[BF_MEMORY_SIZE];
    char *src;
} bf_vm;

/**
 * This structure is used when the virtual machine finishes executing. A result
 * code, operation count, and output string are provided to the caller. Output
 * will only be set if specified in the flags.
 */
typedef struct bf_result {
    int result;
    unsigned long long opcount;
} bf_result;

/**
 * Initializes a brainfuck virtual machine. This function requires that
 * brainfuck source code be passed which will be interpreted later. The src
 * parameter is owned and managed by the virtual machine and should not be used
 * directly.
 */
bf_vm *bf_create_vm(char *src, uint32_t flags);

/**
 * Frees resources contained in a brainfuck virtual machine such as the main
 * memory and brainfuck source code.
 */
void bf_destroy_vm(bf_vm *vm);

/**
 * Prevent buffer over-read after pointer increments.
 */
void bf_check_overread(bf_vm *vm);

/**
 * Scans for the last matching "[" and sets the pc to the opcode after. This is
 * used for conditionals and loops in brainfuck.
 */
void bf_goto_opening(bf_vm *vm);

/**
 * Scans for the next matching "]" and sets the pc to the opcode after. This is
 * used for conditionals and loops in brainfuck.
 */
void bf_goto_closing(bf_vm *vm);

/**
 * Starts the execution loop to execute code on the passed virtual machine and
 * returns once there is no more input (EOF).
 */
bf_result bf_run(bf_vm *vm);

#endif
