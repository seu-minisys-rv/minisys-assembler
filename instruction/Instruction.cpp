// instruction/Instruction.cpp

#include "Instruction.hpp"
#include "InstructionI.hpp"
#include "InstructionL.hpp"
#include "InstructionR.hpp"
#include "InstructionS.hpp"
#include "InstructionB.hpp"
#include "InstructionU.hpp"
#include "InstructionJ.hpp"
#include "InstructionP.hpp"
#include "InstructionM.hpp"

// 包含其他指令类型的头文件，如 InstructionR.hpp 等

#include "../utils/Utils.hpp"
#include <stdexcept>
#include <sstream>

void Instruction::parseLine(const std::string &line)
{
  std::string trimmedLine = Utils::trim(line);

  // 找到操作码
  size_t pos = trimmedLine.find('\t');
  if (pos == std::string::npos)
  {
    opcode = trimmedLine;
    return;
  }

  opcode = trimmedLine.substr(0, pos);
  std::string operandStr = trimmedLine.substr(pos + 1);
  operandStr = Utils::trim(operandStr);

  // 自定义解析操作数
  size_t index = 0;
  while (index < operandStr.size())
  {
    // 跳过分隔符和空白字符
    while (index < operandStr.size() && (operandStr[index] == ',' || isspace(operandStr[index])))
    {
      index++;
    }
    if (index >= operandStr.size())
    {
      break;
    }

    std::string operand;
    if (operandStr[index] == '(')
    {
      // 处理括号内的内容
      size_t endIndex = operandStr.find(')', index);
      if (endIndex == std::string::npos)
      {
        throw std::runtime_error("Unmatched parenthesis in operand: " + operandStr);
      }
      operand = operandStr.substr(index, endIndex - index + 1);
      index = endIndex + 1;
    }
    else if (operandStr[index] == '%')
    {
      // 处理特殊表达式，例如 %hi(sa)
      size_t endIndex = index;
      while (endIndex < operandStr.size() && operandStr[endIndex] != ',' && !isspace(operandStr[endIndex]))
      {
        endIndex++;
      }
      operand = operandStr.substr(index, endIndex - index);
      index = endIndex;
    }
    else
    {
      // 处理一般操作数
      size_t endIndex = index;
      while (endIndex < operandStr.size() && operandStr[endIndex] != ',' && !isspace(operandStr[endIndex]))
      {
        endIndex++;
      }
      operand = operandStr.substr(index, endIndex - index);
      index = endIndex;
    }

    operands.push_back(Utils::trim(operand));
  }
}

// 指令工厂方法，根据操作码创建对应的指令对象
std::unique_ptr<Instruction> Instruction::create(const std::string &line)
{
  // 临时创建一个指令对象以解析操作码
  class TempInstruction : public Instruction
  {
  public:
    TempInstruction(const std::string &line)
    {
      parseLine(line);
    }
    std::vector<uint32_t> encode(const SymbolTable &, RelocationTable &, std::unordered_map<std::string, Section> &, uint32_t, std::string) override
    {
      // 不需要实现
    }
  };

  TempInstruction temp(line);
  const std::string &opcode = temp.opcode;

  // 根据操作码创建对应的指令对象
  if (opcode == "addi" || opcode == "ori" || opcode == "andi" ||
      opcode == "xori" || opcode == "slli" || opcode == "srli" || opcode == "jalr")
  {
    return std::make_unique<InstructionI>(line);
  }
  else if (opcode == "lb" || opcode == "lh" || opcode == "lw" || opcode == "ld")
  {
    return std::make_unique<InstructionL>(line);
  }
  // R 型指令
  else if (opcode == "add" || opcode == "sub" || opcode == "and" ||
           opcode == "or" || opcode == "xor" || opcode == "sll" ||
           opcode == "srl")
  {
    return std::make_unique<InstructionR>(line);
  }
  // S 型指令
  else if (opcode == "sb" || opcode == "sh" || opcode == "sw" || opcode == "sd")
  {
    return std::make_unique<InstructionS>(line);
  }
  // B 型指令
  else if (opcode == "beq" || opcode == "bne" || opcode == "blt" ||
           opcode == "bge" || opcode == "bltu" || opcode == "bgeu")
  {
    return std::make_unique<InstructionB>(line);
  }
  // U 型指令
  else if (opcode == "lui" || opcode == "auipc")
  {
    return std::make_unique<InstructionU>(line);
  }
  // J 型指令
  else if (opcode == "jal")
  {
    return std::make_unique<InstructionJ>(line);
  }
  // P 型伪指令
  else if (opcode == "mv" || opcode == "li" || opcode == "j" ||
           opcode == "nop" || opcode == "call" || opcode == "ret")
  {
    return std::make_unique<InstructionP>(line);
  }
  // M 型指令
  else if (opcode == "mul" || opcode == "mulh" || opcode == "mulhsu" ||
           opcode == "mulhu" || opcode == "div" || opcode == "rem" || opcode == "remu")
  {
    return std::make_unique<InstructionM>(line);
  }
  else
  {
    throw std::runtime_error("Unsupported instruction: " + opcode);
  }
  {
    throw std::runtime_error("Unsupported instruction: " + opcode);
  }
}
