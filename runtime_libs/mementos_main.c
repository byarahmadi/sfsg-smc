/*
 * This file is part of sfsg.
 *
 * Author: Bahram Yarahmadi
 * Copyright (c) 2020  Inria
 *
 */


#include "fr5969.h"
#include "mementos.h"
unsigned int i, j, k;
unsigned int baseaddr;
unsigned int tmpsize;
unsigned int globsize;
extern const int nbbs[];

unsigned int break_addr;
unsigned int begin_addr;
unsigned int end_addr;
unsigned int mid_addr;
unsigned int inst_addr;

unsigned int inst_size = 0;

unsigned int multTo6(unsigned int x) {
  unsigned int result = 0;
  for (unsigned int i = 0; i < x; i++) result += 6;
  return result;
}
#define CHECKPOINT_LOC MEMREF_UINT(LAST_TRACE_LOC - 2)
#define PRE_LOOP_HEAD(x) (MEMREF_UINT(LOOP_HEAD_LOC + multTo6(x) - 6) - 2)
extern unsigned int nst;

/* Debug info */

#define UNREACHING_CHECKPOINT 0xE764
#define SHORT_INTERVAL 0xE768

void sleep() {
  while (1)
    ;
}
unsigned int check_outer_loops_pre_loop_head(unsigned int nst) {
  for (int i = 0; i < nst; i++) {
    /* It means that the PRE_LOOP_HEAD is checkpointed */
    if (MEMREF_UINT(LOOP_HEAD_LOC + i * 6 + 4) ==
        MEMREF_UINT(
            MEMREF_UINT(LOOP_HEAD_LOC +
                        i * 6)))  // If the location in the trace == Loop_head
    {
      if (TRACE_SIZE > 4 &&
          ((TRACE_LOC + TRACE_SIZE)) >= MEMREF_UINT(LOOP_HEAD_LOC + i * 6)) {
        if (MEMREF_UINT(MEMREF_UINT(PRE_LOOP_HEAD(i + 1)) + 2) == &_safepoint) {
          return (i + 1);
        }
      }
    }
  }
  return 0;  // 0 means all pre_headers are checkpointed
}
unsigned int search_traces_in_codecaches() {
  unsigned int ts = TRACE_SIZE - 4;
  while (ts > 2) {
    unsigned int te = MEMREF_UINT(TRACE_LOC + ts);
    unsigned int nA = MEMREF_UINT(ALLOCATED_CODECACHES_LOC);
    if (nA == ZERO) return 0xFFFF;
    for (unsigned int i = nA; i >= 1; i--) {
      if (MEMREF_UINT(ALLOCATED_CODECACHES_LOC + i * 4) == te) {
        return te;
      }
    }

    ts -= 2;
  }
  return 0xFFFF;
}
unsigned int decode(unsigned int addr) {
  unsigned int inst = MEMREF_UINT(addr);
  unsigned int op = inst & 0x3000;
  // Jump instruction
  if (op == 0x2000 || op == 0x3000) {
    return 2;
  }
  // Single operand
  if (op == 0x1000) {
    op = inst & 0x0030;
    if (op == 0x0010) {
      return 4;
    }
    if (op == 0x0030) {
      op = inst & 0x000F;
      if (op == 0x0000) return 4;
    }
    return 2;
  }
  // Double Operand
  op = inst & 0x00B0;
  if (op == 0x0000) {
    return 2;
  }
  unsigned int s_op = inst & 0x0F00;
  if (s_op == 0x0300 || s_op == 0x0100) {  // Some emulated instructions
    if (op >= 0x0080) return 4;
    return 2;
  }

  unsigned int size = 2;
  if (s_op == 0x0200) {
    unsigned int op_2 = op & 0x00b0;
    if (op_2 == 0x00b0 || op_2 == 0x00a0) return 4;
    if (op_2 == 0x0090) return 6;
    if (op_2 == 0x0010) return 4;
    return 2;
  }
  unsigned int source_addrs = op & 0x0030;
  unsigned int des_addrs = op & 0x0080;
  if (des_addrs == 0x0080) size += 2;
  if (source_addrs == 0x0020) return size;
  if ((source_addrs == 0x0010 || source_addrs == 0x0030) && (s_op == 0x0000))
    size = size + 2;
  return size;
}
/* This function breaks a basic block : one power Cycle */
void break_code(unsigned int inst_addr) {
  unsigned int inst_size = decode(inst_addr);
  if (inst_size == 2) inst_size += decode(inst_addr + inst_size);

  unsigned int trampoline_size = MEMREF_UINT(TRAMPOLINE);
  if (trampoline_size == ZERO) trampoline_size = 2;

  unsigned int addr_to_write = TRAMPOLINE + trampoline_size;
  /*  writing the instruction that are going to be re-writed  */
  for (int i = 0; i < inst_size; i = i + 2)
    MEMREF_UINT(addr_to_write + i) = MEMREF_UINT(inst_addr + i);

  /* CALL __checkpoint */
  MEMREF_UINT(addr_to_write + inst_size) = CALL;
  MEMREF_UINT(addr_to_write + inst_size + 2) = __checkpoint;

  /* BR after inst_addr (inst_addr + inst_size) */
  MEMREF_UINT(addr_to_write + inst_size + 4) = BR;
  MEMREF_UINT(addr_to_write + inst_size + 6) = inst_addr + inst_size;

  MEMREF_UINT(inst_addr) = BR;
  MEMREF_UINT(inst_addr + 2) = addr_to_write;

  if (inst_size > 4) {
    unsigned int nop_addr = inst_addr + 4;
    for (unsigned int i = 0; i < inst_size - 4; i++)
      MEMREF_UINT(nop_addr + i) = NOP;
  }

  MEMREF_UINT(TRAMPOLINE) = trampoline_size + inst_size + 8;
}
unsigned int *find_bb(unsigned int bb_begin_addr) {
  int number_bb = nbbs[0];
  unsigned int *addr = nbbs + 1; /* The second element of the array */
  for (int i = 1; i < number_bb + 1; i++) {
    if (*addr == bb_begin_addr) return addr;
    addr = *(addr + 2);
  }
  return 0xFFFF;
}

