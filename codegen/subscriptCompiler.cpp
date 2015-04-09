#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/ExecutionEngine/Interpreter.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetSelect.h"
using namespace llvm;

enum class NodeType : unsigned  { PLUS, MINUS, TIMES, DIVIDEDBY, VARIABLE, CONSTANT};
struct TreeNode {
	TreeNode* left;
	TreeNode* right;
	NodeType type;
	int64_t constant; //optimize: union for constant and left
};

class SubscriptCompiler {
	BasicBlock* BB;
	Type* dataType;
	Function::arg_iterator curArg;
	unsigned i;
public:
	SubscriptCompiler(BasicBlock *BB, Type* dataType,Function *func)
	: 
	BB(BB),
	dataType(dataType),
	curArg(func->arg_begin()),
	i(0)
	{

	};

	Value* compute( TreeNode* node) {
		switch(node->type) {
			case NodeType::CONSTANT:
				{return ConstantInt::get(dataType, node->constant);}
			case NodeType::VARIABLE:
				{
				//curArg->setName("var"+std::to_string(i++));
				Value* val = curArg++;
				val->setName("var");
				return val;}          
			case NodeType::TIMES:
				{return BinaryOperator::CreateMul(compute(node->left), compute(node->right), "times", BB);
				break;}
			case NodeType::PLUS:
				{return BinaryOperator::CreateAdd(compute(node->left), compute(node->right), "plus", BB);
				break;}
			case NodeType::MINUS:
				{return BinaryOperator::CreateSub(compute(node->left), compute(node->right), "minus", BB);
				break;}
			case NodeType::DIVIDEDBY:
				{return BinaryOperator::CreateSDiv(compute(node->left), compute(node->right), "times", BB);
				break;}
		}
	}
	static size_t numVariables(TreeNode* node) {
		if(node->type == NodeType::CONSTANT) {
			return 0;
		} else if(node->type == NodeType::VARIABLE) {
			return 1;
		} else {
			return numVariables(node->left) + numVariables(node->right);
		}
	}
};
static Function *CreateSubscriptFunction(Module *M, LLVMContext &Context, TreeNode* root) {

	Type* dataType = Type::getInt64Ty(Context);

	std::vector<Type*> parameters(SubscriptCompiler::numVariables(root), Type::getInt64Ty(Context));

	ArrayRef<Type*> arRef(parameters);
	FunctionType * funcType = FunctionType::get(dataType, arRef, false);

	Function *SubCompF = cast<Function>(M->getOrInsertFunction("subComp", funcType));

	// Add a basic block to the function.
	BasicBlock *BB = BasicBlock::Create(Context, "EntryBlock", SubCompF);


	SubscriptCompiler subComp(BB, dataType, SubCompF);
	Value* result = subComp.compute(root);

	// Create the return instruction and add it to the basic block
	ReturnInst::Create(Context, result, BB);

	return SubCompF;
}


int main(int argc, char **argv) {
	int n = argc > 1 ? atol(argv[1]) : 24;

	InitializeNativeTarget();
	LLVMContext Context;

	// Create some module to put our function into it.
	OwningPtr<Module> M(new Module("test", Context));


	TreeNode v1;
	v1.type = NodeType::VARIABLE;

	TreeNode c1;
	c1.type = NodeType::CONSTANT;
	c1.constant = 7;

	TreeNode times;
	times.type = NodeType::TIMES;
	times.left = &v1;
	times.right = &c1;

	TreeNode v2;
	v2.type = NodeType::VARIABLE;

	TreeNode c2;
	c2.type = NodeType::CONSTANT;
	c2.constant = 4;

	TreeNode minus;
	minus.type = NodeType::MINUS;
	minus.left = &v2;
	minus.right = &c2;

	TreeNode plus;
	plus.type = NodeType::PLUS;
	plus.left = &times;
	plus.right = &minus;

	
	//if (n<30) ty=Type::getInt32Ty(Context); // decide on type at runtime
	Function *SubCompF = CreateSubscriptFunction(M.get(), Context, &plus);

	// Now we going to create JIT
	//auto engine=EngineKind::Interpreter;
	auto engine=EngineKind::JIT;
	std::string errStr;
	ExecutionEngine *EE =
		EngineBuilder(M.get())
		.setErrorStr(&errStr)
		.setEngineKind(engine)
		.create();

	if (!EE) {
		errs() << argv[0] << ": Failed to construct ExecutionEngine: " << errStr
					 << "\n";
		return 1;
	}

	errs() << "verifying... ";
	if (verifyModule(*M)) {
		errs() << argv[0] << ": Error constructing function!\n";
		return 1;
	}

	errs() << "OK\n";
	errs() << "We just constructed this LLVM module:\n\n---------\n" << *M;
	errs() << "---------\nstarting subscriptCompiler( 3, 7 ) with "<<(engine==EngineKind::Interpreter?"interpreter":(engine==EngineKind::JIT)?"JIT":"???")<<" ...\n";

	// Call the Fibonacci function with argument n:
	std::vector<GenericValue> Args(2);
	Args[0].IntVal = APInt(64, 3, true);
	Args[1].IntVal = APInt(64, 7, true);
	GenericValue GV = EE->runFunction(SubCompF, Args);

	// import result of execution
	outs() << "Result: " << GV.IntVal << "\n";

	return 0;
}

