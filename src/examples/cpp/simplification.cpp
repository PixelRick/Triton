/*
** Output
**
**  400017: xor rax, rax
**          SymExpr 0: ref!0 = (_ bv0 64) ; XOR operation
*/


#include <iostream>
#include <triton/api.hpp>
#include <triton/x86Specifications.hpp>

using namespace triton;
using namespace triton::arch;


struct op {
  unsigned int    addr;
  unsigned char*  inst;
  unsigned int    size;
};

struct op trace[] = {
  {0x400017, (unsigned char *)"\x48\x31\xc0", 3}, /* xor rax, rax */
  {0x0,      nullptr,                         0}
};


/* if (bvxor x x) -> (_ bv0 x_size) */
ast::SharedAbstractNode xor_simplification(API&, const ast::SharedAbstractNode& snode) {
  ast::AbstractNode* node = snode.get();

  if (node->getType() == ast::ZX_NODE) {
    node = node->getChildren()[1].get();
  }

  if (node->getType() == ast::BVXOR_NODE) {
    if (node->getChildren()[0]->equalTo(node->getChildren()[1]))
      return node->getContext().bv(0, node->getBitvectorSize());
  }

  return snode;
}


int main(int ac, const char **av) {

  triton::API api;
  /* Set the arch */
  api.setArchitecture(ARCH_X86_64);

  /* Record a simplification callback */
  api.addCallback(xor_simplification);

  /* optional - Update register state */
  api.setConcreteRegisterValue(api.registers.x86_rax, 12345);

  for (unsigned int i = 0; trace[i].inst; i++) {
    /* Build an instruction */
    Instruction inst;

    /* Setup opcode */
    inst.setOpcode(trace[i].inst, trace[i].size);

    /* optional - Setup address */
    inst.setAddress(trace[i].addr);

    /* Process everything */
    api.processing(inst);

    /* Display all symbolic expression of the instruction */
    std::cout << inst << std::endl;
    for (unsigned int exp_index = 0; exp_index != inst.symbolicExpressions.size(); exp_index++) {
      auto expr = inst.symbolicExpressions[exp_index];
      std::cout << "\tSymExpr " << exp_index << ": " << expr << std::endl;
    }

  }

  return 0;
}