unsigned int find_break_point(unsigned int bb_begin_addr,
                              unsigned int bb_end_addr) {
  unsigned int bb_middle_addr =
      bb_begin_addr + (bb_end_addr - bb_begin_addr) / 2;
  unsigned int inst_addr = bb_begin_addr;
  unsigned int inst_size;
  do {
    inst_size = decode(inst_addr);
    inst_addr += inst_size;
  } while (inst_addr < bb_middle_addr);

  return inst_addr;
}
unsigned int get_resume_point() {
  unsigned int resume_point = MEMREF_UINT(RESUME_POINT);
  if (resume_point > TRAMPOLINE) {
    unsigned int target_addr = MEMREF_UINT(resume_point + 2);
  }
  return 0;
}
void edit_nbbs(unsigned int *node_addr, unsigned int inst_addr) {
  unsigned int rest_nbbs_size = MEMREF_UINT(REST_NBBS);
  if (rest_nbbs_size == ZERO) rest_nbbs_size = 2;
  unsigned int loc = REST_NBBS + rest_nbbs_size;
  MEMREF_UINT(loc) = inst_addr;            /* begining addr of the BB */
  MEMREF_UINT(loc + 2) = *(node_addr + 1); /* end addr of the BB */
  MEMREF_UINT(loc + 4) = *(node_addr + 2); /* link to the next BB */

  *(node_addr + 1) = inst_addr;
  *(node_addr + 2) = REST_NBBS + rest_nbbs_size;
  MEMREF_UINT(REST_NBBS) = rest_nbbs_size + 6;
  int *nbbs_pointer = &nbbs;  //*(nbbs + 0) += 1;  // nbbs[0] += 1;
  unsigned int p = (int)nbbs;
  MEMREF_UINT(p) += 1;
}
void break_bb() {
  unsigned int resume_point = MEMREF_UINT(RESUME_POINT);
  if (resume_point > TRAMPOLINE) {
    unsigned int target_addr = MEMREF_UINT(
        resume_point + 6);  // CALL __checkpoint : 4 byte , BR 0xXXXX : 4 byte
                            // => 0xXXXX is target, SO 4 + 2
    if (MEMREF_UINT(target_addr - 4) == BR)
      resume_point = target_addr - 4;
    else if (MEMREF_UINT(target_addr - 6) == BR)
      resume_point = target_addr - 6;
    else
      resume_point = target_addr - 8;
  }
  unsigned int *addr = find_bb(resume_point);

  begin_addr = *addr + 4;
  end_addr = *(addr + 1);
  mid_addr = begin_addr + (end_addr - begin_addr) / 2;

  inst_addr = find_break_point(begin_addr, end_addr);
  inst_size = decode(inst_addr);

  break_addr = inst_addr;
  break_code(inst_addr);
  edit_nbbs(addr, inst_addr);
}

