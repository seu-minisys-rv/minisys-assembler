// elf_writer/ELFWriter.cpp

#include "ELFWriter.hpp"
#include <cstring>
#include <stdexcept>
#include <iostream>

ELFWriter::ELFWriter(const std::string &outputFile,
                     SymbolTable &symbolTable,
                     std::unordered_map<std::string, std::vector<Section>> &segSecTable,
                     RelocationTable &relocationTable)
    : outputFile(outputFile), symbolTable(symbolTable), segSecTable(segSecTable), relocationTable(relocationTable), fd(-1), elf(nullptr)
{
  initElf();
}

void ELFWriter::initElf()
{
  if (elf_version(EV_CURRENT) == EV_NONE)
  {
    throw std::runtime_error("ELF 库初始化失败");
  }

  fd = open(outputFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd < 0)
  {
    throw std::runtime_error("无法打开输出文件");
  }

  elf = elf_begin(fd, ELF_C_WRITE, nullptr);
  if (!elf)
  {
    throw std::runtime_error("elf_begin() 失败");
  }
}

void ELFWriter::createElfHeader()
{
  if (gelf_newehdr(elf, ELFCLASS32) == nullptr)
  {
    throw std::runtime_error("gelf_newehdr() 失败");
  }

  if (gelf_getehdr(elf, &ehdr) == nullptr)
  {
    throw std::runtime_error("gelf_getehdr() 失败");
  }

  ehdr.e_ident[EI_DATA] = ELFDATA2LSB; // 小端序
  ehdr.e_type = ET_REL;                // 可重定位文件
  ehdr.e_machine = EM_RISCV;           // RISC-V 架构
  ehdr.e_version = EV_CURRENT;
  ehdr.e_entry = 0;

  if (gelf_update_ehdr(elf, &ehdr) == 0)
  {
    throw std::runtime_error("gelf_update_ehdr() 失败");
  }
}

size_t ELFWriter::addToShStrTab(const std::string &str)
{
  size_t offset = shstrtabData.size();
  shstrtabData.insert(shstrtabData.end(), str.begin(), str.end());
  shstrtabData.push_back('\0');
  return offset;
}

size_t ELFWriter::addToStrTab(const std::string &str)
{
  size_t offset = strtabData.size();
  strtabData.insert(strtabData.end(), str.begin(), str.end());
  strtabData.push_back('\0');
  return offset;
}

