#ifndef VIRTUALMACHINE_H
#define VIRTUALMACHINE_H

#include "Instruction.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

struct Instruction;

/**
 * Runs the program give the input array
 */
int runProgram(const std::vector<const Instruction>& code, std::stringstream& outputStream);

/**
 * Find new base pointer lex levels down from inputted base pointer.
 * @param lexLevel How many lex levels to go down from base pointer
 * @param basePointer The starting base pointer
 * @return The new base pointer
 */
int base(int lexLevel, int basePointer);

/** Max stack hight for VM. */
const int MAX_STACK_HEIGHT = 2000;
/** Max code lenth accepted by VM. */
const int MAX_CODE_LENGTH = 500;
/** Max lexicographical levels that can be referenced in instructions. */
const int MAX_LEXI_LEVELS = 3;

/**
 * Code Store. This array will hold the code to be
 * excuted by the Virtual machine.
 */
Instruction CODE[MAX_CODE_LENGTH];

/**
 * Working Execution Stack
 * Holds Activation Records/Stack Frames.
 * Initialized to all 0s.
 *
 * @note An activation record or stack frame is the name given to a data
 * structure which is inserted in the stack, each time a procedure or
 * function is called.
 *
 * The data structure contains information to control sub-routines
 * program execution
 *
 * An Activation Record is defined as follows:
 * - Return Value
 * - Static Link (SL)
 * - Dynamic Link (DL)
 * - Return Address (RA)
 */
int STACK[MAX_STACK_HEIGHT] = {};

// Virtual Machine Registers
/**
 * Base Pointer
 * Register that points to the base of the current
 * activation record (AR) in the stack.
 */
int BP = 1;

/**
 * Stack Pointer
 * Points to the top of the stack.
 */
int SP = 0;

/**
 * Program Counter
 * Also sometimes refered to as the Instruction Pointer.
 */
int PC = 0;

/** Instruction Register */
Instruction* IR = 0;

/** Register File. Initialized to all 0s. */
int RF[16] = {};

/** Flag to tell program to halt execution */
int HALT_FLAG = 0;

