#ifndef INSTRUCTIONI_HPP
#define INSTRUCTIONI_HPP

#include "Instruction.hpp"
#include <unordered_map>
#include <cstdint>

class InstructionI : public Instruction
{
public:
  InstructionI(const std::string &line);

  std::vector<uint32_t> encode(const SymbolTable &symbolTable, RelocationTable &relocationTable, std::unordered_map<std::string, Section> &sectionTable, uint32_t currentAddress, std::string currentSecName) override;

private:
  void parseOperands();

  uint32_t rd;
  uint32_t rs1;
  int32_t imm;

  // 指令操作码、功能码映射表
  static const std::unordered_map<std::string, uint32_t> opcodeMap;
  static const std::unordered_map<std::string, uint32_t> funct3Map;
};

#endif // INSTRUCTIONI_HPP
