// #include "InstructionP.hpp"
// #include "../utils/Utils.hpp"
// #include "../relocation_table/RelocationTable.hpp"
// #include <stdexcept>

// InstructionP::InstructionP(const std::string &line)
// {
//   parseLine(line);
//   parseOperands();
// }

// void InstructionP::parseOperands()
// {
//   if (opcode == "mv")
//   {
//     if (operands.size() != 2)
//     {
//       throw std::runtime_error("Invalid operands for mv: " + opcode);
//     }
//     expandedOpcode = "addi";
//     expandedOperands = {operands[0], operands[1], "0"};
//   }
//   else if (opcode == "li")
//   {
//     if (operands.size() != 2)
//     {
//       throw std::runtime_error("Invalid operands for li: " + opcode);
//     }

//     // 检查立即数是否为符号或大于 12 位
//     std::string immStr = operands[1];
//     int32_t immValue = 0;
//     bool isSymbol = false;
//     bool isLargeImmediate = false;

//     // 检查是否为 '%' 表达式或符号
//     if (!immStr.empty() && immStr[0] == '%')
//     {
//       isSymbol = true;
//     }
//     else if (Utils::isNumber(immStr))
//     {
//       immValue = Utils::stringToImmediate(immStr);
//       // 检查立即数是否超过 12 位
//       if (immValue < -(1 << 11) || immValue >= (1 << 11))
//       {
//         isLargeImmediate = true;
//       }
//     }
//     else
//     {
//       // 非数字，视为符号
//       isSymbol = true;
//     }

//     if (isSymbol || isLargeImmediate)
//     {
//       // 需要展开为 lui 和 addi 指令
//       expandedOpcode = "li_lui_addi";
//       expandedOperands = {operands[0], immStr}; // rd, imm
//     }
//     else
//     {
//       // 可以使用单条 addi 指令
//       expandedOpcode = "addi";
//       expandedOperands = {operands[0], "x0", immStr};
//     }
//   }
//   else if (opcode == "j")
//   {
//     if (operands.size() != 1)
//     {
//       throw std::runtime_error("Invalid operands for j: " + opcode);
//     }
//     expandedOpcode = "jal";
//     expandedOperands = {"x0", operands[0]};
//   }
//   else if (opcode == "nop")
//   {
//     expandedOpcode = "addi";
//     expandedOperands = {"x0", "x0", "0"};
//   }
//   else if (opcode == "call")
//   {
//     if (operands.size() != 1)
//     {
//       throw std::runtime_error("Invalid operands for call: " + opcode);
//     }
//     // 将 call 指令展开为 auipc 和 jalr
//     expandedOpcode = "call_auipc_jalr";
//     expandedOperands = {operands[0]}; // 保存函数名
//   }
//   else if (opcode == "ret")
//   {
//     expandedOpcode = "jalr";
//     expandedOperands = {"x0", "x1", "0"}; // rd=x0, rs1=x1, 偏移量=0
//   }
//   else
//   {
//     throw std::runtime_error("Unsupported P-type instruction: " + opcode);
//   }
// }

// std::vector<uint32_t> InstructionP::encode(const SymbolTable &symbolTable, RelocationTable &relocationTable, std::unordered_map<std::string, Section> &sectionTable, uint32_t currentAddress, std::string currentSecName)
// {
//   if (expandedOpcode == "addi")
//   {
//     uint32_t rd = Utils::getRegisterNumber(expandedOperands[0]);
//     uint32_t rs1 = Utils::getRegisterNumber(expandedOperands[1]);
//     int32_t imm = Utils::stringToImmediate(expandedOperands[2]);

//     return {(0x13 | (rd << 7) | (0x0 << 12) | (rs1 << 15) | ((imm & 0xFFF) << 20))};
//   }
//   else if (expandedOpcode == "li_lui_addi")
//   {
//     // 处理需要展开为 lui 和 addi 的情况

//     // 获取目标寄存器和立即数
//     uint32_t rd = Utils::getRegisterNumber(expandedOperands[0]);
//     std::string immStr = expandedOperands[1];

//     // 创建指令序列
//     std::vector<uint32_t> instructions;

