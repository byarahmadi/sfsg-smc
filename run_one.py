import sys
import os
import subprocess
import time
import re

# TO RUN : python run_one.py #interval name

def compile_cfiles():
    if (int(sys.argv[1]) > 65500):
        interval = 65500
        itr = int(sys.argv[1]) /  65500
        mode = int(sys.argv[1]) % 65500
    else:
        interval =int(sys.argv[1])
        itr = 0
        mode = 0
    print interval , mode, itr    
     
    subprocess.call(['msp430-elf-gcc','-I','../msp430-gcc/include','-L','../msp430-gcc/include','-mmcu=msp430fr5969','-D__MSP430FR5969__','exe_time.c','-o','../outputs/exe_time.out'])
    subprocess.call(['msp430-elf-gcc','-I','../msp430-gcc/include','-L','../msp430-gcc/include','-mmcu=msp430fr5969','-D__MSP430FR5969__','-O3','-DINTERVAL='+str(interval),'-DMODE='+str(mode),'-DITR='+str(itr),'reset_gen.c','-o','../outputs/reset_gen.out'])
    print "The CFiles are compiled"
# The parameters of the main are 
# argv[1] is the time interval
# argv[2] is the output file to write to (**Benchmark name**)
def main():
    reset_gen_tty = 'ttyACM2' #The terminal for the reset generator board
    main_tty = 'ttyACM0'      #The terminal for the main board
    exe_time_tty = 'ttyACM4'      #The termianl for the execution time board
    
    compile_cfiles()

    p_reset_gen = subprocess.Popen(['mspdebug tilib -d ' + reset_gen_tty],shell = True,
           stdin=subprocess.PIPE,
           stdout=subprocess.PIPE,
           stderr=subprocess.PIPE)
    p_reset_gen.stdin.write("prog ../outputs/reset_gen.out\n")
    p_reset_gen.stdin.write("setbreak exit\n");
  #  p_reset_gen.stdin.write("run")
    p_exe_time = subprocess.Popen(['mspdebug tilib -d ' + exe_time_tty],shell = True,
           stdin=subprocess.PIPE,
           stdout=subprocess.PIPE,
           stderr=subprocess.PIPE)
    p_main = subprocess.Popen(['mspdebug tilib -d ' + main_tty],shell = True,
           stdin=subprocess.PIPE,
           stdout=subprocess.PIPE,
           stderr=subprocess.PIPE)
    p_main.stdin.write("prog ../outputs/a.out\n")
   

    p_exe_time.stdin.write("prog ../outputs/exe_time.out\n")
    p_exe_time.stdin.write("exit\n")
    p_exe_time.stdin.close()
    p_exe_time.wait()
    
    p_reset_gen.stdin.write("run")
    p_reset_gen.stdin.close()
    time.sleep(5)



    p_main.stdin.write("exit\n")



    p_main.stdin.close()
    p_main.wait()
