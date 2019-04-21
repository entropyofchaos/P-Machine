#ifndef PARSERANDCODEGENERATOR_H
#define PARSERANDCODEGENERATOR_H

#include "Instruction.h"
#include "Tokens.h"
#include "VirtualMachine.h"

#include <climits>
#include <string>
#include <vector>
#include <utility>

std::stringstream* localOutputStream;

/** Maximum number of names that can be stored in the symbols table */
const unsigned short MAX_NAME_TABLE_SIZE = USHRT_MAX;

/** 
 * Structure of the symbol table record 
 * For constants, the table stores kind, name and value.
 * For variables, the table stores kind, name, L and M.
 * For procedures, the table stores kind, name, L and M.
 */
struct Symbol // namerecord_t
{
    int kind;           /** const = 1, var = 2, proc = 3. */
    std::string name;   /** name up to 11 chars */
    int val;            /** number (ASCII value) */
    int level;          /** L level */
    int adr;            /** M address */
    int mark;		    /** to indicate that code has been generated already for a block. */
};
                       
Symbol symbol_table[MAX_NAME_TABLE_SIZE];

std::vector<std::pair<std::string, token_type>> lexemeTable;
std::pair<std::string, token_type>* token = nullptr;

/** Code Index */
int CX = 0;

/** Register Index */
int RX = 0;

/** Symbol Table Pointer */
int TP = 1;

/** Current Stack Address */
int CSA = 4;

/** Tracks if syntax is correct throughout generation of program. */
bool syntaxCorrect = true;

/** Lexeme Table Index */
int lexItr = 0;

/** Get next token and place it in TOKEN */
#define GET(TOKEN) TOKEN = &lexemeTable[lexItr++];

/** Peek at the next token. */
#define PEEK(TOKEN) TOKEN = &lexemeTable[lexItr];

// Forward declarations
void program();
void block();
void statement();
void expression();
void condition();
void expression();
void term();
void factor();
void codegen(InstructionType instType, int reg, int lexLevOrReg, int op);

/****************************************************************************************
    EBNF of  tiny PL/0:

    program ::= block "." . 
    block ::= const-declaration  var-declaration  statement.
    const-declaration ::= [ “const” ident "=" number {"," ident "=" number} “;"].
    var-declaration  ::= [ "var" ident {"," ident} “;"].
    statement   ::= [ ident ":=" expression
                | "begin" statement { ";" statement } "end" 
                | "if" condition "then" statement 
                | "while" condition "do" statement
                | "read" ident 
                | "write"  ident 
                | e ] .  
    condition ::= "odd" expression 
            | expression  rel-op  expression.  
    rel-op ::= "="|“<>"|"<"|"<="|">"|">=“.
    expression ::= [ "+"|"-"] term { ("+"|"-") term}.
    term ::= factor {("*"|"/") factor}. 
    factor ::= ident | number | "(" expression ")“.
    number ::= digit {digit}.
    ident ::= letter {letter | digit}.
    digit ;;= "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9“.
    letter ::= "a" | "b" | … | "y" | "z" | "A" | "B" | ... |"Y" | "Z".


    program ::= block "." . 
    block ::= const-declaration  var-declaration  procedure-declaration statement.	
    constdeclaration ::= ["const" ident "=" number {"," ident "=" number} ";"].	
    var-declaration  ::= [ "int "ident {"," ident} “;"].
    procedure-declaration ::= { "procedure" ident ";" block ";" }
    statement   ::= [ ident ":=" expression
                | "call" ident
                | "begin" statement { ";" statement } "end" 
                | "if" condition "then" statement ["else" statement]
                | "while" condition "do" statement
                | "read" ident
                | "write" expression
                | e ] .  
    condition ::= "odd" expression 
            | expression  rel-op  expression.  
    rel-op ::= "="|“!="|"<"|"<="|">"|">=“.
    expression ::= [ "+"|"-"] term { ("+"|"-") term}.
    term ::= factor {("*"|"/") factor}. 
    factor ::= ident | number | "(" expression ")“.
    number ::= digit {digit}.
    ident ::= letter {letter | digit}.
    digit ;;= "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9“.
    letter ::= "a" | "b" | … | "y" | "z" | "A" | "B" | ... | "Y" | "Z".

    
    Based on Wirth’s definition for EBNF we have the following rule:
    [ ] means an optional item.
    { } means repeat 0 or more times.
    Terminal symbols are enclosed in quote marks.
    A period is used to indicate the end of the definition of a syntactic class.
*****************************************************************************************/

