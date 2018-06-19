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

#ifndef BF_ERRORS_H
#define BF_ERRORS_H

#define BF_RESULT_SUCCESS 0
#define BF_RESULT_ERROR 1

/*
    NOTE: No macros for automatic return on bf_result failure exist since they
    may be misused when memory needs to be cleaned up and is forgotton about.

    if result.code != 0 {
        if result.message != null {
            printf(result.message);
        }
        // Do cleanup if needed...
    }
 */

/**
 * Success or error result from a function. This is used throughout the code
 * where error handling is needed.
 *
 * The 'code' field holds an integer that can be checked to check for success
 * or failure of a function. Any integer that's not a zero is an error. An
 * optional c string error message may also be set, but isn't always available.
 */
struct bf_result {
    int code;
    char *message;
};

#endif
