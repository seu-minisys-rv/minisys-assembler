#ifndef SECTION_HPP
#define SECTION_HPP

#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept>

class Section
{
public:
  // 构造函数，初始化段的名称、对齐、标志和类型
  Section();
  Section(const std::string &name);

  // 添加数据到段
  void addData(const std::vector<uint8_t> &data);

  // 添加一条机器码指令（32位）
  void addInstruction(std::vector<uint32_t> &instructions);

  // 对齐段内容
  void align(uint32_t alignment, uint32_t &saddress, uint32_t &gaddress, uint32_t &inSecAddress);

  // 获取段的名称
  const std::string &getName() const;

  // 获取段的数据内容
  const std::vector<uint8_t> &getData() const;

  uint32_t getBaseAddress() const;

  // 获取段的对齐值
  uint32_t getAlignment() const;

  // 获取段的标志
  const std::string &getFlags() const;

  // 获取段的类型
  const std::string &getType() const;

  const std::string &getSegmentName() const;
  uint32_t getFillValue() const;

  // 获取段的大小
  uint32_t getSize() const;

public:
  void setName(const std::string &name);
  void setAlignment(uint32_t alignment);
  void setFlags(const std::string &flags);
  void setType(const std::string &type);
  void setSegmentName(const std::string &segmentName);
  void setSectionSize(uint32_t size);
  void setFillValue(uint8_t fillValue);
  void setBaseAddress(uint32_t baseAddress);

private:
  std::string name;           // 段名称
  uint32_t alignment;         // 段对齐
  uint8_t fillValue;          // 填充值
  std::string flags;          // 段标志
  std::string type;           // 段类型
  std::string segmentName;    // 段所属节的名称
  std::uint32_t section_size; // 段大小
  std::vector<uint8_t> data;  // 段数据
  uint32_t baseAddress;       // 段的基地址
};

#endif // SECTION_HPP
