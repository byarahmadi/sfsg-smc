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
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
using namespace std;
using namespace llvm;
namespace {
class AddBasicBlockAddrsPass : public ModulePass {
 public:
  static char ID;
  AddBasicBlockAddrsPass() : ModulePass(ID) {}

  virtual bool runOnModule(Module &M) {
    unsigned int nSoFar = 0;
    for (Module::iterator ff = M.begin(), e = M.end(); ff != e; ++ff) {
      for (Function::iterator bb = ff->begin(), e = ff->end(); bb != e; ++bb) {
        for (BasicBlock::iterator i = bb->begin(), e = bb->end(); i != e; ++i) {
          if (isa<CallInst>(&*i)) {
            CallInst *ci = dyn_cast<CallInst>(i);
            if (isa<InlineAsm>(ci->getCalledValue())) continue;
            if (ci->getCalledValue()->getName() == "_safepoint") {
              nSoFar++;
            }
          }
        }
      }
    }
    ArrayType *arrayTy =
        ArrayType::get(IntegerType::get(M.getContext(), 16), nSoFar * 3 + 1);
    vector<uint16_t> vec(nSoFar * 3 + 1, 14);
    vec[0] = nSoFar;

    GlobalVariable *nSoFars_Global_var = new GlobalVariable(
        M, arrayTy, true, GlobalValue::ExternalLinkage,
        ConstantDataArray::get(M.getContext(), *(new ArrayRef<uint16_t>(vec))),
        "nbbs");
    return true;
  }
};
}  // namespace

char AddBasicBlockAddrsPass::ID = 0;
static RegisterPass<AddBasicBlockAddrsPass> X(
    "add-bb-addrs-pass", "reserve an array for bbs begining and end addrs",
    false, false);
