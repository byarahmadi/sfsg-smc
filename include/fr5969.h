#ifndef __FR5969_H__
#define __FR5969_H__
#ifdef __MSPGCC__
#  include <msp430.h>
#elif defined(__clang__)
#  include <msp430fr5969.h>
#else
#  error Unsupported compiler; use mspgcc or clang.
#endif

#define MEMENTOS_FRAM

#define NOP 0x4303u


#define PUSH_R12 0x120Cu;
#define POP_R12 0x413Cu  //POP R12
#define CALL 0x12B0u //CALL
#define MOV 0x403Cu // MOV something,R12
#define BR 0x4030u

#define RESUME_POINT 0xEF7Au
#define FIRST_TIME 0xEF6Eu
#define TRAMPOLINE 0xE000u
#define REST_NBBS 0xEC00u


#define TRACE_LOC 0x9400u // 0x4400 + 0x5000 (20 * 1024 in deciaml for program text)
#define LOOP_HEAD_LOC (TRACE_LOC + 0x800) //2Kb for Traces I think it's enough 9C00
#define TRACE_SIZE MEMREF_UINT(TRACE_LOC)
#define LAST_TRACE_LOC MEMREF_UINT(TRACE_LOC)  + TRACE_LOC
#define NOT_COLLECT_TRACE 0x0000
#define STABLE_STATE 0x800
#define SYSTEM_STATE_LOC 0xD000


//#define EXE_BBS 0xD002
//#define PSMC 0xD004
//#define nPSMC MEMREF_UINT(PSMC)
//#define TIMING_INFO 0xD006

#define ZERO 0xFFFF
#define COLLECT_TRACE ZERO
#define NST_LOC (TRACE_LOC - 4)
#define LAST_CHECKPOINT_LOC TRACE_LOC - 2
#define CODECACHE_START_LOC (LOOP_HEAD_LOC + 0x400) // LOOP_HEAD_LOC A000
#define CODECACHE_SIZE 16 //This is the size of the code cache maybe changed 
#define CHECKPOINT_TRACE_HISTORY 0xB000
#define CHECKPOINT_HISTORY 0xD002

#define ALLOCATED_CODECACHES_LOC 0xC000u

#define TOPOFSTACK 0x2400
#define STARTOFDATA 0x1C00
#define FRAM_END 0xFF7F /*BRANDON: hacked to accomodate mov vs. movx bug*/

#define FRAM_FIRST_BUNDLE_SEG (FRAM_END+1 - (2*(TOPOFSTACK+1 - STARTOFDATA)))
#define FRAM_SECOND_BUNDLE_SEG (FRAM_FIRST_BUNDLE_SEG + (TOPOFSTACK+1 - STARTOFDATA) - 1)

#define ACTIVE_BUNDLE_PTR (FRAM_FIRST_BUNDLE_SEG - (sizeof(unsigned int)))

#define MEMREF_UINT(x) (*((unsigned int*)(x)))
#define REGISTER_BYTES (sizeof(unsigned)) // bytes in a register
#define BUNDLE_SIZE_REGISTERS 30// (15 * REGISTER_BYTES)
#define BUNDLE_SIZE_HEADER 2 //stack size 1 +dataseg size 1
#define MEMENTOS_MAGIC_NUMBER 0xBEADu
#define ROUND_TO_NEXT_EVEN(x) (((x)+1) & 0xFFFEu)
#endif

