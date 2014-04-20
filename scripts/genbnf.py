#!/usr/bin/env python3

"""
Prints the BNF-like comments from parser.cc which start with //>
"""

import os

def print_bnf(f):
	for line in f:
		s_line = line.strip()
		if s_line.startswith('//> '):
			print(line.rstrip()[4:])
		elif s_line.startswith('//>'):
			print(line.rstrip()[3:])

if __name__ == "__main__":
	import sys
	base_dir = os.path.dirname(os.path.dirname(__file__))
	src_file = os.path.join(base_dir, "soda", "parser.cc")
	with open(src_file) as f:
		print_bnf(f)
