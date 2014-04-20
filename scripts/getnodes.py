#!/usr/bin/env python3

import sys
import re
import os

RE_NODE = re.compile(r'^(class|struct)\s*(?P<name>[a-zA-Z_0-9]+)\s*\:\s*public\s*(?P<base>[a-zA-Z_0-9]+)')

def main(args):
	base_dir = os.path.dirname(os.path.dirname(__file__))
	src_file = os.path.join(base_dir, "soda", "ast.h")
	nodes = []
	with open(src_file) as f:
		for line in f:
			line = line.strip()
			match = RE_NODE.match(line)
			if match:
				name = match.group('name')
				base = match.group('base')
				nodes.append((name, base))
	for node in sorted(nodes):
		sys.stdout.write('{} ({})\n'.format(node[0], node[1]))
	return 0

if __name__ == "__main__": sys.exit(main(sys.argv))
