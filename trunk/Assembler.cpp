// assembler/Assembler.cpp

#include "Assembler.hpp"
#include <fstream>
#include <sstream>
#include "../utils/Utils.hpp"

void Assembler::initializeSegments()
{
  segSecTable[".text"] = {};
  segSecTable[".data"] = {};
  segSecTable[".rodata"] = {};
  segSecTable[".sdata"] = {};
  segSecTable[".bss"] = {};
  segAddressTable[".text"] = 0;
  segAddressTable[".data"] = 0;
  segAddressTable[".rodata"] = 0;
  segAddressTable[".sdata"] = 0;
  segAddressTable[".bss"] = 0;
  baseAddressTable[".text"] = std::make_pair(0, 0);
  baseAddressTable[".data"] = std::make_pair(0, 0);
  baseAddressTable[".rodata"] = std::make_pair(0, 0);
  baseAddressTable[".sdata"] = std::make_pair(0, 0);
  baseAddressTable[".bss"] = std::make_pair(0, 0);
}

// 组装整个流程
void Assembler::assemble(const std::string &inputFile, const std::string &outputFile, bool usingElfWriter)
{
  isUsingElfWriter = usingElfWriter;
  lines = readAssemblyCode(inputFile); // 读取文件中的汇编行
  firstPass();
  secondPass();
  writeFile(outputFile);
}

// 封装的写入文件的函数
void Assembler::writeFile(const std::string &outputFile)
{
  // 打开二进制文件进行写入
  std::ofstream outFile(outputFile, std::ios::binary);
  if (!outFile)
  {
    std::cerr << "无法打开文件 " << outputFile << " 进行写入。" << std::endl;
    return;
  }

  for (uint32_t instr : instructionResult)
  {
    uint32_t dataToWrite = Utils::toLittleEndian(instr); // 确保数据以小端序写入

    // 使用模板函数写入数据
    Utils::writeBinary(outFile, dataToWrite);
  }

  outFile.close();
  std::cout << "指令已写入文件 " << outputFile << std::endl;
}

// 读取文件的每一行
std::vector<std::string> Assembler::readAssemblyCode(const std::string &filename)
{
  std::ifstream infile(filename);
  std::vector<std::string> lines;
  std::string line;

  while (std::getline(infile, line))
  {
    // 删除行内注释
    size_t commentPos = line.find('#');
    if (commentPos != std::string::npos)
    {
      line = line.substr(0, commentPos); // 保留注释前的内容
    }

    line = Utils::trim(line);
    if (!line.empty())
    {
      lines.push_back(line);
    }
  }
  return lines;
}

// std::string name;           // 段名称
// uint32_t alignment;         // 段对齐
// std::string flags;          // 段标志
// std::string type;           // 段类型
// std::string segmentName;    // 段所属节的名称
// std::uint32_t section_size; // 段大小
// std::vector<uint8_t> data;  // 段数据
void Assembler::firstPass()
{
  initializeSegments();

  for (const auto &line : lines)
  {
    if (line.back() == ':')
    { // 处理标签
      std::string label = line.substr(0, line.size() - 1);
      if (!symbolTable.hasSymbol(label))
      {
        symbolTable.addSymbol(label, saddress, gaddress, SymbolType::LABEL, false, currentSecName);
      }
      else
      {
        symbolTable.updateSymbolAddress(label, saddress, gaddress, inSecAddress);
        symbolTable.setType(label, SymbolType::LABEL);
        symbolTable.setSectionName(label, currentSecName);
        auto segName = sectionTable[currentSecName].getSegmentName();
        symbolTable.setSegmentName(label, segName);
      }
    }
    else if (line[0] == '.')
    { // 处理伪指令
      std::istringstream iss(line);
      std::string directive;
      iss >> directive;
      if (directive == ".text")
      {
        isText = true;
        saddress = segAddressTable[".text"];
      }
      else if (directive == ".type")
      {
        handleTypeDirective(iss, currentSecName);
      }
      else if (directive == ".globl")
      {
        handleGloblDirective(iss);
      }
      else if (directive == ".section")
      {
        handleSectionDirective(iss, currentSecName);
      }
      else if (directive == ".p2align")
      {
        handleP2AlignDirective(iss, currentSecName);
      }
      else if (directive == ".size")
      {
        handleSizeDirective(iss, currentSecName);
      }
      else if (directive == ".word")
      {
        handleWordDirective(iss, currentSecName);
      }
      else if (directive == ".asciz")
      {
        handleAscizDirective(iss, currentSecName);
      }
    }
    else
    {
      std::pair<int, std::string> instrucionPair = std::make_pair(saddress, line);
      instructionTable[currentSecName].emplace_back(instrucionPair);
      instructionVector.emplace_back(std::make_pair(gaddress, line));
      saddress += 4;
      gaddress += 4;
      inSecAddress += 4;
    }
  }
}

