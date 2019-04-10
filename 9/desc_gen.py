import sys
import argparse
from ctypes import *
from struct import *

#TODO add type struct to input just word (code/stack), not number

class Descriptor(Structure):
    _pack_ = 1
    _fields_ = [("limit", c_uint, 16),
            ("base_address", c_ulong , 24),
            ("type", c_uint, 4),
            ("storage", c_uint, 1),
            ("dpl", c_uint, 2),
            ("present", c_uint, 1),
            ("limit_high", c_uint, 4),
            ("avl", c_uint, 1),
            ("capacity", c_uint, 1),
            ("default_size", c_uint, 1),
            ("granularity", c_uint, 1),
            ("base_address_high", c_uint, 8)]
    
    def __str__(self):
        out = ""
        for field in self._fields_:
            out += field[0]
            out += ':'
            out += hex(getattr(self, field[0]))
            out += '\n' 
        return out
    
    def print_format(self, mode):
        if mode == 'b':
            print("db", end = ' ')
            for byte in bytearray(self):
                print(bin(byte), end = ",")
            print("")
        elif mode == 'h':
            print("db", end = ' ')
            for byte in bytearray(self):
                print(hex(byte), end = ",")
            print("")
        elif mode == 'w':
            w1,w2,w3,w4 = unpack('HHHH', bytes(self))
            print("dw " + '{:04x}'.format((w1)))
            print("dw " + '{:04x}'.format((w2)))
            print("dw " + '{:04x}'.format((w3)))
            print("dw " + '{:04x}'.format((w4)))
            
def create_parser():
    parser = argparse.ArgumentParser()
    parser.add_argument('-a', '--addr', default = 0x7c00, help = "set address of segment", type = lambda x: int(x,0))
    parser.add_argument('-l', '--limit', default = 0x200, help = "set maximum offset in segment", type = lambda x: int(x,0))
    parser.add_argument('-r', '--ring', default = 0, help = "set ring of segment",  type = int)
    parser.add_argument('-m', '--mode', default = 'w', help = "set output format as binary (b), hex(h), or word(w)", type = str)
    parser.add_argument('-t', '--type', default = 0b0110, help = "set type of segment (data/stack/code), i.e. b0110 - for stack, or 0b1010 - for code", type = lambda x: int(x, 0))
    return parser

def main():
    parser = create_parser()
    namespace = parser.parse_args(sys.argv[1:])
    if (len(sys.argv[1:]) == 0):
        print("Using default values...")
    
    desc = Descriptor()
    desc.limit = namespace.limit & 0xFFFF
    desc.base_address =  (namespace.addr * 0x10)  & 0xFFFFFF 
    desc.accessed = 0
    if namespace.type > 15 or namespace.type < 0:
        desc.type = 0b0110
    else:
        desc.type = namespace.type
    desc.storage = 1
    if namespace.ring > 3 or namespace.ring < 0:
        desc.dpl = 0
    else:
        desc.dpl = namespace.ring
    desc.present = 1
    desc.limit_high = (namespace.limit >> 0x10) & 0xF
    desc.avl = 0
    desc.capacity = 0
    desc.default_size = 1
    desc.granularity = 0 #use bytes, not pages
    desc.base_address_high = ((namespace.addr * 0x10) >> 0x18) & 0xFF
    
    desc.print_format(namespace.mode)
    print("")
    print(str(desc))

if __name__ == '__main__':
    main()
