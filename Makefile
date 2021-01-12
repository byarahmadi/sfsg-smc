includes:=../msp430-gcc/include/
outdir:= ../outputs/
clang_pure:
	clang --target=msp430-elf  -D__MSP430__ -c $(bench_name).c -emit-llvm -I $(includes) $(OPTIMIZE_FLAG) -D$(DEFENISION) -I ../msp430-gcc/msp430-elf/include/ -D__MSP430FR5969__ -o  ../outputs/raw.bc
	clang --target=msp430-elf end_task.c -c -emit-llvm -D$(DEFENISION) -I ../msp430-gcc/include/ -I include/  -D__MSP430FR5969__ -o ../outputs/end_task.bc
	llvm-link ../outputs/raw.bc ../outputs/end_task.bc -o ../outputs/raw.bc
	clang --target=msp430-elf begin_task.c -c -emit-llvm -I ../msp430-gcc/include/ -I include/  -D__MSP430FR5969__ -o ../outputs/begin_task.bc
	llvm-link ../outputs/raw.bc ../outputs/begin_task.bc -o ../outputs/raw.bc
	clang --target=msp430-elf sense.c -c -emit-llvm -I ../msp430-gcc/include/ -I include/  -D__MSP430FR5969__ -o ../outputs/sense.bc
	llvm-link ../outputs/raw.bc ../outputs/sense.bc -o ../outputs/raw.bc
ifeq ($(DEFENISION) , VERIFY)
	clang --target=msp430-elf $(bench_name)_verify_benchmark.c -c -emit-llvm -I ../msp430-gcc/include/ -I include/  -D__MSP430FR5969__ -o ../outputs/verify_benchmark.bc
	llvm-link ../outputs/raw.bc ../outputs/verify_benchmark.bc -o ../outputs/raw.bc
endif
	clang ../outputs/raw.bc -mcpu=msp430 --target=msp430-elf -o ../outputs/raw.o -c
	msp430-elf-gcc ../outputs/raw.o  -mmcu=msp430fr5969 -L ../msp430-gcc/include/ -o../outputs/a.out -lm
	msp430-elf-objdump ../outputs/a.out -D > ../outputs/a.objdump
gcc:
	msp430-elf-gcc -I ../msp430-gcc/include -L ../msp430-gcc/include/ -mmcu=msp430fr5969 -D__MSP430FR5969__ $(bench_name).c  $(OPTIMIZE_FLAG) -D$(DEFENISION) -c  -o ../outputs/a.o -lm
	msp430-elf-gcc -I ../msp430-gcc/include -L ../msp430-gcc/include/ -mmcu=msp430fr5969 -D__MSP430FR5969__ begin_task.c  $(OPTIMIZE_FLAG) -D$(DEFENISION) -c  -o ../outputs/begin_task.o 
	msp430-elf-gcc -I ../msp430-gcc/include -L ../msp430-gcc/include/ -mmcu=msp430fr5969 -D__MSP430FR5969__ end_task.c  $(OPTIMIZE_FLAG) -D$(DEFENISION) -c  -o ../outputs/end_task.o 
	msp430-elf-gcc -I ../msp430-gcc/include -L ../msp430-gcc/include/ -mmcu=msp430fr5969 -D__MSP430FR5969__ sense.c  $(OPTIMIZE_FLAG) -D$(DEFENISION) -c  -o ../outputs/sense.o
	msp430-elf-gcc -I ../msp430-gcc/include -L ../msp430-gcc/include/ -mmcu=msp430fr5969 -D__MSP430FR5969__ check_state.c  $(OPTIMIZE_FLAG) -D$(DEFENISION) -c  -o ../outputs/check_state.o  	
ifeq ($(DEFENISION) , VERIFY)
	msp430-elf-gcc -I ../msp430-gcc/include -L ../msp430-gcc/include/ -mmcu=msp430fr5969 -D__MSP430FR5969__  $(bench_name)_verify_benchmark.c  $(OPTIMIZE_FLAG) -D$(DEFENISION) -c -o ../outputs/verify_benchmark.o 
	 msp430-elf-gcc -I ../msp430-gcc/include -L ../msp430-gcc/include/ -mmcu=msp430fr5969 -D__MSP430FR5969__  $(outdir)/a.o $(outdir)/begin_task.o $(outdir)/end_task.o $(outdir)/sense.o $(outdir)/check_state.o $(outdir)/verify_benchmark.o     $(OPTIMIZE_FLAG) -D$(DEFENISION)  -o ../outputs/a.out -lm
else
	 msp430-elf-gcc -I ../msp430-gcc/include -L ../msp430-gcc/include/ -mmcu=msp430fr5969 -D__MSP430FR5969__ $(outdir)/a.o $(outdir)/begin_task.o $(outdir)/end_task.o $(outdir)/sense.o  $(outdir)/check_state  $(OPTIMIZE_FLAG) -D$(DEFENISION)  -o ../outputs/a.out -lm
endif

clang:$(bench_name)

$(bench_name):$(bench_name).c
	clang --target=msp430-elf  -D__MSP430__ -c $^ -emit-llvm -I $(includes) $(OPTIMIZE_FLAG)  -D$(DEFENISION) -I ../msp430-gcc/msp430-elf/include/ -D__MSP430FR5969__ -o  ../outputs/raw.bc  
	llvm-dis ../outputs/raw.bc
