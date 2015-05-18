#ifndef _SCHEMA_PARSER_HPP_
#define _SCHEMA_PARSER_HPP_

#include <exception>
#include <vector>
#include <istream>

#include "schema/RelationSchema.h"

namespace dbImpl {

  class ParserError : std::exception {
    private:
      std::string msg;
      unsigned line;
    public:
      ParserError(unsigned line, const std::string& m) : msg(m), line(line) {}
      ~ParserError() throw() {}
      const char* what() const throw() {
        return msg.c_str();
      }
      unsigned getLine() { return line; }
  };

  struct SchemaParser {
     std::vector<RelationSchema> parse(std::istream& in);

     private:
       enum class State : unsigned {
         Init, Create, Table, CreateTableBegin, CreateTableEnd,
         TableName, Primary, Key, KeyListBegin, KeyName, KeyListEnd,
         AttributeName, AttributeTypeInt, AttributeTypeChar,
         CharBegin, CharValue, CharEnd, AttributeTypeNumeric, NumericBegin,
         NumericValue1, NumericSeparator, NumericValue2, NumericEnd,
         Not, Null, Separator, Semicolon
       };
       State state = State::Init;

       void nextToken(unsigned line, const std::string& token, std::vector<RelationSchema>* schema);
  };

}

#endif
