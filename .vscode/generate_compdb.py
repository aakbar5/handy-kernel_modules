# Change log:
#   - Updated script to allow the user to specify directory or directories
#     to parse. Current directory will be used if no pararmeter is specificed.
#     usage command: python generate_compdb.py /path/to/dir1 /path/to/dir2

from __future__ import print_function, division

import fnmatch
import json
import math
import multiprocessing
import os
import re
import sys


CMD_VAR_RE = re.compile(r'^\s*cmd_(\S+)\s*:=\s*(.+)\s*$', re.MULTILINE)
SOURCE_VAR_RE = re.compile(r'^\s*source_(\S+)\s*:=\s*(.+)\s*$', re.MULTILINE)

directory = ""

def print_progress_bar(progress):
    progress_bar = '[' + '|' * int(50 * progress) + '-' * int(50 * (1.0 - progress)) + ']'
    print('\r', progress_bar, "{0:.1%}".format(progress), end='\r', file=sys.stderr)


def parse_cmd_file(cmdfile_path):
    with open(cmdfile_path, 'r') as cmdfile:
        cmdfile_content = cmdfile.read()

    commands = { match.group(1): match.group(2) for match in CMD_VAR_RE.finditer(cmdfile_content) }
    sources = { match.group(1): match.group(2) for match in SOURCE_VAR_RE.finditer(cmdfile_content) }

    return [{
            'directory': directory,
            'command': commands[o_file_name],
            'file': source,
            'output': o_file_name
        } for o_file_name, source in sources.items()]


def main(list_of_dirs):
    print("Building *.o.cmd file list...", file=sys.stderr)

    compdb = []
    for curr_dir in list_of_dirs:
        global directory
        directory = curr_dir
        print("\nDir # {}\r".format(directory), file=sys.stderr)

        cmd_files = []
        for cur_dir, subdir, files in os.walk(directory):
            cmd_files.extend(os.path.join(cur_dir, cmdfile_name) for cmdfile_name in fnmatch.filter(files, '*.o.cmd'))

        print("Parsing *.o.cmd files...\r", file=sys.stderr)
        if len(cmd_files) == 0:
            print(" -- Skipping no object file is found\r", file=sys.stderr)
        else:
            n_processed = 0
            print_progress_bar(0)
        
            pool = multiprocessing.Pool()
            try:
                for compdb_chunk in pool.imap_unordered(parse_cmd_file, cmd_files, chunksize=int(math.sqrt(len(cmd_files)))):
                    compdb.extend(compdb_chunk)
                    n_processed += 1
                    print_progress_bar(n_processed / len(cmd_files))

            finally:
                pool.terminate()
                pool.join()

    print(file=sys.stderr)
    print("\nWriting compile_commands.json...", file=sys.stderr)
    with open('compile_commands.json', 'w') as compdb_file:
        json.dump(compdb, compdb_file, indent=1)


if __name__ == '__main__':

    if len(sys.argv) > 1:
        list_of_dirs = sys.argv[1:]
    else:
        list_of_dirs = [os.path.abspath(os.getcwd())]

    main(list_of_dirs)
