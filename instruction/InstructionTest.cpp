#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <cctype>

// 工具函数，用于修剪字符串两端的空白字符
std::string trim(const std::string &str)
{
  size_t first = str.find_first_not_of(" \t\n\r");
  if (first == std::string::npos)
    return "";
  size_t last = str.find_last_not_of(" \t\n\r");
  return str.substr(first, last - first + 1);
}

// 独立的解析函数，用于测试逻辑
void parseLine(const std::string &line, std::string &opcode, std::vector<std::string> &operands)
{
  std::string trimmedLine = trim(line);

  // 找到操作码
  size_t pos = trimmedLine.find(' ');
  if (pos == std::string::npos)
  {
    opcode = trimmedLine;
    return;
  }

  opcode = trimmedLine.substr(0, pos);
  std::string operandStr = trimmedLine.substr(pos + 1);
  operandStr = trim(operandStr);

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

    operands.push_back(trim(operand));
  }
}
int main()
{
  struct TestCase
  {
    std::string line;
    std::string expectedOpcode;
    std::vector<std::string> expectedOperands;
  };

  std::vector<TestCase> testCases = {
      {"addi a0, a0, 5", "addi", {"a0", "a0", "5"}},
      {"lui a1, %hi(sa)", "lui", {"a1", "%hi(sa)"}},
      {"sw s0, 40(sp)", "sw", {"s0", "40(sp)"}},
      {"call factorial", "call", {"factorial"}},
      {"li a0, 5", "li", {"a0", "5"}},
      {"ret", "ret", {}},
      {"jal x1, label", "jal", {"x1", "label"}},
      {"beq a0, a1, loop", "beq", {"a0", "a1", "loop"}}};

  for (const auto &testCase : testCases)
  {
    std::string opcode;
    std::vector<std::string> operands;

    // 调用解析函数
    parseLine(testCase.line, opcode, operands);

    // 验证操作码
    assert(opcode == testCase.expectedOpcode && "Opcode mismatch");

    // 验证操作数的数量
    assert(operands.size() == testCase.expectedOperands.size() && "Operand count mismatch");

    // 验证每个操作数
    for (size_t i = 0; i < operands.size(); ++i)
    {
      assert(operands[i] == testCase.expectedOperands[i] && "Operand mismatch");
    }

    std::cout << "Test passed for: " << testCase.line << std::endl;
  }

  std::cout << "All tests passed successfully!" << std::endl;
  return 0;
}