inline void program() {
    GET(token);
    block();
    if (token->second != token_type::periodSym)
    {
        (*localOutputStream) << "Error: - Period expected.\n";
        syntaxCorrect = false;
    }
}

inline void block() {
    if (token->second == token_type::constSym)
    {
        do
        {
            GET(token);
            if (token->second != token_type::identSym)
            {
                (*localOutputStream) << "Error: - const must be followed by an identifier.\n";
                syntaxCorrect = false;
            }
            // Get symbol name before it changes
            std::string symName = token->first; 

            GET(token);
            if (token->second != token_type::eqSym)
            {
                (*localOutputStream) << "Error: - Identifier must be followed by =.\n";
                syntaxCorrect = false;
            }

            GET(token);
            if (token->second != token_type::numberSym)
            {
                (*localOutputStream) << "Error: - = must be followed by a number.\n";
                syntaxCorrect = false;
            }

            // Add const symbol to symbol table
            symbol_table[TP].kind = 1; // const
            symbol_table[TP].name = symName; // symbol name
            symbol_table[TP].val = std::stoi(token->first); // symbol value
            symbol_table[TP].level = -1; // No lex level for const symbols
            symbol_table[TP].adr = -1; // No memory address for const symbols
            symbol_table[TP].mark = 0; // Set symbol as unmarked

            // Go to next position in symbol table
            ++TP;

            GET(token);
        } while (token->second == token_type::commaSym);
        if (token->second != token_type::semicolonSym)
        {
            (*localOutputStream) << "Error: - semicolon or comma missing.\n";
            syntaxCorrect = false;
        }
        GET(token);
    }
    if (token->second == token_type::varSym)
    {
        do
        {
            GET(token);
            if (token->second != token_type::identSym)
            {
                (*localOutputStream) << "Error: - var must be followed by an identifier.\n";
                syntaxCorrect = false;
            }

            symbol_table[TP].kind = 2; // var
            symbol_table[TP].name = token->first; // symbol name
            symbol_table[TP].val = 0;
            symbol_table[TP].level = 0;
            symbol_table[TP].adr = CSA;
            symbol_table[TP].mark = 0;

            ++TP;   // Go to next position in symbol table
            ++CSA;  // Change to next stack address

            GET(token);
        } while (token->second == token_type::commaSym);
        if (token->second != token_type::semicolonSym)
        {
            (*localOutputStream) << "Error: - semicolon or comma missing.\n";
            syntaxCorrect = false;
        }
        GET(token);

        codegen(INC, 0, 0, CSA);
    }
    // Procedure not yet supported
    if (token->second == token_type::procSym)
    {
        (*localOutputStream) << "Error: - procedure not yet supported.\n";
        syntaxCorrect = false;
    }
    // while (token->second == token_type::procSym)
    // {
    //     GET(token);
    //     if (token->second != token_type::identSym)
    //     {
    //         // Error
    //     }
    //     GET(token);
    //     if (token->second != semicolonSym)
    //     {
    //         // Error
    //     }
    //     block();
    //     if (token->second != token_type::semicolonSym)
    //     {
    //         // Error
    //     }
    //     GET(token);
    // }
    statement();
}

