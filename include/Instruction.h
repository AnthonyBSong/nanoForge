#pragma once
#include <string>
#include <vector>

struct Instruction {
    std::string opcode;
    std::vector<std::string> operands;
    int line;
};