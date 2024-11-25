// instruction/InstructionB.cpp

#include "InstructionB.hpp"
#include "../utils/Utils.hpp"
#include "../relocation_table/RelocationTable.hpp"
#include <stdexcept>

const std::unordered_map<std::string, uint32_t> InstructionB::funct3Map = {
    {"beq", 0x0},
    {"bne", 0x1},
    {"blt", 0x4},
    {"bge", 0x5},
    {"bltu", 0x6},
    {"bgeu", 0x7}
    // 可以添加更多 B 型指令的 funct3 映射
};

InstructionB::InstructionB(const std::string &line)
{
  parseLine(line); // 解析指令行，提取 opcode 和 operands
  parseOperands(); // 解析操作数，提取 rs1, rs2, label
}

void InstructionB::parseOperands()
{
  if (operands.size() != 3)
  {
    throw std::runtime_error("Invalid B-type instruction: " + opcode);
  }

  // 解析 rs1
  rs1 = Utils::getRegisterNumber(operands[0]);

  // 解析 rs2
  rs2 = Utils::getRegisterNumber(operands[1]);

  // 解析 label
  label = operands[2];
}
std::vector<uint32_t> InstructionB::encode(
    const SymbolTable &symbolTable,
    RelocationTable &relocationTable,
    std::unordered_map<std::string, Section> &sectionTable,
    uint32_t currentAddress,
    std::string currentSecName)
{
  uint32_t opcodeVal = 0x63; // B 型指令的 opcode 固定为 0x63

  // 获取 funct3
  auto funct3It = funct3Map.find(opcode);
  if (funct3It == funct3Map.end())
  {
    throw std::runtime_error("Unsupported B-type opcode: " + opcode);
  }
  uint32_t funct3 = funct3It->second;

  int32_t imm_shifted = 0;

  if (symbolTable.hasSymbol(label))
  {
    // 标签存在，计算偏移量
    uint32_t labelAddress = symbolTable.getSymbol(label).getGAddress();
    int32_t imm = static_cast<int32_t>(labelAddress) - static_cast<int32_t>(currentAddress);

    if (imm % 2 != 0)
    {
      throw std::runtime_error("Branch target address must be 2-byte aligned.");
    }

    imm_shifted = imm >> 1;

    if (imm_shifted < -4096 || imm_shifted > 4095)
    {
      throw std::runtime_error("Branch offset out of range.");
    }
  }
  else
  {
    // 标签不存在，添加重定位条目，立即数设为 0
    imm_shifted = 0;
    auto segName = symbolTable.getSymbol(label).getSegmentName();
    relocationTable.addRelocation(segName, currentAddress, label, RelocationType::R_RISCV_BRANCH);
  }

  // 将立即数分割为各个部分
  uint32_t imm12 = (imm_shifted >> 11) & 0x1;   // imm[12]
  uint32_t imm10_5 = (imm_shifted >> 5) & 0x3F; // imm[10:5]
  uint32_t imm4_1 = imm_shifted & 0xF;          // imm[4:1]
  uint32_t imm11 = (imm_shifted >> 4) & 0x1;    // imm[11]

  // 组装指令
  uint32_t instruction = 0;
  instruction |= (imm12 << 31);
  instruction |= (imm10_5 << 25);
  instruction |= ((rs2 & 0x1F) << 20);
  instruction |= ((rs1 & 0x1F) << 15);
  instruction |= (funct3 << 12);
  instruction |= (imm4_1 << 8);
  instruction |= (imm11 << 7);
  instruction |= opcodeVal;

  return {instruction};
}
// std::vector<uint32_t> InstructionB::encode(const SymbolTable &symbolTable, RelocationTable &relocationTable, std::unordered_map<std::string, Section> &sectionTable, uint32_t currentAddress, std::string currentSecName)
// {
//   uint32_t opcodeVal = 0x63; // B 型指令的 opcode 固定为 0x63

//   // 获取 funct3
//   auto funct3It = funct3Map.find(opcode);
//   if (funct3It == funct3Map.end())
//   {
//     throw std::runtime_error("Unsupported B-type funct3 for opcode: " + opcode);
//   }
//   uint32_t funct3 = funct3It->second;

//   // 检查标签是否在符号表中
//   int32_t imm_shifted = 0;

//   // if (symbolTable.hasSymbol(label) && sectionTable[currentSecName].getSegmentName() == symbolTable.getSectionName(label))
//   // {
//   //   // 计算内部符号的偏移量
//   //   uint32_t labelAddress = symbolTable.getSymbol(label).getSAddress();
//   //   imm = static_cast<int32_t>(labelAddress - currentAddress);

//   //   if (imm % 2 != 0)
//   //   {
//   //     throw std::runtime_error("Branch target address must be 2-byte aligned.");
//   //   }

//   //   imm_shifted = imm >> 1;

//   //   if (imm_shifted < -2048 || imm_shifted > 2047)
//   //   {
//   //     throw std::runtime_error("Branch offset out of range.");
//   //   }
//   // }
//   // else
//   // {
//   //   // 外部符号，添加到重定位表并将偏移量设为0
//   //   auto segName = symbolTable.getSymbol(label).getSegmentName();
//   //   relocationTable.addRelocation(segName, currentAddress, label, RelocationType::R_RISCV_BRANCH);
//   // }
//   if (symbolTable.hasSymbol(label))
//   {
//     // 计算内部符号的偏移量
//     uint32_t labelAddress = symbolTable.getSymbol(label).getGAddress();
//     imm = static_cast<int32_t>(labelAddress - currentAddress);

//     if (imm % 2 != 0)
//     {
//       throw std::runtime_error("Branch target address must be 2-byte aligned.");
//     }

//     imm_shifted = imm >> 1;

//     if (imm_shifted < -2048 || imm_shifted > 2047)
//     {
//       throw std::runtime_error("Branch offset out of range.");
//     }
//   }
//   else
//   {
//     // 外部符号，添加到重定位表并将偏移量设为0
//     auto segName = symbolTable.getSymbol(label).getSegmentName();
//     relocationTable.addRelocation(segName, currentAddress, label, RelocationType::R_RISCV_BRANCH);
//   }

//   // 将立即数分割为各个部分
//   uint32_t imm12 = (imm_shifted >> 11) & 0x1;   // imm[12]
//   uint32_t imm10_5 = (imm_shifted >> 5) & 0x3F; // imm[10:5]
//   uint32_t imm4_1 = (imm_shifted >> 1) & 0xF;   // imm[4:1]
//   uint32_t imm11 = (imm_shifted >> 10) & 0x1;   // imm[11]

//   // 组装指令
//   uint32_t instruction = 0;
//   instruction |= (imm12 & 0x1) << 31;    // imm[12]: bit 31
//   instruction |= (imm10_5 & 0x3F) << 25; // imm[10:5]: bits 25-30
//   instruction |= (rs2 & 0x1F) << 20;     // rs2: bits 20-24
//   instruction |= (rs1 & 0x1F) << 15;     // rs1: bits 15-19
//   instruction |= (funct3 & 0x7) << 12;   // funct3: bits 12-14
//   instruction |= (imm4_1 & 0xF) << 8;    // imm[4:1]: bits 8-11
//   instruction |= (imm11 & 0x1) << 7;     // imm[11]: bit 7
//   instruction |= (opcodeVal & 0x7F);     // opcode: bits 0-6

//   return {instruction};
// }