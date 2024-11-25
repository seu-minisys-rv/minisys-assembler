// instruction/InstructionU.hpp

#ifndef INSTRUCTIONU_HPP
#define INSTRUCTIONU_HPP

#include "Instruction.hpp"
#include <unordered_map>
#include <cstdint>

class InstructionU : public Instruction
{
public:
  InstructionU(const std::string &line);

  std::vector<uint32_t> encode(const SymbolTable &symbolTable, RelocationTable &relocationTable, std::unordered_map<std::string, Section> &sectionTable, uint32_t currentAddress, std::string currentSecName) override;

private:
  void parseOperands();

  uint32_t rd; // 目标寄存器
  int32_t imm; // 20 位立即数

  // 操作码映射表，用于存储 U 型指令的 opcode
  static const std::unordered_map<std::string, uint32_t> opcodeMap;
};

#endif // INSTRUCTIONU_HPP
