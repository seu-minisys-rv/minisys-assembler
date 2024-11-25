// instruction/InstructionU.cpp

#include "InstructionU.hpp"
#include "../utils/Utils.hpp"
#include <stdexcept>

// U 型指令的 opcode 映射表
const std::unordered_map<std::string, uint32_t> InstructionU::opcodeMap = {
    {"lui", 0x37},
    {"auipc", 0x17}};

InstructionU::InstructionU(const std::string &line)
{
  parseLine(line); // 解析指令行，提取 opcode 和 operands
  parseOperands(); // 解析操作数，提取 rd 和 imm
}

void InstructionU::parseOperands()
{
  if (operands.size() != 2)
  {
    throw std::runtime_error("Invalid U-type instruction: " + opcode);
  }

  // 解析 rd
  rd = Utils::getRegisterNumber(operands[0]);

  std::string immStr = operands[1];

  // 检查是否为 '%' 表达式或标签
  if (!immStr.empty() && immStr[0] == '%')
  {
    size_t funcEnd = immStr.find('(');
    size_t funcStart = 1; // 跳过 '%'
    if (funcEnd == std::string::npos)
    {
      throw std::runtime_error("Invalid immediate format in U-type instruction: " + immStr);
    }
    immFunction = immStr.substr(funcStart, funcEnd - funcStart);
    size_t symbolStart = funcEnd + 1;
    size_t symbolEnd = immStr.find(')', symbolStart);
    if (symbolEnd == std::string::npos)
    {
      throw std::runtime_error("Invalid immediate format in U-type instruction: " + immStr);
    }
    immSymbol = immStr.substr(symbolStart, symbolEnd - symbolStart);
    imm = 0; // 立即数将在链接时确定
  }
  else if (!immStr.empty() && !isdigit(immStr[0]) && immStr[0] != '-')
  {
    // 立即数是标签
    immSymbol = immStr;
    immFunction.clear();
    imm = 0; // 立即数将在链接时确定
  }
  else
  {
    // 立即数是简单数字
    imm = Utils::stringToImmediate(immStr);

    // 检查立即数范围是否在 20 位范围内
    if (imm < -(1 << 19) || imm >= (1 << 20))
    {
      throw std::runtime_error("Immediate value out of range for U-type instruction: " + std::to_string(imm));
    }
    immFunction.clear();
    immSymbol.clear();
  }
}

std::vector<uint32_t> InstructionU::encode(
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
    throw std::runtime_error("Unsupported U-type instruction: " + opcode);
  }
  uint32_t opcodeVal = opcodeIt->second;

  if (!immFunction.empty())
  {
    // 处理 '%' 表达式
    if (symbolTable.hasSymbol(immSymbol))
    {
      const Symbol &symbol = symbolTable.getSymbol(immSymbol);
      uint32_t symbolAddress = symbol.getGAddress();

      if (immFunction == "hi")
      {
        // 计算 %hi(symbol)
        imm = (symbolAddress + 0x800) >> 12;
      }
      else if (immFunction == "pcrel_hi")
      {
        // 计算 PC 相对的 %pcrel_hi(symbol)
        int32_t offset = static_cast<int32_t>(symbolAddress) - static_cast<int32_t>(currentAddress);
        imm = (offset + 0x800) >> 12;
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
      RelocationType relocType = (immFunction == "hi") ? RelocationType::R_RISCV_HI20 : RelocationType::R_RISCV_PCREL_HI20;
      relocationTable.addRelocation(
          currentSecName,
          currentAddress,
          immSymbol,
          relocType);
    }
  }
  else if (!immSymbol.empty())
  {
    // 立即数是标签
    if (symbolTable.hasSymbol(immSymbol))
    {
      const Symbol &symbol = symbolTable.getSymbol(immSymbol);
      imm = symbol.getGAddress() >> 12;
    }
    else
    {
      // 符号不存在，添加重定位条目
      imm = 0;
      relocationTable.addRelocation(
          currentSecName,
          currentAddress,
          immSymbol,
          RelocationType::R_RISCV_HI20);
    }
  }
  // 否则，立即数已解析

  // 检查立即数范围
  if (imm < -(1 << 19) || imm >= (1 << 20))
  {
    throw std::runtime_error("Immediate value out of range for U-type instruction: " + std::to_string(imm));
  }

  // 组装指令
  uint32_t instruction = 0;
  instruction |= ((imm & 0xFFFFF) << 12); // 立即数部分（20 位），位于 12-31 位
  instruction |= ((rd & 0x1F) << 7);      // 目标寄存器 rd，位于 7-11 位
  instruction |= (opcodeVal & 0x7F);      // opcode，位于 0-6 位

  return {instruction};
}
