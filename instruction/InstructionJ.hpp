// instruction/InstructionJ.hpp

#ifndef INSTRUCTIONJ_HPP
#define INSTRUCTIONJ_HPP

#include "Instruction.hpp"
#include <unordered_map>
#include <cstdint>

class InstructionJ : public Instruction
{
public:
  InstructionJ(const std::string &line);

  std::vector<uint32_t> encode(const SymbolTable &symbolTable, RelocationTable &relocationTable, std::unordered_map<std::string, Section> &sectionTable, uint32_t currentAddress, std::string currentSecName) override;

private:
  void parseOperands();

  uint32_t rd; // 目标寄存器
  int32_t imm; // 跳转的立即数（偏移量）

  // 操作码映射表，用于存储 J 型指令的 opcode
  static const std::unordered_map<std::string, uint32_t> opcodeMap;
};

#endif // INSTRUCTIONJ_HPP
