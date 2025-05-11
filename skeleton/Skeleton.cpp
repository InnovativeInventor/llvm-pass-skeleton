#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Support/RandomNumberGenerator.h"

using namespace llvm;

namespace {

/*
 * A super simple inliner fuzzer.
 */

struct SkeletonPass : public PassInfoMixin<SkeletonPass> {
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
      auto counter = 0;
      auto rng = M.createRNG("test");
        for (auto &F : M) {
          for (auto *caller : F.users()) {
            if (auto* inst = dyn_cast<CallBase>(caller)) {
              if (Function *func = inst->getCalledFunction()) {
                // To prevent segfaults, conditions are a subset from:
                // llvm/lib/Transforms/IPO/AlwaysInliner.cpp
                if (func == &F 
                    && isInlineViable(*func).isSuccess() 
                    && !func->isPresplitCoroutine()
                    && !func->isDeclaration() 
                    ) {
                  if ((*rng)() > (*rng)()) {
                    counter++;
                    InlineFunctionInfo info;
                    InlineResult res = InlineFunction(*inst, info);
                    llvm::outs() << "Inline attempted, success?: " << res.isSuccess() << "\n";
                  } else {
                    llvm::outs() << "Inline ignored.\n";
                  }
                }
              }
            }
          }
        }
        llvm::outs() << "Total inlines available: " << counter << "\n";
        return PreservedAnalyses::none();
    };
};

}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return {
        .APIVersion = LLVM_PLUGIN_API_VERSION,
        .PluginName = "Inliner fuzzer",
        .PluginVersion = "v0.1",
        .RegisterPassBuilderCallbacks = [](PassBuilder &PB) {
            PB.registerPipelineStartEPCallback(
                [](ModulePassManager &MPM, OptimizationLevel Level) {
                    MPM.addPass(SkeletonPass());
                });
        }
    };
}