//     // 处理 lui 指令
//     uint32_t luiOpcode = 0x37; // lui 的 opcode
//     uint32_t luiImm = 0;
//     uint32_t addiImm = 0;
//     bool needAddi = true;

//     if (!immStr.empty() && immStr[0] == '%')
//     {
//       // 处理 '%' 表达式，例如 %hi(symbol)
//       size_t start = immStr.find('(');
//       size_t end = immStr.find(')');
//       if (start == std::string::npos || end == std::string::npos || end <= start)
//       {
//         throw std::runtime_error("Invalid immediate format in li instruction: " + immStr);
//       }
//       std::string immFunction = immStr.substr(1, start - 1);             // 提取函数名
//       std::string immSymbol = immStr.substr(start + 1, end - start - 1); // 提取符号名

//       if (immFunction == "hi")
//       {
//         // lui rd, %hi(symbol)
//         auto segName = symbolTable.getSymbol(label).getSegmentName();
//         relocationTable.addRelocation(segName, currentAddress, label, RelocationType::R_RISCV_HI20);

//         uint32_t luiInstr = (luiOpcode) | (rd << 7) | (0 << 12); // imm 部分设为 0，等待链接器修正
//         instructions.push_back(luiInstr);

//         // addi rd, rd, %lo(symbol)
//         auto segName = symbolTable.getSymbol(label).getSegmentName();
//         relocationTable.addRelocation(segName, currentAddress, label, RelocationType::R_RISCV_LO12_I);

//         uint32_t addiInstr = (0x13) | (rd << 7) | (0x0 << 12) | (rd << 15) | (0 << 20); // imm 部分设为 0
//         instructions.push_back(addiInstr);
//       }
//       else
//       {
//         throw std::runtime_error("Unsupported immediate function in li instruction: " + immFunction);
//       }
//     }
//     else if (symbolTable.hasSymbol(immStr))
//     {
//       // 立即数是符号，处理符号引用
//       std::string immSymbol = immStr;

//       // lui rd, %hi(symbol)
//       auto segName = symbolTable.getSymbol(label).getSegmentName();
//       relocationTable.addRelocation(segName, currentAddress, label, RelocationType::R_RISCV_HI20);

//       uint32_t luiInstr = (luiOpcode) | (rd << 7) | (0 << 12); // imm 部分设为 0
//       instructions.push_back(luiInstr);

//       // addi rd, rd, %lo(symbol)
//       auto segName = symbolTable.getSymbol(label).getSegmentName();
//       relocationTable.addRelocation(segName, currentAddress, label, RelocationType::R_RISCV_LO12_I);

//       uint32_t addiInstr = (0x13) | (rd << 7) | (0x0 << 12) | (rd << 15) | (0 << 20); // imm 部分设为 0
//       instructions.push_back(addiInstr);
//     }
//     else
//     {
//       // 立即数是大于 12 位的数字，拆分高 20 位和低 12 位
//       int32_t immValue = Utils::stringToImmediate(immStr);
//       luiImm = (immValue + (1 << 11)) >> 12; // 向上取整，防止负数错误

//       if (luiImm != 0)
//       {
//         uint32_t luiInstr = (luiOpcode) | (rd << 7) | (luiImm << 12);
//         instructions.push_back(luiInstr);

//         addiImm = immValue - (luiImm << 12);
//         needAddi = (addiImm != 0);
//       }
//       else
//       {
//         // 高 20 位为 0，不需要 lui 指令
//         needAddi = true;
//       }

//       if (needAddi)
//       {
//         uint32_t addiInstr = (0x13) | (rd << 7) | (0x0 << 12) | (rd << 15) | ((addiImm & 0xFFF) << 20);
//         instructions.push_back(addiInstr);
//       }
//     }
//     if (!instructions.empty())
//     {
//       return instructions;
//     }
//     else
//     {
//       throw std::runtime_error("Failed to generate instructions for li.");
//     }
//   }
//   else if (expandedOpcode == "call_auipc_jalr")
//   {
//     // 获取函数名
//     std::string label = expandedOperands[0];
//     uint32_t rd = 1;     // x1，用于保存返回地址
//     uint32_t tmpReg = 6; // 使用 x6 作为临时寄存器

//     // 创建指令序列
//     std::vector<uint32_t> instructions;