inline void statement()
{
    switch(token->second)
    {
        case token_type::identSym:
        {
            // Find identifier in symbol table
            int i;
            for (i = TP - 1; i > 0; --i)
            {
                if (symbol_table[i].name == token->first)
                {
                    // Symbol found
                    break;
                }
            }
            if (i == 0)
            {
                (*localOutputStream) << "Error: - Undeclared identifier.\n";
                syntaxCorrect = false;
            }
            if (symbol_table[i].kind != 2)
            {
                (*localOutputStream) << "Error: - Assignment to constant or procedure is not allowed.\n";
                syntaxCorrect = false;
                i = 0;
            }

            GET(token);
            if (token->second != token_type::becomesSym)
            {
                (*localOutputStream) << "Error: - Assignment operator expected.\n";
                syntaxCorrect = false;
            }
            GET(token);

            int reg1 = RX;

            expression();

            if (i != 0)
            {
                // Generate store call
                codegen(STO, reg1, 0, symbol_table[i].adr);
                --RX;
            }

            break;
        }
        case token_type::callSym:
        {
            (*localOutputStream) << "Error: - call not yet supported.\n";
            syntaxCorrect = false;

            // GET(token);
            // if (token->second != token_type::identSym)
            // {
            //     // Error
            // }
            // GET(token);
            break;
        }
        case token_type::beginSym:
        {
            // Get the next token after the begin token 
            // and handle statement
            GET(token);
            statement();
            
            // As long as the next symbol is a starting statement token,
            // keep parsing/generating statement code
            while (STATEMENT_TOKENS.count(token->second))
            {
                // If the next symbol is a semicolon, get the next token
                // so we handle the next statement.
                while (token->second == token_type::semicolonSym)
                {
                    GET(token);
                }
                // else
                // {
                //     // If not a semicolon, don't grab next symbol
                //     // so that statement can still continue running.
                //     (*localOutputStream) << "Warning: - Semicolon between statements missing.\n";
                // }

                statement();
            }

            if (token->second != token_type::endSym)
            {
                (*localOutputStream) << "Error: - Incorrect symbol after statement. end, semicolon or } expected.\n";
                syntaxCorrect = false;
            }
            GET(token);
            break;
        }
        case token_type::ifSym:
        {
            int reg1 = RX;

            GET(token);
            condition();
            if (token->second != token_type::thenSym)
            {
                (*localOutputStream) << "Error: - then expected.\n";
                syntaxCorrect = false;
            }
            
            GET(token);
            
            int ctemp = CX;
            codegen(JPC, reg1, 0, 0);

            statement();

            if (token->second == token_type::semicolonSym)
            {
                auto tokenTemp = token;
                PEEK(token);

                if (token->second == token_type::elseSym)
                {
                    // If the token after the semicolon is an else token, 
                    // then get the next token so the else code can be 
                    // processed. 
                    GET(token)
                }
                else
                {
                    // If it isn't an else token, revert the token back
                    // to its previous held value so it can be processed
                    // as the end of a statment section of code when this
                    // function returns.
                    token = tokenTemp;
                }
                
            }

            if (token->second == token_type::elseSym)
            {
                // Token after else token
                GET(token);

                // Create jump that will bring the
                // stack pointer to the code after the
                // else statment should the if statement
                // execute.
                int ctemp2 = CX;
                codegen(JMP, reg1, 0, 0);
                
                // Update jump that will bring the
                // stack pointer to the code in the else
                // condition if the if condition fails
                CODE[ctemp].mMOperand = CX;

                statement();

                // Update the jump at the end of the if statment.
                // The stack pointer will need to be moved to the
                // currently stored stack index which is right after
                // the else statemnt.
                CODE[ctemp2].mMOperand = CX;
            }
            else 
            {
                CODE[ctemp].mMOperand = CX;
            }

            break;
        }
        case token_type::whileSym:
        {
            int reg1 = RX;
            int ctemp1 = CX;
            GET(token);
            condition();
            
            int ctemp2 = CX;
            codegen(JPC, reg1, 0, 0);

            if (token->second != token_type::doSym)
            {
                (*localOutputStream) << "Error: - do expected.\n";
                syntaxCorrect = false;
            }
            GET(token);
            statement();

            codegen(JMP, 0, 0, ctemp1);
            CODE[ctemp2].mMOperand = CX;
            break;
        }
        case token_type::readSym:
        {
            GET(token);

            // Find identifier in symbol table
            int i;
            for (i = TP - 1; i > 0; --i)
            {
                if (symbol_table[i].name == token->first)
                {
                    // Symbol found
                    break;
                }
            }
            if (i == 0)
            {
                (*localOutputStream) << "Error: - Undeclared identifier.\n";
                syntaxCorrect = false;
            }
            if (symbol_table[i].kind != 2)
            {
                (*localOutputStream) << "Error: - Cannot write to a constant or procedure.\n";
                syntaxCorrect = false;
                i = 0;
            }

            ++RX;
            codegen(SIO2, RX, 0, 0);

            if (i != 0)
            {
                // Store value in register RX into variable at adr from symbol table
                codegen(STO, RX, 0, symbol_table[i].adr);
                --RX;
            }
            GET(token);

            break;
        }
        case token_type::writeSym:
        {   
            GET(token);
            
            if (token->second == token_type::identSym)
            {
                // Find identifier in symbol table
                int i;
                for (i = TP - 1; i > 0; --i)
                {
                    if (symbol_table[i].name == token->first)
                    {
                        // Symbol found
                        break;
                    }
                }
                if (i == 0)
                {
                    (*localOutputStream) << "Error: - Undeclared identifier.\n";
                    syntaxCorrect = false;
                }

                ++RX;
                // Copy value at found address into register at RX
                codegen(LOD, RX, 0, symbol_table[i].adr);

                // Print value stored register at RX
                codegen(SIO1, RX, 0, 0);
                --RX;

                GET(token);  
            }
            else
            {
                (*localOutputStream) << "Error: - Write must be followed by an identifier.\n";
                    syntaxCorrect = false;
            }

            break;
        }
        default:
        {
            // Handle an empty statement
            
            // (*localOutputStream) << "Error: - statement expected.\n";
            // syntaxCorrect = false;
        }
    }
}

