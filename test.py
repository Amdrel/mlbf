#!/usr/bin/env python3

# Copyright (c) 2017 Walter Kuppens
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import glob
import os
import subprocess

MLBF_PATH = './builddir/mlbf'
MLBF_TEST_DIR = './tests'


def test_mlbf():
    """Executes all brainfuck test scripts and tests their output."""

    if not mlbf_exists():
        raise RuntimeError("mlbf hasn't been compiled yet.")

    scripts = glob.glob(os.path.join(MLBF_TEST_DIR, '*.b'))

    for script in scripts:
        test_script(*generate_script_names(script))


def generate_script_names(fpath):
    filename, extension = os.path.splitext(fpath)

    if extension != '.b':
        raise ValueError('Only pass brainfuck source code to this function.')

    return (
        '{}.b'.format(filename),
        '{}.b.in'.format(filename),
        '{}.b.out'.format(filename),
    )


def mlbf_exists():
    """Ensures mlbf has been compiled and is available."""

    return os.path.isfile(MLBF_PATH)


def test_script(source, input, output):
    """Executes a brainfuck script and tests the output."""

    if not os.path.isfile(source):
        raise RuntimeError("Source file '{}' doesn't exist.".format(source))

    stdin = subprocess.DEVNULL

    try:
        if os.path.isfile(input):
            stdin = open(input, 'rb')

        pipe = subprocess.Popen(
            [MLBF_PATH, source],
            stdin=stdin,
            stdout=subprocess.PIPE)
        data = pipe.communicate()[0]

        if pipe.returncode != 0:
            raise RuntimeError("Got non-zero exit code ({}) from mlbf.".format(pipe.returncode))

        if os.path.isfile(output):
            with open(output, 'rb') as f:
                expected_data = f.read()
                if expected_data != data:
                    raise RuntimeError("Expected -> {}, Got -> {}".format(
                        expected_data, data))
    finally:
        if stdin != subprocess.DEVNULL:
            stdin.close()


if __name__ == '__main__':
    test_mlbf()
