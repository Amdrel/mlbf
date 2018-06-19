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
#include <stdlib.h>

#include "interpreter.h"

/** Allocation size used when reading brainfuck from stdin. */
#define STDIN_ALLOC_SIZE 64
#define FILE_ALLOC_SIZE 64

/**
 * Reads a file and returns a string containing the contents. A size must be
 * specified which allows the caller to control allocation size.
 */
char *read_file(FILE *fp, size_t size)
{
    char *str; // Will contain file contents when the function completes.
    int ch; // Holder for last character read from the file.
    size_t len = 0; // Current length of the string (not allocated size).

    str = calloc(1, sizeof(char) * size);
    if (!str) {
        goto error1;
    }

    // Read bytes from the file copying them into the string. The string will
    // be dynamically reallocated as needed.
    while ((ch = getc(fp)) != EOF && ch != '|') {
        str[len++] = ch;

        if (len >= size) {
            size += STDIN_ALLOC_SIZE;
            char *nstr = realloc(str, sizeof(char) * size);
            if (!nstr) {
                goto error1; // Failed realloc, cleanup.
            }
            str = nstr;
        }
    }
    str[len++] = '\0';

    return str;

error1:
    free(str);
    return NULL;
}

int main(int argc, char *argv[])
{
    FILE *fp;
    size_t alloc_size;
    char *src;

    // Read the source code from a file if an argument is specified, otherwise
    // read the source code from stdin. Both options are available since meson
    // tests do not allow specifying input to stdin.
    if (argc >= 2) {
        fp = fopen(argv[1], "r");
        if (fp == NULL) {
            fprintf(stderr, "Unable to open file '%s'.\n", argv[1]);
            return 1;
        }
        alloc_size = FILE_ALLOC_SIZE;
    } else {
        fp = stdin;
        alloc_size = STDIN_ALLOC_SIZE;
    }

    src = read_file(fp, alloc_size);
    if (src == NULL) {
        fprintf(stderr, "Unable to source code.\n");
        return 1;
    }
    if (fp != stdin) {
        fclose(fp);
    }

    // Read brainfuck source code from stdin and initialize the virtual machine.
    // TODO: Add a compilation before this call once the bytecode is defined.
    struct bf_vm *vm = bf_vm_create(src, 0);
    if (!vm) {
        return 1;
    }

    // Start executing brainfuck in the virtual machine. Cleanup resources used
    // by the virtual machine before quitting and after bf_vm_run returns (program
    // finished running).
    bf_vm_run(vm);
    bf_vm_destroy(vm);

    return 0;
}
