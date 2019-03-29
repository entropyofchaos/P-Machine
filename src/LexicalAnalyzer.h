#ifndef LEXICALANALYZER_H
#define LEXICALANALYZER_H

#include "Tokens.h"

#include <fstream>
#include <iomanip> // setw()
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

/** Identifies are limited to 11 characters. */
const short MAX_IDENTIFIER_LENGTH = 11;
/** Numbers have a max length of 5 characters. */
const short MAX_NUMBER_LENGTH = 5;

/**
 * Analizes the code and returns a lexeme list
 */
inline bool analyzeCode(std::stringstream& inputStream, std::stringstream& outputStream, std::vector<std::pair<std::string, token_type>>& lexemeTable)
{
    std::vector<std::string> lexemeList;

    // Print source program into the output file
    outputStream << "Source Program: \n" 
        << inputStream.str() << "\n\n";

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
    while(inputStream.peek() != EOF)
    { 
        // State 1
        inputStream.get(ch);
        currentToken = ch;

        // Check if the next character is a letter
        if (isalpha(ch))
        {
            isWord = true;
            isNumber = false;
            isWhitespaceOrComment = false;

            // State 2
            char ch2 = inputStream.peek();
            while (ch2 != EOF && isalnum(ch2))
            {
                // Put next character into ch.
                inputStream.get(ch);
                ch2 = inputStream.peek();

                // State 3
                currentToken += ch;
            }
            
            // Check if identifier token is too long
            if (currentToken.length() > MAX_IDENTIFIER_LENGTH)
            {
                errorFound = true;
                outputStream << "\n\nError: Current identifier token " << currentToken << " exceeds " 
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
            char ch2 = inputStream.peek();

            // Check if the number is followed directly by a letter. This means someone
            // tried writing an identifier that starts with a number.
            if (isalpha(ch2))
            {
                errorFound = true;
                outputStream << "\n\nError: Current identifier token " << currentToken 
                    << " starts with a number which is not allowed.\n"
                    << "Error found on line " << currentLineNumber << ".\n";
            }

            // Keep checking for more digits after the first digit.
            while (ch2 != EOF && isdigit(ch2))
            {
                // Put next character into ch.
                inputStream.get(ch);
                ch2 = inputStream.peek();

                // State 6
                currentToken += ch;
            }

            // Check if token is too long
            if (currentToken.length() > MAX_NUMBER_LENGTH)
            {
                errorFound = true;
                outputStream << "\n\nError: Current number token " << currentToken << " exceeds " 
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
                char ch2 = inputStream.peek();
                if (ch2 != EOF)
                {
                    // Check for <=
                    if (ch2 == '=')
                    {
                        inputStream.get(ch);
                        currentToken = "<=";
                    }
                    // Check for <>
                    else if (ch2 == '>')
                    {
                        inputStream.get(ch);
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
                char ch2 = inputStream.peek();
                if (ch2 != EOF)
                {
                    // Check for >=
                    if (ch2 == '=')
                    {
                        inputStream.get(ch);
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
                char ch2 = inputStream.peek();
                if (ch2 != EOF)
                {
                    // Check for :=
                    if (ch2 == '=')
                    {
                        inputStream.get(ch);
                        currentToken = ":=";
                    }
                    else
                    {
                        errorFound = true;
                        outputStream << "\n\nError: Found : not followed by =.\n"
                            << "Error found on line " << currentLineNumber << ".\n";
                    }
                }
            }
            else if (ch == '/')
            {
                char ch2 = inputStream.peek();
                if (ch2 != EOF)
                {
                    // Check for beginning of a comment denoted by /*
                    if (ch2 == '*')
                    {
                        // Flush * from inputStream. ch now holds *.
                        inputStream.get(ch);

                        // Get first character after /*
                        inputStream.get(ch);

                        do
                        {
                            // Look for the end of a comment denoted by */
                            if (ch == '*' && inputStream.peek() == '/')
                            {
                                // We reached the end of a comment

                                // Flush '/' from inputStream. ch now holds /.
                                inputStream.get(ch);
                                
                                // Set that we just parsed a comment
                                isWhitespaceOrComment = true;
                            }
                            else
                            {
                                // Flush next character from inside comment.
                                // We don't store commented out characters.
                                // ch now holds a character from inside a comment.
                                inputStream.get(ch);
                            }
                        }
                        // Exit comment flushing loop when the end of comment sequence */ is found
                        // or we hit the end of the file.
                        while (ch != EOF && !isWhitespaceOrComment);

                        // We have an error. A comment started but was never ended.
                        if (!isWhitespaceOrComment && ch == EOF)
                        {
                            errorFound = true;
                            outputStream << "\n\nError: Comment started but never closed.\n"
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
                    outputStream << "\n\nError: Unknow symbol type found: " << currentToken << ".\n"
                        << "Error found on line " << currentLineNumber << ".\n";
                }
            }
            std::string a = currentToken;
            currentToken.clear();
        }
    }

    outputStream << "\nLexeme Table:\n"
        << std::setw(10) << std::left << "lexeme"
        << std::setw(10) << std::left << "token type"
        << "\n";

    for (auto itr = lexemeTable.cbegin(); itr != lexemeTable.cend(); ++itr)
    {
        outputStream << std::setw(10) << std::left << itr->first
            << std::setw(10) << std::left << itr->second
            << "\n";
    }

    outputStream << "\nLexeme List:\n";

    for (auto itr = lexemeTable.cbegin(); itr != lexemeTable.cend(); ++itr)
    {
        outputStream << itr->second << " ";
        lexemeList.push_back(std::to_string(itr->second));

        if (itr->second == token_type::identSym || itr->second == token_type::numberSym)
        {
            outputStream << itr->first << " ";
            lexemeList.push_back(itr->first);
        }
    }

    return !errorFound;
}

#endif // LEXICALANALYZER_H