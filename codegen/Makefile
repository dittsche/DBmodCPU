all: fibonacci subscriptCompiler

fibonacci: fibonacci.cpp
	clang++ -g -std=c++11 fibonacci.cpp `llvm-config-3.4 --cxxflags --ldflags --libs jit interpreter nativecodegen` -o fibonacci

subscriptCompiler: subscriptCompiler.cpp
	clang++  -g -std=c++11 subscriptCompiler.cpp `llvm-config-3.4 --cxxflags --ldflags --libs jit interpreter nativecodegen` -o subscriptCompiler