void Assembler::secondPass()
{
  if (isUsingElfWriter)
  {
    for (auto &instr : instructionTable)
    {
      std::string secName = instr.first;
      std::vector<std::pair<int, std::string>> instructionPair = instr.second;
      for (auto &pair : instructionPair)
      {
        int address = pair.first;
        std::string line = pair.second;
        handleInstruction(address, line, secName);
      }
    }
    handleSegmentTable();
  }
  else
  {
    for (auto &instr : instructionVector)
    {
      uint32_t addr = instr.first;
      std::string instruction = instr.second;
      handleInstruction(addr, instruction, "");
    }
    handleSegmentTable();
  }
}

void Assembler::handleTypeDirective(std::istringstream &iss, std::string &currentSecName)
{
  //.type	factorial,@function
  // 读取 .type 后面的整行内容
  std::string restOfLine;
  std::getline(iss, restOfLine);

  // 去除首尾空白
  restOfLine = Utils::trim(restOfLine);

  // 使用 split 函数按逗号分割
  std::vector<std::string> parts = Utils::split(restOfLine, ',');

  // 检查分割结果是否有效
  if (parts.size() != 2)
  {
    throw std::runtime_error("Invalid format in .type directive: " + restOfLine);
  }
  currentSecName = Utils::trim(parts[0]);
  std::string type = Utils::trim(parts[1]);

  // 设置符号类型
  if (type == "@function" || type == "@object")
  {
    if (sectionTable.find(currentSecName) == sectionTable.end())
    {
      sectionTable[currentSecName] = Section(currentSecName);
      inSecAddress = 0;
      if (type == "@function")
      {
        sectionTable[currentSecName].setSegmentName(".text");
        sectionTable[currentSecName].setAlignment(1 << 2);
        sectionTable[currentSecName].setFillValue(0x0);
      }
      else
      {
        isText = false;
      }
    }
    else
    {
      throw std::runtime_error("Section already exists");
    }
  }
  else
  {
    throw std::runtime_error("Unknown symbol type in .type directive: " + type);
  }
}

void Assembler::handleGloblDirective(std::istringstream &iss)
{
  std::string symbol;
  if (!(iss >> symbol))
  {
    throw std::runtime_error("Invalid format in .globl directive");
  }
  if (!symbolTable.hasSymbol(symbol))
  {
    symbolTable.addSymbol(symbol);
  }
  symbolTable.setGlobal(symbol, true);
  if (currentSecName.empty())
  {
    currentSecName = symbol;
  }
  // if (sectionTable.find(currentSecName) == sectionTable.end())
  // {
  //   sectionTable[currentSecName] = Section(currentSecName);
  //   inSecAddress = 0;
  // }
}

