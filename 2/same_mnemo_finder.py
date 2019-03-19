from capstone import *
import binascii
import struct

COUNT_OF_SAME_MNEMO = 2

arr = {}
md = Cs(CS_ARCH_X86, CS_MODE_16)
for j in range(0, 65535):
	for i in md.disasm(struct.pack(">H", j), 0x1000):
		full_mnemo = i.mnemonic + " " + i.op_str
		if full_mnemo in arr:
			arr[full_mnemo].append(struct.pack(">H", j))
		else:
			arr[full_mnemo] = [struct.pack(">H", j)]

for opcode in arr.keys():
	if len(arr.get(opcode)) == COUNT_OF_SAME_MNEMO :
		print ("Opcode:" + opcode + "\t values:" + str(arr.get(opcode)))