void ELFWriter::createSections()
{
  // 创建 .shstrtab 段
  Elf_Scn *shstrtab_scn = elf_newscn(elf);
  if (!shstrtab_scn)
  {
    throw std::runtime_error("elf_newscn() 创建 .shstrtab 失败");
  }

  // 初始化一个 GElf_Shdr 结构体，用于存储 .shstrtab 段的段头信息
  GElf_Shdr shdr;
  gelf_getshdr(shstrtab_scn, &shdr);

  shdr.sh_name = 0; // 稍后设置
  shdr.sh_type = SHT_STRTAB;
  shdr.sh_flags = 0;
  shdr.sh_addr = 0;
  shdr.sh_offset = 0;
  shdr.sh_size = 0; // 稍后设置
  shdr.sh_link = 0;
  shdr.sh_info = 0;
  shdr.sh_addralign = 1;
  shdr.sh_entsize = 0;

  // gelf_update_shdr 将段头信息写回 ELF 数据结构
  if (gelf_update_shdr(shstrtab_scn, &shdr) == 0)
  {
    throw std::runtime_error("gelf_update_shdr() 更新 .shstrtab 失败");
  }

  // 获取 .shstrtab 段在 ELF 文件中的段索引
  shstrtabIndex = elf_ndxscn(shstrtab_scn);

  // 初始化 shstrtabData，以空字符开始
  shstrtabData.push_back('\0');

  // 添加 .shstrtab 到字符串表，并记录偏移量
  size_t shstrtabNameOffset = addToShStrTab(".shstrtab"); // 索引 1

  // 用于记录每个段的文件偏移，以处理段之间的对齐
  uint32_t fileOffset = 0;

  // 基于 segSecTable 创建段
  for (const auto &segEntry : segSecTable)
  {
    const std::string &segmentName = segEntry.first;
    const std::vector<Section> &sections = segEntry.second;

    // 跳过空的段
    if (sections.empty())
    {
      continue;
    }

    // 创建新段
    Elf_Scn *scn = elf_newscn(elf);
    if (!scn)
    {
      throw std::runtime_error("elf_newscn() 创建 " + segmentName + " 失败");
    }

    GElf_Shdr shdr;
    gelf_getshdr(scn, &shdr);

    size_t nameOffset = addToShStrTab(segmentName);
    shdr.sh_name = nameOffset;

    // 根据段名设置段类型和标志
    if (segmentName == ".text")
    {
      shdr.sh_type = SHT_PROGBITS;
      shdr.sh_flags = SHF_ALLOC | SHF_EXECINSTR;
      shdr.sh_addralign = 4;
    }
    else if (segmentName == ".data" || segmentName == ".sdata")
    {
      shdr.sh_type = SHT_PROGBITS;
      shdr.sh_flags = SHF_ALLOC | SHF_WRITE;
      shdr.sh_addralign = 4;
    }
    else if (segmentName == ".rodata")
    {
      shdr.sh_type = SHT_PROGBITS;
      shdr.sh_flags = SHF_ALLOC;
      shdr.sh_addralign = 4;
    }
    else if (segmentName == ".bss")
    {
      shdr.sh_type = SHT_NOBITS;
      shdr.sh_flags = SHF_ALLOC | SHF_WRITE;
      shdr.sh_addralign = 4;
    }
    else
    {
      // 处理其他段（如果需要）
      continue;
    }

    // 对齐文件偏移
    uint32_t segmentAlignment = shdr.sh_addralign;
    uint32_t padding = (segmentAlignment - (fileOffset % segmentAlignment)) % segmentAlignment;
    fileOffset += padding;

    // 设置段的文件偏移
    shdr.sh_offset = fileOffset;

    // 将所有 section 的数据合并到一个段中
    std::vector<uint8_t> segmentData;
    uint32_t segmentOffset = 0; // 段内偏移

    // 用于记录每个 section 在段内的起始偏移
    std::unordered_map<std::string, uint32_t> sectionBaseAddressMap;

    for (const Section &section : sections)
    {
      // 对齐段内偏移
      uint32_t alignment = section.getAlignment();
      uint32_t padding = (alignment - (segmentOffset % alignment)) % alignment;
      segmentData.insert(segmentData.end(), padding, section.getFillValue());
      segmentOffset += padding;

      // 记录当前 section 在段内的起始偏移
      uint32_t sectionOffsetInSegment = segmentOffset;
      sectionBaseAddressMap[section.getName()] = sectionOffsetInSegment;

      // 添加 section 数据
      const std::vector<uint8_t> &data = section.getData();
      segmentData.insert(segmentData.end(), data.begin(), data.end());
      segmentOffset += data.size();
    }

    if (segmentName != ".bss")
    {
      // 对于有数据的段
      Elf_Data *data = elf_newdata(scn);
      data->d_buf = segmentData.data();
      data->d_size = segmentData.size();
      data->d_align = shdr.sh_addralign;
      data->d_type = ELF_T_BYTE;
      data->d_version = EV_CURRENT;

      shdr.sh_size = segmentData.size();
    }
    else
    {
      // 对于 .bss 段
      shdr.sh_size = segmentOffset;
    }

    // 更新段头
    if (gelf_update_shdr(scn, &shdr) == 0)
    {
      throw std::runtime_error("gelf_update_shdr() 更新 " + segmentName + " 失败");
    }

    // 更新符号地址
    for (auto &symbolPair : symbolTable.getSymbols())
    {
      Symbol &symbol = symbolPair.second;
      if (symbol.getSegmentName() == segmentName)
      {
        const std::string &sectionName = symbol.getSectionName();
        uint32_t symbolSectionOffset = symbol.getInSecAddress(); // 符号在节内的偏移

        // 获取 section 在段内的基地址
        uint32_t sectionBaseAddress = sectionBaseAddressMap[sectionName];

        // 计算符号的段内地址
        uint32_t saddress = sectionBaseAddress + symbolSectionOffset;
        symbol.setSAddress(saddress);
      }
    }

    // 更新文件偏移
    fileOffset += shdr.sh_size;

    // 将段名映射到 Elf_Scn
    sectionMap[segmentName] = scn;
  }

  // 更新 .shstrtab 段数据
  Elf_Scn *shstrtab_scn_update = elf_getscn(elf, shstrtabIndex);
  Elf_Data *shstrtab_data = elf_newdata(shstrtab_scn_update);
  shstrtab_data->d_buf = shstrtabData.data();
  shstrtab_data->d_size = shstrtabData.size();
  shstrtab_data->d_align = 1;
  shstrtab_data->d_type = ELF_T_BYTE;
  shstrtab_data->d_version = EV_CURRENT;

  // 更新 .shstrtab 段的 sh_name
  gelf_getshdr(shstrtab_scn_update, &shdr);
  shdr.sh_name = shstrtabNameOffset;
  shdr.sh_size = shstrtabData.size();
  if (gelf_update_shdr(shstrtab_scn_update, &shdr) == 0)
  {
    throw std::runtime_error("gelf_update_shdr() 更新 .shstrtab 失败");
  }

  // 更新 ELF 头中的 e_shstrndx，指向 .shstrtab 段
  ehdr.e_shstrndx = shstrtabIndex;
  if (gelf_update_ehdr(elf, &ehdr) == 0)
  {
    throw std::runtime_error("gelf_update_ehdr() 更新失败");
  }
}

