import sys
import os
import subprocess
def generate_addrs():
    subprocess.call("./bb_sort.sh")
    so_far_addrs = "so_far_addrs.txt"
    fp_output = open("bb_size.txt","w")
    with open(so_far_addrs) as addrs:
        for sp_addr in addrs:
            objdump_filepath = "../outputs/a.objdump"
            addr=sp_addr.split("\n")
            find=0
            with open(objdump_filepath) as lines:
                for line in lines:
                    if (line.find(addr[0])!= -1 and find==0):
                        find=1
                    if (find==1 and (line.find("jmp")!=-1 or line.find("ret")!=-1)):
                        find=0;
                        end_addr=line.split(':')
                        final_end_addr=(end_addr[0].split(' '))
                        final_begining_addr=addr[0].split(':')
                        fp_output.write("%s\n" % final_begining_addr[0])
                        fp_output.write("%s\n" % final_end_addr[4])

                lines.close()
if __name__ == "__main__":
    main()

