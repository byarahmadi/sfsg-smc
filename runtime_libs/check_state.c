/*
 * This file is part of sfsg.
 *
 * Author: Bahram Yarahmadi
 * Copyright (c) 2020  Inria
 *
 */


#include <msp430.h>
#include "fr5969.h"

void check_state() {
  if (MEMREF_UINT(SYSTEM_STATE_LOC) == ZERO &&
      MEMREF_UINT(CHECKPOINT_HISTORY) != ZERO) {
    MEMREF_UINT(SYSTEM_STATE_LOC) = STABLE_STATE;
  }
}
