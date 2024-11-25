// #include "SymbolTable.hpp"
// #include <stdexcept>

// SymbolTable::SymbolTable()
// {
//   // 可以在这里初始化一些预定义符号（如果需要）
// }

// void SymbolTable::addSymbol(const std::string &name, uint32_t secAddress = 0, uint32_t gloAddress = 0, SymbolType type = SymbolType::UNDEFINED, bool isGlobal = false, std::string sectionName)
// {
//   auto it = symbols.find(name);
//   if (it == symbols.end())
//   {
//     // 符号不存在，创建新的符号
//     symbols[name] = {name, secAddress, gloAddress, type, isGlobal, 0, sectionName};
//   }
//   else
//   {
//     // 符号已存在，只更新提供的属性
//     if (type != SymbolType::UNDEFINED)
//     {
//       it->second.type = type;
//     }
//     it->second.isGlobal = isGlobal;
//   }
// }

// bool SymbolTable::hasSymbol(const std::string &name) const
// {
//   return symbols.find(name) != symbols.end();
// }

// Symbol SymbolTable::getSymbol(const std::string &name) const
// {
//   auto it = symbols.find(name);
//   if (it != symbols.end())
//   {
//     return it->second;
//   }
//   else
//   {
//     throw std::runtime_error("Symbol not found: " + name);
//   }
// }

// void SymbolTable::updateSymbolAddress(const std::string &name, uint32_t saddress, uint32_t gaddress)
// {
//   auto it = symbols.find(name);
//   if (it != symbols.end())
//   {
//     it->second.saddress = saddress;
//     it->second.gaddress = gaddress;
//   }
//   else
//   {
//     // 如果符号不存在，则创建新的符号并设置地址
//     Symbol newSymbol;
//     newSymbol.name = name;
//     newSymbol.saddress = saddress;
//     newSymbol.gaddress = gaddress;
//     newSymbol.type = SymbolType::UNDEFINED;
//     newSymbol.isGlobal = false;
//     symbols[name] = newSymbol;
//   }
// }

// void SymbolTable::setGlobal(const std::string &name, bool isGlobal)
// {
//   auto it = symbols.find(name);
//   if (it != symbols.end())
//   {
//     it->second.isGlobal = isGlobal;
//   }
//   else
//   {
//     // 如果符号不存在，则创建新的符号并设置全局标志
//     Symbol newSymbol;
//     newSymbol.name = name;
//     newSymbol.saddress = 0;
//     newSymbol.gaddress = 0;
//     newSymbol.type = SymbolType::UNDEFINED;
//     newSymbol.isGlobal = isGlobal;
//     symbols[name] = newSymbol;
//   }
// }

// void SymbolTable::setType(const std::string &name, SymbolType type)
// {
//   auto it = symbols.find(name);
//   if (it != symbols.end())
//   {
//     it->second.type = type;
//   }
//   else
//   {
//     // 如果符号不存在，则创建新的符号并设置类型
//     Symbol newSymbol;
//     newSymbol.name = name;
//     newSymbol.saddress = 0;
//     newSymbol.gaddress = 0;
//     newSymbol.type = type;
//     newSymbol.isGlobal = false;
//     symbols[name] = newSymbol;
//   }
// }

// void SymbolTable::setSize(const std::string &name, int size)
// {
//   auto it = symbols.find(name);
//   if (it != symbols.end())
//   {
//     it->second.size = size;
//   }
//   else
//   {
//     // 如果符号不存在，则创建新的符号并设置大小
//     Symbol newSymbol;
//     newSymbol.name = name;
//     newSymbol.saddress = 0;
//     newSymbol.gaddress = 0;
//     newSymbol.type = SymbolType::UNDEFINED;
//     newSymbol.isGlobal = false;
//     newSymbol.size = size;
//     symbols[name] = newSymbol;
//   }
// }

// std::string SymbolTable::getSectionName(const std::string &symbolName) const
// {
//   return symbols.at(symbolName).sectionName;
// }

// const std::unordered_map<std::string, Symbol> &SymbolTable::getAllSymbols() const
// {
//   return symbols;
// }

// void SymbolTable::setSectionName(const std::string &name, const std::string &sectionName)
// {
//   auto it = symbols.find(name);
//   if (it != symbols.end())
//   {
//     it->second.sectionName = sectionName;
//   }
//   else
//   {
//     // 如果符号不存在，则创建新的符号并设置段名称
//     Symbol newSymbol;
//     newSymbol.name = name;
//     newSymbol.saddress = 0;
//     newSymbol.gaddress = 0;
//     newSymbol.type = SymbolType::UNDEFINED;
//     newSymbol.isGlobal = false;
//     newSymbol.sectionName = sectionName;
//     symbols[name] = newSymbol;
//   }
// }
#include "SymbolTable.hpp"
#include <stdexcept>

// -------------------- Symbol 类实现 --------------------

Symbol::Symbol()
    : name(""), saddress(0), gaddress(0), type(SymbolType::UNDEFINED), globalFlag(false), size(0), sectionName("")
{
}

Symbol::Symbol(const std::string &name,
               uint32_t saddress,
               uint32_t gaddress,
               SymbolType type,
               bool isGlobal,
               int size,
               const std::string &sectionName)
    : name(name), saddress(saddress), gaddress(gaddress), type(type), globalFlag(isGlobal), size(size), sectionName(sectionName)
{
}

const std::string &Symbol::getName() const
{
  return name;
}

uint32_t Symbol::getSAddress() const
{
  return saddress;
}

