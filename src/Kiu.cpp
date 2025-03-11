#include <cstdint>
#include <fstream>
#include <iosfwd>
#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <utility>

enum class NumberKind {
  Decimal,
  Rational,
};

enum class TokenKind {
  Eof = -1,
  Plus = 1,
  Minus,
  Asterisk,
  Slash,
  Precent,

  Dot,
  Comma,
  Colon,
  Semicolon,

  DecimalNumber,
  RationalNumber,
};

enum class OperationKind {
  Add,
  Sub,
  Mul,
  Div,
};

class Token {
public:
  Token(TokenKind kind) : kind(kind) {}
  Token(TokenKind kind, std::string literal) : kind(kind), literal(literal) {}

  std::string to_string() const {
    switch (this->kind) {
    case TokenKind::Eof:
      return "eof";
    case TokenKind::Plus:
      return "pls";
    case TokenKind::Minus:
      return "min";
    case TokenKind::Asterisk:
      return "mul";
    case TokenKind::Slash:
      return "div";
    case TokenKind::Precent:
      return "mod";
    case TokenKind::Dot:
      return ".";
    case TokenKind::Comma:
      return ",";
    case TokenKind::Colon:
      return ":";
    case TokenKind::Semicolon:
      return ";";
    case TokenKind::DecimalNumber:
    case TokenKind::RationalNumber:
      return "num(" + this->literal + ")";
    default:
      return "?";
    }
  }

private:
  TokenKind kind;
  std::string literal;
};

class Lexer {
public:
  Lexer(std::string source_path) : file(source_path) {
    if (!this->file.is_open()) {
      std::cerr << "Error opening the file!";
      return;
    }

    // Find file size.
    this->file.seekg(0, std::ios::end);
    this->fsize = this->file.tellg();
    this->file.seekg(0, std::ios::beg);

    this->tokens = {};
  }

  void lex() {
    while (!eof()) {
      this->skip_trivia();
      this->start = get_pos();
      this->current = this->advance();
      switch (this->current) {
      case '+':
        new_token(TokenKind::Plus);
        break;
      case '-':
        new_token(TokenKind::Minus);
        break;
      case '*':
        new_token(TokenKind::Asterisk);
        break;
      case '/':
        new_token(TokenKind::Slash);
        break;
      case '%':
        new_token(TokenKind::Precent);
        break;
      case '.':
        new_token(TokenKind::Dot);
        break;
      case ',':
        new_token(TokenKind::Precent);
        break;
      case ':':
        new_token(TokenKind::Colon);
        break;
      case ';':
        new_token(TokenKind::Semicolon);
        break;
      default:
        if (is_decimal_num(this->current)) {
          lex_numeric();
        }
        break;
      }
    }
  }

  void new_token(TokenKind kind) { this->tokens.push_back(Token(kind)); }
  void new_token(TokenKind kind, std::string literal) {
    this->tokens.push_back(Token(kind, literal));
  }

  ~Lexer() {
    if (this->file.is_open()) {
      this->file.close();
    }
  }

  std::list<Token> tokens;

private:
  char current;
  std::streampos start = 0;
  std::streampos fsize = 0;
  std::ifstream file;
  std::streampos get_pos() { return this->file.tellg(); }

  /**
   * Return true if at End of file.
   */
  bool eof() { return this->file.eof() || this->file.tellg() == -1; }

  char peek() { return this->file.peek(); }

  std::string peek_many(uint32_t count) {
    std::string s = std::string(count, {});
    uint32_t i = count;
    for (; i != 0; --i) {
      s[count - i] = advance();
    }
    unwind_many(count);
    return s;
  }

  char advance() {
    char ret;
    this->file.get(ret);
    return ret;
  }

  void unwind_many(int32_t count) { this->file.seekg(-count, std::ios::cur); }

  void advance_many(int32_t count) { this->file.seekg(count, std::ios::cur); }

  void skip_trivia() {
    while (!eof()) {
      char c = peek();
      if (is_ascii_whitespace(c)) {
        this->current = advance();
        continue;
      }

      if (peek_many(2) == "//") {
        advance_many(2);
        while (!eof()) {
          if (peek() != '\n') {
            this->current = advance();
          } else {
            this->current = advance();
            break;
          }
        }
      }
      break;
    }
  }

  void unwind() { this->file.unget(); }

  bool is_ascii_whitespace(char c) {
    // TODO: this can turn into a perfect hashmap.
    return '\t' == c || '\n' == c || '\r' == c || ' ' == c || 0x0c == c;
  }

  bool is_binary_num(char c) { return '0' == c || '1' == c; }

  bool is_octal_num(char c) { return (0x2F < c) && (0x38 > c); }

  bool is_hex_num(char c) {
    return ((0x2F < c) && (0x3A > c)) || ((0x40 < c) && (0x47 > c)) ||
           ((0x60 < c) && (0x67 > c));
  }

  bool is_decimal_num(char c) { return (0x2F < c) && (0x3A > c); }

  void lex_numeric() {
    bool is_float = false;
    bool is_binary = false;
    bool is_octal = false;
    bool is_hex = false;

    std::string prob_radix = peek_many(2);
    if (prob_radix == "0b") {
      is_binary = true;
    } else if (prob_radix == "0o") {
      is_octal = true;
    } else if (prob_radix == "0x") {
      is_hex = true;
    }

    if (is_binary || is_octal || is_hex) {
      advance_many(2);
    }

    while (!eof()) {
      char c = peek();
      if (is_binary) {
        if (is_binary_num(c)) {
          advance();
          continue;
        }
      } else if (is_octal) {
        if (is_octal_num(c)) {
          advance();
          continue;
        }
      } else if (is_hex) {
        if (is_hex_num(c)) {
          advance();
          continue;
        }
      } else {
        if (is_decimal_num(c) || c == '.') {
          if (!is_float && c == '.') {
            is_float = true;
            advance();
            continue;
          }

          if (is_float && c == '.') {
            break;
          }

          advance();
          continue;
        } else {
          break;
        }
      }
    }

    int32_t len = this->get_pos() - this->start;
    std::string literal = std::string(len, {});

    this->file.seekg(-len, std::ios::cur);
    int32_t i = 0;
    for (; i < len; i++) {
      this->file.get(literal[i]);
    }

    new_token(is_float ? TokenKind::RationalNumber : TokenKind::DecimalNumber,
              literal);
  }
};

class ExprAST {
public:
  virtual ~ExprAST() = default;
};

class RationalExprAST : public ExprAST {
public:
  RationalExprAST(Token token) : token(token) {}

private:
  Token token;
};

class BinaryExprAST : public ExprAST {
public:
  BinaryExprAST(OperationKind op, std::unique_ptr<ExprAST> left,
                std::unique_ptr<ExprAST> right)
      : op(op), left(std::move(left)), right(std::move(right)) {}

private:
  OperationKind op;
  std::unique_ptr<ExprAST> left, right;
};

int main(int argc, char *argv[]) {
  auto lexer = std::make_unique<Lexer>(std::string(argv[1]));
  lexer->lex();

  for (const auto &item : lexer->tokens) {
    std::cout << item.to_string() << " ";
  }

  return 0;
}
