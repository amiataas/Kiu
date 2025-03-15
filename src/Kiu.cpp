#include <cstdint>
#include <fstream>
#include <iosfwd>
#include <iostream>
#include <list>
#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>
#include <utility>

enum class TokenKind {
  Eof = -1,
  Plus = 1,      // +
  Minus,         // -
  Asterisk,      // *
  Slash,         // /
  Percent,       // %
  Pipe,          // |
  Amp,           // &
  Equal,         // =
  Less,          // <
  Great,         // >
  Tilde,         // ~
  Caret,         // ^
  Exclam,        // !
  Dot,           // .
  Comma,         // ,
  Colon,         // :
  Semicolon,     // ;
  LBrak,         // [
  RBrak,         // ]
  LParn,         // (
  RParn,         // )
  LBrac,         // {
  RBrac,         // }
  DEqual,        // ==
  NEqual,        // !=
  PlusPlus,      // ++
  PlusEqual,     // +=
  MinusMinus,    // --
  MinusEqual,    // -=
  AsteriskEqual, // *=
  SlashEqual,    // /=
  PercentEqual,  // %=
  LessEqual,     // <=
  GreatEqual,    // >=
  DLess,         // <<
  DGreat,        // >>
  DPipe,         // ||
  DAmp,          // &&
  Arrow,         // ->
  EArrow,        // =>
  DecimalNumber,
  RationalNumber,
  String,
  Identifier,
  If,    // if
  Elif,  // elif
  Else,  // else
  Loop,  // loop
  While, // while
  For,   // for
  Brk,   // brk
  Cont,  // cont
  Ret,   // ret
  Let,   // let
  Match, // match
  True,  // true
  False, // false
  Struc, // struc
  Enum,  // enum
  Type,  // type
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
      return "+";
    case TokenKind::Minus:
      return "-";
    case TokenKind::Asterisk:
      return "*";
    case TokenKind::Slash:
      return "/";
    case TokenKind::Percent:
      return "%";
    case TokenKind::Pipe:
      return "|";
    case TokenKind::Amp:
      return "&";
    case TokenKind::Equal:
      return "=";
    case TokenKind::Less:
      return "<";
    case TokenKind::Great:
      return ">";
    case TokenKind::Tilde:
      return "~";
    case TokenKind::Caret:
      return "^";
    case TokenKind::Exclam:
      return "!";
    case TokenKind::Dot:
      return ".";
    case TokenKind::Comma:
      return ",";
    case TokenKind::Colon:
      return ":";
    case TokenKind::Semicolon:
      return ";";
    case TokenKind::LBrak:
      return "[";
    case TokenKind::RBrak:
      return "]";
    case TokenKind::LParn:
      return "(";
    case TokenKind::RParn:
      return ")";
    case TokenKind::LBrac:
      return "{";
    case TokenKind::RBrac:
      return "}";
    case TokenKind::DEqual:
      return "==";
    case TokenKind::NEqual:
      return "!=";
    case TokenKind::PlusPlus:
      return "++";
    case TokenKind::PlusEqual:
      return "+=";
    case TokenKind::MinusMinus:
      return "--";
    case TokenKind::MinusEqual:
      return "-=";
    case TokenKind::AsteriskEqual:
      return "*=";
    case TokenKind::SlashEqual:
      return "/=";
    case TokenKind::PercentEqual:
      return "%=";
    case TokenKind::LessEqual:
      return "<=";
    case TokenKind::GreatEqual:
      return ">=";
    case TokenKind::DLess:
      return "<<";
    case TokenKind::DGreat:
      return ">>";
    case TokenKind::DPipe:
      return "||";
    case TokenKind::DAmp:
      return "&&";
    case TokenKind::Arrow:
      return "->";
    case TokenKind::EArrow:
      return "=>";
    case TokenKind::DecimalNumber:
    case TokenKind::RationalNumber:
      return "num(" + this->literal + ")";
    case TokenKind::String:
      return "str(" + this->literal + ")";
    case TokenKind::Identifier:
      return "id(" + this->literal + ")";
    case TokenKind::If:
      return "if";
    case TokenKind::Elif:
      return "elif";
    case TokenKind::Else:
      return "else";
    case TokenKind::Loop:
      return "loop";
    case TokenKind::While:
      return "while";
    case TokenKind::For:
      return "for";
    case TokenKind::Brk:
      return "brk";
    case TokenKind::Cont:
      return "cont";
    case TokenKind::Ret:
      return "ret";
    case TokenKind::Let:
      return "let";
    case TokenKind::Match:
      return "match";
    case TokenKind::True:
      return "true";
    case TokenKind::False:
      return "false";
    case TokenKind::Struc:
      return "struc";
    case TokenKind::Enum:
      return "enum";
    case TokenKind::Type:
      return "type";
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

      if (this->start == -1) {
        break;
      }