//     // auipc x6, %pcrel_hi(label)
//     uint32_t auipcOpcode = 0x17; // auipc 的 opcode

//     // 添加重定位条目，类型为 R_RISCV_PCREL_HI20
//     auto segName = symbolTable.getSymbol(label).getSegmentName();
//     relocationTable.addRelocation(segName, currentAddress, label, RelocationType::R_RISCV_PCREL_HI20);

//     // 生成 auipc 指令，立即数部分设为 0，等待链接器修正
//     uint32_t auipcInstr = (auipcOpcode) | (tmpReg << 7); // imm[31:12] = 0
//     instructions.push_back(auipcInstr);

//     // jalr x1, x6, %pcrel_lo(label)
//     uint32_t jalrOpcode = 0x67; // jalr 的 opcode
//     uint32_t funct3 = 0x0;      // jalr 的 funct3 固定为 0
//     uint32_t rs1 = tmpReg;      // rs1 = x6
//     uint32_t jalrImm = 0;       // 立即数设为 0，等待链接器修正

//     // 添加重定位条目，类型为 R_RISCV_PCREL_LO12_I
//     auto segName = symbolTable.getSymbol(label).getSegmentName();
//     relocationTable.addRelocation(segName, currentAddress, label, RelocationType::R_RISCV_PCREL_LO12_I);

//     uint32_t jalrInstr = jalrOpcode |
//                          (rd << 7) |
//                          (funct3 << 12) |
//                          (rs1 << 15) |
//                          (jalrImm << 20);
//     instructions.push_back(jalrInstr);

//     // 返回指令序列
//     return instructions;
//   }
//   else if (expandedOpcode == "jal")
//   {
//     uint32_t rd = Utils::getRegisterNumber(expandedOperands[0]);
//     std::string label = expandedOperands[1];
//     int32_t imm = 0;

//     if (symbolTable.hasSymbol(label) && sectionTable[currentSecName].getSegmentName() == symbolTable.getSectionName(label))
//     {
//       // 计算内部符号的偏移量
//       uint32_t labelAddress = symbolTable.getSymbol(label).getSAddress();
//       imm = static_cast<int32_t>(labelAddress - currentAddress);

//       // 确保立即数在 20 位范围内
//       if (imm < -(1 << 20) || imm >= (1 << 20))
//       {
//         throw std::runtime_error("Jump offset out of range for J-type instruction: " + std::to_string(imm));
//       }
//     }
//     else
//     {
//       // 外部符号，添加到重定位表并将偏移量设为 0
//       auto segName = symbolTable.getSymbol(label).getSegmentName();
//       relocationTable.addRelocation(segName, currentAddress, label, RelocationType::R_RISCV_JAL);
//     }

//     uint32_t imm20 = (imm >> 20) & 0x1;     // imm[20]
//     uint32_t imm10_1 = (imm >> 1) & 0x3FF;  // imm[10:1]
//     uint32_t imm11 = (imm >> 11) & 0x1;     // imm[11]
//     uint32_t imm19_12 = (imm >> 12) & 0xFF; // imm[19:12]

//     uint32_t instruction = 0;
//     instruction |= (imm20 & 0x1) << 31;     // imm[20]: bit 31
//     instruction |= (imm19_12 & 0xFF) << 12; // imm[19:12]: bits 12-19
//     instruction |= (imm11 & 0x1) << 20;     // imm[11]: bit 20
//     instruction |= (imm10_1 & 0x3FF) << 21; // imm[10:1]: bits 21-30
//     instruction |= (rd & 0x1F) << 7;        // rd: bits 7-11
//     instruction |= (0x6F);                  // opcode: bits 0-6

//     return {instruction};
//   }
//   else if (expandedOpcode == "jalr")
//   {
//     uint32_t rd = Utils::getRegisterNumber(expandedOperands[0]);
//     uint32_t rs1 = Utils::getRegisterNumber(expandedOperands[1]);
//     int32_t imm = Utils::stringToImmediate(expandedOperands[2]);

