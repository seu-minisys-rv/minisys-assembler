// instruction/InstructionS.hpp

#ifndef INSTRUCTIONS_HPP
#define INSTRUCTIONS_HPP

#include "Instruction.hpp"
#include <unordered_map>
#include <cstdint>

class InstructionS : public Instruction
{
public:
  // 构造函数，接受指令行字符串
  InstructionS(const std::string &line);

  // 实现基类的 encode 方法，返回32位机器码
  std::vector<uint32_t> encode(const SymbolTable &symbolTable, RelocationTable &relocationTable, std::unordered_map<std::string, Section> &sectionTable, uint32_t currentAddress, std::string currentSecName) override;

protected:
  // 解析操作数
  void parseOperands();

private:
  uint32_t rs2; // 源寄存器2
  uint32_t rs1; // 源寄存器1（基址寄存器）
  int32_t imm;  // 偏移量

  // S 型指令的 funct3 映射表
  static const std::unordered_map<std::string, uint32_t> funct3Map;
};

#endif // INSTRUCTIONS_HPP
