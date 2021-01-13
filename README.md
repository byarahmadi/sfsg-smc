 sfsg-smc is a Dynamic compiler based on Self-Modifying Code that instrument checkpoint trigger call based on the power failure at runtime
 
 To run & test :
 
 make sure you have llvm tool chain in your system.
 
 3 MSP30fr5969 is needed
 The pins in each MSP430 must be set based on exe_time.c, reset_gen.c, begin_task.c and end_task.c
 
 To compile llvm Passes : 
 ```
$ ./compile_llvm_pass.sh
```
You can also compile them within llvm source directory https://llvm.org/docs/WritingAnLLVMPass.html#setting-up-the-build-environment
 
 To run the test benchmark :
 ```
$ ./benchmarks.sh
```
 
