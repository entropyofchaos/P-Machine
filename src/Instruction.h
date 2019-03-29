#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <string>

/** Enumeration containing the different OP codes the system can execute */
enum InstructionType : int
{
    LIT = 1, // LIT    R, 0, M    Loads a constant value (literal) M into Register R
    RTN, // RTN    0, 0, 0    Returns from a subroutine and restore the caller environment
    LOD, // LOD    R, L, M    Load value into a selected register from the stack location at offset M from L lexicographical levels down
    STO, // STO    R, L, M    Store value from a selected register in the stack location at offset M from L lexicographical levels down
    CAL, // CAL    0, L, M    Call procedure at code index M (generates new Activation Record and pc <- M)
    INC, // INC    0, 0, M    Allocate M locals (increment sp by M). First four are Functional Value, Static Link (SL), Dynamic Link (DL), and Return Address (RA)
    JMP, // JMP    0, 0, M    Jump to instruction M
    JPC, // JPC    R, 0, M    Jump to instruction M if R = 0
    SIO1, // SIO    R, 0, 1   Write a register to the screen
    SIO2, // SIO    R, 0, 2   Read in input from the user and store it in a register
    SIO3, // SIO    0, 0, 3   End of program (program stops running)
    NEG,
    ADD,
    SUB,
    MUL, // 15
    DIV,
    ODD,
    MOD,
    EQL,
    NEQ,
    LSS,
    LEQ,
    GTR,
    GEQ // = 24
};

const std::string InstructionTypeLookupTable[] =
{
    "invalid", // 0
    "lit", // 1
    "rtn", // 2
    "lod", // 3
    "sto", // 4
    "cal", // 5
    "inc", // 6
    "jmp", // 7
    "jpc", // 8
    "sio", // 9
    "sio", // 10
    "sio", // 11
    "neg", // 12
    "add", // 13
    "sub", // 14
    "mul", // 15
    "div", // 16
    "odd", // 17
    "mod", // 18
    "eql", // 19
    "neq", // 20
    "lss", // 21
    "leq", // 22
    "gtr", // 23
    "geq"  // 24
};

/** Struct representing one instruction to execute. */
struct Instruction
{
    /** OP - Operation Code */
    InstructionType mOpCode;
    /** R - Register being referenced */
    int mRegister;
    /**
     * L - Lexicographical Level or a Register
     * in Arithmetic and Logic Instructions
     */
    int mLexLevelOrReg;
    /**
     * M - Operation Operand. Usage varies based on operational code.
     * - A number (instructions: LIT, INC).
     * - A program address (instructions: JMP, JPC, CAL).
     * - A data address (instructions: LOD, STO)
     * - A register in arithmetic and logic instructions.
     */
    int mMOperand;
};

#endif // INSTRUCTION_H