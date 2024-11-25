#ifndef INSTRUCTIONM_HPP
#define INSTRUCTIONM_HPP

#include "Instruction.hpp"
#include <unordered_map>
#include <cstdint>

class InstructionM : public Instruction
{
public:
  InstructionM(const std::string &line);

  std::vector<uint32_t> encode(const SymbolTable &symbolTable, RelocationTable &relocationTable, std::unordered_map<std::string, Section> &sectionTable, uint32_t currentAddress, std::string currentSecName) override;

private:
  void parseOperands();

  uint32_t rd;
  uint32_t rs1;
  uint32_t rs2;

  // 操作码和功能码映射表
  static const uint32_t opcodeVal;
  static const std::unordered_map<std::string, uint32_t> funct3Map;
  static const std::unordered_map<std::string, uint32_t> funct7Map;
};

#endif // INSTRUCTIONM_HPP