void update_checkpoint_histoty() {}
/* This function must be executed in one charge of capacitor */
/* when the system is in NOT_COLLECT_TRACE mode:It collect trace in SRAM */
int main(void) {
  begin_task();

  if (MEMREF_UINT(TRACE_LOC) == STABLE_STATE) {
    MEMREF_UINT(TRACE_LOC) = NOT_COLLECT_TRACE;

  } else if (MEMREF_UINT(TRACE_LOC) == NOT_COLLECT_TRACE) {
    MEMREF_UINT(TRACE_LOC) = COLLECT_TRACE;

  } else if (MEMREF_UINT(TRACE_LOC) != ZERO) {
    // end_task();
    unsigned int checkpoint_loc = CHECKPOINT_LOC;
    nst = MEMREF_UINT(NST_LOC);
    unsigned int old_itr = 0;
    if (MEMREF_UINT(LAST_CHECKPOINT_LOC) != ZERO) {
      // TODO : Maybe for the first Instrcution we should check
      if (MEMREF_UINT(MEMREF_UINT(LAST_CHECKPOINT_LOC)) == BR) {
        old_itr =
            MEMREF_UINT(MEMREF_UINT(MEMREF_UINT(LAST_CHECKPOINT_LOC) + 2) + 4);
        MEMREF_UINT(MEMREF_UINT(LAST_CHECKPOINT_LOC)) = CALL;
        __free_last_located_codecache();
      }
      MEMREF_UINT(MEMREF_UINT(LAST_CHECKPOINT_LOC) + 2) = &_safepoint;
      MEMREF_UINT(CHECKPOINT_HISTORY) -=
          1;  // The number of checkpoints must be decreased

      /* Debug */
      MEMREF_UINT(UNREACHING_CHECKPOINT)++;

      // Do something about last unsuccessfull checkpoint
    }
    unsigned int itr = 1;
    if (nst > 0) {
      itr = MEMREF_UINT(LOOP_HEAD_LOC + MEMREF_UINT(NST_LOC) * 6 - 4);
      MEMREF_UINT(CHECKPOINT_LOC + 2) = &_sp_checkpoint;
    } else {  // The failure happend outside the loop
      MEMREF_UINT(CHECKPOINT_LOC + 2) =
          &__checkpoint;  // indirect call to _checkpoint. (callInstAddr for
                          // breaking BBs
    }

    MEMREF_UINT(LAST_CHECKPOINT_LOC) = checkpoint_loc;
    if (MEMREF_UINT(CHECKPOINT_HISTORY) == ZERO) {
      //       MEMREF_UINT(CHECKPOINT_TRACE_HISTORY) = 2;
      MEMREF_UINT(CHECKPOINT_HISTORY) = 1;
    } else {
      MEMREF_UINT(CHECKPOINT_HISTORY) += 1;
    }
    // unsigned int ntElements = MEMREF_UINT(CHECKPOINT_TRACE_HISTORY); //
    // number of current stored trace elements
    //  unsigned int trace_pos = CHECKPOINT_TRACE_HISTORY + ntElements;

    //  unsigned int ts = 2;
    unsigned int trace_first_element = MEMREF_UINT(TRACE_LOC + 2);
    unsigned int xor_middle_trace = getXOR_trace(TRACE_LOC);

    //  MEMREF_UINT(CHECKPOINT_TRACE_HISTORY) += ts - 2;

    unsigned int number_of_checkpoints = MEMREF_UINT(CHECKPOINT_HISTORY);
    unsigned int position = CHECKPOINT_HISTORY + number_of_checkpoints * 10;
    MEMREF_UINT(position) = itr;
    MEMREF_UINT(position - 2) = trace_first_element;
    MEMREF_UINT(position - 4) = checkpoint_loc;
    MEMREF_UINT(position - 6) = TRACE_SIZE;
    MEMREF_UINT(position - 8) = xor_middle_trace;

    //  MEMREF_UINT(CHECKPOINT_HISTORY) += number_of_checkpoints + 1;

    MEMREF_UINT(TRACE_LOC) = NOT_COLLECT_TRACE;
    //       sleep();// I don't know for the expriments these two sleep points
    //       are important or not
    /* If the TRACE LOC is empty it means that the program cannot have forward
     * progress because of large BBs */
  } else {
    if (MEMREF_UINT(FIRST_TIME) == ZERO)
      MEMREF_UINT(FIRST_TIME) = 0x0001;
    else
      break_bb();
  }

  i = __mementos_find_active_bundle();

  if (__mementos_bundle_in_range(i)) {
    /* there's an active bundle inside one of the reserved segments */
    //  __mementos_inactive_cleanup(i);
    __mementos_restore(i);
    return 13;  // shouldn't happen!  return 13 indicates bad luck.
  }

  /* invoke the original program */
  i = _old_main();

  return i;
}
void __mementos_restore(unsigned int b) {
  /* b is a pointer to a valid bundle found by __mementos_find_active_bundle.
   * the first word of the bundle is split: the high byte designates the size
   * (in bytes) of the stack portion of the bundle and the low byte designates
   * the size (in bytes) of the data-segment portion of the bundle. */
  baseaddr = b;  // XXX

  // __mementos_log_event(MEMENTOS_STATUS_STARTING_RESTORATION);

  /*
  // disable interrupts -- they wouldn't be helpful here
  asm volatile ("DINT");
  */
  //              GS SS
  /*baseaddr+0-->[--|--|      //global size and stack size
    baseaddr+4--> R0|R1|R2|R4 //note: there is no r3
                  R5|R6|R7|R8
                  R9|RA|RB|RC
                  RD|RE|RF
    baseaddr+34-->[--... SS bytes ...--] //stack
 baseaddr+34+SS-->[0xBEAD];
baseaddr+34+SS+1-->{business as usual...}

  */
  /* restore the stack by walking from the top to the bottom of the stack
   * portion of the checkpoint */
  tmpsize = MEMREF_UINT(baseaddr + 2);  // stack size

  for (i = 0; i < tmpsize; i += 2) {
    /* summary:
    MEMREF(TOPOFSTACK - i) = MEMREF(baseaddr + 30 + tmpsize - i);
    */

    // j = TOPOFSTACK - i - 2;
    asm volatile("MOV #" xstr(TOPOFSTACK) ", %0" : "=m"(j));
    asm volatile("SUB %0, %1" : "=m"(i) : "m"(j));
    asm volatile("DECD.W %0" ::"m"(j));

    // k = baseaddr + BUNDLE_SIZE_REGISTERS + 4 + tmpsize - i;
    asm volatile("MOV %1, %0" : "=m"(k) : "m"(baseaddr));
    asm volatile("ADD #" xstr(BUNDLE_SIZE_REGISTERS) ", %0" ::"m"(k));
    asm volatile("ADD #2, %0" ::"m"(k));
    asm volatile("ADD %1, %0" : "=m"(k) : "m"(tmpsize));
    asm volatile("SUB %1, %0" : "=m"(k) : "m"(i));

    // MEMREF(j) = MEMREF(k);
    asm volatile("MOV %0, R7" ::"m"(k));
    asm volatile("MOV %0, R8" ::"m"(j));
    asm volatile("MOV @R7, 0(R8)");
  }

  globsize = MEMREF_UINT(baseaddr);

  k = baseaddr + BUNDLE_SIZE_HEADER + BUNDLE_SIZE_REGISTERS + 2 + tmpsize +
      globsize;
  nst = MEMREF_UINT(k);
  j = 0;
  k = k + 2;
  for (i = 0; i < nst; i++) {
    MEMREF_UINT(LOOP_HEAD_LOC + j) = MEMREF_UINT(k);
    j = j + 2;
    k = k + 2;
    MEMREF_UINT(LOOP_HEAD_LOC + j) = MEMREF_UINT(k);
    j = j + 2;
    k = k + 2;
    MEMREF_UINT(LOOP_HEAD_LOC + j) = MEMREF_UINT(k);
  }

  /* restore the data segment without trampling on our own globals.
   * pseudocode:
   *
   * baseaddr = beginning of checkpoint bundle
   * stacksize = size of stack portion of checkpoint
   * regfilesize = size of register portion of checkpoint (30 bytes)
   * headersize = size of bundle header (2 bytes)
   *
   * for (i = 0; i < globalsize; i += 2) {
   *     memory[STARTOFDATA + i] =
   *         memory[baseaddr + stacksize + regfilesize + headersize + i]
   * }
   */

  // grab the size of the size of the globals we'll have to restore
  asm volatile("MOV %0, R7" ::"m"(tmpsize));   // R7(stacksize) = tmpsize
  asm volatile("MOV %0, R6" ::"m"(baseaddr));  // R6(baseaddr)  = baseaddr
  asm volatile("MOV @R6, R8");                 // R8(globalsize) =
  asm volatile("AND #255, R8");                //   MEMREF(baseaddr) & 0x00FF

  asm volatile("CLR.W R9");    // R9(i) = 0 // induction var
  asm volatile("rdloop:");     // will jump back up here
  asm volatile("CMP R8, R9");  // if (i >= globalsize)
  asm volatile("JC afterrd");  //   <stop looping>

  // copy one word at a time from checkpoint to data segment
  asm volatile("MOV R6, R10");  // R10 = baseaddr
  asm volatile(
      "ADD #34, R10");  // this constant is ugly now, but 34 is SS + GS + REGS
  asm volatile("ADD R7, R10");  // + stacksize
  asm volatile(
      "ADD R9, R10");  // + i, which is the counter for which glob we're on
  asm volatile(
      "MOV 0(R10), " xstr(STARTOFDATA) "(R9)");  // MEMREF(STARTOFDATA+i(R9)) =
                                                 //    MEMREF(R10)
  asm volatile("INCD R9");                       // i += 2
  asm volatile("JMP rdloop");                    // to beginning of loop

  asm volatile("afterrd:");  // jump here when done

  /* set baseaddr back to whatever it was -- BRANDON: why???*/
  asm volatile("MOV R6, %0" : "=m"(baseaddr));

  /* finally, restore all the registers, starting at R15 and counting down to
   * R0/PC.  setting R0/PC is an implicit jump, so we have to do it last. */

  /* j = <PC to restore> (note: R6 still contains baseaddr) */
  asm volatile("MOV 4(R6), %0" : "=m"(j));

  /* set the SP first, so we can PUSH stuff; we'll reset it later */
  asm volatile("MOV 6(R6), R1");
  /* now push the saved register values onto the stack (R6=baseaddr) */

  asm volatile("MOV 32(R6), R15");  // R15
  asm volatile("MOV 30(R6), R14");  // R14
  asm volatile("MOV 28(R6), R13");  // R13
  asm volatile("MOV 26(R6), R12");  // R12
  asm volatile("MOV 24(R6), R11");  // R11
  asm volatile("MOV 22(R6), R10");  // R10
  asm volatile("MOV 20(R6), R9");   // R9
  asm volatile("MOV 18(R6), R8");   // R8
  asm volatile("MOV 16(R6), R7");   // R7
  asm volatile("MOV 12(R6), R5");   // R5
  asm volatile("MOV 10(R6), R4");   // R4
  // skip R3 (CG)
  asm volatile("MOV 8(R6), R2");  // R2
  asm volatile("MOV 6(R6), R1");  // R1

  /*Why do R6 and R0 this way?  Because baseaddr is
    in R6.  That means we need to save R0 before over
    writing R6 with its value from the checkpoint.

    Then, after we recover R6, we can pop R0 from the stack into its home,
    redirecting control flow to the point in the checkpoint.
  */
  asm volatile("PUSH 4(R6)");      // R6
  asm volatile("MOV 14(R6), R6");  // R6
  asm volatile("POP R0");          // R0
}