inline void condition()
{
    if (token->second == token_type::oddSym)
    {
        // TODO
        GET(token);
        expression();
    }
    else
    {
        expression();
        if (relationOperator.count(token->second) == 0)
        {
            (*localOutputStream) << "Error: - relation operator expected.\n";
            syntaxCorrect = false;
        }
        token_type relop = token->second;
        
        int reg1 = RX - 1;
        int reg2 = RX;

        GET(token);
        expression();

        switch(relop)
        {
            case neqSym:
            {
                codegen(NEQ, reg1, reg1, reg2);
                break;
            }
            case eqSym:
            {
                codegen(EQL, reg1, reg1, reg2);
                break;
            }
            case lesSym:
            {
                codegen(LSS, reg1, reg1, reg2);
                break;
            }
            case leqSym:
            {
                codegen(LEQ, reg1, reg1, reg2);
                break;
            }
            case gtrSym:
            {
                codegen(GTR, reg1, reg1, reg2);
                break;
            }
            case geqSym:
            {
                codegen(GEQ, reg1, reg1, reg2);
                break;
            }
            default:
            {
                (*localOutputStream) << "Error: - relationship operator not handled.\n";
            }
        }
    }
}

inline void expression()
{
    // We can ignore the condition of a + before a term since this
    // doesn't effect the result.
    if (token->second == token_type::minusSym)
    {
        int reg1 = RX - 1;
        int reg2 = RX;

        GET(token);
        term();

        codegen(NEG, reg1, reg1, 0);
        --RX;
    }
    else
    {
        term();
        while (token->second == token_type::plusSym || token->second == token_type::minusSym)
        {
            token_type plusMinusOp = token->second;

            int reg1 = RX;
            int reg2 = RX - 1;

            GET(token);
            term();

            switch (plusMinusOp)
            {
                case token_type::plusSym:
                {
                    codegen(ADD, reg2, reg2, reg1);
                    --RX;
                    break;
                }
                case token_type::minusSym:
                {
                    codegen(SUB, reg2, reg2, reg1);
                    --RX;
                    break;
                }
                default:
                {
                    (*localOutputStream) << "Error - term operator not handled.\n";
                }
            }
        }
    }
}

