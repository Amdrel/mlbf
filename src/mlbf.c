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

#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "compiler.h"
#include "interpreter.h"
#include "program.h"
#include "transpiler.h"
#include "utils.h"

const char *mlbf_version()
{
    return "1.0.0";
}

void mlbf_print_usage()
{
    fprintf(
        stderr,

        "Usage: mlbf [options] [script]\n"
        "\n"
        "If no script is supplied, stdin is read for source code.\n"
        "\n"
        "Options:\n"
        "  -h, --help     Print this usage message.\n"
        "  -v, --version  Print mlbf version (\"%s\").\n"
        "  -d, --dump     Dump compiled bytecode to stdout.\n"
        "  -o, --output   Dump C source code to the provided path.\n"
        "\n"
        "For reporting bugs / viewing source code, please see:\n"
        "<https://github.com/Reshurum/mlbf>\n",

        mlbf_version());
}

int main(int argc, char *argv[])
{
    int c;
    int option_index;
    FILE *fp;
    size_t alloc_size;
    char *src;
    struct bf_vm *vm;
    struct bf_program *program;

    // Command-line flags from getopt.
    char *output_path = NULL;
    int help_flag = 0;
    int version_flag = 0;
    int dump_flag = 0;

    const struct option long_options[] = {
        { "help", no_argument, &help_flag, 'h' },
        { "version", no_argument, &version_flag, 'v' },
        { "dump", no_argument, &dump_flag, 'd' },
        { "output", required_argument, NULL, 'o' },
        { NULL, 0, NULL, 0 },
    };

    opterr = 0;
    while ((c = getopt_long(argc, argv, "hvdo:", long_options, &option_index)) != -1) {
        switch (c) {
        case 0:
            break;
        case 'h':
            help_flag = 1;
            break;
        case 'v':
            version_flag = 1;
            break;
        case 'd':
            dump_flag = 1;
            break;
        case 'o':
            output_path = strdup(optarg);
            break;
        case '?':
            if (optopt == 'o') {
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
            } else if (isprint(optopt)) {
                fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                mlbf_print_usage();
            } else {
                mlbf_print_usage();
            }
            goto error1;
        default:
            goto error1;
        }
    }

    if (help_flag) {
        mlbf_print_usage();
        goto error1;
    } else if (version_flag) {
        printf("%s\n", mlbf_version());
        goto success1;
    }

    // Read the source code from a file if an argument is specified, otherwise
    // read the source code from stdin. Both options are available since meson
    // tests do not allow specifying input to stdin.
    if (optind < argc) {
        fp = fopen(argv[optind], "r");
        if (fp == NULL) {
            fprintf(stderr, "Unable to open file '%s'.\n", argv[optind]);
            goto error1;
        }
        alloc_size = FILE_ALLOC_SIZE;
    } else {
        fp = stdin;
        alloc_size = STDIN_ALLOC_SIZE;
    }

    // Read and compile the brainfuck source code.
    src = read_file(fp, alloc_size);
    if (src == NULL) {
        fprintf(stderr, "Unable to read source code.\n");
        goto error1;
    }
    if (fp != stdin) {
        fclose(fp);
    }
    program = bf_compile(src);
    if (!program) {
        fprintf(stderr, "Unable to compile source code.\n");
        goto error2;
    }

    if (dump_flag) {
        bf_program_dump(program);
        bf_program_destroy(program);
    } else if (output_path) {
        if (access(output_path, W_OK) != -1) {
            remove(output_path);
        }

        FILE *output_file = fopen(output_path, "w");
        if (output_file == NULL) {
            fprintf(stderr, "Unable to write compiled output to '%s'.", output_path);
            goto error2;
        }

        bf_transpile_program(program, output_file);
        bf_program_destroy(program);
        fclose(output_file);
    } else {
        // Read brainfuck source code from stdin and initialize the virtual
        // machine. TODO: Add a compilation before this call once the bytecode
        // is defined.
        vm = bf_vm_create(program, 0);
        if (!vm) {
            fprintf(stderr, "Unable to initialize vm.\n");
            goto error2;
        }

        // Start executing brainfuck in the virtual machine. Cleanup resources
        // used by the virtual machine before quitting and after bf_vm_run
        // returns (program finished running).
        bf_vm_run(vm);
        bf_vm_destroy(vm);
    }

    free(src);
success1:
    if (output_path) {
        free(output_path);
    }
    return 0;

error2:
    free(src);
error1:
    if (output_path) {
        free(output_path);
    }
    return 1;
}