void ELFWriter::createSymbolTable()
{
  // 创建 .strtab 段
  Elf_Scn *strtab_scn = elf_newscn(elf);
  if (!strtab_scn)
  {
    throw std::runtime_error("elf_newscn() 创建 .strtab 失败");
  }

  GElf_Shdr strtab_shdr;
  gelf_getshdr(strtab_scn, &strtab_shdr);
  size_t strtabNameOffset = addToShStrTab(".strtab");
  strtab_shdr.sh_name = strtabNameOffset;
  strtab_shdr.sh_type = SHT_STRTAB;
  strtab_shdr.sh_flags = 0;
  strtab_shdr.sh_addr = 0;
  strtab_shdr.sh_offset = 0;
  strtab_shdr.sh_size = 0; // 稍后设置
  strtab_shdr.sh_link = 0;
  strtab_shdr.sh_info = 0;
  strtab_shdr.sh_addralign = 1;
  strtab_shdr.sh_entsize = 0;

  if (gelf_update_shdr(strtab_scn, &strtab_shdr) == 0)
  {
    throw std::runtime_error("gelf_update_shdr() 更新 .strtab 失败");
  }

  strtabIndex = elf_ndxscn(strtab_scn);

  // 初始化 strtabData，以空字符开始
  strtabData.push_back('\0');

  // 创建 .symtab 段
  Elf_Scn *symtab_scn = elf_newscn(elf);
  if (!symtab_scn)
  {
    throw std::runtime_error("elf_newscn() 创建 .symtab 失败");
  }

  GElf_Shdr symtab_shdr;
  gelf_getshdr(symtab_scn, &symtab_shdr);
  size_t symtabNameOffset = addToShStrTab(".symtab");
  symtab_shdr.sh_name = symtabNameOffset;
  symtab_shdr.sh_type = SHT_SYMTAB;
  symtab_shdr.sh_flags = 0;
  symtab_shdr.sh_addr = 0;
  symtab_shdr.sh_offset = 0;
  symtab_shdr.sh_size = 0;           // 稍后设置
  symtab_shdr.sh_link = strtabIndex; // 链接到 .strtab
  symtab_shdr.sh_info = 0;           // 稍后设置
  symtab_shdr.sh_addralign = 4;
  symtab_shdr.sh_entsize = sizeof(GElf_Sym);

  if (gelf_update_shdr(symtab_scn, &symtab_shdr) == 0)
  {
    throw std::runtime_error("gelf_update_shdr() 更新 .symtab 失败");
  }

  symtabIndex = elf_ndxscn(symtab_scn);

  // 准备符号表条目
  std::vector<GElf_Sym> symbols;

  // 第一个符号总是未定义符号
  GElf_Sym sym;
  memset(&sym, 0, sizeof(GElf_Sym));
  symbols.push_back(sym);

  // 从符号表中添加符号
  size_t localSymbolCount = 1; // 从 1 开始计数（跳过未定义符号）
  for (const auto &symEntry : symbolTable.getSymbols())
  {
    const std::string &symName = symEntry.first;
    const Symbol &symbol = symEntry.second;

    memset(&sym, 0, sizeof(GElf_Sym));
    size_t nameOffset = addToStrTab(symName);
    sym.st_name = nameOffset;
    sym.st_value = symbol.getSAddress();
    sym.st_size = symbol.getSize();
    sym.st_info = GELF_ST_INFO(symbol.isGlobal() ? STB_GLOBAL : STB_LOCAL,
                               symbol.getType() == SymbolType::FUNCTION ? STT_FUNC : STT_OBJECT);

    // 获取段索引
    const std::string &sectionName = symbol.getSectionName();
    if (sectionName.empty())
    {
      sym.st_shndx = SHN_UNDEF; // 未定义
    }
    else
    {
      if (sectionMap.find(sectionName) == sectionMap.end())
      {
        throw std::runtime_error("找不到符号所在的段：" + symName);
      }
      sym.st_shndx = elf_ndxscn(sectionMap[sectionName]);
    }

    symbols.push_back(sym);
    size_t symbolIndex = symbols.size() - 1;
    symbolIndices[symName] = symbolIndex;

    if (!symbol.isGlobal())
    {
      localSymbolCount++;
    }
  }

  // 更新 sh_info，设置第一个全局符号的索引
  symtab_shdr.sh_info = localSymbolCount;

  // 写入符号表数据
  Elf_Data *symtab_data = elf_newdata(symtab_scn);
  symtab_data->d_buf = symbols.data();
  symtab_data->d_size = symbols.size() * sizeof(GElf_Sym);
  symtab_data->d_align = 4;
  symtab_data->d_type = ELF_T_SYM;
  symtab_data->d_version = EV_CURRENT;

  // 更新 .symtab 段大小
  symtab_shdr.sh_size = symtab_data->d_size;
  if (gelf_update_shdr(symtab_scn, &symtab_shdr) == 0)
  {
    throw std::runtime_error("gelf_update_shdr() 更新 .symtab 失败");
  }

  // 写入 .strtab 数据
  Elf_Data *strtab_data = elf_newdata(strtab_scn);
  strtab_data->d_buf = strtabData.data();
  strtab_data->d_size = strtabData.size();
  strtab_data->d_align = 1;
  strtab_data->d_type = ELF_T_BYTE;
  strtab_data->d_version = EV_CURRENT;

  // 更新 .strtab 段大小
  strtab_shdr.sh_size = strtab_data->d_size;
  if (gelf_update_shdr(strtab_scn, &strtab_shdr) == 0)
  {
    throw std::runtime_error("gelf_update_shdr() 更新 .strtab 失败");
  }
}

