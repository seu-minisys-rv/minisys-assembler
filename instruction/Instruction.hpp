// instruction/Instruction.hpp

#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

#include <string>
#include <memory>
#include <vector>
#include "../symbol_table/SymbolTable.hpp"
#include "../relocation_table/RelocationTable.hpp"
#include "../section/Section.hpp"

class Instruction
{
public:
  // 析构函数
  virtual ~Instruction() = default;

  // 编码函数，返回32位机器码
  virtual std::vector<uint32_t> encode(const SymbolTable &symbolTable, RelocationTable &relocationTable, std::unordered_map<std::string, Section> &sectionTable, uint32_t currentAddress, std::string currentSecName) = 0;

  // 创建指令对象的工厂方法
  static std::unique_ptr<Instruction> create(const std::string &line);

public:
  // 解析指令行，提取操作码和操作数
  void parseLine(const std::string &line);

  std::string opcode;                // 操作码
  std::vector<std::string> operands; // 操作数列表
  std::string label;                 // 标签
  std::string immFunction;           // 立即数函数名
  std::string immSymbol;             // 立即数符号名
};

#endif // INSTRUCTION_HPP
