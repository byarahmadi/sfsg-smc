/*
 * This file is part of sfsg.
 *
 * Author: Bahram Yarahmadi
 * Copyright (c) 2020  Inria
 *
 */


#include <fr5969.h>
#include <mementos.h>

unsigned int __locate_free_codecache();

unsigned int pow(unsigned int baseV, unsigned powV) {
  unsigned int result = 1;
  for (unsigned int i = 0; i < powV; i++) {
    result *= baseV;
  }
  return result;
}
unsigned int getXOR_trace(unsigned int trace_loc) {
  unsigned int ts = MEMREF_UINT(trace_loc);
  unsigned int xor_middle_trace = 0;

  for (unsigned int i = 2; i < ts - 2; i += 2) {
    unsigned int trace_element = MEMREF_UINT(trace_loc + i);
    xor_middle_trace ^= trace_element;
  }

  return xor_middle_trace;
}
void __remove_codecache(unsigned int checkpoint_loc) {
  unsigned int codecacheId = MEMREF_UINT(checkpoint_loc + 2);
  unsigned int blockId = 0;
  unsigned int nA = MEMREF_UINT(ALLOCATED_CODECACHES_LOC);
  for (unsigned int i = 0; i < nA; i++) {
    if (codecacheId == CODECACHE_START_LOC + i * 16 + 2) {
      blockId = i;
      break;
    }
  }
  MEMREF_UINT(checkpoint_loc) = CALL;
  MEMREF_UINT(checkpoint_loc + 2) = &_safepoint;

  MEMREF_UINT(ALLOCATED_CODECACHES_LOC + (blockId + 1) * 4 - 2) = 0xFFFF;
  MEMREF_UINT(ALLOCATED_CODECACHES_LOC + (blockId + 1) * 4) = 0xFFFF;

  unsigned int *bitVect = CODECACHE_START_LOC;
  *bitVect |= pow(2, blockId);
}

void __create_or_change_codecache(unsigned int checkpoint_loc,
                                  unsigned int itr) {
  if (MEMREF_UINT(checkpoint_loc) == BR) {
    MEMREF_UINT(MEMREF_UINT(checkpoint_loc + 2) + 4) = itr;
    // Changing the return addr is also important ** I am not still sure
  } else {
    unsigned int blockId;
    MEMREF_UINT(checkpoint_loc) = BR;
    unsigned int *codecache_loc = __locate_free_codecache(&blockId);
    MEMREF_UINT(checkpoint_loc + 2) = codecache_loc;
    *codecache_loc = PUSH_R12;
    *(codecache_loc + 1) = MOV;
    *(codecache_loc + 2) = itr;
    *(codecache_loc + 3) = CALL;  // Call somewhere
    *(codecache_loc + 4) = &hook_checkpoint;
    *(codecache_loc + 5) = POP_R12;
    *(codecache_loc + 6) = BR;
    *(codecache_loc + 7) = checkpoint_loc + 4;

    /* This is the largest  allocated codecache block */

    if (MEMREF_UINT(ALLOCATED_CODECACHES_LOC) == ZERO)
      MEMREF_UINT(ALLOCATED_CODECACHES_LOC) = 1;
    else if (MEMREF_UINT(ALLOCATED_CODECACHES_LOC) < blockId)
      MEMREF_UINT(ALLOCATED_CODECACHES_LOC) = blockId;

    // codecash addr --- call addr
    MEMREF_UINT(ALLOCATED_CODECACHES_LOC + blockId * 4 - 2) = codecache_loc;
    MEMREF_UINT(ALLOCATED_CODECACHES_LOC + blockId * 4) = checkpoint_loc;
  }
}

unsigned int __locate_free_codecache(unsigned int *blockId) {
  unsigned int *addr = CODECACHE_START_LOC;
  unsigned int bitVect = *addr;
  unsigned int mask = 1;

  unsigned int *blockAddr = CODECACHE_START_LOC + 2;
  *blockId = 0;
  for (int i = 1; i <= 16; i++) {
    if (bitVect & mask) {
      *blockId = i;
      *addr = bitVect & (~mask);
      break;
    } else {
      mask = mask << 1;
    }
  }
  if (*blockId) {
    blockAddr = (CODECACHE_START_LOC + 2) + (*blockId - 1) * CODECACHE_SIZE;
  }
  return blockAddr;
}
void __free_last_located_codecache() {
  unsigned int bitVect = MEMREF_UINT(CODECACHE_START_LOC);
  unsigned int mask = 1;
  for (int i = 0; i < 16; i++) {
    if (bitVect & mask) {
      mask = (mask >> 1);

      MEMREF_UINT(CODECACHE_START_LOC) = bitVect | mask;
      break;
    } else {
      mask = mask << 1;
    }
  }
}

unsigned int __find_call_Addr_codecache(unsigned int codecashAddr) {
  unsigned int callAddr;
  unsigned int nA = MEMREF_UINT(ALLOCATED_CODECACHES_LOC);
  for (int i = 0; i < nA; i++) {
    if (codecashAddr == MEMREF_UINT(ALLOCATED_CODECACHES_LOC + i * 4 + 2)) {
      callAddr = MEMREF_UINT(ALLOCATED_CODECACHES_LOC + i * 4 + 4);
      break;
    }
  }
  return callAddr;
}

unsigned int __mementos_locate_next_bundle() {
  unsigned int baseaddr;
  unsigned int target;

  baseaddr = __mementos_find_active_bundle();
  switch (baseaddr) {
    case FRAM_FIRST_BUNDLE_SEG:
      target = FRAM_SECOND_BUNDLE_SEG;
      break;
    case FRAM_SECOND_BUNDLE_SEG:
      target = FRAM_FIRST_BUNDLE_SEG;
    default:  // case 0xFFFFu:
      target = FRAM_FIRST_BUNDLE_SEG;
      break;
  }
  return target;
}
unsigned int __mementos_find_active_bundle(void) {
  unsigned int active_ptr = MEMREF_UINT(ACTIVE_BUNDLE_PTR);
  if (__mementos_bundle_in_range(active_ptr)) return active_ptr;
  return 0xffff;
}

void __mementos_atboot_cleanup(void) {}

void __mementos_inactive_cleanup(unsigned int active_bundle_addr) {}

unsigned int __mementos_bundle_in_range(unsigned int bun_addr) {
  return ((bun_addr >= FRAM_FIRST_BUNDLE_SEG) && (bun_addr < FRAM_END));
}