#    p_reset_gen.stdin.close()
   # p_reset_gen.communicate()
    i = 0
    status="OK"
    while p_reset_gen.poll() is None:
        i=i+1
        time.sleep(6)
        if (i == 60):
            p_reset_gen.terminate()
            p_reset_gen = subprocess.Popen(['mspdebug tilib -d ' + reset_gen_tty],shell = True,
             stdin=subprocess.PIPE,
             stdout=subprocess.PIPE,
             stderr=subprocess.PIPE)
            p_reset_gen.stdin.write("prog ../outputs/a.out\n")
            p_reset_gen.communicate()
            p_reset_gen.wait()
            status="ERROR"
            break
    p_exe_time = subprocess.Popen(['mspdebug tilib -d ' + exe_time_tty ],shell = True,
            stdout=subprocess.PIPE,
            stdin=subprocess.PIPE,
            stderr=subprocess.PIPE)
    p_exe_time.stdin.write("md 0xE760")
    p_exe_time.stdin.close()
    p_exe_time.wait()
    
    
       
    output = p_exe_time.stdout.read()
    print output
    splited_tokens = output.split("0e760")
    numbers = splited_tokens[1].split()
    print "Timer=",numbers[4],numbers[3]
    print "Interval=",numbers[2],numbers[1]
    execution_time = (int(numbers[2]+numbers[1],16) * 65535 + int(numbers[4]+numbers[3],16)) * 32.5 / 1000000
    print "Execution Time in Hex is :", execution_time

    p_main = subprocess.Popen(['mspdebug tilib -d ' + main_tty],shell = True,
           stdin=subprocess.PIPE,
           stdout=subprocess.PIPE,
           stderr=subprocess.PIPE)
    p_main.stdin.write("md 0xE760")
    p_main.stdin.close()
    p_main.wait()
    output = p_main.stdout.read()
 #   print output
    splited_tokens = output.split("0e760")
    numbers = splited_tokens[1].split()
    completed_checkpoints = int(numbers[2]+numbers[1],16)
    uncompleted_checkpoints = int(numbers[4] + numbers[3],16)
    unreachable_checkpoints = int (numbers[6] + numbers[5],16)
    nesting = int(numbers[8] + numbers[7],16)
    special_err = int (numbers[10] + numbers[9],16)
    print "Completed checkpoints=", completed_checkpoints," uncomplited checkpoints=",uncompleted_checkpoints," unreachable checkpoints=",unreachable_checkpoints," nesting=",nesting," special_err=",special_err

    p_main = subprocess.Popen(['mspdebug tilib -d ' + main_tty],shell = True,
           stdin=subprocess.PIPE,
           stdout=subprocess.PIPE,
           stderr=subprocess.PIPE)
    p_main.stdin.write("md 0xd002 200")
    p_main.stdin.close()
    p_main.wait()
    output = p_main.stdout.read()
    splited_tokens = output.split("0d002")


    numbers =  re.findall('[0-9a-fA-F]{2}\s',splited_tokens[1])
    print numbers
    static_checkpoints = []
    f=open(sys.argv[2] +'_checkpoint_locations'+'.txt','a+')
    f.write('\n'+sys.argv[1]+'\n')

    n_checkpoints = int(numbers[1].rstrip() + numbers[0].rstrip(),16) 
    print "The number of checkpoints is :",n_checkpoints
    for x in range(n_checkpoints):
        static_checkpoints.append(numbers[x * 8 + 3].rstrip()+numbers[x * 8 + 2].rstrip())
        static_checkpoints.append(numbers[x * 8 + 5].rstrip()+numbers[x * 8 + 4].rstrip())
        static_checkpoints.append(numbers[x * 8 + 7].rstrip()+numbers[x * 8 + 6].rstrip())
        static_checkpoints.append(numbers[x * 8 + 9].rstrip()+numbers[x * 8 + 8].rstrip())
        f.write('\n'+static_checkpoints[x * 4] + " " + static_checkpoints[x * 4 + 1] + " " + static_checkpoints[x * 4 + 2] + " " + static_checkpoints[x * 4 + 3])
    f.close()
    f=open(sys.argv[2] + '.txt','a+')
    f.write("\n"+sys.argv[1] +" " + str(execution_time)+" "+str(completed_checkpoints)+" "+str(uncompleted_checkpoints)+" "+str(unreachable_checkpoints)+" " +str(nesting)+" "+status )
    f.close()

    p_main = subprocess.Popen(['mspdebug tilib -d ' + main_tty],shell = True,
           stdin=subprocess.PIPE,
           stdout=subprocess.PIPE,
           stderr=subprocess.PIPE)
    p_main.stdin.write("md 0xD000 100")
    p_main.stdin.close()
    p_main.wait()
    output = p_main.stdout.read()
    splited_data = output.split('0d000')
    numbers = re.findall('[0-9a-fA-F]{2}\s',splited_data[1])
    print numbers
    failure_times = []
    f=open(sys.argv[2] +'_failure_times'+'.txt','a+')
    f.write('\n'+sys.argv[1]+'\n')
    n_failures = int(numbers[5].rstrip()+numbers[4].rstrip(),16) / 2
    
    if (n_failures <= 30):
        f.write('\n' + str(int(numbers[3].rstrip()+numbers[2].rstrip(),16)))
        for x in range (n_failures-1):
            failure_times.append(int(numbers[x * 2 + 7].rstrip() + numbers[x * 2 + 6].rstrip(),16))
            print failure_times[x]
            f.write('\n'+str(failure_times[x]))
    print "The numbers of failures is :",n_failures

if __name__ == "__main__":
    main()