inline int runProgram(std::stringstream& outputStream)
{
    // Printing out initial values
    int i = 0;  
    std::stringstream out;
    out << "Input ASM code:\n";
    out << "Line       OP        R    L    M\n";
    while (CODE[i].mOpCode != 0)
    {
        out << std::setw(11) << std::left << i
            << std::setw(10) << std::left << InstructionTypeLookupTable[CODE[i].mOpCode]
            << CODE[i].mRegister << "    "
            << CODE[i].mLexLevelOrReg << "    "
            << CODE[i].mMOperand << "\n";
        ++i;
    }
    
    out << "\n\n"
        << "InstrNum   OP        R    L    M"
        << "        PC    BP    SP        "
        << std::setw(50) << std::left << "Stack "
        << "Registers\n";
    
    outputStream << out.str();
    out.str("");
    out.clear();
    
    // Continue until halt flag is set
    while (HALT_FLAG != 1)
    {
        // Fetch Cycle
        // In the Fetch Cycle, an instruction is fetched from the “code” store
        // and placed in the IR register (IR <- code[PC]). Afterwards, the
        // program counter is incremented by 1 to point to the next instruction
        // to be executed (PC <- PC + 1).
        
        // Fetch instruction.
        // Since the CODE block is statically allocated, we will have
        // IR hold the address to the instruction in the code array
        IR = &(CODE[PC]);
        
        // Print Initial Values of Instruction
        out << std::setw(11) << std::left << PC
        << std::setw(10) << std::left << InstructionTypeLookupTable[IR->mOpCode]
        << std::setw(5) << std::left << IR->mRegister
        << std::setw(5) << std::left << IR->mLexLevelOrReg
        << std::setw(9) << std::left << IR->mMOperand;
        
        // Grab next instruction
        PC += 1;
        
        // Execute Cycle
        // In the Execute Cycle, the instruction that was fetched is executed
        // by the VM. The OP component that is stored in the IR register (IR.OP)
        // indicates the operation to be executed. For example, if IR.OP is the
        // ISA instruction ADD (IR.OP = 12), then the R, L, M component of the
        // instruction in the IR register (IR.R, IR.L, IR.M) are used as a
        // register and execute the appropriate arithmetic or logical instruction.
        
        // Switch based on the Operation Code type
        switch (IR->mOpCode)
        {
            // 01 – LIT    R, 0, M
            //     R[i] <- M;
            case LIT:
                // Load literal value (MOperand) from Instruction into Register File i, where i
                // is R in the instruction
                RF[IR->mRegister] = IR->mMOperand;
                break;
            // 02 – RTN  0, 0, 0
            // sp <- bp - 1;
            // bp <- stack[sp + 3];
            // pc <- stack[sp + 4];
            case RTN:
                SP = BP - 1;
                BP = STACK[SP + 3];
                PC = STACK[SP + 4];
                break;
            // 03 – LOD R, L, M
            // R[i] <- stack[base(L, bp) + M];
            // Copy from stack to a register
            case LOD:
                RF[IR->mRegister] = STACK[(base(IR->mLexLevelOrReg, BP) + IR->mMOperand)];
                break;
            // 04 – STO R, L, M
            // stack[base(L, bp) + M] <- R[i];
            // Copy from register to the stack
            case STO:
                STACK[(base(IR->mLexLevelOrReg, BP) + IR->mMOperand)] = RF[IR->mRegister];
                break;
            // 05 - CAL   0, L, M
            // stack[sp + 1]  <- 0;                 // space to return value
            // stack[sp + 2]  <- base(L, bp);       // static link (SL)
            // stack[sp + 3]  <- bp;                // dynamic link (DL)
            // stack[sp + 4]  <- pc;                // return address (RA)
            // bp <- sp + 1;
            // pc <- M;
            case CAL:
                STACK[SP + 1] = 0;                              // Return value
                STACK[SP + 2] = base(IR->mLexLevelOrReg, BP);   // Static Link (SL)
                STACK[SP + 3] = BP;                             // Dynamic Link (DL)
                STACK[SP + 4] = PC;                             // Return Address (RA)
                BP = SP + 1;
                PC = IR->mMOperand;
                break;
            // 06 – INC   0, 0, M
            // sp <- sp + M;
            case INC:
                SP = SP + IR->mMOperand;
                break;
            // 07 – JMP   0, 0, M
            // pc <- M;
            case JMP:
                PC = IR->mMOperand;
                break;
            // 08 – JPC   R, 0, M
            // if (R[i] == 0)
            // then
            // {
            //     pc <- M;
            // }
            case JPC:
                if (RF[IR->mRegister] == 0)
                {
                    PC = IR->mMOperand;
                }
                break;
            // 09 – SIO   R, 0, 1
            // print(R[i]);
            case SIO1:
                std::cout << RF[IR->mRegister] << std::endl;
                break;
            // 10 - SIO   R, 0, 2
            // read(R[i]);
            case SIO2:
                std::cin >> RF[IR->mRegister];
                break;
            // 11 – SIO   R, 0, 3
            // Set Halt flag to one
            case SIO3:
                HALT_FLAG = 1;
                break;
            // 12 - NEG
            // R[i] <- -R[j]
            case NEG:
                RF[IR->mRegister] = -RF[IR->mLexLevelOrReg];
                break;
            // 13 - ADD
            // R[i] <- R[j] + R[k]
            case ADD:
                RF[IR->mRegister] = RF[IR->mLexLevelOrReg] + RF[IR->mMOperand];
                break;
            // 14 - SUB
            // R[i] <- R[j] - R[k]
            case SUB:
                RF[IR->mRegister] = RF[IR->mLexLevelOrReg] - RF[IR->mMOperand];
                break;
            // 15 - MUL
            // R[i] <- R[j] * R[k]
            case MUL:
                RF[IR->mRegister] = RF[IR->mLexLevelOrReg] * RF[IR->mMOperand];
                break;
            // 16 - DIV
            // R[i] <- R[j] / R[k]
            case DIV:
                RF[IR->mRegister] = RF[IR->mLexLevelOrReg] / RF[IR->mMOperand];
                break;
            // 17 - ODD
            // R[i] <- R[i] mod 2
            // or ord(odd(R[i]))
            case ODD:
                RF[IR->mRegister] = RF[IR->mRegister] % 2;
                break;
            // 18 - MOD
            // R[i] <- R[j] mod  R[k]
            case MOD:
                RF[IR->mRegister] = RF[IR->mLexLevelOrReg] % RF[IR->mMOperand];
                break;
            // 19 - EQL
            // R[i] <- R[j] = = R[k]
            case EQL:
                RF[IR->mRegister] = RF[IR->mLexLevelOrReg] == RF[IR->mMOperand];
                break;
            // 20 - NEQ
            // R[i] <- R[j] != R[k]
            case NEQ:
                RF[IR->mRegister] = RF[IR->mLexLevelOrReg] != RF[IR->mMOperand];
                break;
            // 21 - LSS
            // R[i] <- R[j] < R[k]
            case LSS:
                RF[IR->mRegister] = static_cast<int>(RF[IR->mLexLevelOrReg] < RF[IR->mMOperand]);
                break;
            // 22 - LEQ
            // R[i] <- R[j] <= R[k]
            case LEQ:
                RF[IR->mRegister] = static_cast<int>(RF[IR->mLexLevelOrReg] <= RF[IR->mMOperand]);
                break;
            // 23 - GTR
            // R[i] <- R[j] > R[k]
            case GTR:
                RF[IR->mRegister] = static_cast<int>(RF[IR->mLexLevelOrReg] > RF[IR->mMOperand]);
                break;
            // 24 - GEQ
            // R[i] <- R[j] >= R[k]
            case GEQ:
                RF[IR->mRegister] = static_cast<int>(RF[IR->mLexLevelOrReg] >= RF[IR->mMOperand]);
                break;
            default:
                break;
        }
        
        // Print out state of Registers after execution
        out << std::setw(6) << std::left << PC
            << std::setw(6) << std::left << BP
            << std::setw(10) << std::left << SP;
        // Getting Dynamic Link to determine how many Lex Levels
        // into the stack the current Activation Record is.
        int nextLexLvl = STACK[BP + 1];
        // Calculating next base pointer from retrieved number
        // of Lex Levels Down
        int nextBP = base(nextLexLvl, BP);
        for (int i = 1; i <= SP; ++i)
        {
            if (i == nextBP)
            {
                // Calculate next lexicographical level based on current
                // level and next base pointer based on the current on
                // the next lexicographical level. We are going up the
                // stack instead of down.
                nextBP = base(--nextLexLvl, BP);
                if (i > 1)
                {
                    out << "| ";
                }
            }
            int val = STACK[i];
            out << val << " ";
            if (val < 10)
            {
                // Adding an extra space for values over 10 to allow for
                // space for both digits to be printed.
                out << " ";
            }
        }
        
        std::stringstream streamFormatter;
        streamFormatter << std::left << std::setw(112) << out.str();
        out.str("");
        out.clear();
        out << streamFormatter.str();
        
        streamFormatter.str("");
        streamFormatter.clear();
        
        for (int i = 0; i < 8; ++i)
        {
            streamFormatter << std::setw(3) << std::left << RF[i];
        }
        
        streamFormatter << "\n";
        
        out << std::right << streamFormatter.str();
        
        outputStream << out.str();
        out.str("");
        out.clear();
    }
    
    return 0;
}

inline int base(int lexLevelsDown, int basePointer)
{
    int newBasePointer = basePointer; // Find L levels down
    while (lexLevelsDown > 0)
    {
        newBasePointer = STACK[newBasePointer + 1];
        lexLevelsDown--;
    }
    return newBasePointer;
}

#endif // VIRTUALMACHINE_H