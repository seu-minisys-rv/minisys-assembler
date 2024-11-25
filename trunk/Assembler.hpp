#ifndef ASSEMBLER_HPP
#define ASSEMBLER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <type_traits>
#include "../symbol_table/SymbolTable.hpp"
#include "../relocation_table/RelocationTable.hpp"
#include "../instruction/Instruction.hpp"
#include "../section/Section.hpp"

class Assembler
{
public:
  void assemble(const std::string &inputFile, const std::string &outputFile, bool);

private:
  void firstPass();
  void secondPass();
  void writeFile(const std::string &outputFile);
  std::vector<std::string> readAssemblyCode(const std::string &filename);
  void initializeSegments();

private:
  void handleTypeDirective(std::istringstream &iss, std::string &currentSecName);
  void handleGloblDirective(std::istringstream &iss);
  void handleSectionDirective(std::istringstream &iss, std::string &currentSecName);
  void handleP2AlignDirective(std::istringstream &iss, const std::string &currentSecName);
  void handleSizeDirective(std::istringstream &iss, const std::string &currentSecName);
  void handleWordDirective(std::istringstream &iss, const std::string &currentSecName);
  void handleAscizDirective(std::istringstream &iss, const std::string &currentSecName);
  void handleInstruction(const int address, const std::string &line, const std::string &currentSecName);
  void handleSegmentTable();
  // 哈希表1：sec_name -> Section
  std::unordered_map<std::string, Section> sectionTable;

  // 哈希表2：段名 -> vector<Section>
  std::unordered_map<std::string, std::vector<Section>> segSecTable;

  std::unordered_map<std::string, std::vector<std::pair<int, std::string>>> instructionTable;
  // std::unordered_map<std::string, std::vector<std::string>> secSymbolTable;

  std::unordered_map<std::string, std::pair<uint32_t, uint32_t>> baseAddressTable;
  std::unordered_map<std::string, uint32_t> segAddressTable;
  std::vector<std::pair<uint32_t, std::string>> instructionVector;
  std::vector<uint32_t> instructionResult;
  bool isUsingElfWriter = false;

  SymbolTable symbolTable;
  RelocationTable relocationTable;
  std::string currentSecName = "";
  std::vector<std::string> lines; // 汇编代码的行集合
  uint32_t saddress = 0;
  uint32_t gaddress = 0;
  uint32_t inSecAddress = 0;
  bool isText = false;
};

#endif // ASSEMBLER_HPP
