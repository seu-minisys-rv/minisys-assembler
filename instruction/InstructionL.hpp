// instruction/InstructionL.hpp

#ifndef INSTRUCTIONL_HPP
#define INSTRUCTIONL_HPP

#include "Instruction.hpp"
#include <unordered_map>
#include <cstdint>

class InstructionL : public Instruction
{
public:
  InstructionL(const std::string &line);

  std::vector<uint32_t> encode(const SymbolTable &symbolTable, RelocationTable &relocationTable, std::unordered_map<std::string, Section> &sectionTable, uint32_t currentAddress, std::string currentSecName) override;

protected:
  void parseOperands();

private:
  uint32_t rd;
  uint32_t rs1;
  int32_t imm;

  // Load 指令的 opcode 和 funct3 映射表
  static const std::unordered_map<std::string, uint32_t> opcodeMap;
  static const std::unordered_map<std::string, uint32_t> funct3Map;
};

#endif // INSTRUCTIONL_HPP
