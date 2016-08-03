FLEX=flex
FLEX_SRC=lex.yy.c
PARSER_SRC=bighorn.tab
SRC_DIR=./src
GEN_DIR=./generated
OUTPUT_DIR=./build
DEBUG_FLAG=-g 

all: bighorn final
bighorn : output_dir scan_gen parser_gen 
	@g++ -DYYTOKENTYPE -I$(SRC_DIR) -I$(GEN_DIR) $(GEN_DIR)/$(FLEX_SRC) $(GEN_DIR)/$(PARSER_SRC).cc $(SRC_DIR)/sym_tab.cc $(SRC_DIR)/bighornAST.cc $(SRC_DIR)/bighornIR.cc -o micro -lfl

output_dir :
	@mkdir -p  $(OUTPUT_DIR)

scan_gen : flex_gen
	@mv $(FLEX_SRC) $(GEN_DIR)

flex_gen:
	@flex -s $(SRC_DIR)/bighorn.l

parser_gen: bison_gen
	@mv $(PARSER_SRC).cc $(PARSER_SRC).hh location.hh position.hh stack.hh $(GEN_DIR)

bison_gen:
	@bison -o $(PARSER_SRC).cc $(SRC_DIR)/bighorn.yy

final:
	@mv micro $(OUTPUT_DIR)

.PHONY : clean

group:
	@echo "Srivatsan Bhaskar"

clean:
	@rm -rf $(OUTPUT_DIR)/*.*
	@rm -rf $(GEN_DIR)/*.*
	@rm -rf ./*.cc
	@rm -rf ./*.hh
	@rm -rf ./*.h
	@rm -rf ./*.c
	@rm -rf ./bighorn.output