      this->current = this->advance();
      switch (this->current) {
      case '+': {
        char p = peek();
        if (p == '+') {
          advance();
          new_token(TokenKind::PlusPlus);
          break;
        } else if (p == '=') {
          advance();
          new_token(TokenKind::PlusEqual);
          break;
        }
        new_token(TokenKind::Plus);
        break;
      }
      case '-': {
        char p = peek();
        if (p == '-') {
          advance();
          new_token(TokenKind::MinusMinus);
        } else if (p == '>') {
          advance();
          new_token(TokenKind::Arrow);
        } else if (p == '=') {
          advance();
          new_token(TokenKind::MinusEqual);
        } else {
          new_token(TokenKind::Minus);
        }
        break;
      }
      case '*': {
        char p = peek();
        if (p == '=') {
          advance();
          new_token(TokenKind::AsteriskEqual);
        } else {
          new_token(TokenKind::Asterisk);
        }
        break;
      }
      case '/': {
        char p = peek();
        if (p == '=') {
          advance();
          new_token(TokenKind::SlashEqual);
        } else {
          new_token(TokenKind::Slash);
        }
        break;
      }
      case '%': {
        char p = peek();
        if (p == '=') {
          advance();
          new_token(TokenKind::PercentEqual);
        } else {
          new_token(TokenKind::Percent);
        }
        break;
      }
      case '=': {
        char p = peek();
        if (p == '=') {
          advance();
          new_token(TokenKind::DEqual);
        } else if (p == '>') {
          advance();
          new_token(TokenKind::EArrow);
        } else {
          new_token(TokenKind::Equal);
        }
        break;
      }
      case '|': {
        char p = peek();
        if (p == '|') {
          advance();
          new_token(TokenKind::DPipe);
          break;
        } else {
          new_token(TokenKind::Pipe);
          break;
        }
      }
      case '&': {
        char p = peek();
        if (p == '&') {
          advance();
          new_token(TokenKind::DAmp);
          break;
        } else {
          new_token(TokenKind::Amp);
          break;
        }
      }
      case '.':
        new_token(TokenKind::Dot);
        break;
      case ',':
        new_token(TokenKind::Percent);
        break;
      case ':':
        new_token(TokenKind::Colon);
        break;
      case ';':
        new_token(TokenKind::Semicolon);
        break;
      case '[':
        new_token(TokenKind::LBrak);
        break;
      case ']':
        new_token(TokenKind::RBrak);
        break;
      case '(':
        new_token(TokenKind::LParn);
        break;
      case ')':
        new_token(TokenKind::RParn);
        break;
      case '{':
        new_token(TokenKind::LBrac);
        break;
      case '}':
        new_token(TokenKind::RBrac);
        break;
      case '~':
        new_token(TokenKind::Tilde);
        break;
      case '^':
        new_token(TokenKind::Caret);
        break;
      case '!': {
        char p = peek();
        if (p == '=') {
          advance();
          new_token(TokenKind::NEqual);
        } else {
          new_token(TokenKind::Exclam);
        }
        break;
      }
      case '>': {
        char p = peek();
        if (p == '=') {
          advance();
          new_token(TokenKind::GreatEqual);
        } else if (p == '>') {
					advance();
					new_token(TokenKind::DGreat);
        } else {
          new_token(TokenKind::Great);
        }
        break;
      }
      case '<': {
        char p = peek();
        if (p == '=') {
          advance();
          new_token(TokenKind::LessEqual);
        } else if (p == '<') {
          advance();
          new_token(TokenKind::DLess);
        } else {
          new_token(TokenKind::Less);
        }
        break;
      }
      default:
        if (is_decimal_num(this->current)) {
          lex_numeric();
        } else {
          while (!eof() && is_id_char(peek())) {
            this->advance();
          }

          int32_t len = this->get_pos() - this->start;
          std::string literal = std::string(len, {});

          this->file.seekg(-len, std::ios::cur);
          int32_t i = 0;
          for (; i < len; i++) {
            this->file.get(literal[i]);
          }

          if (auto search = _keywords.find(literal);
              search != _keywords.end()) {
            new_token(search->second);
          } else {
            new_token(TokenKind::Identifier, literal);
          }
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
  const std::unordered_map<std::string, TokenKind> _keywords = {
      {"if", TokenKind::If},       {"elif", TokenKind::Elif},
      {"else", TokenKind::Else},   {"loop", TokenKind::Loop},
      {"while", TokenKind::While}, {"for", TokenKind::For},
      {"brk", TokenKind::Brk},     {"cont", TokenKind::Cont},
      {"ret", TokenKind::Ret},     {"let", TokenKind::Let},
      {"match", TokenKind::Match}, {"true", TokenKind::True},
      {"false", TokenKind::False}, {"struc", TokenKind::Struc},
      {"enum", TokenKind::Enum},   {"type", TokenKind::Type},
  };

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

  bool is_low_alpha(char c) { return (0x60 < c) && (0x7B > c); }
  bool is_upp_alpha(char c) { return (0x40 < c) && (0x5B > c); }

  bool is_alpha(char c) { return is_low_alpha(c) || is_upp_alpha(c); }
  bool is_alpha_numberic(char c) { return is_alpha(c) || is_decimal_num(c); }
  bool is_id_char(char c) { return is_alpha_numberic(c) || c == '_'; }

  void lex_numeric() {
    bool is_float = false;
    bool is_binary = false;
    bool is_octal = false;
    bool is_hex = false;

    char prob_radix = peek();
    if (prob_radix == 'b') {
      is_binary = true;
    } else if (prob_radix == 'o') {
      is_octal = true;
    } else if (prob_radix == 'x') {
      is_hex = true;
    }

    if (is_binary || is_octal || is_hex) {
			advance();
    }

    while (!eof()) {
      char c = peek();
      if (is_binary) {
        if (is_binary_num(c)) {
          advance();
          continue;
        } else {
					break;
				}
      } else if (is_octal) {
        if (is_octal_num(c)) {
          advance();
          continue;
        } else {
					break;
				}
      } else if (is_hex) {
        if (is_hex_num(c)) {
          advance();
          continue;
        } else {
					break;
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