#	opt -load=./llvmBasicBlockBreakerPass/basicblock_breaker_pass.so -basic-block-breaker-pass ../outputs/raw.bc -o ../outputs/raw.bc
	opt -load=./llvmAddGlobalsPass/addglobals.so -add-globals-pass ../outputs/raw.bc -o ../outputs/after_add_globals.bc
	clang --target=msp430-elf runtime_libs/gVars.c  -c -emit-llvm -I ../msp430-gcc/include/ -I include/  -D__MSP430FR5969__ -o ../outputs/gVars.bc
	llvm-link ../outputs/after_add_globals.bc ../outputs/gVars.bc -o ../outputs/after_add_globals.bc
	opt -load=./llvmBBIdPass/bbid.so -bb-id-pass ../outputs/after_add_globals.bc -o ../outputs/after_bb_id.bc
	llvm-dis ../outputs/after_add_globals.bc
	./generate_cfg.sh
	llvm-dis ../outputs/after_bb_id.bc
	opt -load=./llvmAddBasicBlockAddrsPass/add_basicblock_addrs_pass.so -add-bb-addrs-pass ../outputs/after_bb_id.bc -o ../outputs/after_bb_id.bc
	clang --target=msp430-elf runtime_libs/safepoint.c -O3 -c -emit-llvm -I ../msp430-gcc/include/ -I include/  -D__MSP430FR5969__ -o ../outputs/safepoint.bc
	llvm-link ../outputs/after_bb_id.bc ../outputs/safepoint.bc -o ../outputs/after_safepoint.bc
	clang --target=msp430-elf runtime_libs/collect_trace.c -O3 -c -emit-llvm -I ../msp430-gcc/include/ -I include/  -D__MSP430FR5969__ -o ../outputs/collect_trace.bc
	llvm-link ../outputs/after_safepoint.bc ../outputs/collect_trace.bc -o ../outputs/after_safepoint.bc
	llvm-dis ../outputs/after_safepoint.bc
	clang --target=msp430-elf end_task.c -c -emit-llvm -D$(DEFENISION) -I ../msp430-gcc/include/ -I include/  -D__MSP430FR5969__ -o ../outputs/end_task.bc
	llvm-link ../outputs/after_safepoint.bc ../outputs/end_task.bc -o ../outputs/after_safepoint.bc
	clang --target=msp430-elf sense.c -c -emit-llvm -I ../msp430-gcc/include/ -I include/  -D__MSP430FR5969__ -o ../outputs/sense.bc
	llvm-link ../outputs/after_safepoint.bc ../outputs/sense.bc -o ../outputs/after_safepoint.bc
	clang --target=msp430-elf runtime_libs/check_state.c -c -emit-llvm -I ../msp430-gcc/include/ -I include/  -D__MSP430FR5969__ -o ../outputs/check_state.bc
	llvm-link ../outputs/after_safepoint.bc ../outputs/check_state.bc -o ../outputs/after_safepoint.bc
	clang --target=msp430-elf begin_task.c -c -emit-llvm -I ../msp430-gcc/include/ -I include/  -D__MSP430FR5969__ -o ../outputs/begin_task.bc
	llvm-link ../outputs/after_safepoint.bc ../outputs/begin_task.bc -o ../outputs/after_safepoint.bc
ifeq ($(DEFENISION) , VERIFY)
	clang --target=msp430-elf $(bench_name)_verify_benchmark.c -c -emit-llvm -I ../msp430-gcc/include/ -I include/  -D__MSP430FR5969__ -o ../outputs/verify_benchmark.bc
	llvm-link ../outputs/after_safepoint.bc ../outputs/verify_benchmark.bc -o ../outputs/after_safepoint.bc
endif
	clang --target=msp430-elf runtime_libs/mementos_checkpoint.c  -c -emit-llvm -I ../msp430-gcc/include/ -I include/  -D__MSP430FR5969__ -o ../outputs/mementos_checkpoint.bc
	llvm-link ../outputs/after_safepoint.bc ../outputs/mementos_checkpoint.bc -o ../outputs/after_mementos_checkpoint.bc
	clang --target=msp430-elf runtime_libs/specialized_checkpoint.c  -c -emit-llvm -I ../msp430-gcc/include/ -I include/  -D__MSP430FR5969__ -o ../outputs/specialized_checkpoint.bc
	llvm-link ../outputs/after_mementos_checkpoint.bc ../outputs/specialized_checkpoint.bc -o ../outputs/after_mementos_checkpoint.bc
	opt -load=./llvmRenameMainPass/rename_main.so -rename-main-pass ../outputs/after_mementos_checkpoint.bc -o ../outputs/after_rename_main.bc 
	clang --target=msp430-elf runtime_libs/mementos_main.c  -c -emit-llvm -I ../msp430-gcc/include/ -I./include/  -D__MSP430FR5969__ -o ../outputs/mementos_main.bc
	llvm-link ../outputs/after_rename_main.bc ../outputs/mementos_main.bc -o ../outputs/after_mementos_main.bc
	clang --target=msp430-elf runtime_libs/mementos.c -c -emit-llvm -I ../msp430-gcc/include/ -I include/  -D__MSP430FR5969__ -o ../outputs/mementos.bc
	llvm-link ../outputs/after_mementos_main.bc ../outputs/mementos.bc -o ../outputs/after_mementos_main.bc
	clang ../outputs/after_mementos_main.bc -mcpu=msp430 --target=msp430-elf -o ../outputs/a.s -c -S 
	msp430-elf-gcc ../outputs/a.s  -mmcu=msp430fr5969 -L ../msp430-gcc/include/ -o../outputs/a.out -lm
	msp430-elf-objdump ../outputs/a.out -D > ../outputs/a.objdump
	python beg_end_address.py
	#sed -i  's/call\s#_safepoint/NOP\n\tNOP/g' ../outputs/after_mementos_main.s  
	msp430-elf-gcc ../outputs/a_o.s  -mmcu=msp430fr5969 -L ../msp430-gcc/include/ -o../outputs/a.out -lm 
	msp430-elf-objdump ../outputs/a.out -D > ../outputs/a.objdump

clean:
	rm ../outputs/*
