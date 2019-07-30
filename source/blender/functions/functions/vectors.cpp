#include "vectors.hpp"
#include "FN_types.hpp"

#include "FN_tuple_call.hpp"
#include "FN_llvm.hpp"
#include "BLI_lazy_init.hpp"
#include "BLI_math.h"

namespace FN {
namespace Functions {

using namespace Types;

class CombineVectorGen : public LLVMBuildIRBody {
  void build_ir(CodeBuilder &builder,
                CodeInterface &interface,
                const BuildIRSettings &UNUSED(settings)) const override
  {
    llvm::Type *vector_ty = get_llvm_type(GET_TYPE_float3(), builder.getContext());

    llvm::Value *vector = builder.getUndef(vector_ty);
    vector = builder.CreateInsertElement(vector, interface.get_input(0), 0);
    vector = builder.CreateInsertElement(vector, interface.get_input(1), 1);
    vector = builder.CreateInsertElement(vector, interface.get_input(2), 2);
    interface.set_output(0, vector);
  }
};

BLI_LAZY_INIT(SharedFunction, GET_FN_combine_vector)
{
  FunctionBuilder builder;
  builder.add_input("X", GET_TYPE_float());
  builder.add_input("Y", GET_TYPE_float());
  builder.add_input("Z", GET_TYPE_float());
  builder.add_output("Vector", GET_TYPE_float3());

  auto fn = builder.build("Combine Vector");
  fn->add_body<CombineVectorGen>();
  return fn;
}

class SeparateVector : public LLVMBuildIRBody {
  void build_ir(CodeBuilder &builder,
                CodeInterface &interface,
                const BuildIRSettings &UNUSED(settings)) const override
  {
    llvm::Value *vector = interface.get_input(0);
    interface.set_output(0, builder.CreateExtractElement(vector, 0));
    interface.set_output(1, builder.CreateExtractElement(vector, 1));
    interface.set_output(2, builder.CreateExtractElement(vector, 2));
  }
};

BLI_LAZY_INIT(SharedFunction, GET_FN_separate_vector)
{
  FunctionBuilder builder;
  builder.add_input("Vector", GET_TYPE_float3());
  builder.add_output("X", GET_TYPE_float());
  builder.add_output("Y", GET_TYPE_float());
  builder.add_output("Z", GET_TYPE_float());

  auto fn = builder.build("Separate Vector");
  fn->add_body<SeparateVector>();
  return fn;
}

class VectorDistance : public TupleCallBody {
  void call(Tuple &fn_in, Tuple &fn_out, ExecutionContext &UNUSED(ctx)) const override
  {
    float3 a = fn_in.get<float3>(0);
    float3 b = fn_in.get<float3>(1);
    fn_out.set<float>(0, float3::distance(a, b));
  }
};

BLI_LAZY_INIT(SharedFunction, GET_FN_vector_distance)
{
  FunctionBuilder builder;
  builder.add_input("A", GET_TYPE_float3());
  builder.add_input("B", GET_TYPE_float3());
  builder.add_output("Distance", GET_TYPE_float());

  auto fn = builder.build("Vector Distance");
  fn->add_body<VectorDistance>();
  return fn;
}

static SharedFunction get_math_function__two_inputs(std::string name)
{
  FunctionBuilder builder;
  builder.add_input("A", GET_TYPE_float3());
  builder.add_input("B", GET_TYPE_float3());
  builder.add_output("Result", GET_TYPE_float3());
  return builder.build(name);
}

class AddVectors : public TupleCallBody {
  void call(Tuple &fn_in, Tuple &fn_out, ExecutionContext &UNUSED(ctx)) const override
  {
    float3 a = fn_in.get<float3>(0);
    float3 b = fn_in.get<float3>(1);
    fn_out.set<float3>(0, a + b);
  }
};

class AddVectorsGen : public LLVMBuildIRBody {
  void build_ir(CodeBuilder &builder,
                CodeInterface &interface,
                const BuildIRSettings &UNUSED(settings)) const override
  {
    llvm::Value *a = interface.get_input(0);
    llvm::Value *b = interface.get_input(1);
    llvm::Value *result = builder.CreateFAdd(a, b);
    interface.set_output(0, result);
  }
};

BLI_LAZY_INIT(SharedFunction, GET_FN_add_vectors)
{
  auto fn = get_math_function__two_inputs("Add Vectors");
  fn->add_body<AddVectors>();
  fn->add_body<AddVectorsGen>();
  return fn;
}

}  // namespace Functions
}  // namespace FN