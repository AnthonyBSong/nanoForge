#include "../include/Instruction.h"
#include "../include/Token.h"
#include "Parser.h"

Instruction parseInstruction(const std::vector<Token>& tokens, int& current) {
    Instruction inst;
    inst.opcode = tokens[current].lexeme; // fix this. Lexeme does not give the instruction opcode!
    current++;
    while (current < tokens.size() && tokens[current].type != TokenType::END_OF_LINE) {
        if (tokens[current].type == TokenType::REGISTER ||
            tokens[current].type == TokenType::IMMEDIATE ||
            tokens[current].type == TokenType::LABEL) {
            inst.operands.push_back(tokens[current].lexeme);
        }
        current++;
    }
    return inst;
}

bool validateInstruction(const Instruction& inst, const std::map<std::string, int>& labelMap) {
    if (inst.opcode == "ADD") {
        if (inst.operands.size() != 3) return false;
    }
    return true;
}
