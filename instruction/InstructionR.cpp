// instruction/InstructionR.cpp

#include "InstructionR.hpp"
#include "../utils/Utils.hpp"
#include <stdexcept>

// 初始化静态成员变量：funct3 映射表
const std::unordered_map<std::string, uint32_t> InstructionR::funct3Map = {
    {"add", 0x0},
    {"sub", 0x0},
    {"sll", 0x1},
    {"slt", 0x2},
    {"sltu", 0x3},
    {"xor", 0x4},
    {"srl", 0x5},
    {"sra", 0x5},
    {"or", 0x6},
    {"and", 0x7}
    // 可以添加更多 R 型指令的 funct3 映射
};

// 初始化静态成员变量：funct7 映射表
const std::unordered_map<std::string, uint32_t> InstructionR::funct7Map = {
    {"add", 0x00},
    {"sub", 0x20},
    {"sll", 0x00},
    {"slt", 0x00},
    {"sltu", 0x00},
    {"xor", 0x00},
    {"srl", 0x00},
    {"sra", 0x20},
    {"or", 0x00},
    {"and", 0x00}
    // 可以添加更多 R 型指令的 funct7 映射
};

// 构造函数
InstructionR::InstructionR(const std::string &line)
{
  parseLine(line); // 解析指令行，提取 opcode 和 operands
  parseOperands(); // 解析操作数，提取 rd, rs1, rs2
}

// 解析操作数
void InstructionR::parseOperands()
{
  if (operands.size() != 3)
  {
    throw std::runtime_error("Invalid R-type instruction operands: " + opcode);
  }
  rd = Utils::getRegisterNumber(operands[0]);  // 目标寄存器
  rs1 = Utils::getRegisterNumber(operands[1]); // 源寄存器1
  rs2 = Utils::getRegisterNumber(operands[2]); // 源寄存器2
}

// 编码函数
std::vector<uint32_t> InstructionR::encode(const SymbolTable &symbolTable, RelocationTable &relocationTable, std::unordered_map<std::string, Section> &sectionTable, uint32_t currentAddress, std::string currentSecName)
{
  // R 型指令的 opcode 固定为 0x33
  uint32_t opcodeVal = 0x33;

  // 获取 funct3
  auto funct3It = funct3Map.find(opcode);
  if (funct3It == funct3Map.end())
  {
    throw std::runtime_error("Unsupported R-type funct3 for opcode: " + opcode);
  }
  uint32_t funct3 = funct3It->second;

  // 获取 funct7
  auto funct7It = funct7Map.find(opcode);
  if (funct7It == funct7Map.end())
  {
    throw std::runtime_error("Unsupported R-type funct7 for opcode: " + opcode);
  }
  uint32_t funct7 = funct7It->second;

  // 组装指令
  uint32_t instruction = 0;
  instruction |= (funct7 & 0x7F) << 25; // funct7: bits 25-31
  instruction |= (rs2 & 0x1F) << 20;    // rs2: bits 20-24
  instruction |= (rs1 & 0x1F) << 15;    // rs1: bits 15-19
  instruction |= (funct3 & 0x7) << 12;  // funct3: bits 12-14
  instruction |= (rd & 0x1F) << 7;      // rd: bits 7-11
  instruction |= (opcodeVal & 0x7F);    // opcode: bits 0-6

  return {instruction};
}
