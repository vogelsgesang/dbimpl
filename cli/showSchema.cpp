#include <iostream>
#include <fstream>
#include <memory>
#include "buffer/bufferManager.h"
#include "schema/RelationSchema.h"
#include "schema/SchemaSegment.h"

using namespace dbImpl;

int main(int argc, char*[]) {
  if (argc != 1) {
    std::cerr << "this program takes no parameters" << std::endl;
    return -1;
  }

  BufferManager bm(100);
  SchemaSegment segment(bm, 0);

  std::vector<RelationSchema> schema = segment.read();
  for(auto relationSchema : schema) {
    std::cout << relationSchema << std::endl;
  }

  return 0;
}
