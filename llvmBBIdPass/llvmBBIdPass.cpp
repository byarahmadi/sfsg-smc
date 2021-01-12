/*
 * This file is part of sfsg.
 *
 * Author: Bahram Yarahmadi
 * Copyright (c) 2020  Inria
 *
 */

#include <fstream>
#include "llvm/Analysis/CFG.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
using namespace std;
using namespace llvm;

//

namespace {
class BBIdPass : public FunctionPass {
 public:
  static char ID;
  SmallVector<BasicBlock *, 100> addedBBs;  // Just BBs realted to increment hl
                                            // variable or maybe for dec nst

  BBIdPass() : FunctionPass(ID) {}
  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.setPreservesAll();
  }

  bool searchInAddedBBs(const BasicBlock *bb) {
    for (unsigned int i = 0; i < addedBBs.size(); i++) {
      if (addedBBs[i] == bb) return true;
    }
    return false;
  }

  GlobalVariable *getGVar(Module *m, const char *vname) {
    GlobalVariable *gVar;

    for (Module::global_iterator i = m->global_begin(); i != m->global_end();
         ++i) {
      const StringRef s = i->getName();
      if (!s.compare(vname)) {
        gVar = &*i;
      }
    }

    return gVar;
  }
  vector<BasicBlock *> getLoopHeads(Function &F) {
    vector<BasicBlock *> heads;
    for (Function::iterator itrBB = F.begin(); itrBB != F.end(); ++itrBB) {
      if (isLoopHead(&*itrBB)) heads.push_back(&*itrBB);
    }

    return heads;
  }
  Loop *getLoopInSubLoops(BasicBlock *head, Loop *L) {
    BasicBlock *lh = L->getHeader();
    if (head == lh) return L;
    vector<Loop *> subLoops = L->getSubLoops();
    Loop *foundedLoop = NULL;
    for (unsigned int i = 0; i < subLoops.size(); i++) {
      foundedLoop = getLoopInSubLoops(head, subLoops[i]);
      if (foundedLoop != NULL) break;
    }
    return foundedLoop;
  }
  Loop *getLoop(BasicBlock *head) {
    LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    for (LoopInfo::iterator itrLoop = LI.begin(); itrLoop != LI.end();
         ++itrLoop) {
      Loop *L = *itrLoop;
      BasicBlock *lh = L->getHeader();
      if (head == lh) return L;
      Loop *foundedLoop = NULL;
      bool flag = false;
      vector<Loop *> subLoops = L->getSubLoops();
      for (unsigned int i = 0; i < subLoops.size(); i++) {
        foundedLoop = getLoopInSubLoops(head, subLoops[i]);
        if (foundedLoop != NULL) return foundedLoop;
      }
    }
    return NULL;
  }

  unsigned int getNumberOfLoops() {
    LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    for (LoopInfo::iterator itrLoop = LI.begin(); itrLoop != LI.end();
         ++itrLoop) {
    }
  }
  bool isInLoop(BasicBlock *bb, Loop *l) {
    Loop *L = l;
    for (Loop::block_iterator itrBB = L->block_begin(); itrBB != L->block_end();
         ++itrBB) {
      if (*itrBB == bb) return true;
    }

    return false;
  }
  bool isLoopHeadInSubLoop(BasicBlock *bb, Loop *l) {
    BasicBlock *lh = l->getHeader();
    if (bb == lh) return true;
    bool isHead = false;
    vector<Loop *> subLoops = l->getSubLoops();
    for (unsigned int i = 0; i < subLoops.size(); i++) {
      isHead = isLoopHeadInSubLoop(bb, subLoops[i]);
      if (isHead == true) return true;
    }
    return false;
  }
  bool isLoopHead(BasicBlock *bb) {
    LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    for (LoopInfo::iterator itrLoop = LI.begin(); itrLoop != LI.end();
         ++itrLoop) {
      Loop *L = *itrLoop;
      BasicBlock *lh = L->getHeader();
      if (bb == lh) return true;

      vector<Loop *> subLoops = L->getSubLoops();
      bool isHead = false;
      for (unsigned int i = 0; i < subLoops.size(); i++) {
        isHead = isLoopHeadInSubLoop(bb, subLoops[i]);
        if (isHead == true) return true;
      }
    }
    return false;
  }

  void printDominatorOf(BasicBlock *bb) {
    DominatorTree &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
    for (auto node = GraphTraits<DominatorTree *>::nodes_begin(&DT);
         node != GraphTraits<DominatorTree *>::nodes_end(&DT); ++node) {
      BasicBlock *BB = node->getBlock();
      BB->printAsOperand(errs(), false);
    }
  }

