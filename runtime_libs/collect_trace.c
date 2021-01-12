/*
 * This file is part of sfsg.
 *
 * Author: Bahram Yarahmadi
 * Copyright (c) 2020  Inria
 *
 */


#include "fr5969.h"
extern unsigned int nst;
extern unsigned int firstTimeInLoop;
extern unsigned int lh;
extern unsigned int callInstAddr;
unsigned int volatile_trace_size = 0;
extern unsigned int volatile_trace_arr[100];

void non_volatile_collect_trace() {
  if (TRACE_SIZE == ZERO) {
    volatile_trace_size = 2;
  } else {
    volatile_trace_size = TRACE_SIZE;
  }
  /* For collecting TRACES, TRACE size must be known */
  /* For Consistency  and performace reasons, I put it into volatile_trace_size
   * variable */
  /* If the BB is not in the loop, there is no need to worried about consistency
   * isuus in a trace */
  if (!nst) {
    MEMREF_UINT(TRACE_LOC + volatile_trace_size) = callInstAddr;
  } else {
    if (firstTimeInLoop) {
      /* it is the first time that we are inside of the loop */
      MEMREF_UINT(LOOP_HEAD_LOC + nst * 6 - 6) =
          TRACE_LOC + volatile_trace_size;
      MEMREF_UINT(LOOP_HEAD_LOC + nst * 6 - 4) =
          1; /* We are at the fisrt itr of the loop */
      MEMREF_UINT(LOOP_HEAD_LOC + nst * 6 - 2) = callInstAddr;
      MEMREF_UINT(TRACE_LOC + volatile_trace_size) = callInstAddr;
      firstTimeInLoop = 0;
    } else if (lh) {
      volatile_trace_size =
          MEMREF_UINT(LOOP_HEAD_LOC + nst * 6 - 6) - TRACE_LOC;
      if (MEMREF_UINT(TRACE_LOC + volatile_trace_size) != callInstAddr) {
        volatile_trace_size = 2;
        MEMREF_UINT(LOOP_HEAD_LOC + nst * 6 - 6) =
            TRACE_LOC + volatile_trace_size;
        MEMREF_UINT(TRACE_LOC + volatile_trace_size) =
            callInstAddr;  // problematic
      }
      MEMREF_UINT(LOOP_HEAD_LOC + nst * 6 - 4) +=
          1;  // It was last_loc_trace before
      lh = 0;
    } else {
      if (MEMREF_UINT(TRACE_LOC + volatile_trace_size) != callInstAddr) {
        MEMREF_UINT(TRACE_LOC + volatile_trace_size) = callInstAddr;
      }
    }
  }
  TRACE_SIZE = volatile_trace_size + 2;
  MEMREF_UINT(NST_LOC) = nst;
}
void volatile_collect_trace() {
  volatile_trace_size += 2;
  const unsigned int pointer = &volatile_trace_arr;

  if (!nst) {
    MEMREF_UINT(pointer + volatile_trace_size) = callInstAddr;
  } else {
    if (firstTimeInLoop) {
      MEMREF_UINT(LOOP_HEAD_LOC + nst * 6 - 6) = pointer + volatile_trace_size;
      MEMREF_UINT(LOOP_HEAD_LOC + nst * 6 - 4) = 1;
      MEMREF_UINT(pointer + volatile_trace_size) = callInstAddr;
      firstTimeInLoop = 0;
    } else if (lh) {
      volatile_trace_size =
          MEMREF_UINT(LOOP_HEAD_LOC + nst * 6 - 6) - volatile_trace_size;
      if (MEMREF_UINT(pointer + volatile_trace_size) != callInstAddr) {
        volatile_trace_size = 2;
        MEMREF_UINT(LOOP_HEAD_LOC + nst * 6 - 6) =
            pointer + volatile_trace_size;
        MEMREF_UINT(pointer + volatile_trace_size) =
            callInstAddr;  // problematic
      }
      MEMREF_UINT(LOOP_HEAD_LOC + nst * 6 - 4) +=
          1;  // It was last_loc_trace before
      lh = 0;
    } else {
      if (MEMREF_UINT(pointer + volatile_trace_size) != callInstAddr) {
        MEMREF_UINT(pointer + volatile_trace_size) = callInstAddr;
      }
    }
  }
  MEMREF_UINT(pointer) = volatile_trace_size + 2;
}