void Assembler::handleSectionDirective(std::istringstream &iss, std::string &currentSecName)
{
  //.section	.rodata,"a",@progbits
  // 读取 .section 后的所有内容
  std::string restOfLine;
  std::getline(iss, restOfLine);
  restOfLine = Utils::trim(restOfLine);

  // 分割成节名、标志、类型
  std::vector<std::string> sectionParts = Utils::split(restOfLine, ',');

  if (sectionParts.size() < 1)
  {
    throw std::runtime_error("Invalid .section directive format: " + restOfLine);
  }

  // 解析节名
  std::string segmentName = Utils::trim(sectionParts[0]);
  // 解析标志和类型，如果有的话
  std::string flags = (sectionParts.size() > 1) ? Utils::trim(sectionParts[1]) : "";
  std::string type = (sectionParts.size() > 2) ? Utils::trim(sectionParts[2]) : "";
  sectionTable[currentSecName].setSegmentName(segmentName);
  sectionTable[currentSecName].setFlags(flags);
  sectionTable[currentSecName].setType(type);
  saddress = segAddressTable[segmentName];
}

void Assembler::handleP2AlignDirective(std::istringstream &iss, const std::string &currentSecName)
{
  // 获取对齐参数行，并去除多余空格
  std::string restOfLine;
  std::getline(iss, restOfLine);
  if (!isText)
  {
    restOfLine = Utils::trim(restOfLine);

    // 分割参数，去除空白
    auto alignArgs = Utils::split(restOfLine, ',');

    if (alignArgs.empty())
    {
      throw std::runtime_error("Missing alignment value in .p2align directive");
    }

    int align = std::stoi(Utils::trim(alignArgs[0]));
    uint8_t fillValue = (alignArgs.size() > 1) ? std::stoi(Utils::trim(alignArgs[1]), nullptr, 0) : 0;

    if (align < 0 || align > 31)
    {
      throw std::runtime_error("Invalid alignment value in .p2align directive: " + std::to_string(align));
    }

    // address = Utils::alignAddress(address, 1 << align);
    sectionTable[currentSecName].setAlignment(1 << align);
    sectionTable[currentSecName].setFillValue(fillValue);
  }
}

void Assembler::handleSizeDirective(std::istringstream &iss, const std::string &currentSecName)
{
  // 读取符号名称和大小部分
  std::string sizeLine;
  std::getline(iss, sizeLine);                                      // 获取完整的行内容
  std::vector<std::string> sizeParts = Utils::split(sizeLine, ','); // 按逗号分割
  if (sizeParts.size() != 2 || sizeParts[0] != currentSecName)
  {
    throw std::runtime_error("Invalid format in .size directive for: " + currentSecName);
  }

  std::string symbol = sizeParts[0];
  std::string sizeExpr = sizeParts[1];

  if (sizeExpr.find('-') != std::string::npos)
  {
    // 表达式格式，分割并计算
    std::vector<std::string> exprParts = Utils::split(sizeExpr, '-');
    if (exprParts.size() == 2)
    {
      std::string startSymbol = exprParts[0];
      std::string endSymbol = exprParts[1];

      // 获取符号地址并计算差值
      uint32_t startAddr = symbolTable.getSymbol(startSymbol).getGAddress();
      uint32_t endAddr = symbolTable.getSymbol(endSymbol).getGAddress();
      uint32_t size = startAddr - endAddr;

      symbolTable.setSize(symbol, size);
      sectionTable[currentSecName].setSectionSize(size);
    }
  }
  else
  {
    // 直接数值
    int size = std::stoi(sizeExpr);
    symbolTable.setSize(symbol, size);
    sectionTable[currentSecName].setSectionSize(size);
  }
  sectionTable[currentSecName].align(sectionTable[currentSecName].getAlignment(), saddress, gaddress, inSecAddress);
  uint32_t prevBaseAddress = baseAddressTable[sectionTable[currentSecName].getSegmentName()].first;
  uint32_t prevSize = baseAddressTable[sectionTable[currentSecName].getSegmentName()].second;
  sectionTable[currentSecName].setBaseAddress(prevBaseAddress + prevSize);
  baseAddressTable[sectionTable[currentSecName].getSegmentName()].first = prevBaseAddress + prevSize;
  baseAddressTable[sectionTable[currentSecName].getSegmentName()].second = sectionTable[currentSecName].getSize();
  segAddressTable[sectionTable[currentSecName].getSegmentName()] = saddress;
}

