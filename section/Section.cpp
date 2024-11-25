#include "Section.hpp"

Section::Section()
{
  name = "";
  alignment = 1 << 2;
  fillValue = 0x0;
  flags = "";
  type = "";
  baseAddress = 0;
}

// 构造函数：初始化段的名称、对齐、标志和类型
Section::Section(const std::string &name)
    : name(name)
{
  alignment = 1 << 2;
  fillValue = 0x0;
}

// 添加数据到段
void Section::addData(const std::vector<uint8_t> &dataToAdd)
{
  if (dataToAdd.empty())
  {
    throw std::runtime_error("Data to add cannot be empty");
  }
  data.insert(data.end(), dataToAdd.begin(), dataToAdd.end());
}

// 添加一条32位机器码指令（小端序）
void Section::addInstruction(std::vector<uint32_t> &instructions)
{
  for (auto instruction : instructions)
  {
    data.push_back(instruction & 0xFF);
    data.push_back((instruction >> 8) & 0xFF);
    data.push_back((instruction >> 16) & 0xFF);
    data.push_back((instruction >> 24) & 0xFF);
  }
}

// 对齐段内容到指定字节边界，并填充指定的字节值
void Section::align(uint32_t alignment, uint32_t &saddress, uint32_t &gaddress, uint32_t &inSecAddress)
{
  while (data.size() % alignment != 0)
  {
    data.push_back(fillValue);
    saddress++;
    gaddress++;
    inSecAddress++;
  }
}

// 获取段名称
const std::string &Section::getName() const
{
  return name;
}

// 获取段的数据内容
const std::vector<uint8_t> &Section::getData() const
{
  return data;
}

// 获取段对齐值
uint32_t Section::getAlignment() const
{
  return alignment;
}

// 获取段标志
const std::string &Section::getFlags() const
{
  return flags;
}

// 获取段类型
const std::string &Section::getType() const
{
  return type;
}

// 获取段的大小
uint32_t Section::getSize() const
{
  return data.size();
}
// 设置段名称
void Section::setName(const std::string &name)
{
  this->name = name;
}

// 设置段对齐值
void Section::setAlignment(uint32_t alignment)
{
  this->alignment = alignment;
}

// 设置段标志
void Section::setFlags(const std::string &flags)
{
  this->flags = flags;
}

// 设置段类型
void Section::setType(const std::string &type)
{
  this->type = type;
}

// 设置段的大小
void Section::setSectionSize(uint32_t size)
{
  // data.resize(size);
  section_size = size;
}
void Section::setSegmentName(const std::string &segmentName)
{
  this->segmentName = segmentName;
}
void Section::setFillValue(uint8_t fillValue)
{
  this->fillValue = fillValue;
}
const std::string &Section::getSegmentName() const
{
  return segmentName;
}
void Section::setBaseAddress(uint32_t baseAddress)
{
  this->baseAddress = baseAddress;
}
uint32_t Section::getFillValue() const
{
  return fillValue;
}
uint32_t Section::getBaseAddress() const
{
  return baseAddress;
}