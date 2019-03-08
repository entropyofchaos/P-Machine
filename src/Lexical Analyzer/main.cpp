#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>

/** Identifies are limited to 11 characters. */
const short MAX_IDENTIFIER_LENGTH = 11;
/** Numbers have a max length of 5 characters. */
const short MAX_NUMBER_LENGTH = 5;
///** Maximum number of names that can be stored in the symbols table */
// const ushort MAX_NAME_TABLE_SIZE = USHRT_MAX;

/** Enumeration containing the different token types that can be found in a PL/0 application. */
enum token_type : int
{
    nulSym = 1, identSym, numberSym, plusSym, minusSym,
    multSym,  slashSym, oddSym, eqSym, neqSym, lesSym, leqSym,
    gtrSym, geqSym, lparentSym, rparentSym, commaSym, semicolonSym,
    periodSym, becomesSym, beginSym, endSym, ifSym, thenSym,
    whileSym, doSym, callSym, constSym, varSym, procSym, writeSym,
    readSym , elseSym // = 33
};

/** Hash table of reserved words mapping them to their enum token_type */
const std::unordered_map<std::string, token_type> reservedWords =
{
    {"null", nulSym},
    {"begin", beginSym},
    {"call", callSym},
    {"const", constSym},
    {"do", doSym},
    {"else", elseSym},
    {"end", endSym},
    {"if", ifSym},
    {"odd", oddSym},
    {"procedure", procSym},
    {"read", readSym},
    {"then", thenSym},
    {"var", varSym},
    {"while", whileSym},
    {"write", writeSym}
};

/** Hash table of special symbols mapping them to their enum token_type */
const std::unordered_map<std::string, token_type> specialSymbols =
{
    {"+", plusSym},
    {"-", minusSym},
    {"*", multSym},
    {"/", slashSym},
    {"(", lparentSym},
    {")", rparentSym},
    {"<>", neqSym},
    {"=", eqSym},
    {",", commaSym},
    {".", periodSym},
    {"<", lesSym},
    {"<=", leqSym},
    {">", gtrSym},
    {">=", geqSym},
    {";", semicolonSym},
    {":=", becomesSym}
};

// /** structure of the symbol table record */
// struct namerecord_t
// {
//     int kind;         /* const = 1, var = 2, proc = 3. */
//     char name[10];    /* name up to 11 chars */
//     int val;          /* number (ASCII value) */
//     int level;        /* L  level */
//     int adr;          /* M  address */
// };
                       
// namerecord_t symbol_table[MAX_NAME_TABLE_SIZE];

std::vector<std::pair<std::string, token_type>> lexemeTable;
std::vector<std::string> lexemeList;

