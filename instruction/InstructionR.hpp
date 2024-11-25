// instruction/InstructionR.hpp

#ifndef INSTRUCTIONR_HPP
#define INSTRUCTIONR_HPP

#include "Instruction.hpp"
#include <unordered_map>
#include <cstdint>

class InstructionR : public Instruction
{
public:
  // 构造函数，接受指令行字符串
  InstructionR(const std::string &line);

  // 实现基类的 encode 方法，返回32位机器码
  std::vector<uint32_t> encode(const SymbolTable &symbolTable, RelocationTable &relocationTable, std::unordered_map<std::string, Section> &sectionTable, uint32_t currentAddress, std::string currentSecName) override;

protected:
  // 解析操作数，覆盖基类方法
  void parseOperands();

private:
  uint32_t rd;  // 目标寄存器
  uint32_t rs1; // 源寄存器1
  uint32_t rs2; // 源寄存器2

  // R 型指令的 funct3 和 funct7 映射表
  static const std::unordered_map<std::string, uint32_t> funct3Map;
  static const std::unordered_map<std::string, uint32_t> funct7Map;
};

#endif // INSTRUCTIONR_HPP