//     uint32_t instruction = 0x67;        // opcode for jalr
//     instruction |= (rd & 0x1F) << 7;    // rd 位于 7-11 位
//     instruction |= (0x0 & 0x7) << 12;   // funct3 固定为 0
//     instruction |= (rs1 & 0x1F) << 15;  // rs1 位于 15-19 位
//     instruction |= (imm & 0xFFF) << 20; // 偏移量 imm 位于 20-31 位

//     return {instruction};
//   }
//   else
//   {
//     throw std::runtime_error("Unsupported expanded P-type instruction: " + expandedOpcode);
//   }
// }
// instruction/InstructionP.cpp

// instruction/InstructionP.cpp

#include "InstructionP.hpp"
#include "../utils/Utils.hpp"
#include "../relocation_table/RelocationTable.hpp"
#include <stdexcept>

// 构造函数，解析指令行和操作数
InstructionP::InstructionP(const std::string &line)
{
  parseLine(line);
  parseOperands();
}

// 解析操作数，根据伪指令类型展开实际指令
void InstructionP::parseOperands()
{
  if (opcode == "mv")
  {
    if (operands.size() != 2)
    {
      throw std::runtime_error("Invalid operands for mv: " + opcode);
    }
    // mv rd, rs => addi rd, rs, 0
    expandedOpcode = "addi";
    expandedOperands = {operands[0], operands[1], "0"};
  }
  else if (opcode == "li")
  {
    if (operands.size() != 2)
    {
      throw std::runtime_error("Invalid operands for li: " + opcode);
    }
    // li rd, imm
    expandedOpcode = "li_expand";
    expandedOperands = operands;
  }
  else if (opcode == "j")
  {
    if (operands.size() != 1)
    {
      throw std::runtime_error("Invalid operands for j: " + opcode);
    }
    // j label => jal x0, label
    expandedOpcode = "jal";
    expandedOperands = {"x0", operands[0]};
  }
  else if (opcode == "nop")
  {
    // nop => addi x0, x0, 0
    expandedOpcode = "addi";
    expandedOperands = {"x0", "x0", "0"};
  }
  else if (opcode == "call")
  {
    if (operands.size() != 1)
    {
      throw std::runtime_error("Invalid operands for call: " + opcode);
    }
    // call label => auipc x6, %pcrel_hi(label); jalr x1, x6, %pcrel_lo(label)
    expandedOpcode = "call_expand";
    expandedOperands = operands;
  }
  else if (opcode == "ret")
  {
    // ret => jalr x0, x1, 0
    expandedOpcode = "jalr";
    expandedOperands = {"x0", "x1", "0"};
  }
  else
  {
    throw std::runtime_error("Unsupported pseudo-instruction: " + opcode);
  }
}

