
/*
 * This file is part of sfsg.
 *
 * Author: Bahram Yarahmadi
 * Copyright (c) 2020  Inria
 *
 */

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Function.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/IR/Dominators.h"

#include <fstream>
#include <unordered_set>
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
using namespace std;
using namespace llvm;
namespace {
class BasicBlockBreakerPass : public ModulePass {
 public:
  static char ID;
  BasicBlockBreakerPass() : ModulePass(ID) {}
  bool checkInstInBB(const BasicBlock *bb, const Instruction *inst) {
    for (auto itrInst = bb->begin(); itrInst != bb->end(); ++itrInst) {
      if (inst == &*itrInst) return true;
    }
    return false;
  }

  virtual bool runOnModule(Module &m) {
    vector<Instruction *> instrPts;
    vector<BasicBlock *> bbs;
    bool found = false;
    Instruction *instPt = nullptr;
    for (Module::iterator ff = m.begin(), e = m.end(); ff != e; ++ff) {
      for (Function::iterator bb = ff->begin(), e = ff->end(); bb != e; ++bb) {
        unordered_set<string> uset;
        for (BasicBlock::iterator i = bb->begin(), e = bb->end(); i != e; ++i) {
          if (isa<CallInst>(&*i)) {
            CallInst *ci = dyn_cast<CallInst>(i);
            if (isa<InlineAsm>(ci->getCalledValue())) continue;
            string functionName = ci->getCalledValue()->getName();
            string llvmFunctionName = "llvm.";
            if (functionName.compare(0, 5, llvmFunctionName) == 0) continue;

            auto result = uset.find(ci->getCalledValue()->getName());

            if (result != uset.end()) {
              BasicBlock *newBB = SplitBlock(&*bb, &*ci);
              break;
            } else {
              uset.insert(ci->getCalledValue()->getName());
            }
          }
        }
      }
    }

    BasicBlock *newBB = nullptr;
    int instn = 0;
    int bbn = 0;
    while (instn < instrPts.size()) {
      if (checkInstInBB(bbs[bbn], instrPts[instn])) {
        newBB = SplitBlock(bbs[bbn], instrPts[instn]);
        bbn++;
        instn++;
        while (checkInstInBB(newBB, instrPts[instn])) {
          newBB = SplitBlock(newBB, instrPts[instn]);
          instn++;
        }

      } else {
        bbn++;
      }
    }

    return true;
  }
};
}  // namespace

char BasicBlockBreakerPass::ID = 0;
static RegisterPass<BasicBlockBreakerPass> X("basic-block-breaker-pass",
                                             "split basic block", false, false);
