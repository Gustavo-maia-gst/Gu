#ifndef _lexer
#define _lexer
#include <cctype>
#include <fstream>
#include <istream>
#include <queue>
#include <stdexcept>
#include <string>
#include <map>
#include <unordered_set>

const int READ_BUFF_SIZE = 2048;

enum class TokenType {
  NAME,
  NUMBER,
  STRING,
  CHAR,
  GENERAL,
  END_OF_INPUT,
};

struct Token {
  std::string raw;
  int mappedType;
  TokenType rawType;

  int line;
  int start;
  int end;
};

class Lexer {
public:
  ~Lexer();

  static Lexer *fromFile(std::string &path);
  static Lexer *fromStream(std::istream *stream);
  void setTypeMapper(std::map<std::string, int> *tokenMapper);

  const Token &look();
  const Token &get();
  void unget();

private:
  Lexer();

  std::map<std::string, int> *tokenMapper;
  std::queue<char> buff;
  std::istream *stream;

  Token current;
  bool ungetted = false;

  int line = 1;
  int column = 1;

  void nextToken();
  void nextToken_Indentifier();
  void nextToken_Number();
  void nextToken_Char();
  void nextToken_String();
  void nextToken_General();

  char getChar();
  char lookChar();

  bool reloadBuffer();

  inline void passBlanks();
  inline void error(std::string msg);
};

#endif