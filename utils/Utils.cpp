// utils/utils.cpp

#include "Utils.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>

const std::unordered_map<std::string, uint32_t> Utils::regMap = {
    // 通用寄存器
    {"zero", 0},
    {"x0", 0},
    {"ra", 1},
    {"x1", 1},
    {"sp", 2},
    {"x2", 2},
    {"gp", 3},
    {"x3", 3},
    {"tp", 4},
    {"x4", 4},
    {"t0", 5},
    {"x5", 5},
    {"t1", 6},
    {"x6", 6},
    {"t2", 7},
    {"x7", 7},
    {"s0", 8},
    {"fp", 8},
    {"x8", 8},
    {"s1", 9},
    {"x9", 9},
    {"a0", 10},
    {"x10", 10},
    {"a1", 11},
    {"x11", 11},
    {"a2", 12},
    {"x12", 12},
    {"a3", 13},
    {"x13", 13},
    {"a4", 14},
    {"x14", 14},
    {"a5", 15},
    {"x15", 15},
    {"a6", 16},
    {"x16", 16},
    {"a7", 17},
    {"x17", 17},
    {"s2", 18},
    {"x18", 18},
    {"s3", 19},
    {"x19", 19},
    {"s4", 20},
    {"x20", 20},
    {"s5", 21},
    {"x21", 21},
    {"s6", 22},
    {"x22", 22},
    {"s7", 23},
    {"x23", 23},
    {"s8", 24},
    {"x24", 24},
    {"s9", 25},
    {"x25", 25},
    {"s10", 26},
    {"x26", 26},
    {"s11", 27},
    {"x27", 27},
    {"t3", 28},
    {"x28", 28},
    {"t4", 29},
    {"x29", 29},
    {"t5", 30},
    {"x30", 30},
    {"t6", 31},
    {"x31", 31}};

std::string Utils::trim(const std::string &str)
{
  size_t first = str.find_first_not_of(" \t\n\r");
  if (first == std::string::npos)
  {
    return ""; // 全是空白字符
  }
  size_t last = str.find_last_not_of(" \t\n\r");
  return str.substr(first, (last - first + 1));
}

int32_t Utils::stringToImmediate(const std::string &str)
{
  std::string s = trim(str);
  try
  {
    if (s.substr(0, 2) == "0x" || s.substr(0, 2) == "0X")
    {
      // 十六进制数
      return static_cast<int32_t>(std::stol(s, nullptr, 16));
    }
    else
    {
      // 十进制数，可能带有正负号
      return static_cast<int32_t>(std::stol(s, nullptr, 10));
    }
  }
  catch (const std::invalid_argument &e)
  {
    throw std::runtime_error("Invalid immediate value: " + s);
  }
  catch (const std::out_of_range &e)
  {
    throw std::runtime_error("Immediate value out of range: " + s);
  }
}

std::vector<uint8_t> Utils::intToBytes(uint32_t value)
{
  std::vector<uint8_t> bytes(4);
  bytes[0] = static_cast<uint8_t>(value & 0xFF);         // 低 8 位
  bytes[1] = static_cast<uint8_t>((value >> 8) & 0xFF);  // 第 9-16 位
  bytes[2] = static_cast<uint8_t>((value >> 16) & 0xFF); // 第 17-24 位
  bytes[3] = static_cast<uint8_t>((value >> 24) & 0xFF); // 高 8 位
  return bytes;
}

uint32_t Utils::getRegisterNumber(const std::string &regName)
{
  auto it = regMap.find(regName);
  if (it != regMap.end())
  {
    return it->second;
  }
  else
  {
    throw std::runtime_error("Invalid register name: " + regName);
  }
}

uint32_t Utils::alignAddress(uint32_t address, uint32_t alignment)
{
  if (alignment == 0)
  {
    return address; // 不对齐，直接返回原地址
  }
  return (address + alignment - 1) & ~(alignment - 1);
}

// 按指定字符分割字符串
std::vector<std::string> Utils::split(const std::string &str, char delimiter)
{
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream tokenStream(str);

  while (std::getline(tokenStream, token, delimiter))
  {
    token = Utils::trim(token); // 去除每个分割部分的首尾空格
    if (!token.empty())
    { // 跳过空字符串
      tokens.push_back(token);
    }
  }
  return tokens;
}
bool Utils::isNumber(const std::string &str)
{
  if (str.empty())
    return false;

  size_t start = 0;
  // 检查第一个字符是否为 '+' 或 '-'
  if (str[0] == '+' || str[0] == '-')
  {
    if (str.length() == 1)
      return false; // 只有一个 '+' 或 '-'，不是数字
    start = 1;
  }

  // 检查剩余的字符是否都是数字
  for (size_t i = start; i < str.length(); ++i)
  {
    if (!std::isdigit(str[i]))
      return false;
  }

  return true;
}

// 检查系统是否为小端序
bool isSystemLittleEndian()
{
  uint16_t number = 0x1;
  return reinterpret_cast<uint8_t *>(&number)[0] == 1;
}

// 字节序转换函数（当系统为大端序时，将数据转换为小端序）
uint32_t Utils::toLittleEndian(uint32_t value)
{
  if (isSystemLittleEndian())
  {
    return value; // 系统为小端序，无需转换
  }
  else
  {
    // 系统为大端序，进行字节序转换
    return ((value & 0xFF000000) >> 24) |
           ((value & 0x00FF0000) >> 8) |
           ((value & 0x0000FF00) << 8) |
           ((value & 0x000000FF) << 24);
  }
}