void ELFWriter::createRelocationSections()
{
  // 遍历重定位表，创建相应的重定位段
  const auto &relocationsMap = relocationTable.getRelocations();

  for (const auto &relEntry : relocationsMap)
  {
    const std::string &sectionName = relEntry.first; // 重定位目标段名，例如 ".text"
    const std::vector<RelocationEntry> &relocations = relEntry.second;

    // 获取目标段的 Elf_Scn
    if (sectionMap.find(sectionName) == sectionMap.end())
    {
      throw std::runtime_error("找不到重定位目标段：" + sectionName);
    }
    Elf_Scn *targetScn = sectionMap[sectionName];

    // 创建重定位段，例如 ".rel.text"
    std::string relSectionName = ".rel" + sectionName;
    Elf_Scn *relScn = elf_newscn(elf);
    if (!relScn)
    {
      throw std::runtime_error("elf_newscn() 创建重定位段失败：" + relSectionName);
    }

    GElf_Shdr relShdr;
    gelf_getshdr(relScn, &relShdr);
    size_t nameOffset = addToShStrTab(relSectionName);
    relShdr.sh_name = nameOffset;
    relShdr.sh_type = SHT_REL;
    relShdr.sh_flags = 0;
    relShdr.sh_addr = 0;
    relShdr.sh_offset = 0;
    relShdr.sh_size = 0;                     // 稍后更新
    relShdr.sh_link = symtabIndex;           // 链接到符号表
    relShdr.sh_info = elf_ndxscn(targetScn); // 目标段索引
    relShdr.sh_addralign = 4;
    relShdr.sh_entsize = sizeof(GElf_Rel);

    if (gelf_update_shdr(relScn, &relShdr) == 0)
    {
      throw std::runtime_error("gelf_update_shdr() 更新重定位段失败：" + relSectionName);
    }

    // 准备重定位条目
    std::vector<GElf_Rel> rels;
    for (const RelocationEntry &rel : relocations)
    {
      GElf_Rel gelfRel;
      memset(&gelfRel, 0, sizeof(GElf_Rel));
      gelfRel.r_offset = rel.offset;

      // 获取符号在符号表中的索引
      auto symIt = symbolIndices.find(rel.symbolName);
      if (symIt != symbolIndices.end())
      {
        size_t symIndex = symIt->second;

        // 构建 r_info，注意需要将 rel.type 转换为整数类型
        gelfRel.r_info = GELF_R_INFO(symIndex, static_cast<uint32_t>(rel.type));

        rels.push_back(gelfRel);
      }
      else
      {
        throw std::runtime_error("重定位引用了未知符号：" + rel.symbolName);
      }
    }

    // 写入重定位段数据
    Elf_Data *relData = elf_newdata(relScn);
    relData->d_buf = rels.data();
    relData->d_size = rels.size() * sizeof(GElf_Rel);
    relData->d_align = 4;
    relData->d_type = ELF_T_REL;
    relData->d_version = EV_CURRENT;

    // 更新重定位段大小
    relShdr.sh_size = relData->d_size;
    if (gelf_update_shdr(relScn, &relShdr) == 0)
    {
      throw std::runtime_error("gelf_update_shdr() 更新重定位段失败：" + relSectionName);
    }

    // 将重定位段名映射到 Elf_Scn（可选，如果需要在其他地方使用）
    sectionMap[relSectionName] = relScn;
  }
}

void ELFWriter::writeToFile()
{
  if (elf_update(elf, ELF_C_WRITE) < 0)
  {
    throw std::runtime_error("elf_update() 失败");
  }
}

void ELFWriter::finalize()
{
  if (elf_end(elf) != 0)
  {
    throw std::runtime_error("elf_end() 失败");
  }
  close(fd);
}

void ELFWriter::write()
{
  createElfHeader();
  createSections();
  createSymbolTable();
  createRelocationSections(); // 处理重定位表
  writeToFile();
  finalize();
}
