#pragma once

#include <cctype>
#include <cmath>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace archsynth::json {

struct Value {
  enum class Type { Null, Number, String, Array, Object, Boolean };
  Type type = Type::Null;
  double number = 0.0;
  bool boolean = false;
  std::string string;
  std::vector<Value> array;
  std::map<std::string, Value> object;

  const Value& at(const std::string& key) const {
    const auto it = object.find(key);
    if (type != Type::Object || it == object.end())
      throw std::invalid_argument("missing JSON field: " + key);
    return it->second;
  }
};

inline std::string escape(const std::string& input) {
  std::ostringstream out;
  for (const unsigned char c : input) {
    switch (c) {
      case '"': out << "\\\""; break;
      case '\\': out << "\\\\"; break;
      case '\b': out << "\\b"; break;
      case '\f': out << "\\f"; break;
      case '\n': out << "\\n"; break;
      case '\r': out << "\\r"; break;
      case '\t': out << "\\t"; break;
      default:
        if (c < 0x20) {
          const char* hex = "0123456789abcdef";
          out << "\\u00" << hex[c >> 4] << hex[c & 15];
        } else {
          out << static_cast<char>(c);
        }
    }
  }
  return out.str();
}

class Parser {
 public:
  explicit Parser(const std::string& text) : text_(text) {}
  Value parse() {
    Value result = parse_value();
    whitespace();
    if (pos_ != text_.size()) fail("trailing characters");
    return result;
  }

 private:
  const std::string& text_;
  std::size_t pos_ = 0;

  [[noreturn]] void fail(const std::string& message) const {
    throw std::invalid_argument("invalid JSON at byte " + std::to_string(pos_) + ": " + message);
  }
  void whitespace() { while (pos_ < text_.size() && std::isspace(static_cast<unsigned char>(text_[pos_]))) ++pos_; }
  char take() { if (pos_ >= text_.size()) fail("unexpected end"); return text_[pos_++]; }
  void literal(const char* word) { while (*word) if (take() != *word++) fail("invalid literal"); }

  Value parse_value() {
    whitespace();
    if (pos_ >= text_.size()) fail("expected value");
    if (text_[pos_] == '{') return parse_object();
    if (text_[pos_] == '[') return parse_array();
    if (text_[pos_] == '"') { Value v; v.type = Value::Type::String; v.string = parse_string(); return v; }
    if (text_[pos_] == 't') { literal("true"); Value v; v.type = Value::Type::Boolean; v.boolean = true; return v; }
    if (text_[pos_] == 'f') { literal("false"); Value v; v.type = Value::Type::Boolean; return v; }
    if (text_[pos_] == 'n') { literal("null"); return {}; }
    return parse_number();
  }
  std::string parse_string() {
    if (take() != '"') fail("expected string");
    std::string out;
    while (pos_ < text_.size()) {
      char c = take();
      if (c == '"') return out;
      if (c != '\\') { if (static_cast<unsigned char>(c) < 0x20) fail("control character in string"); out += c; continue; }
      const char escaped = take();
      switch (escaped) {
        case '"': out += '"'; break; case '\\': out += '\\'; break; case '/': out += '/'; break;
        case 'b': out += '\b'; break; case 'f': out += '\f'; break; case 'n': out += '\n'; break;
        case 'r': out += '\r'; break; case 't': out += '\t'; break;
        default: fail("unsupported string escape");
      }
    }
    fail("unterminated string");
  }
  Value parse_number() {
    const std::size_t start = pos_;
    if (text_[pos_] == '-') ++pos_;
    if (pos_ >= text_.size() || !std::isdigit(static_cast<unsigned char>(text_[pos_]))) fail("expected number");
    while (pos_ < text_.size() && std::isdigit(static_cast<unsigned char>(text_[pos_]))) ++pos_;
    if (pos_ < text_.size() && text_[pos_] == '.') { ++pos_; while (pos_ < text_.size() && std::isdigit(static_cast<unsigned char>(text_[pos_]))) ++pos_; }
    if (pos_ < text_.size() && (text_[pos_] == 'e' || text_[pos_] == 'E')) {
      ++pos_; if (pos_ < text_.size() && (text_[pos_] == '+' || text_[pos_] == '-')) ++pos_;
      while (pos_ < text_.size() && std::isdigit(static_cast<unsigned char>(text_[pos_]))) ++pos_;
    }
    Value v; v.type = Value::Type::Number;
    try { v.number = std::stod(text_.substr(start, pos_ - start)); } catch (...) { fail("invalid number"); }
    if (!std::isfinite(v.number)) fail("non-finite number");
    return v;
  }
  Value parse_array() {
    take(); Value v; v.type = Value::Type::Array; whitespace();
    if (pos_ < text_.size() && text_[pos_] == ']') { ++pos_; return v; }
    for (;;) { v.array.push_back(parse_value()); whitespace(); const char c = take(); if (c == ']') return v; if (c != ',') fail("expected ',' or ']'"); }
  }
  Value parse_object() {
    take(); Value v; v.type = Value::Type::Object; whitespace();
    if (pos_ < text_.size() && text_[pos_] == '}') { ++pos_; return v; }
    for (;;) {
      whitespace(); if (pos_ >= text_.size() || text_[pos_] != '"') fail("expected object key");
      const std::string key = parse_string(); whitespace(); if (take() != ':') fail("expected ':'");
      if (!v.object.emplace(key, parse_value()).second) fail("duplicate object key");
      whitespace(); const char c = take(); if (c == '}') return v; if (c != ',') fail("expected ',' or '}'");
    }
  }
};

inline Value parse(const std::string& text) { return Parser(text).parse(); }
inline const std::string& string(const Value& v, const std::string& name) {
  if (v.type != Value::Type::String) throw std::invalid_argument(name + " must be a string");
  return v.string;
}
inline double number(const Value& v, const std::string& name) {
  if (v.type != Value::Type::Number) throw std::invalid_argument(name + " must be a number");
  return v.number;
}

}  // namespace archsynth::json
