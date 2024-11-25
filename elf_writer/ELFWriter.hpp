// elf_writer/ELFWriter.hpp

#ifndef ELF_WRITER_HPP
#define ELF_WRITER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <libelf.h>
#include <gelf.h>
#include <fcntl.h>
#include <unistd.h>
#include "../symbol_table/SymbolTable.hpp"
#include "../section/Section.hpp"
#include "../relocation_table/RelocationTable.hpp"

class ELFWriter
{
public:
  ELFWriter(const std::string &outputFile,
            SymbolTable &symbolTable,
            std::unordered_map<std::string, std::vector<Section>> &segSecTable,
            RelocationTable &relocationTable);

  void write();

private:
  void initElf();
  void createElfHeader();
  void createSections();
  void createSymbolTable();
  void createRelocationSections(); // 新增的函数，用于处理重定位表
  void writeToFile();
  void finalize();

  // 辅助函数
  size_t addToStrTab(const std::string &str);
  size_t addToShStrTab(const std::string &str);

private:
  std::string outputFile;
  SymbolTable &symbolTable;
  std::unordered_map<std::string, std::vector<Section>> &segSecTable;
  RelocationTable &relocationTable; // 引用重定位表

  int fd;
  Elf *elf;
  GElf_Ehdr ehdr;

  // 段索引
  size_t shstrtabIndex; // 段头字符串表（.shstrtab）的段索引
  size_t strtabIndex;   // 字符串表（.strtab）的段索引
  size_t symtabIndex;   // 符号表（.symtab）的段索引

  // 段数据
  std::vector<char> shstrtabData; // 段头字符串表的数据，存储所有段名（Section 名称）
  std::vector<char> strtabData;   // 字符串表的数据，存储所有符号名

  // 从段名到 Elf_Scn（ELF 段的句柄） 的映射
  std::unordered_map<std::string, Elf_Scn *> sectionMap;
  // 从符号名到符号表索引的映射
  std::unordered_map<std::string, size_t> symbolIndices;
};

#endif // ELF_WRITER_HPP
