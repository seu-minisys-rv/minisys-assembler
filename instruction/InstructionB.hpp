// instruction/InstructionB.hpp

#ifndef INSTRUCTIONB_HPP
#define INSTRUCTIONB_HPP

#include "Instruction.hpp"
#include <unordered_map>
#include <cstdint>

class InstructionB : public Instruction
{
public:
  // 构造函数，接受指令行字符串
  InstructionB(const std::string &line);

  // 实现基类的 encode 方法，返回32位机器码
  std::vector<uint32_t> encode(const SymbolTable &symbolTable, RelocationTable &relocationTable, std::unordered_map<std::string, Section> &sectionTable, uint32_t currentAddress, std::string currentSecName) override;

private:
  // 解析操作数的方法
  void parseOperands();

  uint32_t rs1; // 源寄存器1
  uint32_t rs2; // 源寄存器2
  int32_t imm;  // 分支偏移量（相对于当前地址）

  // B 型指令的 funct3 映射表
  static const std::unordered_map<std::string, uint32_t> funct3Map;
};

#endif // INSTRUCTIONB_HPP
