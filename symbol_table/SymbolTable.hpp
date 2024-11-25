// #ifndef SYMBOL_TABLE_H
// #define SYMBOL_TABLE_H

// #include <string>
// #include <unordered_map>
// #include <cstdint>
// #include <vector>

// // 符号的类型
// enum class SymbolType
// {
//   LABEL,
//   FUNCTION,
//   OBJECT,
//   UNDEFINED
// };

// // 符号结构
// struct Symbol
// {
//   std::string name;                        // 符号名称
//   uint32_t saddress = 0;                   // 符号地址
//   uint32_t gaddress = 0;                   // 全局符号地址,用于计算size
//   SymbolType type = SymbolType::UNDEFINED; // 符号类型，默认为 UNDEFINED
//   bool isGlobal = false;                   // 是否为全局符号
//   int size = 0;                            // 符号大小，默认为 0
//   std::string sectionName;                 // 符号所在sec的名称
// };

// class SymbolTable
// {
// public:
//   SymbolTable();

//   // 添加符号到符号表
//   void addSymbol(const std::string &name, uint32_t secAddress = 0, uint32_t gloAddress = 0, SymbolType type = SymbolType::UNDEFINED, bool isGlobal = false, std::string sectionName);

//   // 检查符号是否存在
//   bool hasSymbol(const std::string &name) const;

//   // 获取符号信息
//   Symbol getSymbol(const std::string &name) const;

//   // 更新符号的地址
//   void updateSymbolAddress(const std::string &name, uint32_t saddress, uint32_t gaddress);

//   // 设置符号为全局
//   void setGlobal(const std::string &name, bool isGlobal);

//   void setSectionName(const std::string &name, const std::string &sectionName);

//   // 设置符号的类型
//   void setType(const std::string &name, SymbolType type);

//   // 设置符号的大小
//   void setSize(const std::string &name, int size);

//   std::string getSectionName(const std::string &symbolName) const;

//   // 获取所有符号
//   const std::unordered_map<std::string, Symbol> &getAllSymbols() const;

// private:
//   std::unordered_map<std::string, Symbol> symbols;
// };

// #endif // SYMBOL_TABLE_H
#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <string>
#include <unordered_map>
#include <cstdint>

// 符号的类型
enum class SymbolType
{
  LABEL,
  FUNCTION,
  OBJECT,
  UNDEFINED
};

// 符号类
class Symbol
{
public:
  Symbol();
  Symbol(const std::string &name,
         uint32_t saddress = 0,
         uint32_t gaddress = 0,
         SymbolType type = SymbolType::UNDEFINED,
         bool isGlobal = false,
         int size = 0,
         const std::string &sectionName = "");

  // Getter 方法
  const std::string &getName() const;
  uint32_t getSAddress() const;
  uint32_t getGAddress() const;
  SymbolType getType() const;
  bool isGlobal() const;
  int getSize() const;
  const std::string &getSectionName() const;
  const std::string &getSegmentName() const;
  uint32_t getInSecAddress() const;

  // Setter 方法
  void setSAddress(uint32_t address);
  void setGAddress(uint32_t address);
  void setType(SymbolType type);
  void setGlobal(bool isGlobal);
  void setSize(int size);
  void setSectionName(const std::string &sectionName);
  void setSegmentName(const std::string &segmentName);
  void setInSecAddress(uint32_t inSecAddress);

private:
  std::string name;          // 符号名称
  uint32_t saddress = 0;     // 符号地址（段内地址）
  uint32_t gaddress = 0;     // 全局符号地址，用于计算 size
  uint32_t inSecAddress = 0; // 段内地址
  SymbolType type;           // 符号类型
  bool globalFlag;           // 是否为全局符号
  int size = 0;              // 符号大小
  std::string sectionName;   // 符号所在段的名称
  std::string segmentName;   // 符号所在节的名称
};

class SymbolTable
{
public:
  SymbolTable();

  // 添加符号到符号表
  void addSymbol(const std::string &name,
                 uint32_t saddress = 0,
                 uint32_t gaddress = 0,
                 SymbolType type = SymbolType::UNDEFINED,
                 bool isGlobal = false,
                 const std::string &sectionName = "");

  // 检查符号是否存在
  bool hasSymbol(const std::string &name) const;

  // 获取符号信息
  Symbol &getSymbol(const std::string &name);
  const Symbol &getSymbol(const std::string &name) const;

  // 更新符号的地址
  void updateSymbolAddress(const std::string &name, uint32_t saddress, uint32_t gaddress, uint32_t inSecAddress);

  // 设置符号为全局
  void setGlobal(const std::string &name, bool isGlobal);

  // 设置符号的类型
  void setType(const std::string &name, SymbolType type);

  // 设置符号的大小
  void setSize(const std::string &name, int size);

  // 设置符号的段名称
  void setSectionName(const std::string &name, const std::string &sectionName);

  void setSegmentName(const std::string &name, const std::string &segmentName);

  // 获取符号的段名称
  std::string getSectionName(const std::string &symbolName) const;

  // 获取所有符号
  std::unordered_map<std::string, Symbol> &getSymbols();

private:
  std::unordered_map<std::string, Symbol> symbols;
};

#endif // SYMBOL_TABLE_HPP
