cd build
LLVM_DIR=$(brew --prefix llvm)/lib/cmake/llvm cmake ..
make clean
make -j 12
cd ..
$(brew --prefix llvm)/bin/clang -fpass-plugin=build/skeleton/SkeletonPass.dylib -emit-llvm -S -o - a.c
$(brew --prefix llvm)/bin/clang -fpass-plugin=build/skeleton/SkeletonPass.dylib a.c