uint32_t Symbol::getGAddress() const
{
  return gaddress;
}
uint32_t Symbol::getInSecAddress() const
{
  return inSecAddress;
}
SymbolType Symbol::getType() const
{
  return type;
}

bool Symbol::isGlobal() const
{
  return globalFlag;
}

int Symbol::getSize() const
{
  return size;
}

const std::string &Symbol::getSectionName() const
{
  return sectionName;
}
const std::string &Symbol::getSegmentName() const
{
  return segmentName;
}

void Symbol::setSAddress(uint32_t address)
{
  saddress = address;
}

void Symbol::setGAddress(uint32_t address)
{
  gaddress = address;
}
void Symbol::setInSecAddress(uint32_t inSecAddress)
{
  this->inSecAddress = inSecAddress;
}
void Symbol::setType(SymbolType type)
{
  this->type = type;
}

void Symbol::setGlobal(bool isGlobal)
{
  globalFlag = isGlobal;
}

void Symbol::setSize(int size)
{
  this->size = size;
}

void Symbol::setSectionName(const std::string &sectionName)
{
  this->sectionName = sectionName;
}
void Symbol::setSegmentName(const std::string &segmentName)
{
  this->segmentName = segmentName;
}

// -------------------- SymbolTable 类实现 --------------------

SymbolTable::SymbolTable()
{
  // 可以在这里初始化一些预定义符号（如果需要）
}

void SymbolTable::addSymbol(const std::string &name,
                            uint32_t saddress,
                            uint32_t gaddress,
                            SymbolType type,
                            bool isGlobal,
                            const std::string &sectionName)
{
  auto it = symbols.find(name);
  if (it == symbols.end())
  {
    // 符号不存在，创建新的符号
    symbols[name] = Symbol(name, saddress, gaddress, type, isGlobal, 0, sectionName);
  }
  else
  {
    // 符号已存在，只更新提供的属性
    if (type != SymbolType::UNDEFINED)
    {
      it->second.setType(type);
    }
    if (saddress != 0)
    {
      it->second.setSAddress(saddress);
    }
    if (gaddress != 0)
    {
      it->second.setGAddress(gaddress);
    }
    it->second.setGlobal(isGlobal);
    if (!sectionName.empty())
    {
      it->second.setSectionName(sectionName);
    }
  }
}

bool SymbolTable::hasSymbol(const std::string &name) const
{
  return symbols.find(name) != symbols.end();
}

Symbol &SymbolTable::getSymbol(const std::string &name)
{
  auto it = symbols.find(name);
  if (it != symbols.end())
  {
    return it->second;
  }
  else
  {
    throw std::runtime_error("Symbol not found: " + name);
  }
}

const Symbol &SymbolTable::getSymbol(const std::string &name) const
{
  auto it = symbols.find(name);
  if (it != symbols.end())
  {
    return it->second;
  }
  else
  {
    throw std::runtime_error("Symbol not found: " + name);
  }
}

void SymbolTable::updateSymbolAddress(const std::string &name, uint32_t saddress, uint32_t gaddress, uint32_t inSecAddress)
{
  auto it = symbols.find(name);
  if (it != symbols.end())
  {
    it->second.setSAddress(saddress);
    it->second.setGAddress(gaddress);
    it->second.setInSecAddress(inSecAddress);
  }
  else
  {
    // 如果符号不存在，则创建新的符号并设置地址
    symbols[name] = Symbol(name, saddress, gaddress);
  }
}

void SymbolTable::setGlobal(const std::string &name, bool isGlobal)
{
  auto it = symbols.find(name);
  if (it != symbols.end())
  {
    it->second.setGlobal(isGlobal);
  }
  else
  {
    // 如果符号不存在，则创建新的符号并设置全局标志
    symbols[name] = Symbol(name);
    symbols[name].setGlobal(isGlobal);
  }
}

void SymbolTable::setType(const std::string &name, SymbolType type)
{
  auto it = symbols.find(name);
  if (it != symbols.end())
  {
    it->second.setType(type);
  }
  else
  {
    // 如果符号不存在，则创建新的符号并设置类型
    symbols[name] = Symbol(name);
    symbols[name].setType(type);
  }
}

void SymbolTable::setSize(const std::string &name, int size)
{
  auto it = symbols.find(name);
  if (it != symbols.end())
  {
    it->second.setSize(size);
  }
  else
  {
    // 如果符号不存在，则创建新的符号并设置大小
    symbols[name] = Symbol(name);
    symbols[name].setSize(size);
  }
}

void SymbolTable::setSectionName(const std::string &name, const std::string &sectionName)
{
  auto it = symbols.find(name);
  if (it != symbols.end())
  {
    it->second.setSectionName(sectionName);
  }
  else
  {
    // 如果符号不存在，则创建新的符号并设置段名称
    symbols[name] = Symbol(name);
    symbols[name].setSectionName(sectionName);
  }
}

void SymbolTable::setSegmentName(const std::string &name, const std::string &segmentName)
{
  auto it = symbols.find(name);
  if (it != symbols.end())
  {
    it->second.setSegmentName(segmentName);
  }
  else
  {
    // 如果符号不存在，则创建新的符号并设置段名称
    symbols[name] = Symbol(name);
    symbols[name].setSegmentName(segmentName);
  }
}

std::string SymbolTable::getSectionName(const std::string &symbolName) const
{
  auto it = symbols.find(symbolName);
  if (it != symbols.end())
  {
    return it->second.getSectionName();
  }
  else
  {
    throw std::runtime_error("Symbol not found: " + symbolName);
  }
}

std::unordered_map<std::string, Symbol> &SymbolTable::getSymbols()
{
  return symbols;
}
void setSegmentName(const std::string &segmentName);