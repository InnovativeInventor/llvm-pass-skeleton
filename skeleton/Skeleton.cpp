#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/Cloning.h"

using namespace llvm;

namespace {

struct SkeletonPass : public PassInfoMixin<SkeletonPass> {
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
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
                  InlineFunctionInfo info;
                  InlineResult res = InlineFunction(*inst, info);
                  llvm::outs() << "Inline attempted, success: " << res << "\n";
                }
              }
            }
          }
        }
        return PreservedAnalyses::none();
    };
};

}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return {
        .APIVersion = LLVM_PLUGIN_API_VERSION,
        .PluginName = "Skeleton pass",
        .PluginVersion = "v0.1",
        .RegisterPassBuilderCallbacks = [](PassBuilder &PB) {
            PB.registerPipelineStartEPCallback(
                [](ModulePassManager &MPM, OptimizationLevel Level) {
                    MPM.addPass(SkeletonPass());
                });
        }
    };
}
