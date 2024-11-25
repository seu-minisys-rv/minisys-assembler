# Makefile for minisys-assembler

# 编译器及选项
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g

# 包含目录
INCLUDE_DIRS = -I. -Iinstruction -Itrunk -Iutils -Isymbol_table -Irelocation_table -Isection

# 源文件列表
SRCS = main.cpp \
			 instruction/Instruction.cpp \
       instruction/InstructionI.cpp \
       instruction/InstructionJ.cpp \
       instruction/InstructionL.cpp \
       instruction/InstructionS.cpp \
       instruction/InstructionU.cpp \
       instruction/InstructionB.cpp \
       instruction/InstructionM.cpp \
       instruction/InstructionR.cpp \
			 instruction/InstructionP.cpp \
       utils/Utils.cpp \
       symbol_table/SymbolTable.cpp \
       relocation_table/RelocationTable.cpp \
       section/Section.cpp \
			 trunk/Assembler.cpp \
# 如有更多源文件，请在此处添加

# 目标文件列表
OBJS = $(SRCS:.cpp=.o)

# 可执行文件名称
TARGET = assembler

# 默认目标
all: $(TARGET)

# 链接规则
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# 编译规则
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -c -o $@ $<

# 清理
clean:
	rm -f $(OBJS) $(TARGET)

# 伪目标
.PHONY: all clean
