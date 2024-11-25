// instruction/InstructionI.cpp

#include "InstructionI.hpp"
#include "../utils/Utils.hpp"
#include <stdexcept>

// 添加支持的操作码和 funct3 映射
const std::unordered_map<std::string, uint32_t> InstructionI::opcodeMap = {
    {"addi", 0x13}, {"ori", 0x13}, {"andi", 0x13}, {"xori", 0x13}, {"slli", 0x13}, {"srli", 0x13}, {"jalr", 0x67}};

const std::unordered_map<std::string, uint32_t> InstructionI::funct3Map = {
    {"addi", 0x0}, {"ori", 0x6}, {"andi", 0x7}, {"xori", 0x4}, {"slli", 0x1}, {"srli", 0x5}, {"jalr", 0x0}};

InstructionI::InstructionI(const std::string &line)
{
  parseLine(line);
  parseOperands();
}

void InstructionI::parseOperands()
{
  if (opcode == "jalr")
  {
    if (operands.size() != 2)
    {
      throw std::runtime_error("Invalid JALR instruction format: " + opcode);
    }

    // 解析 rd
    rd = Utils::getRegisterNumber(operands[0]);

    // 解析 offset(rs1)
    std::string addr = operands[1];
    size_t pos1 = addr.find('(');
    size_t pos2 = addr.find(')');
    if (pos1 == std::string::npos || pos2 == std::string::npos || pos2 <= pos1)
    {
      throw std::runtime_error("Invalid address format in JALR instruction: " + operands[1]);
    }

    std::string immStr = addr.substr(0, pos1);
    std::string rs1Str = addr.substr(pos1 + 1, pos2 - pos1 - 1);

    // 去除空白字符
    immStr = Utils::trim(immStr);
    rs1Str = Utils::trim(rs1Str);

    // 解析 rs1
    rs1 = Utils::getRegisterNumber(rs1Str);

    // 检查立即数是否为 '%' 表达式或标签
    if (!immStr.empty() && immStr[0] == '%')
    {
      // 处理 '%' 表达式
      size_t funcEnd = immStr.find('(');
      if (funcEnd == std::string::npos)
      {
        funcEnd = immStr.length();
      }
      immFunction = immStr.substr(1, funcEnd - 1);
      immSymbol = immStr.substr(funcEnd);
      imm = 0; // 立即数将在链接时解析
    }
    else if (!immStr.empty() && !isdigit(immStr[0]) && immStr[0] != '-')
    {
      // 立即数是标签
      immSymbol = immStr;
      immFunction.clear();
      imm = 0; // 立即数将在链接时解析
    }
    else
    {
      // 立即数是数字
      imm = Utils::stringToImmediate(immStr);
      immFunction.clear();
      immSymbol.clear();
    }
  }
  else
  {
    // 处理其他 I 型指令
    if (operands.size() != 3)
    {
      throw std::runtime_error("Invalid I-type instruction: " + opcode);
    }

    rd = Utils::getRegisterNumber(operands[0]);
    rs1 = Utils::getRegisterNumber(operands[1]);

    std::string immStr = operands[2];

    // 检查立即数是否为 '%' 表达式或标签
    if (!immStr.empty() && immStr[0] == '%')
    {
      // 处理 '%' 表达式
      size_t funcEnd = immStr.find('(');
      if (funcEnd == std::string::npos)
      {
        funcEnd = immStr.length();
      }
      immFunction = immStr.substr(1, funcEnd - 1);
      immSymbol = immStr.substr(funcEnd);
      imm = 0; // 立即数将在链接时解析
    }
    else if (!immStr.empty() && !isdigit(immStr[0]) && immStr[0] != '-')
    {
      // 立即数是标签
      immSymbol = immStr;
      immFunction.clear();
      imm = 0; // 立即数将在链接时解析
    }
    else
    {
      // 立即数是数字
      imm = Utils::stringToImmediate(immStr);
      immFunction.clear();
      immSymbol.clear();
    }
  }
}

std::vector<uint32_t> InstructionI::encode(
    const SymbolTable &symbolTable,
    RelocationTable &relocationTable,
    std::unordered_map<std::string, Section> &sectionTable,
    uint32_t currentAddress,
    std::string currentSecName)
{
  uint32_t opcodeVal = opcodeMap.at(opcode);
  uint32_t funct3 = funct3Map.at(opcode);

  uint32_t instruction = 0;
  instruction |= (opcodeVal & 0x7F);
  instruction |= ((rd & 0x1F) << 7);
  instruction |= ((funct3 & 0x7) << 12);
  instruction |= ((rs1 & 0x1F) << 15);

  if (!immFunction.empty())
  {
    // 处理 '%' 表达式
    if (symbolTable.hasSymbol(immSymbol))
    {
      const Symbol &symbol = symbolTable.getSymbol(immSymbol);
      uint32_t symbolAddress = symbol.getGAddress();

      if (immFunction == "lo")
      {
        imm = symbolAddress & 0xFFF;
      }
      else if (immFunction == "hi")
      {
        throw std::runtime_error("I-type instructions cannot use %hi function.");
      }
      else
      {
        throw std::runtime_error("Unsupported immediate function: " + immFunction);
      }
    }
    else
    {
      // 符号不存在，添加重定位条目
      imm = 0;
      relocationTable.addRelocation(
          currentSecName,
          currentAddress,
          immSymbol,
          RelocationType::R_RISCV_LO12_I);
    }
  }
  else if (!immSymbol.empty())
  {
    // 立即数是标签
    if (symbolTable.hasSymbol(immSymbol))
    {
      const Symbol &symbol = symbolTable.getSymbol(immSymbol);
      imm = symbol.getGAddress();
    }
    else
    {
      // 符号不存在，添加重定位条目
      imm = 0;
      relocationTable.addRelocation(
          currentSecName,
          currentAddress,
          immSymbol,
          RelocationType::R_RISCV_32);
    }
  }
  // 否则，立即数已解析

  // 检查立即数范围
  if (imm < -2048 || imm > 2047)
  {
    throw std::runtime_error("Immediate value out of range for I-type instruction: " + std::to_string(imm));
  }

  instruction |= ((imm & 0xFFF) << 20);

  return {instruction};
}
