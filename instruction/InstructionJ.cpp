// instruction/InstructionJ.cpp

#include "InstructionJ.hpp"
#include "../utils/Utils.hpp"
#include <stdexcept>

// J 型指令的 opcode 映射表
const std::unordered_map<std::string, uint32_t> InstructionJ::opcodeMap = {
    {"jal", 0x6F}};

InstructionJ::InstructionJ(const std::string &line)
{
  parseLine(line);
  parseOperands();
}

void InstructionJ::parseOperands()
{
  if (operands.size() != 2)
  {
    throw std::runtime_error("Invalid J-type instruction: " + opcode);
  }

  // 解析 rd
  rd = Utils::getRegisterNumber(operands[0]);

  // 解析 label 或者 '%' 表达式
  std::string labelOrExpr = operands[1];

  if (labelOrExpr[0] == '%')
  {
    // 处理 '%' 表达式
    size_t funcEnd = labelOrExpr.find('(');
    if (funcEnd == std::string::npos)
    {
      funcEnd = labelOrExpr.length();
    }
    immFunction = labelOrExpr.substr(1, funcEnd - 1);
    immSymbol = labelOrExpr.substr(funcEnd);
    label.clear();
  }
  else
  {
    // 直接是标签
    label = labelOrExpr;
    immFunction.clear();
    immSymbol.clear();
  }
}

std::vector<uint32_t> InstructionJ::encode(
    const SymbolTable &symbolTable,
    RelocationTable &relocationTable,
    std::unordered_map<std::string, Section> &sectionTable,
    uint32_t currentAddress,
    std::string currentSecName)
{
  // 获取操作码
  auto opcodeIt = opcodeMap.find(opcode);
  if (opcodeIt == opcodeMap.end())
  {
    throw std::runtime_error("Unsupported J-type instruction: " + opcode);
  }
  uint32_t opcodeVal = opcodeIt->second;

  int32_t imm = 0;

  if (!immFunction.empty())
  {
    // 处理 '%' 表达式
    if (symbolTable.hasSymbol(immSymbol))
    {
      const Symbol &symbol = symbolTable.getSymbol(immSymbol);
      uint32_t symbolAddress = symbol.getGAddress();
      imm = static_cast<int32_t>(symbolAddress) - static_cast<int32_t>(currentAddress);
    }
    else
    {
      // 符号不存在，添加重定位条目
      imm = 0;
      relocationTable.addRelocation(
          currentSecName,
          currentAddress,
          immSymbol,
          RelocationType::R_RISCV_JAL);
    }
  }
  else if (!label.empty())
  {
    // 处理标签
    if (symbolTable.hasSymbol(label))
    {
      const Symbol &symbol = symbolTable.getSymbol(label);
      uint32_t labelAddress = symbol.getGAddress();
      imm = static_cast<int32_t>(labelAddress) - static_cast<int32_t>(currentAddress);
    }
    else
    {
      // 符号不存在，添加重定位条目
      imm = 0;
      relocationTable.addRelocation(
          currentSecName,
          currentAddress,
          label,
          RelocationType::R_RISCV_JAL);
    }
  }
  else
  {
    throw std::runtime_error("Invalid operand in J-type instruction.");
  }

  // 检查偏移量是否对齐到 4 字节
  if (imm % 4 != 0)
  {
    throw std::runtime_error("Jump target address must be 4-byte aligned.");
  }

  // 检查偏移量范围
  if (imm < -(1 << 20) || imm >= (1 << 20))
  {
    throw std::runtime_error("Jump offset out of range for J-type instruction: " + std::to_string(imm));
  }

  // 提取立即数位
  uint32_t imm_enc = ((imm & 0xFFF00000) >> 12) | ((imm & 0x000FF000) >> 12);

  uint32_t imm20 = (imm >> 20) & 0x1;     // imm[20]
  uint32_t imm10_1 = (imm >> 1) & 0x3FF;  // imm[10:1]
  uint32_t imm11 = (imm >> 11) & 0x1;     // imm[11]
  uint32_t imm19_12 = (imm >> 12) & 0xFF; // imm[19:12]

  // 组装指令
  uint32_t instruction = 0;
  instruction |= (imm20 << 31);
  instruction |= (imm19_12 << 12);
  instruction |= (imm11 << 20);
  instruction |= (imm10_1 << 21);
  instruction |= ((rd & 0x1F) << 7);
  instruction |= (opcodeVal & 0x7F);

  return {instruction};
}
