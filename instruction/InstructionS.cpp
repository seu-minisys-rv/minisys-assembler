// instruction/InstructionS.cpp

#include "InstructionS.hpp"
#include "../utils/Utils.hpp"
#include <stdexcept>

// 初始化静态成员变量：funct3 映射表
const std::unordered_map<std::string, uint32_t> InstructionS::funct3Map = {
    {"sb", 0x0},
    {"sh", 0x1},
    {"sw", 0x2},
    {"sd", 0x3}
    // 可以添加更多 S 型指令的 funct3 映射
};

// 构造函数
InstructionS::InstructionS(const std::string &line)
{
  parseLine(line); // 解析指令行，提取 opcode 和 operands
  parseOperands(); // 解析操作数，提取 rs2, rs1, imm
}

// 解析操作数
void InstructionS::parseOperands()
{
  if (operands.size() != 2)
  {
    throw std::runtime_error("Invalid S-type instruction operands: " + opcode);
  }

  // 解析 rs2
  rs2 = Utils::getRegisterNumber(operands[0]);

  // 解析 offset(rs1) 或带 '%' 的表达式
  std::string addr = operands[1];
  size_t pos1 = addr.find('(');
  size_t pos2 = addr.find(')');
  if (pos1 != std::string::npos && pos2 != std::string::npos && pos2 > pos1)
  {
    // 标准的 offset(rs1) 格式
    std::string offsetStr = addr.substr(0, pos1);
    std::string rs1Str = addr.substr(pos1 + 1, pos2 - pos1 - 1);

    // 去除空白字符
    offsetStr = Utils::trim(offsetStr);
    rs1Str = Utils::trim(rs1Str);

    // 解析 rs1
    rs1 = Utils::getRegisterNumber(rs1Str);

    // 检查立即数是否为 '%' 表达式或标签
    if (!offsetStr.empty() && offsetStr[0] == '%')
    {
      // 处理 '%' 表达式
      size_t funcEnd = offsetStr.find('(');
      if (funcEnd == std::string::npos)
      {
        funcEnd = offsetStr.length();
      }
      immFunction = offsetStr.substr(1, funcEnd - 1);
      immSymbol = offsetStr.substr(funcEnd);
      imm = 0; // 立即数将在链接时解析
    }
    else if (!offsetStr.empty() && !isdigit(offsetStr[0]) && offsetStr[0] != '-')
    {
      // 立即数是标签
      immSymbol = offsetStr;
      immFunction.clear();
      imm = 0; // 立即数将在链接时解析
    }
    else
    {
      // 立即数是数字
      imm = Utils::stringToImmediate(offsetStr);
      immFunction.clear();
      immSymbol.clear();
    }
  }
  else
  {
    throw std::runtime_error("Invalid address format in S-type instruction: " + operands[1]);
  }
}

// 编码函数
std::vector<uint32_t> InstructionS::encode(
    const SymbolTable &symbolTable,
    RelocationTable &relocationTable,
    std::unordered_map<std::string, Section> &sectionTable,
    uint32_t currentAddress,
    std::string currentSecName)
{
  // S 型指令的 opcode 固定为 0x23
  uint32_t opcodeVal = 0x23;

  // 获取 funct3
  auto funct3It = funct3Map.find(opcode);
  if (funct3It == funct3Map.end())
  {
    throw std::runtime_error("Unsupported S-type funct3 for opcode: " + opcode);
  }
  uint32_t funct3 = funct3It->second;

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
        throw std::runtime_error("S-type instructions cannot use %hi function.");
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
          RelocationType::R_RISCV_LO12_S);
    }
  }
  else if (!immSymbol.empty())
  {
    // 立即数是标签
    if (symbolTable.hasSymbol(immSymbol))
    {
      const Symbol &symbol = symbolTable.getSymbol(immSymbol);
      imm = symbol.getGAddress() & 0xFFF;
    }
    else
    {
      // 符号不存在，添加重定位条目
      imm = 0;
      relocationTable.addRelocation(
          currentSecName,
          currentAddress,
          immSymbol,
          RelocationType::R_RISCV_LO12_S);
    }
  }
  // 否则，立即数已解析

  // 检查立即数范围
  if (imm < -2048 || imm > 2047)
  {
    throw std::runtime_error("Immediate value out of range for S-type instruction: " + std::to_string(imm));
  }

  // 将立即数分割为 imm[11:5] 和 imm[4:0]
  uint32_t imm11_5 = (imm >> 5) & 0x7F; // 7 bits
  uint32_t imm4_0 = imm & 0x1F;         // 5 bits

  // 组装指令
  uint32_t instruction = 0;
  instruction |= (imm11_5 << 25);        // imm[11:5]: bits 25-31
  instruction |= ((rs2 & 0x1F) << 20);   // rs2: bits 20-24
  instruction |= ((rs1 & 0x1F) << 15);   // rs1: bits 15-19
  instruction |= ((funct3 & 0x7) << 12); // funct3: bits 12-14
  instruction |= ((imm4_0 & 0x1F) << 7); // imm[4:0]: bits 7-11
  instruction |= (opcodeVal & 0x7F);     // opcode: bits 0-6

  return {instruction};
}
