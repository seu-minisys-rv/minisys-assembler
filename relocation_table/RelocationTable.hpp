#ifndef RELOCATIONTABLE_HPP
#define RELOCATIONTABLE_HPP

#include <vector>
#include <string>
#include <unordered_map>

enum RelocationType
{
  R_RISCV_NONE = 0,          // 无重定位
  R_RISCV_32 = 1,            // 32 位绝对重定位
  R_RISCV_64 = 2,            // 64 位绝对重定位（如适用）
  R_RISCV_RELATIVE = 3,      // 相对地址重定位
  R_RISCV_COPY = 4,          // 运行时复制符号（用于动态链接）
  R_RISCV_JUMP_SLOT = 5,     // 设置用于过程链接表（PLT）的跳转槽
  R_RISCV_BRANCH = 16,       // PC 相对分支重定位（用于 B 型指令）
  R_RISCV_JAL = 17,          // PC 相对跳转重定位（用于 JAL 指令）
  R_RISCV_CALL = 18,         // PC 相对调用重定位（用于函数调用）
  R_RISCV_CALL_PLT = 19,     // 通过 PLT 进行的函数调用重定位
  R_RISCV_PCREL_HI20 = 23,   // 符号相对于 PC 的高 20 位重定位（用于 AUIPC）
  R_RISCV_PCREL_LO12_I = 24, // 符号相对于 PC 的低 12 位重定位，适用于 I 型指令
  R_RISCV_PCREL_LO12_S = 25, // 符号相对于 PC 的低 12 位重定位，适用于 S 型指令
  R_RISCV_HI20 = 26,         // 符号绝对地址的高 20 位重定位
  R_RISCV_LO12_I = 27,       // 符号绝对地址的低 12 位重定位，适用于 I 型指令
  R_RISCV_LO12_S = 28,       // 符号绝对地址的低 12 位重定位，适用于 S 型指令
  // 根据需要可以添加更多重定位类型
};

struct RelocationEntry
{
  std::string segmentName; // 重定位所在的节名
  uint32_t offset;         // 重定位位置在节内的偏移
  std::string symbolName;  // 符号名称
  RelocationType type;     // 重定位类型
  int32_t addend;          // 附加值
};

class RelocationTable
{
public:
  // 添加一个新的重定位项
  void addRelocation(const std::string &sectionName, uint32_t offset, const std::string &symbolName, RelocationType type, int32_t addend = 0);

  // 获取所有重定位项
  const std::unordered_map<std::string, std::vector<RelocationEntry>> &getRelocations() const;

private:
  std::unordered_map<std::string, std::vector<RelocationEntry>> relocations; // 段名到重定位项列表的映射
};

#endif // RELOCATIONTABLE_HPP
