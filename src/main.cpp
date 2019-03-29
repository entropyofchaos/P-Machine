#include "Instruction.h"
#include "LexicalAnalyzer.h"
#include "ParserAndCodeGenerator.h"
#include "VirtualMachine.h"

#include <cstring>

int main(int argc, char *argv[])
{
    bool printLex = false;
    bool printAsm = false;
    bool printVm = false;

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "-l" ) == 0)
        {
            printLex = true;
        }
        if (strcmp(argv[i], "-a") == 0)
        {
            printAsm = true;
        }
        if (strcmp(argv[i], "-v" ) == 0)
        {
            printVm = true;
        }
    }

    std::ofstream outputFile("outputFile.txt");
    std::ifstream inputFile("inputFile.txt");

    // Copy file into stringstream buffer so that the program is memory
    std::stringstream buffer;
    buffer << inputFile.rdbuf();
    inputFile.close();

    std::vector<std::pair<std::string, token_type>> lexemeTable;

    std::stringstream outputStream;
    analyzeCode(buffer, outputStream, lexemeTable);
    outputStream << "\n\n\n";
    outputFile << outputStream.str() << std::flush;
    if (printLex)
    {
        std::cout << outputStream.str();
    }
    outputStream.str("");
    outputStream.clear();

    bool runnableCode = parseAndGenerage(lexemeTable, outputStream);
    outputStream << "\n\n";
    outputFile << outputStream.str() << std::flush;
    
    if (printAsm)
    {
        std::cout << outputStream.str();
    }
    else if (runnableCode)
    {
        std::cout << "No errors, program is syntactically correct.\n\n\n";
    }
    
    outputStream.str("");
    outputStream.clear();

    if (runnableCode)
    {
        runProgram(outputStream);
        outputFile << outputStream.str();
        if (printVm)
        {
            std::cout << "\n\n" << outputStream.str();
        }
    }
    else
    {
        outputStream << "Code not run since generated code is invalid and could cause system instability.\n\n";
    }

    outputFile.close();
}