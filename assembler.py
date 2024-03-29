#!/usr/bin/env python3

import json, struct
import sys, os

path = sys.argv[1]
ENTRY = 65535
INTVEC = 65534

with open (path, "r") as source:
	lines = source.read().split('\n')

with open (f"{os.getcwd()}/machineCode.json") as file:
	machineCode = json.load(file)

def strtohex(str):
	k = lambda x: ord(x) - 87
	integer = 0
	for half in str:
		integer = integer << 4
		try:
			num = int(half)
		except:
			num = k(half)
		integer = integer | num

	return integer


def getValue(value):
	tmp = 0
	if value.startswith("$"):  #   $ >> Variable
		tmp = int(var[value[1:]+':'])
	elif value.startswith("%"): #  % >> Hex
		tmp = strtohex(value[1:])
	elif value.startswith("#"): #  # >> Immediate
		tmp = int(value[1:])
	else:
		print("Invalid data identifier")
		quit()
	return tmp


var = {}
addr = 0
offset = 0
for lineNum, line in enumerate(lines):
        line = line.strip()
        if line.startswith("!") or len(line) < 2:
            continue
        tokens = line.split(" ")
        
        if len(line) == 0 or line.startswith("!"):
                continue

        op = tokens[0]
        if ":" in op:
                var[op] = addr
        elif op == ".org":
                addr = getValue(tokens[1])
        elif op == ".word":
                addr += len(tokens[1])
        elif op == ".start" or "=" in op or op == ".intvec" or op.startswith("$"):
                pass
        else:
                addr += machineCode[op][1]


addr = 0
memory = [0 for i in range(0x10000)]
for line in lines:
	
	line = line.strip()

	if line.startswith("!") or len(line) < 2:
		continue
	tokens = line.split(" ")

	intLen = len(tokens)
	args = tokens[1:intLen]
	nextFilled = False

	if len(line) == 0 or line.startswith("!"):
		continue
	else:
		line.strip()
	op = tokens[0]
	if ":" in op:
		var[op] = addr
	elif op == ".org":
		addr = getValue(tokens[1])
	elif op == ".start":
		memory[ENTRY] = addr
	elif op.startswith("$"):
		memory[var[op[1:]+':']] = getValue(tokens[2])
	elif  op == ".intvec":
		memory[INTVEC] = addr
	else:
		word = machineCode[op][0]
		for i, arg in enumerate(args):
			if ("#" in arg) or ("$" in arg) or ("%" in arg):
				memory[addr+1] = getValue(arg)
			else:
				arg_ = machineCode[arg]
				if i == 0:
					word = word |  (arg_  << 8)
				else:
					word = word |  (arg_ << 12)
		memory[addr] = word
		addr += machineCode[op][1]



with open("rom.bin", "wb") as file:
	print("Flashing rom...")
	for word in memory:
		file.write(struct.pack("H", word))
print("Done")
