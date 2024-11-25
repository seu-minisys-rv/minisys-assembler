#ifndef INSTRUCTIONP_HPP
#define INSTRUCTIONP_HPP

#include "Instruction.hpp"
#include <unordered_map>
#include <cstdint>

class InstructionP : public Instruction
{
public:
  InstructionP(const std::string &line);

  std::vector<uint32_t> encode(const SymbolTable &symbolTable, RelocationTable &relocationTable, std::unordered_map<std::string, Section> &sectionTable, uint32_t currentAddress, std::string currentSecName) override;

private:
  void parseOperands();
  std::string expandedOpcode;
  std::vector<std::string> expandedOperands;

  uint32_t rd;
  uint32_t rs1;
  int32_t imm;
};

#endif // INSTRUCTIONP_HPP
