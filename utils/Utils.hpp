// utils/Utils.h

#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>

class Utils
{
private:
  // 寄存器名称到编号的映射表
  static const std::unordered_map<std::string, uint32_t> regMap;

public:
  // 去除字符串首尾的空白字符
  static std::string trim(const std::string &str);

  // 将表示立即数的字符串转换为整数，支持十进制和十六进制格式
  static int32_t stringToImmediate(const std::string &str);

  // 将 32 位整数转换为小端序字节序列
  static std::vector<uint8_t> intToBytes(uint32_t value);

  static uint32_t getRegisterNumber(const std::string &regName);

  static uint32_t alignAddress(uint32_t address, uint32_t alignment);

  static std::vector<std::string> split(const std::string &str, char delimiter);
  static bool isNumber(const std::string &str);
  static uint32_t toLittleEndian(uint32_t value);
  template <typename T>
  static void writeBinary(std::ostream &os, const T &value)
  {
    static_assert(std::is_trivially_copyable<T>::value, "类型必须是可平凡复制的");
    os.write(reinterpret_cast<const char *>(&value), sizeof(T));
  }
};

#endif // UTILS_H
