#include "RelocationTable.hpp"

void RelocationTable::addRelocation(const std::string &sectionName, uint32_t offset, const std::string &symbolName, RelocationType type, int32_t addend)
{
  relocations[sectionName].push_back({sectionName, offset, symbolName, type, addend});
}

const std::unordered_map<std::string, std::vector<RelocationEntry>> &RelocationTable::getRelocations() const
{
  return relocations;
}
