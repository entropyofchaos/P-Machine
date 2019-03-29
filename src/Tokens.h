#ifndef TOKENS_H
#define TOKENS_H

#include <string>
#include <unordered_set>
#include <unordered_map>

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

/** Hash table of reserved words mapping them to their enum token_type values. */
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

/** Hash table of special symbols mapping them to their enum token_type values. */
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

/** Hash set of relationship operator token_type values. */
const std::unordered_set<token_type> relationOperator =
{
    neqSym,
    eqSym,
    lesSym,
    leqSym,
    gtrSym,
    geqSym
};

#endif // TOKENS_H