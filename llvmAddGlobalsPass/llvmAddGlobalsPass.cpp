/*
 * This file is part of sfsg.
 *
 * Author: Bahram Yarahmadi
 * Copyright (c) 2020  Inria
 *
 */

#include <fstream>
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
using namespace std;
using namespace llvm;

namespace {
class AddGlobalsPass : public ModulePass {
 public:
  static char ID;

  AddGlobalsPass() : ModulePass(ID) {}
  virtual bool runOnModule(Module &M) {
    unsigned tsbytes = 0;
    DataLayout TD = M.getDataLayout();
    for (Module::const_global_iterator i = M.global_begin();
         i != M.global_end(); ++i) {
      Type *T = i->getType()->getElementType();
      const unsigned int bytes = TD.getTypeAllocSize(T);
      const StringRef s = i->getName();
      tsbytes += bytes;
    }
    Type *Int16 = Type::getInt16Ty(M.getContext());
    Constant *TotalSizeInBytes = ConstantInt::get(Int16, tsbytes);
    GlobalVariable *GlobalAllocSize =
        new GlobalVariable(M, Int16, true, GlobalValue::ExternalLinkage,
                           TotalSizeInBytes, "GlobalAllocSize");

    return true;
  }
};
}  // namespace
char AddGlobalsPass::ID = 0;

static RegisterPass<AddGlobalsPass> X("add-globals-pass", "Add globals Pass",
                                      false, false);
