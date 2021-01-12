/*
 * This file is part of sfsg.
 *
 * Author: Bahram Yarahmadi
 * Copyright (c) 2020  Inria
 *
 */

#include <fstream>
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
using namespace std;
using namespace llvm;

namespace {
class CheckpointPlacerPass : public FunctionPass {
 public:
  static char ID;
  ;
  CheckpointPlacerPass() : FunctionPass(ID) {}

  virtual bool runOnFunction(Function &F) {
    Module *m = F.begin()->getModule();
    FunctionType *fnType =
        FunctionType::get(Type::getVoidTy(m->getContext()), false);
    const char *spFunctionName = "_safepoint";
    Constant *hookFn = m->getOrInsertFunction(spFunctionName, fnType);
    Function *spFunctionPtr = cast<Function>(hookFn);

    Function::iterator itrBB = F.begin();
    BasicBlock::iterator itrInst = itrBB->begin();
    Instruction *callInst = CallInst::Create(spFunctionPtr, "");
    itrBB->getInstList().insert(itrInst, callInst);

    return true;
  }
};
}  // namespace
char CheckpointPlacerPass::ID = 0;

static RegisterPass<CheckpointPlacerPass> X("checkpointplacerpass",
                                            "Checkpoint Placer Pass", false,
                                            false);
