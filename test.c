/*
 * This file is part of sfsg.
 *
 * Author: Bahram Yarahmadi
 * Copyright (c) 2020  Inria
 *
 */
/* A simple test program containing different control flow structure (e.g., loops, branch) and function call
 */
#include <msp430.h>
unsigned int loop_bound = 1;  // 20;
unsigned int loop_bound2 = 20;
unsigned int arr[3] __attribute__((section(".data"))) = {0};
unsigned int zz __attribute__((section(".data"))) = 0;
int doSomething() {
  static int mVar = 1;
  mVar++;
  asm volatile("MOV #0xE864,R14");
  asm volatile("MOV %0,0(R14)" : "=m"(mVar));
}
int doSomethingElse() {
  static int mVar = 1;
  mVar++;
  asm volatile("MOV #0xE862,R14");
  asm volatile("MOV %0,0(R14)" : "=m"(mVar));
}
int main(void) {
  int mVar = 1;
  for (unsigned int k = 0; k < loop_bound; k++) {
    asm volatile("NOP");
    asm volatile("NOP");
    for (unsigned int i = 0; i < loop_bound2; i++) {
      mVar++;
      doSomething();
      zz++;

      //	    sense(5000);
      if (i % 3 == 0)
        sense(4000);
      else
        sense(5000);
    }

    check_state();
  }

  asm volatile("MOV #0xE866,R14");
  asm volatile("MOV %0,0(R14)" : "=m"(mVar));
  end_task();
  return 0;
}
