/*
 * This file is part of sfsg.
 *
 * Author: Bahram Yarahmadi
 * Copyright (c) 2020  Inria
 *
 */


#include <fr5969.h>
extern unsigned int nst;
extern unsigned int lh;
extern unsigned int firstTimeInLoop;
extern unsigned int callInstAddr;
extern unsigned int volatile_trace_size;
extern unsigned int volatile_trace_arr[100];
/* memtype 0 is trace in volatile memory & 1 is trace in non-volatile memory */

int Lookup(int mem_type, unsigned int callInstAddr) {
  unsigned int number_of_checkpoints = MEMREF_UINT(CHECKPOINT_HISTORY);
  unsigned int trace_loc;
  if (mem_type == 0)
    trace_loc = &volatile_trace_arr;
  else
    trace_loc = TRACE_LOC;

  unsigned int current_itr = MEMREF_UINT(LOOP_HEAD_LOC + nst * 6 - 4);
  for (unsigned int i = 1; i <= number_of_checkpoints; i++) {
    unsigned int position = CHECKPOINT_HISTORY + i * 10;
    unsigned int itr = MEMREF_UINT(position);
    unsigned int trace_first_element = MEMREF_UINT(position - 2);
    unsigned int checkpoint_loc = MEMREF_UINT(position - 4);
    unsigned int ts = MEMREF_UINT(position - 6);
    unsigned int xor_middle_trace = MEMREF_UINT(position - 8);

    if (checkpoint_loc == callInstAddr &&
        trace_first_element == MEMREF_UINT(trace_loc + 2) &&
        ts == MEMREF_UINT(trace_loc) && current_itr % itr == 0) {
      if (xor_middle_trace == getXOR_trace(trace_loc)) return 1;
    }
  }

  return 0;
}
void hook_checkpoint(int itr) {
  //    MEMREF_UINT(EXE_BBS)++;
  asm volatile("MOV 16(R1),R15");
  asm volatile("ADD #4,R15");
  asm volatile("MOV 0(R15),R15");
  asm volatile("SUB #4,R15");
  asm volatile("MOV R15,%0" : "=m"(callInstAddr));

  int to_checkpoint;

  if (TRACE_SIZE != NOT_COLLECT_TRACE) {
    non_volatile_collect_trace();
    to_checkpoint = Lookup(1, callInstAddr);
  } else {
    volatile_collect_trace();
    to_checkpoint = Lookup(0, callInstAddr);
  }

  // TRACE_SIZE = volatile_trace_size + 2;
  unsigned int loopItr = MEMREF_UINT(LOOP_HEAD_LOC + nst * 6 - 4);
  if (loopItr % itr == 0 && to_checkpoint) _checkpoint();
}
void _sp_checkpoint() {
  //  MEMREF_UINT(EXE_BBS)++;
  asm volatile("MOV 10(R1),R15");
  asm volatile("SUB #4,R15");
  asm volatile(
      "MOV R15,%0"
      : "=m"(
          callInstAddr));  // **R15** has the call Addr as well as callInstAddr

  int to_checkpoint;
  if (TRACE_SIZE != NOT_COLLECT_TRACE) {
    non_volatile_collect_trace();  //
    to_checkpoint = Lookup(1, callInstAddr);
  } else {
    volatile_collect_trace();
    to_checkpoint = Lookup(0, callInstAddr);
  }
  if (to_checkpoint) _checkpoint();
}
