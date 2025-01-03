#include "lexer.h"
#include <cctype>
#include <stdexcept>
#include <string>
#include <unordered_set>

// These characters are single-chars token
const std::unordered_set<char> grammarChars = {'(', ')', '[', ']', '{',
                                               '}', ';', ',', '.'};
const std::unordered_set<std::string> reservedWords = {
    "_LEXER__CHAR__", "_LEXER__STRING__", "_LEXER__NUMBER__",
    "_LEXER__IDENTIFIER__"};

Lexer::Lexer() {}

Lexer::~Lexer() { delete this->stream; }

Lexer *Lexer::fromFile(std::string &path) {
  auto fileStream = new std::ifstream(path);
  if (!fileStream)
    throw new std::runtime_error("Was not possible to open the file");

  auto sc = new Lexer;
  sc->stream = fileStream;
  return sc;
}

Lexer *Lexer::fromStream(std::istream *stream) {
  auto sc = new Lexer;
  sc->stream = stream;
  if (!sc->stream)
    throw new std::runtime_error("Was not possible to open the file");
  return sc;
}

void Lexer::setTypeMapper(std::map<std::string, int> *tokenMapper) {
  this->tokenMapper = tokenMapper;
}

const Token &Lexer::look() {
  return current;
}

void Lexer::unget() {
  if (ungetted) {
    throw new std::runtime_error("Ungetted two tokens");
  }
  ungetted = true;
}

const Token &Lexer::get() {
  if (ungetted) {
    ungetted = false;
    return current;
  }

  passBlanks();

  char nextChar = lookChar();

  current.line = line;
  current.start = column;

  if (std::isalpha(nextChar) || nextChar == '_')
    nextToken_Indentifier();
  else if (std::isdigit(nextChar))
    nextToken_Number();
  else if (nextChar == '"')
    nextToken_String();
  else if (nextChar == '\'')
    nextToken_Char();
  else if (std::ispunct(nextChar))
    nextToken_General();
  else {
    current.raw = "";
    current.mappedType = 0;
    current.rawType = TokenType::END_OF_INPUT;
  }

  current.end = column;

  return current;
}

void Lexer::nextToken_Indentifier() {
  std::string s = "";
  while (isalnum((lookChar())) || lookChar() == '_')
    s += getChar();

  if (reservedWords.find(s) != reservedWords.end())
    error("Use of reserved word: " + s);

  current.raw = s;
  current.mappedType =
      tokenMapper ? (*tokenMapper)[s] ? (*tokenMapper)[s]
                                      : (*tokenMapper)["_LEXER__IDENTIFIER__"]
                  : 0;
  current.rawType = TokenType::NAME;
}

void Lexer::nextToken_String() {
  getChar();
  std::string s = "";
  while (lookChar() && lookChar() != '"') {
    char c = getChar();
    if (c == '\\') {
      s += getChar();
      continue;
    }

    s += c;
  }
  if (!lookChar())
    error("Unexpected EOF reading string");
  getChar();

  current.raw = s;
  current.rawType = TokenType::STRING;
  current.mappedType = tokenMapper ? (*tokenMapper)["_LEXER__STRING__"] : 0;
}

void Lexer::nextToken_Char() {
  getChar();

  std::string s = "";

  char c = lookChar();
  if (c && c != '\'') {
    s += getChar();
  }

  if (getChar() != '\'')
    error("Unexpected value reading char");

  current.raw = s;
  current.rawType = TokenType::CHAR;
  current.mappedType = tokenMapper ? (*tokenMapper)["_LEXER__CHAR__"] : 0;
}

void Lexer::nextToken_Number() {
  std::string s = "";
  s += getChar();
  if (s == "0" && lookChar() == 'x') {
    getChar();
    s = "";
    while (std::isxdigit(lookChar()))
      s += getChar();
    int shex = std::stoi(s, nullptr, 16);
    s = std::to_string(shex);
  } else if (s == "0" && lookChar() == 'b') {
    getChar();
    int val = 0;
    while (std::isdigit(lookChar())) {
      char c = getChar();
      if (c != '0' && c != '1')
        error("Invalid binary value");

      val = c == '0' ? 2 * val : 2 * val + 1;
    }
    s = std::to_string(val);
  } else {
    while (std::isdigit(lookChar()))
      s += getChar();
    if (lookChar() == '.') {
      getChar();
      while (std::isdigit(lookChar()))
        s += getChar();
    }
  }

  current.raw = s;
  current.rawType = TokenType::NUMBER;
  current.mappedType = tokenMapper ? (*tokenMapper)["_LEXER__NUMBER__"] : 0;
}

void Lexer::nextToken_General() {
  std::string s = "";
  s += getChar();
  if (grammarChars.find(s[0]) == grammarChars.end()) {
    while (std::ispunct(lookChar()) &&
           grammarChars.find(lookChar()) == grammarChars.end())
      s += getChar();
  }

  current.raw = s;
  current.mappedType = tokenMapper ? (*tokenMapper)[s] : 0;
  current.rawType = TokenType::GENERAL;
}

char Lexer::getChar() {
  char c = lookChar();
  buff.pop();
  return column++, c;
}

char Lexer::lookChar() {
  if (buff.empty() && !reloadBuffer())
    return '\0';
  char c = buff.front();
  return c;
}

bool Lexer::reloadBuffer() {
  char buff[READ_BUFF_SIZE];

  stream->read(buff, READ_BUFF_SIZE);
  ssize_t readen = stream->gcount();
  if (!readen)
    return false;

  for (int i = 0; i < readen; i++)
    this->buff.push(buff[i]);

  return true;
}

void inline Lexer::error(std::string msg) {
  throw new std::runtime_error("lexer: " + msg + " | " + std::to_string(line) +
                               ":" + std::to_string(column));
}

void inline Lexer::passBlanks() {
  while (std::isspace(lookChar())) {
    if (getChar() == '\n') {
      line++;
      column = 1;
    }
  }
}