  virtual bool runOnFunction(Function &F) {
    LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    DominatorTree &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();

    vector<BasicBlock *> heads = getLoopHeads(F);

    Module *m = F.begin()->getModule();

    for (unsigned int i = 0; i < heads.size(); i++) {
      Loop *L = getLoop(heads[i]);
      SmallVector<BasicBlock *, 10> ExitingBlocks;
      L->getExitingBlocks(ExitingBlocks);
      for (int i = 0; i < ExitingBlocks.size(); i++) {
        for (auto it = succ_begin(ExitingBlocks[i]);
             it != succ_end(ExitingBlocks[i]); ++it) {
          BasicBlock *succBB = *it;
          if (!isInLoop(succBB, L)) {
            const TerminatorInst *TI = ExitingBlocks[i]->getTerminator();
            int successors = TI->getNumSuccessors();
            bool isCritical = false;
            for (int j = 0; j < successors; j++) {
              BasicBlock *TargetBB = TI->getSuccessor(j);
              if (TargetBB == succBB) {
                if (llvm::isCriticalEdge(TI, j)) {
                  isCritical = true;
                  break;
                }
              }
            }
            BasicBlock *newBB = SplitEdge(ExitingBlocks[i], succBB, &DT);
            BasicBlock *BBA;
            if (isCritical)
              BBA = newBB;
            else
              BBA = succBB;
            addedBBs.push_back(BBA);
            BasicBlock::iterator itrInst = BBA->getFirstInsertionPt();
            IRBuilder<> builder(BBA);
            builder.SetInsertPoint(&*itrInst);
            LoadInst *load = builder.CreateLoad(getGVar(m, "nst"));
            Value *dec = builder.CreateSub(load, builder.getInt16(1));
            StoreInst *store = builder.CreateStore(dec, getGVar(m, "nst"));
          }
        }
      }
    }

    for (unsigned int i = 0; i < heads.size(); i++) {
      Loop *L = getLoop(heads[i]);
      for (auto it = pred_begin(heads[i]); it != pred_end(heads[i]); ++it) {
        BasicBlock *pred = *it;
        pred->print(llvm::errs(), nullptr);
        if (pred != (heads[i]) && DT.dominates(pred, heads[i])) {
          BasicBlock *newBB;
          if (pred->getInstList().size() == 1)
            newBB = pred;
          else
            newBB = SplitEdge(pred, heads[i], &DT);
          BasicBlock::iterator itrInst = newBB->getFirstInsertionPt();

          IRBuilder<> builder(newBB);
          builder.SetInsertPoint(&*itrInst);
          LoadInst *load = builder.CreateLoad(getGVar(m, "nst"));
          Value *inc = builder.CreateAdd(builder.getInt16(1), load);
          StoreInst *store = builder.CreateStore(inc, getGVar(m, "nst"));
          {
            LoadInst *load = builder.CreateLoad(getGVar(m, "firstTimeInLoop"));
            Value *inc = builder.CreateAdd(builder.getInt16(1), load);
            StoreInst *store =
                builder.CreateStore(inc, getGVar(m, "firstTimeInLoop"));
          }
        }
      }
    }

    for (unsigned int i = 0; i < heads.size(); i++) {
      Loop *L = getLoop(heads[i]);
      SmallVector<BasicBlock *, 10> LoopLatches;
      L->getLoopLatches(LoopLatches);
      if (LoopLatches.size() == 0) {
        for (auto it = pred_begin(heads[i]); it != pred_end(heads[i]); ++it) {
          BasicBlock *pred = *it;
          pred->print(llvm::errs(), nullptr);
          if (pred != (heads[i]) && !DT.dominates(pred, heads[i])) {
            BasicBlock *newBB = SplitEdge(pred, heads[i], &DT);
            BasicBlock::iterator itrInst = newBB->getFirstInsertionPt();
            IRBuilder<> builder(newBB);
            builder.SetInsertPoint(&*itrInst);
            LoadInst *load = builder.CreateLoad(getGVar(m, "lh"));
            Value *inc = builder.CreateAdd(builder.getInt16(1), load);
            StoreInst *store = builder.CreateStore(inc, getGVar(m, "lh"));
            addedBBs.push_back(newBB);
          }
        }
      }
      for (BasicBlock *BB : LoopLatches) {
        BasicBlock *newBB = SplitEdge(BB, heads[i], &DT);
        BasicBlock::iterator itrInst = newBB->getFirstInsertionPt();
        IRBuilder<> builder(newBB);
        builder.SetInsertPoint(&*itrInst);
        LoadInst *load = builder.CreateLoad(getGVar(m, "lh"));
        Value *inc = builder.CreateAdd(builder.getInt16(1), load);
        StoreInst *store = builder.CreateStore(inc, getGVar(m, "lh"));
        addedBBs.push_back(newBB);
      }
    }

    FunctionType *fnType =
        FunctionType::get(Type::getVoidTy(m->getContext()), false);
    const char *spFunctionName = "_safepoint";

    Constant *hookSpFn = m->getOrInsertFunction(spFunctionName, fnType);
    Function *spFunctionPtr = cast<Function>(hookSpFn);

    int nSoFar = 0;
    for (Function::iterator itrBB = F.begin(); itrBB != F.end(); ++itrBB) {
      if (!searchInAddedBBs(&*itrBB)) {
        nSoFar++;
        BasicBlock::iterator itrInst = itrBB->getFirstInsertionPt();

        Instruction *callInst = CallInst::Create(spFunctionPtr, "");
        itrBB->getInstList().insert(itrInst, callInst);
      }
    }

    return true;
  }
};
}  // namespace
char BBIdPass::ID = 0;

static RegisterPass<BBIdPass> X("bb-id-pass", "Basic Block Identification Pass",
                                false, false);