inline void term()
{
    factor(); // this will do a load and modify RX
    while (token->second == token_type::multSym || token->second == token_type::slashSym)
    {
        token_type mulDivOp = token->second;
        
        int reg1 = RX - 1;
        int reg2 = RX;

        GET(token);
        factor(); // this will do a load and modify RX
        
        if (mulDivOp == token_type::multSym)
        {
            codegen(MUL, reg1, reg1, reg2);
            --RX;
        }
        else
        {
            codegen(DIV, reg1, reg1, reg2);
            --RX;
            break;
        }
    }
}

inline void factor()
{
    if (token->second == token_type::identSym)
    {
        // Find identifier in symbol table
        int i;
        for (i = TP - 1; i > 0; --i)
        {
            if (symbol_table[i].name == token->first)
            {
                // Symbol found
                break;
            }
        }
        if (i == 0)
        {
            (*localOutputStream) << "Error: - Undeclared identifier.\n";
            syntaxCorrect = false;
        }

        codegen(LOD, RX, 0, symbol_table[i].adr);
        ++RX;

        GET(token);
    }
    else if (token->second == token_type::numberSym)
    {
        codegen(LIT, RX, 0, std::stoi(token->first));
        ++RX;

        GET(token);
    }
    else if (token->second == token_type::lparentSym)
    {
        GET(token);
        expression();
        if (token->second != token_type::rparentSym)
        {
            (*localOutputStream) << "Error: - Right parenthesis missing.\n";
            syntaxCorrect = false;
        }
        GET(token);
    }
    else
    {
        (*localOutputStream) << "Error: - The preceding factor cannot begin with this symbol.\n";
        syntaxCorrect = false;
    }
}

inline void codegen(InstructionType instType, int reg, int lexLevOrReg, int op)
{
    if (CX > MAX_CODE_LENGTH)
    {
        (*localOutputStream) << "Error: - Generated code length became too large.\n";
        syntaxCorrect = false;
    }
    else
    {
        CODE[CX].mOpCode = instType;
        CODE[CX].mRegister = reg;
        CODE[CX].mLexLevelOrReg = lexLevOrReg;
        CODE[CX].mMOperand = op;
        ++CX;
    }
}

inline bool parseAndGenerage(const std::vector<std::pair<std::string, token_type>>& lexemes, 
    std::stringstream& outputStream)
{
    lexemeTable = lexemes;

    localOutputStream = &outputStream;

    program();

    codegen(SIO3, 0, 0, 3);

    (*localOutputStream) << "Generated Code:\n";
    int i = 0;  
    outputStream << "Line       OP        R    L    M\n";
    while (CODE[i].mOpCode != 0)
    {
        outputStream << std::setw(11) << std::left << i
            << std::setw(10) << std::left << InstructionTypeLookupTable[CODE[i].mOpCode]
            << CODE[i].mRegister << "    "
            << CODE[i].mLexLevelOrReg << "    "
            << CODE[i].mMOperand << "\n";
        ++i;
    }

    if (syntaxCorrect)
    {
        (*localOutputStream) << "\n\nNo errors, program is syntactically correct.\n";
    }

    return syntaxCorrect;
}

#endif // PARSERANDCODEGENERATOR_H