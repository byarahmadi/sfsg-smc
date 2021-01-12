#!/bin/bash
cd llvmBasicBlockBreakerPass/ && make  && cd .. 
cd llvmAddGlobalsPass/ && make  && cd ..
cd llvmRenameMainPass/ && make  && cd ..
cd llvmCheckpointPlacerPass/ && make  && cd ..
cd llvmAddBasicBlockAddrsPass/ && make  && cd ..
cd llvmBBIdPass/ && make  && cd ..