void Assembler::handleWordDirective(std::istringstream &iss, const std::string &currentSecName)
{
  uint32_t value;
  std::vector<uint32_t> data;
  while (iss >> value) // 读取每个32位整数值
  {
    // 将值按小端序添加到段数据
    data.emplace_back(value);
    sectionTable[currentSecName].addInstruction(data);
    saddress += 4;
    gaddress += 4;
    inSecAddress += 4;
  }
}
void Assembler::handleAscizDirective(std::istringstream &iss, const std::string &currentSecName)
{
  std::string strValue;
  std::getline(iss, strValue); // 获取字符串内容（包含引号）
  strValue = Utils::trim(strValue);

  // 去掉首尾的引号，得到实际字符串内容
  if (strValue.size() >= 2 && strValue.front() == '"' && strValue.back() == '"')
  {
    strValue = strValue.substr(1, strValue.size() - 2);
  }

  std::vector<uint8_t> data;

  // 逐个字符解析字符串内容，处理转义字符
  for (size_t i = 0; i < strValue.size(); ++i)
  {
    if (strValue[i] == '\\')
    {
      // 处理转义字符
      if (i + 1 < strValue.size())
      {
        if (strValue[i + 1] == '0')
        {
          // 处理 \000 格式
          int zeroCount = 0;
          size_t j = i + 1;
          while (j < strValue.size() && strValue[j] == '0' && zeroCount < 3)
          {
            zeroCount++;
            j++;
          }
          data.push_back('\0');
          i += zeroCount; // 跳过已经处理的字符
        }
        else
        {
          // 处理其他单字符转义，如 \n, \t, \\ 等
          switch (strValue[i + 1])
          {
          case 'n':
            data.push_back('\n');
            break;
          case 't':
            data.push_back('\t');
            break;
          case '\\':
            data.push_back('\\');
            break;
          case '\"':
            data.push_back('\"');
            break;
          default:
            throw std::runtime_error("Unknown escape sequence in .asciz directive");
          }
          i++; // 跳过转义字符
        }
      }
    }
    else
    {
      // 非转义字符直接添加
      data.push_back(static_cast<uint8_t>(strValue[i]));
    }
  }

  // 保证结尾有一个 '\0' 字节作为终止符

  data.push_back('\0');

  // 将解析后的数据添加到当前 section
  sectionTable[currentSecName].addData(data);

  // 更新地址，增加数据长度
  saddress += data.size();
  gaddress += data.size();
  inSecAddress += data.size();
  uint32_t secAlignment = sectionTable[currentSecName].getAlignment();
  sectionTable[currentSecName].align(secAlignment, saddress, gaddress, inSecAddress);
}
void Assembler::handleInstruction(const int address, const std::string &line, const std::string &currentSecName)
{
  std::cout << "正在处理指令：" << line << std::endl;
  // 使用 Instruction 工厂方法解析并创建指令对象
  auto instruction = Instruction::create(line);

  // 编码指令，生成机器码
  std::vector<uint32_t> machineCode = instruction->encode(symbolTable, relocationTable, sectionTable, address, currentSecName);
  if (isUsingElfWriter)
  {
    // 将机器码添加到当前节的数据
    sectionTable[currentSecName].addInstruction(machineCode);
  }
  else
  {
    instructionResult.insert(instructionResult.end(), machineCode.begin(), machineCode.end());
  }
}

void Assembler::handleSegmentTable()
{
  for (const auto &sec : sectionTable)
  {
    const std::string &secName = sec.first;
    const Section &section = sec.second;
    const std::string &segmentName = section.getSegmentName();
    segSecTable[segmentName].push_back(section);
  }
}