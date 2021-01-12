/*
 * This file is part of sfsg.
 *
 * Author: Bahram Yarahmadi
 * Copyright (c) 2020  Inria
 *
 */


#include <msp430.h>
#include "fr5969.h"
#include "mementos.h"

extern unsigned int i, j, k;
extern unsigned int baseaddr;
unsigned int GlobalAllocSize_safepoint = 0;
extern unsigned int nst, lh, firstTimeInLoop;
extern unsigned int callInstAddr;

#define COMPLETE_CHECKPOINTS 0xE760
#define UNCOMPLETE_CHECKPOINTS 0xE762
void __checkpoint() {
  asm volatile("MOV 2(R1),R15");
  asm volatile("SUB #4,R15");
  asm volatile(
      "MOV R15,%0"
      : "=m"(
          callInstAddr));  // **R15** has the call Addr as well as callInstAddr
  _checkpoint();
}
void _checkpoint() {
  MEMREF_UINT(UNCOMPLETE_CHECKPOINTS)++;  // Maybe it is not a good idea
  /*
  * Mementos checkpoint routine from : orginal mementos paper and Abstract github 
  * push all the registers onto the stack; they will be copied to NVRAM from
  * there.  any funny business here is to capture the values of the registers
   * as they appeared just before this function was called -- some
   * backtracking is necessary. */

  /*Note on R4:
    R4 was pushed to the stack during the preamble.  We need to get it back*/
  /*That push means R1+2, then we push PC, SP, and R2, which means we need
   * R1+8*/

  /*Note on NOP:
    There appears to be an implementation bug in the MSP430FR5969 that
    incorrectly increments the PC by only 2 after the PUSH 4(R1) that executes.
    The PC is then wrong and execution diverges.  I think this is related to
    erratum CPU40 documented here
    http://www.ti.com/lit/er/slaz473f/slaz473f.pdf.  The setup does not exactly
    match but the behavior seems to be the same.
*/

  asm volatile(" PUSH 4(R1)");  // PC will appear at 28(R1)
  asm volatile(" NOP");  // This NOP is required.  See note above.//Something to
                         // note that there is no problem without this NOP I
                         // just leave it Just in case
  asm volatile(" PUSH R1");  // SP will appear at 26(R1)
  asm volatile(
      " ADD #6, 0(R1)");  // +6 to account for FP (pushed in preamble) + PC + R1
  asm volatile(" PUSH R2");  // R2  will appear at 24(R1)
  // skip R3 (constant generator)
  asm volatile(" PUSH 8(R1)");  // R4  will appear at 22(R1) [see note above]
  asm volatile(" PUSH R5");     // R5  will appear at 20(R1)
  asm volatile(" PUSH R6");     // R6  will appear at 18(R1)
  asm volatile(" PUSH R7");     // R7  will appear at 16(R1)
  asm volatile(" PUSH R8");     // R8  will appear at 14(R1)
  asm volatile(" PUSH R9");     // R9  will appear at 12(R1)
  asm volatile(" PUSH R10");    // R10 will appear at 10(R1)
  asm volatile(" PUSH R11");    // R11 will appear at 8(R1)
  asm volatile(" PUSH R12");    // R12 will appear at 6(R1)
  asm volatile(" PUSH R13");    // R13 will appear at 4(R1)
  asm volatile(" PUSH R14");    // R14 will appear at 2(R1)
  asm volatile(" PUSH R15");    // R15 will appear at 0(R1)

  /**** figure out where to put this checkpoint bundle ****/
  /* precompute the size of the stack portion of the bundle */
  asm volatile("MOV 26(R1), %0" : "=m"(j));  // j = SP
  /* j now contains the pre-call value of the stack pointer */

  baseaddr = __mementos_locate_next_bundle();

  /********** phase #0: save size header (2 bytes) **********/

  // Save these registers so we can put them back after
  // we're done using them to compute the size of the stack
  asm volatile("PUSH R12");
  asm volatile("PUSH R13");

  // compute size of stack (in bytes) into R13
  asm volatile("MOV #" xstr(TOPOFSTACK) ", R13");
  asm volatile("SUB %0, R13" ::"m"(j));  // j == old stack pointer

  // write size of stack (R13) to high word at baseaddr
  asm volatile("MOV %0, R12" ::"m"(baseaddr));
  asm volatile("MOV R13, 2(R12)");

  // store GlobalAllocSize into R13, round it up to the next word boundary
  asm volatile("MOV &GlobalAllocSize, R13");
  asm volatile("INC R13");
  asm volatile("AND #0xFE, R13");

  // write GlobalAllocSize to low word at baseaddr
  asm volatile("MOV R13, 0(R12)");

  // Setting GlobalAllocSize_safepoint
  //    asm volatile ("MOV &GlobalAllocSize,R13");
  asm volatile("MOV R13,&GlobalAllocSize_safepoint");

  asm volatile("POP R13");
  asm volatile("POP R12");

  /********** phase #1: checkpoint registers. **********/
  asm volatile("MOV %0, R14" ::"m"(baseaddr));
  asm volatile("POP 32(R14)");  // R15
  asm volatile("POP 30(R14)");  // R14
  asm volatile("POP 28(R14)");  // R13
  asm volatile("POP 26(R14)");  // R12
  asm volatile("POP 24(R14)");  // R11
  asm volatile("POP 22(R14)");  // R10
  asm volatile("POP 20(R14)");  // R9
  asm volatile("POP 18(R14)");  // R8
  asm volatile("POP 16(R14)");  // R7
  asm volatile("POP 14(R14)");  // R6
  asm volatile("POP 12(R14)");  // R5
  asm volatile("POP 10(R14)");  // R4
  // skip R3 (constant generator)
  asm volatile("POP 8(R14)");  // R2
  asm volatile("POP 6(R14)");  // R1
  asm volatile("POP 4(R14)");  // R0

  /********** phase #2: checkpoint memory. **********/
  /* checkpoint the stack by walking from SP to ToS */
  /*There needs to be a "+ 2" on this expression to get
    past the last register, which is at baseaddr + BSH + BSR.
    The constant 2 refers to the size of the last register*/
  k = baseaddr + BUNDLE_SIZE_HEADER + BUNDLE_SIZE_REGISTERS + 2;
  for (i = j; i < TOPOFSTACK; i += 2 /*sizeof(unsigned)*/) {
    MEMREF_UINT(k + (i - j)) = MEMREF_UINT(i);
  }
  k += (i - j);  // skip over checkpointed stack

  /* checkpoint as much of the data segment as is necessary */
  for (i = STARTOFDATA;
       i < STARTOFDATA + ROUND_TO_NEXT_EVEN(GlobalAllocSize_safepoint);
       i += sizeof(unsigned int)) {
    MEMREF_UINT(k + (i - STARTOFDATA)) = MEMREF_UINT(i);
  }
  k += (i - STARTOFDATA);  // skip over checkpointed globals

  MEMREF_UINT(k) = nst;  // save the nesting variable
  k = k + 2;

  j = 0;
  for (i = 0; i < nst; i++) {
    MEMREF_UINT(k) = MEMREF_UINT(LOOP_HEAD_LOC + j);
    k = k + 2;
    j = j + 2;
    MEMREF_UINT(k) = MEMREF_UINT(LOOP_HEAD_LOC + j);
    k = k + 2;
    j = j + 2;
    MEMREF_UINT(k) = MEMREF_UINT(LOOP_HEAD_LOC + j);
    k = k + 2;
    j = j + 2;

  }

  MEMREF_UINT(LAST_CHECKPOINT_LOC) = ZERO;

  MEMREF_UINT(RESUME_POINT) = callInstAddr;
  MEMREF_UINT(k) = MEMENTOS_MAGIC_NUMBER;
  MEMREF_UINT(ACTIVE_BUNDLE_PTR) = baseaddr;
  if (MEMREF_UINT(SYSTEM_STATE_LOC) == ZERO)
    MEMREF_UINT(TRACE_LOC) = NOT_COLLECT_TRACE;
  else
    MEMREF_UINT(TRACE_LOC) = STABLE_STATE;

  MEMREF_UINT(COMPLETE_CHECKPOINTS)++;

  while (1)
    ;
}
