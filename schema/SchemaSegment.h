#ifndef _SCHEMA_SEGMENT_HPP_
#define _SCHEMA_SEGMENT_HPP_

#include <cstdint>
#include <vector>

namespace dbImpl {
  class BufferManager;
  class RelationSchema;

  class SchemaSegment {
    public:
      SchemaSegment(BufferManager& bm, uint32_t segmentNr);
      void store(const std::vector<RelationSchema>& s);
      std::vector<RelationSchema> read();

    private:
      BufferManager& bufferManager;
      uint32_t segmentNr;
  };

}

#endif
