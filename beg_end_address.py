# This script generates the begining and the end address of each basic block
# Inputs : Program assembly  file, And the size of each basic block

import sys
import os
from generate_addrs import generate_addrs
def main():

    generate_addrs()
    input_asm_file = open("../outputs/a.s","r")
    so_far_addrs = open("bb_size.txt","r")
    output_asm_file = open("../outputs/a_o.s","w")
    nbbs_array_file = open("nbbsArrayAddr.txt","r")
    nbbs_array_addr = int(nbbs_array_file.readline(),16)
    nbbs_array_addr = nbbs_array_addr + 8
    nbbs_array_file.close()
    find_loc = 0
    first_elm = 0
    var_i = 1;
    for line in input_asm_file:
        if (line.find('nbbs:') != -1) :
            find_loc = 1
            output_asm_file.write(line)
        elif (find_loc == 1 and line.find('.short') == -1):
            find_loc = 0
            output_asm_file.write(line)
        elif (find_loc) :
            if (first_elm == 0) :
                first_elm = 1
                output_asm_file.write(line)
            else :
                if (var_i == 3) :
                    var_i = 1
                    output_asm_file.write('\t.short  ' + str(nbbs_array_addr) + "\n")
                    nbbs_array_addr = nbbs_array_addr + 6
                else :
                    var_i = var_i + 1
                    addr = so_far_addrs.readline()
                    if addr == "" :
                        output_asm_file.write(line)
                    else:
                        output_asm_file.write('\t.short  ' + str(int(addr,16)) + "\n")
        else:
            output_asm_file.write(line)
    input_asm_file.close()
    output_asm_file.close()

if __name__ == "__main__":
    main()

