#!/usr/bin/env python
#
# Takes a directory of files and zips them up (as uncompressed files).
# This then gets converted into a C data structure which can be read
# like a filesystem at runtime.
#
# This is somewhat like frozen modules in python, but allows arbitrary files
# to be used.
#
# This file is part of the MicroPython project, http://micropython.org/
#
# The MIT License (MIT)
#
# Copyright (c) 2013-2019 Damien P. George, and others
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
# of the Software, and to permit persons to whom the Software is furnished to do
# so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

from __future__ import print_function

import argparse
import os
import subprocess
import sys
import types
from zipfile import ZipFile

def create_zip(zip_filename, zip_dir):
    abs_zip_filename = os.path.abspath(zip_filename)
    save_cwd = os.getcwd()
    os.chdir(zip_dir)
    if os.path.exists(abs_zip_filename):
        os.remove(abs_zip_filename)
    with ZipFile(abs_zip_filename, 'w') as zipObj:
        # Iterate over all the files in directory
        for folderName, subfolders, filenames in os.walk('.'):
            for filename in filenames:
                #create complete filepath of file in directory
                filePath = os.path.join(folderName, filename)
                # Add file to zip
                zipObj.write(filePath)

    os.chdir(save_cwd)

def create_c_from_file(c_filename, zip_filename):
    with open(zip_filename, 'rb') as zip_file:
        with open(c_filename, 'wt') as c_file:
            print('#include <stdint.h>', file=c_file)
            print('', file=c_file)
            print('const uint8_t memzip_data[] = {', file=c_file)
            while True:
                buf = zip_file.read(16)
                if not buf:
                    break
                print('   ', end='', file=c_file)
                for byte in buf:
                    if type(byte) is str:
                        print(' 0x{:02x},'.format(ord(byte)), end='', file=c_file)
                    else:
                        print(' 0x{:02x},'.format(byte), end='', file=c_file)
                print('', file=c_file)
            print('};', file=c_file)

def main():
    parser = argparse.ArgumentParser(
        prog='make-memzip.py',
        usage='%(prog)s [options] [command]',
        description='Generates a C source memzip file.'
    )
    parser.add_argument(
        '-z', '--zip-file',
        dest='zip_filename',
        help='Specifies the name of the created zip file.',
        default='memzip_files.zip'
    )
    parser.add_argument(
        '-c', '--c-file',
        dest='c_filename',
        help='Specifies the name of the created C source file.',
        default='memzip_files.c'
    )
    parser.add_argument(
        dest='source_dir',
        default='memzip_files'
    )
    args = parser.parse_args(sys.argv[1:])

    print('args.zip_filename =', args.zip_filename)
    print('args.c_filename =', args.c_filename)
    print('args.source_dir =', args.source_dir)

    create_zip(args.zip_filename, args.source_dir)
    create_c_from_file(args.c_filename, args.zip_filename)

if __name__ == "__main__":
    main()