// 编码指令，处理展开后的实际指令
std::vector<uint32_t> InstructionP::encode(
    const SymbolTable &symbolTable,
    RelocationTable &relocationTable,
    std::unordered_map<std::string, Section> &sectionTable,
    uint32_t currentAddress,
    std::string currentSecName)
{
  std::vector<uint32_t> instructions;

  if (expandedOpcode == "addi")
  {
    // 处理 addi 指令
    uint32_t rd = Utils::getRegisterNumber(expandedOperands[0]);
    uint32_t rs1 = Utils::getRegisterNumber(expandedOperands[1]);
    int32_t imm = Utils::stringToImmediate(expandedOperands[2]);

    // 检查立即数范围
    if (imm < -2048 || imm > 2047)
    {
      throw std::runtime_error("Immediate value out of range for addi: " + std::to_string(imm));
    }

    uint32_t instruction = 0x13; // opcode for addi
    instruction |= (rd & 0x1F) << 7;
    instruction |= (0x0 & 0x7) << 12; // funct3 = 0
    instruction |= (rs1 & 0x1F) << 15;
    instruction |= (imm & 0xFFF) << 20;

    instructions.push_back(instruction);
  }
  else if (expandedOpcode == "li_expand")
  {
    // 处理 li 指令的展开
    uint32_t rd = Utils::getRegisterNumber(expandedOperands[0]);
    std::string immStr = expandedOperands[1];

    // 检查立即数是否为符号或数值
    if (!Utils::isNumber(immStr) && immStr[0] != '-')
    {
      // 立即数是符号，展开为 lui 和 addi
      if (symbolTable.hasSymbol(immStr))
      {
        uint32_t symbolAddress = symbolTable.getSymbol(immStr).getGAddress();
        uint32_t luiImm = (symbolAddress + 0x800) >> 12;
        uint32_t addiImm = symbolAddress & 0xFFF;

        // 生成 lui 指令
        uint32_t luiInstr = 0x37; // opcode for lui
        luiInstr |= (rd & 0x1F) << 7;
        luiInstr |= (luiImm & 0xFFFFF) << 12;
        instructions.push_back(luiInstr);

        // 生成 addi 指令
        uint32_t addiInstr = 0x13; // opcode for addi
        addiInstr |= (rd & 0x1F) << 7;
        addiInstr |= (0x0 & 0x7) << 12; // funct3 = 0
        addiInstr |= (rd & 0x1F) << 15;
        addiInstr |= (addiImm & 0xFFF) << 20;
        instructions.push_back(addiInstr);
      }
      else
      {
        // 符号未知，添加重定位条目
        // 生成 lui 指令
        uint32_t luiInstr = 0x37; // opcode for lui
        luiInstr |= (rd & 0x1F) << 7;
        instructions.push_back(luiInstr);

        relocationTable.addRelocation(
            currentSecName,
            currentAddress,
            immStr,
            RelocationType::R_RISCV_HI20);

        // 生成 addi 指令
        uint32_t addiInstr = 0x13; // opcode for addi
        addiInstr |= (rd & 0x1F) << 7;
        addiInstr |= (0x0 & 0x7) << 12; // funct3 = 0
        addiInstr |= (rd & 0x1F) << 15;
        instructions.push_back(addiInstr);

        relocationTable.addRelocation(
            currentSecName,
            currentAddress + 4,
            immStr,
            RelocationType::R_RISCV_LO12_I);
      }
    }
    else
    {
      // 立即数是数值
      int32_t imm = Utils::stringToImmediate(immStr);
      if (imm >= -2048 && imm <= 2047)
      {
        // 可以直接使用 addi 指令
        uint32_t instruction = 0x13; // opcode for addi
        instruction |= (rd & 0x1F) << 7;
        instruction |= (0x0 & 0x7) << 12;  // funct3 = 0
        instruction |= (0x0 & 0x1F) << 15; // rs1 = x0
        instruction |= (imm & 0xFFF) << 20;
        instructions.push_back(instruction);
      }
      else
      {
        // 需要展开为 lui 和 addi
        uint32_t luiImm = (imm + 0x800) >> 12;
        int32_t addiImm = imm - (luiImm << 12);

        // 生成 lui 指令
        uint32_t luiInstr = 0x37; // opcode for lui
        luiInstr |= (rd & 0x1F) << 7;
        luiInstr |= (luiImm & 0xFFFFF) << 12;
        instructions.push_back(luiInstr);

        // 生成 addi 指令
        uint32_t addiInstr = 0x13; // opcode for addi
        addiInstr |= (rd & 0x1F) << 7;
        addiInstr |= (0x0 & 0x7) << 12; // funct3 = 0
        addiInstr |= (rd & 0x1F) << 15;
        addiInstr |= (addiImm & 0xFFF) << 20;
        instructions.push_back(addiInstr);
      }
    }
  }
  else if (expandedOpcode == "jal")
  {
    // 处理 jal 指令
    uint32_t rd = Utils::getRegisterNumber(expandedOperands[0]);
    std::string label = expandedOperands[1];
    int32_t imm = 0;

    if (symbolTable.hasSymbol(label))
    {
      uint32_t labelAddress = symbolTable.getSymbol(label).getGAddress();
      imm = static_cast<int32_t>(labelAddress) - static_cast<int32_t>(currentAddress);

      // 检查偏移量是否对齐到 4 字节
      if (imm % 4 != 0)
      {
        throw std::runtime_error("Jump target address must be 4-byte aligned.");
      }

      // 检查偏移量是否在范围内
      if (imm < -(1 << 20) || imm >= (1 << 20))
      {
        throw std::runtime_error("Jump offset out of range for jal: " + std::to_string(imm));
      }
    }
    else
    {
      // 符号未知，立即数设为 0，添加重定位条目
      imm = 0;
      relocationTable.addRelocation(
          currentSecName,
          currentAddress,
          label,
          RelocationType::R_RISCV_JAL);
    }

    // 编码 jal 指令
    uint32_t opcodeVal = 0x6F; // opcode for jal

    uint32_t imm20 = (imm >> 20) & 0x1;     // imm[20]
    uint32_t imm10_1 = (imm >> 1) & 0x3FF;  // imm[10:1]
    uint32_t imm11 = (imm >> 11) & 0x1;     // imm[11]
    uint32_t imm19_12 = (imm >> 12) & 0xFF; // imm[19:12]

    uint32_t instruction = 0;
    instruction |= (imm20 << 31);
    instruction |= (imm19_12 << 12);
    instruction |= (imm11 << 20);
    instruction |= (imm10_1 << 21);
    instruction |= (rd & 0x1F) << 7;
    instruction |= opcodeVal;

    instructions.push_back(instruction);
  }
  else if (expandedOpcode == "call_expand")
  {
    // 处理 call 指令的展开
    std::string label = expandedOperands[0];
    uint32_t rd = 1;     // x1，用于保存返回地址
    uint32_t tmpReg = 5; // 使用 x5 作为临时寄存器

    if (symbolTable.hasSymbol(label))
    {
      uint32_t labelAddress = symbolTable.getSymbol(label).getGAddress();
      int32_t offset = static_cast<int32_t>(labelAddress) - static_cast<int32_t>(currentAddress);

      uint32_t auipcImm = (offset + 0x800) >> 12;
      int32_t jalrImm = offset - (auipcImm << 12);

      // 生成 auipc 指令
      uint32_t auipcInstr = 0x17; // opcode for auipc
      auipcInstr |= (tmpReg & 0x1F) << 7;
      auipcInstr |= (auipcImm & 0xFFFFF) << 12;
      instructions.push_back(auipcInstr);

      // 生成 jalr 指令
      uint32_t jalrInstr = 0x67; // opcode for jalr
      jalrInstr |= (rd & 0x1F) << 7;
      jalrInstr |= (0x0 & 0x7) << 12; // funct3 = 0
      jalrInstr |= (tmpReg & 0x1F) << 15;
      jalrInstr |= (jalrImm & 0xFFF) << 20;
      instructions.push_back(jalrInstr);
    }
    else
    {
      // 符号未知，添加重定位条目
      // 生成 auipc 指令
      uint32_t auipcInstr = 0x17; // opcode for auipc
      auipcInstr |= (tmpReg & 0x1F) << 7;
      instructions.push_back(auipcInstr);

      relocationTable.addRelocation(
          currentSecName,
          currentAddress,
          label,
          RelocationType::R_RISCV_PCREL_HI20);

      // 生成 jalr 指令
      uint32_t jalrInstr = 0x67; // opcode for jalr
      jalrInstr |= (rd & 0x1F) << 7;
      jalrInstr |= (0x0 & 0x7) << 12; // funct3 = 0
      jalrInstr |= (tmpReg & 0x1F) << 15;
      instructions.push_back(jalrInstr);

      relocationTable.addRelocation(
          currentSecName,
          currentAddress + 4,
          label,
          RelocationType::R_RISCV_PCREL_LO12_I);
    }
  }
  else if (expandedOpcode == "jalr")
  {
    // 处理 jalr 指令
    uint32_t rd = Utils::getRegisterNumber(expandedOperands[0]);
    uint32_t rs1 = Utils::getRegisterNumber(expandedOperands[1]);
    int32_t imm = Utils::stringToImmediate(expandedOperands[2]);

    // 检查立即数范围
    if (imm < -2048 || imm > 2047)
    {
      throw std::runtime_error("Immediate value out of range for jalr: " + std::to_string(imm));
    }

    uint32_t instruction = 0x67; // opcode for jalr
    instruction |= (rd & 0x1F) << 7;
    instruction |= (0x0 & 0x7) << 12; // funct3 = 0
    instruction |= (rs1 & 0x1F) << 15;
    instruction |= (imm & 0xFFF) << 20;

    instructions.push_back(instruction);
  }
  else
  {
    throw std::runtime_error("Unsupported expanded pseudo-instruction: " + expandedOpcode);
  }

  return instructions;
}
