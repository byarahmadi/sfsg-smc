/*
 * This file is part of sfsg.
 *
 * Author: Bahram Yarahmadi
 * Copyright (c) 2020  Inria
 *
 */

#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

namespace {
class RenameMainPass : public FunctionPass {
 public:
  static char ID;
  RenameMainPass() : FunctionPass(ID) {}
  virtual bool runOnFunction(Function& F) {
    const std::string& moduleId = F.getParent()->getModuleIdentifier();
    if (F.getName() != "main") return false;
    const std::string& newName = "_old_main";
    F.setName(newName);

    return true;
  }
};
}  // namespace
char RenameMainPass::ID = 0;
static RegisterPass<RenameMainPass> X("rename-main-pass", "rename the main",
                                      "false", "false");
