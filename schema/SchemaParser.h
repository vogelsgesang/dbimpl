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

  //parses an SQL schema definition from a stream
  std::vector<RelationSchema> parseSqlSchema(std::istream inStream);

}

#endif
