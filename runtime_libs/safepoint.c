/*
 * This file is part of sfsg.
 *
 * Author: Bahram Yarahmadi
 * Copyright (c) 2020  Inria
 *
 */


#include <fr5969.h>
#include <msp430.h>
extern unsigned int nst;
extern unsigned int firstTimeInLoop;
extern unsigned int lh;
extern unsigned int callInstAddr;

void _safepoint() {
  asm volatile("MOV 6(R1),R15");
  asm volatile("SUB #4,R15");
  asm volatile("MOV R15,%0"
               : "=m"(callInstAddr));  // **R15** has the call Addr as
  if (TRACE_SIZE != NOT_COLLECT_TRACE)
    non_volatile_collect_trace();
  else
    volatile_collect_trace();
}
