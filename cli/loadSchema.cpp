#include <iostream>
#include <fstream>
#include <memory>
#include "buffer/bufferManager.h"
#include "schema/RelationSchema.h"
#include "schema/SchemaParser.h"
#include "schema/SchemaSegment.h"

using namespace dbImpl;

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "usage: " << argv[0] << " <schema file>" << std::endl;
    return -1;
  }

  std::ifstream file;
  file.open(argv[1]);
  if(!file.is_open()) {
    std::cerr << "unable to open input file" << std::endl;
    return -1;
  }

  SchemaParser p;
  BufferManager bm(100);
  SchemaSegment segment(bm, 0);
  try {
    std::vector<RelationSchema> schema = p.parse(file);
    segment.store(schema);
  } catch (ParserError& e) {
    file.close();
    std::cerr << e.what() << std::endl;
    return -1;
  }
  file.close();
  return 0;
}