int main(int argc, char *argv[])
{
    std::ofstream outputFile("outputFile.txt");
    std::ifstream inputFile("inputFile.txt");

    // Copy file into stringstream buffer so that the program is memory
    std::stringstream buffer;
    buffer << inputFile.rdbuf();
    inputFile.close();

    // Print source program into the output file
    outputFile << "Source Program: \n" 
        << buffer.str() << "\n\n";

    std::string currentToken;
    char ch;

    bool isWord;
    bool isNumber;
    bool isWhitespaceOrComment;
    bool errorFound = false;

    // Offset current line number by two. The first offset is for
    // the output line that says input file. The second is for 
    // the fact that we won't reach the newline for the current
    // line the error occurs on.
    int currentLineNumber = 2;

    // Confirm we haven't run into an error and we haven't reached the end of the file.
    while(buffer.peek() != EOF)
    { 
        // State 1
        buffer.get(ch);
        currentToken = ch;

        // Check if the next character is a letter
        if (isalpha(ch))
        {
            isWord = true;
            isNumber = false;
            isWhitespaceOrComment = false;

            // State 2
            char ch2 = buffer.peek();
            while (ch2 != EOF && isalnum(ch2))
            {
                // Put next character into ch.
                buffer.get(ch);
                ch2 = buffer.peek();

                // State 3
                currentToken += ch;
            }
            
            // Check if identifier token is too long
            if (currentToken.length() > MAX_IDENTIFIER_LENGTH)
            {
                errorFound = true;
                outputFile << "\n\nError: Current identifier token " << currentToken << " exceeds " 
                    << MAX_IDENTIFIER_LENGTH  << " characters.\n"
                    << "Error found on line " << currentLineNumber << ".\n";
            }

        }
        // State 4
        // Check for the first digit in a number
        else if (isdigit(ch))
        {
            isWord = false;
            isNumber = true;
            isWhitespaceOrComment = false;
            
            // State 5
            char ch2 = buffer.peek();

            // Check if the number is followed directly by a letter. This means someone
            // tried writing an identifier that starts with a number.
            if (isalpha(ch2))
            {
                errorFound = true;
                outputFile << "\n\nError: Current identifier token " << currentToken 
                    << " starts with a number which is not allowed.\n"
                    << "Error found on line " << currentLineNumber << ".\n";
            }

            // Keep checking for more digits after the first digit.
            while (ch2 != EOF && isdigit(ch2))
            {
                // Put next character into ch.
                buffer.get(ch);
                ch2 = buffer.peek();

                // State 6
                currentToken += ch;
            }

            // Check if token is too long
            if (currentToken.length() > MAX_NUMBER_LENGTH)
            {
                errorFound = true;
                outputFile << "\n\nError: Current number token " << currentToken << " exceeds " 
                    << MAX_NUMBER_LENGTH << " characters.\n"
                    << "Error found on line " << currentLineNumber << ".\n";
            }
        }
        else if (isspace(ch))
        {
            // A new line means we are moving to the next line in the code.
            if (ch == '\n')
            {
                ++currentLineNumber;
            }

            // We don't do anything for whitespace characters
            isWord = false;
            isNumber = false;
            isWhitespaceOrComment = true;
        }
        else
        {
            // Not a letter or digit.
            // It must be a special symbol or whitespace character.
            isWord = false;
            isNumber = false;
            isWhitespaceOrComment = false; // If comment is found, this will be set to true.

            // If we find a <, then we will need to handle if there
            // is a <= or <>. So we then look ahead one character.
            if (ch == '<')
            {
                char ch2 = buffer.peek();
                if (ch2 != EOF)
                {
                    // Check for <=
                    if (ch2 == '=')
                    {
                        buffer.get(ch);
                        currentToken = "<=";
                    }
                    // Check for <>
                    else if (ch2 == '>')
                    {
                        buffer.get(ch);
                        currentToken = "<>";
                    }
                    // The token is only <
                    else
                    {
                        currentToken = ch;
                    }
                }
            }
            // If we find a >, then we will need to handle if there
            // is a >=. So we then look ahead one character.
            else if (ch == '>')
            {
                char ch2 = buffer.peek();
                if (ch2 != EOF)
                {
                    // Check for >=
                    if (ch2 == '=')
                    {
                        buffer.get(ch);
                        currentToken = ">=";
                    }
                    else
                    {
                        currentToken = ch;
                    }
                }
            }
            else if (ch == ':')
            {
                char ch2 = buffer.peek();
                if (ch2 != EOF)
                {
                    // Check for :=
                    if (ch2 == '=')
                    {
                        buffer.get(ch);
                        currentToken = ":=";
                    }
                    else
                    {
                        errorFound = true;
                        outputFile << "\n\nError: Found : not followed by =.\n"
                            << "Error found on line " << currentLineNumber << ".\n";
                    }
                }
            }
            else if (ch == '/')
            {
                char ch2 = buffer.peek();
                if (ch2 != EOF)
                {
                    // Check for beginning of a comment denoted by /*
                    if (ch2 == '*')
                    {
                        // Flush * from buffer. ch now holds *.
                        buffer.get(ch);

                        // Get first character after /*
                        buffer.get(ch);

                        do
                        {
                            // Look for the end of a comment denoted by */
                            if (ch == '*' && buffer.peek() == '/')
                            {
                                // We reached the end of a comment

                                // Flush '/' from buffer. ch now holds /.
                                buffer.get(ch);
                                
                                // Set that we just parsed a comment
                                isWhitespaceOrComment = true;
                            }
                            else
                            {
                                // Flush next character from inside comment.
                                // We don't store commented out characters.
                                // ch now holds a character from inside a comment.
                                buffer.get(ch);
                            }
                        }
                        // Exit comment flushing loop when the end of comment sequence */ is found
                        // or we hit the end of the file.
                        while (ch != EOF || !isWhitespaceOrComment);

                        // We have an error. A comment started but was never ended.
                        if (!isWhitespaceOrComment && ch == EOF)
                        {
                            errorFound = true;
                            outputFile << "\n\nError: Comment started but never closed.\n"
                                << "Error found on line " << currentLineNumber << ".\n";
                        }
                    }
                    else
                    {
                        // ch holds the symbol /
                        currentToken = ch;
                    }
                }
                else
                {
                    // ch holds the symbol /
                    currentToken = ch;
                } 
            }
        }
        
        // Ignore whitespace characters and comments
        if (!isWhitespaceOrComment && !currentToken.empty())
        {
            // Add lexeme mapping to list of lexemes
            if (isWord)
            {
                // Check if found token is a reserved word
                const auto itr = reservedWords.find(currentToken);
                if (itr != reservedWords.cend())
                {
                    // Add the lexeme and token type to lexeme list
                    lexemeTable.push_back(std::make_pair(currentToken, itr->second));
                }
                else
                {
                    // Add the lexeme and token type to lexeme list
                    lexemeTable.push_back(std::make_pair(currentToken, identSym));
                }
            }
            else if (isNumber)
            {
                // Add the lexeme and token type to lexeme list
                lexemeTable.push_back(std::make_pair(currentToken, numberSym));
            }
            else // Special Symbol
            {
                // Check if found token is a special symbol
                const auto itr = specialSymbols.find(currentToken);
                if (itr != specialSymbols.cend())
                {
                    // Add the lexeme and token type to lexeme list
                    lexemeTable.push_back(std::make_pair(currentToken, itr->second));
                }
                else
                {
                    errorFound = true;
                    outputFile << "\n\nError: Unknow symbol type found: " << currentToken << ".\n"
                        << "Error found on line " << currentLineNumber << ".\n";
                }
            }

            currentToken.clear();
        }
    }

    outputFile << "\nLexeme Table:\n"
        << std::setw(10) << std::left << "lexeme"
        << std::setw(10) << std::left << "token type"
        << "\n";

    for (auto itr = lexemeTable.cbegin(); itr != lexemeTable.cend(); ++itr)
    {
        outputFile << std::setw(10) << std::left << itr->first
            << std::setw(10) << std::left << itr->second
            << "\n";
    }

    outputFile << "\nLexeme List:\n";

    for (auto itr = lexemeTable.cbegin(); itr != lexemeTable.cend(); ++itr)
    {
        outputFile << itr->second << " ";
        lexemeList.push_back(std::to_string(itr->second));

        if (itr->second == token_type::identSym || itr->second == token_type::numberSym)
        {
            outputFile << itr->first << " ";
            lexemeList.push_back(itr->first);
        }
    }

    outputFile.close();

    return !errorFound;
}

