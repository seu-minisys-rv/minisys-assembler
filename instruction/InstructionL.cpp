// instruction/InstructionL.cpp

#include "InstructionL.hpp"
#include "../utils/Utils.hpp"
#include <stdexcept>

const std::unordered_map<std::string, uint32_t> InstructionL::opcodeMap = {
    {"lb", 0x03},
    {"lh", 0x03},
    {"lw", 0x03},
    {"lbu", 0x03},
    {"lhu", 0x03},
    // 添加更多的 Load 指令
};

const std::unordered_map<std::string, uint32_t> InstructionL::funct3Map = {
    {"lb", 0x0},
    {"lh", 0x1},
    {"lw", 0x2},
    {"lbu", 0x4},
    {"lhu", 0x5},
    // 添加更多的 Load 指令的 funct3
};

InstructionL::InstructionL(const std::string &line)
{
  parseLine(line);
  parseOperands();
}

void InstructionL::parseOperands()
{
  if (operands.size() != 2)
  {
    throw std::runtime_error("Invalid Load instruction operands: " + opcode);
  }

  rd = Utils::getRegisterNumber(operands[0]);

  // 解析 offset(rs1)、%function(symbol)(rs1)、%function(symbol) 或 symbol 格式
  std::string addr = operands[1];
  size_t posClose = addr.find(')');
  size_t posOpen = addr.find('(');
  std::string offsetPart = (posClose != std::string::npos) ? addr.substr(0, posClose + 1) : addr;
  std::string regPart = (posOpen != std::string::npos && posClose != std::string::npos) ? addr.substr(posClose + 1) : "";

  // 去除空白字符
  offsetPart = Utils::trim(offsetPart);
  regPart = Utils::trim(regPart);

  if (posOpen == std::string::npos)
  {
    immSymbol = offsetPart;
    immFunction.clear();
    imm = 0; // 等待符号地址解析
    rs1 = 0; // 无基址寄存器
  }
  else if (regPart.empty())
  {
    // **分支 1: 标准形式 offset(rs1)**
    // 示例指令: `lw a1, 100(a2)`
    if (!offsetPart.empty() && (isdigit(offsetPart[0]) || offsetPart[0] == '-'))
    {
      auto pos1 = offsetPart.find('(');
      auto pos2 = offsetPart.find(')');
      imm = Utils::stringToImmediate(offsetPart.substr(0, pos1));
      rs1 = Utils::getRegisterNumber(offsetPart.substr(pos1 + 1, pos2 - pos1 - 1));
      immFunction.clear();
      immSymbol.clear();
    }
    // **分支 3: %function(symbol)**
    // 示例指令: `lw a1, %lo(sa)`
    else if (!offsetPart.empty() && offsetPart[0] == '%')
    {
      size_t pos1 = offsetPart.find('(');
      if (pos1 == std::string::npos)
      {
        throw std::runtime_error("Invalid \%function(symbol) format in Load instruction: " + offsetPart);
      }
      immFunction = offsetPart.substr(1, pos1 - 1);                          // 提取 function 名
      immSymbol = offsetPart.substr(pos1 + 1, offsetPart.size() - pos1 - 2); // 提取 symbol 名
      imm = 0;                                                               // 立即数将在链接时解析
      rs1 = 0;                                                               // 无基址寄存器
    }
  }
  else if (!offsetPart.empty() && !regPart.empty() && offsetPart[0] == '%')
  {
    size_t pos1 = offsetPart.find('(');
    size_t pos2 = offsetPart.find(')');
    size_t pos3 = regPart.find('(');
    size_t pos4 = regPart.find(')');
    immFunction = offsetPart.substr(1, pos1 - 1);             // 提取 function 名
    immSymbol = offsetPart.substr(pos1 + 1, pos2 - pos1 - 1); // 提取 symbol 名
    imm = 0;
    rs1 = Utils::getRegisterNumber(regPart.substr(pos3 + 1, pos4 - pos3 - 1));
  }
  else
  {
    throw std::runtime_error("Invalid Load instruction format: " + operands[1]);
  }
}

std::vector<uint32_t> InstructionL::encode(
    const SymbolTable &symbolTable,
    RelocationTable &relocationTable,
    std::unordered_map<std::string, Section> &sectionTable,
    uint32_t currentAddress,
    std::string currentSecName)
{
  auto opcodeIt = opcodeMap.find(opcode);
  if (opcodeIt == opcodeMap.end())
  {
    throw std::runtime_error("Unsupported Load opcode: " + opcode);
  }
  uint32_t opcodeVal = opcodeIt->second;

  auto funct3It = funct3Map.find(opcode);
  if (funct3It == funct3Map.end())
  {
    throw std::runtime_error("Unsupported Load funct3 for opcode: " + opcode);
  }
  uint32_t funct3 = funct3It->second;

  uint32_t instruction = 0;

  if (!immFunction.empty())
  {
    // **情况1: %function(symbol)(rs1) 或 %function(symbol)**
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
        throw std::runtime_error("Load instructions cannot use %hi function.");
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

    // 如果有基址寄存器（%function(symbol)(rs1)）
    if (rs1 != 0)
    {
      instruction |= ((imm & 0xFFF) << 20); // imm[11:0]
      instruction |= ((rs1 & 0x1F) << 15);  // rs1
    }
    else
    {
      // 没有基址寄存器（%function(symbol)）
      instruction |= (imm & 0xFFF) << 20; // imm[11:0]
      instruction |= (0 << 15);           // rs1 = x0
    }
  }
  else if (!immSymbol.empty())
  {
    // **情况2: symbol 或 symbol(rs1)**
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
          RelocationType::R_RISCV_LO12_I);
    }

    instruction |= ((imm & 0xFFF) << 20); // imm[11:0]
    instruction |= ((rs1 & 0x1F) << 15);  // rs1
  }
  else
  {
    // **情况3: offset(rs1)**
    instruction |= ((imm & 0xFFF) << 20); // imm[11:0]
    instruction |= ((rs1 & 0x1F) << 15);  // rs1
  }

  instruction |= ((funct3 & 0x7) << 12); // funct3
  instruction |= ((rd & 0x1F) << 7);     // rd
  instruction |= (opcodeVal & 0x7F);     // opcode

  return {instruction};
}
