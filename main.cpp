#include "trunk/Assembler.hpp"

int main()
{
  Assembler assembler;
  assembler.assemble("clang.s", "clang.o", false);
  return 0;
}