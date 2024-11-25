#include "InstructionM.hpp"
#include "../utils/Utils.hpp"
#include <stdexcept>

// 定义操作码为 0x33，RISC-V M 型指令的标准操作码
const uint32_t InstructionM::opcodeVal = 0x33;

// M 型指令的 funct3 映射表
const std::unordered_map<std::string, uint32_t> InstructionM::funct3Map = {
    {"mul", 0x0}, {"mulh", 0x1}, {"mulhsu", 0x2}, {"mulhu", 0x3}, {"div", 0x4}, {"divu", 0x5}, {"rem", 0x6}, {"remu", 0x7}};

// M 型指令的 funct7 映射表
const std::unordered_map<std::string, uint32_t> InstructionM::funct7Map = {
    {"mul", 0x1}, {"mulh", 0x1}, {"mulhsu", 0x1}, {"mulhu", 0x1}, {"div", 0x1}, {"divu", 0x1}, {"rem", 0x1}, {"remu", 0x1}};

InstructionM::InstructionM(const std::string &line)
{
  parseLine(line);
  parseOperands();
}

void InstructionM::parseOperands()
{
  if (operands.size() != 3)
  {
    throw std::runtime_error("Invalid M-type instruction operands: " + opcode);
  }

  // 解析 rd, rs1, rs2
  rd = Utils::getRegisterNumber(operands[0]);
  rs1 = Utils::getRegisterNumber(operands[1]);
  rs2 = Utils::getRegisterNumber(operands[2]);
}

std::vector<uint32_t> InstructionM::encode(const SymbolTable &symbolTable, RelocationTable &relocationTable, std::unordered_map<std::string, Section> &sectionTable, uint32_t currentAddress, std::string currentSecName)
{
  auto funct3It = funct3Map.find(opcode);
  auto funct7It = funct7Map.find(opcode);

  if (funct3It == funct3Map.end() || funct7It == funct7Map.end())
  {
    throw std::runtime_error("Unsupported M-type instruction: " + opcode);
  }

  uint32_t funct3 = funct3It->second;
  uint32_t funct7 = funct7It->second;

  // 组装指令
  uint32_t instruction = 0;
  instruction |= (opcodeVal & 0x7F);      // 操作码（bits 0-6）
  instruction |= ((rd & 0x1F) << 7);      // 目的寄存器（rd）（bits 7-11）
  instruction |= ((funct3 & 0x7) << 12);  // funct3（bits 12-14）
  instruction |= ((rs1 & 0x1F) << 15);    // 源寄存器 rs1（bits 15-19）
  instruction |= ((rs2 & 0x1F) << 20);    // 源寄存器 rs2（bits 20-24）
  instruction |= ((funct7 & 0x7F) << 25); // funct7（bits 25-31）

  return {instruction};
}
