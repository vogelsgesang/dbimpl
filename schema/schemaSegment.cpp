#include "schema/schemaSegment.h"

#include <stdexcept>

#include "schema/relationSchema.h"
#include "buffer/bufferFrame.h"
#include "buffer/bufferManager.h"
#include "utils/finally.h"

namespace dbImpl {

  SchemaSegment::SchemaSegment(BufferManager& bm, uint32_t segmentNr)
    : bufferManager(bm), segmentNr(segmentNr) {}

  void SchemaSegment::store(const std::vector<RelationSchema>& s) {
    BufferFrame& frame = bufferManager.fixPage(BufferManager::buildPageId(segmentNr, 0), true);
    auto finallyUnfixFrame = finally([&frame, this] { bufferManager.unfixPage(frame, true); });
    uint8_t* data = frame.getData();

    uint32_t offset = 0;
    //serialize all RelationSchemas
    for(const auto& relationSchema : s) {
      Record serialized = relationSchema.serializeToRecord();
      if(offset + sizeof(uint64_t) + serialized.getLen() > BufferManager::pageSize) {
        throw std::runtime_error("schema segment too small to hold all schema data");
      }
      uint64_t* sizePtr = reinterpret_cast<uint64_t*>(data + offset);;
      *sizePtr = serialized.getLen();
      offset += sizeof(uint64_t);
      std::memcpy(data + offset, serialized.getData(), serialized.getLen());
      offset += serialized.getLen();
    }
    //write terminator
    if(offset + sizeof(uint64_t) > BufferManager::pageSize) {
      throw std::runtime_error("schema segment too small to hold all schema data");
    }
    *reinterpret_cast<uint64_t*>(data + offset) = 0;
  }

  std::vector<RelationSchema> SchemaSegment::read() {
    BufferFrame& frame = bufferManager.fixPage(BufferManager::buildPageId(segmentNr, 0), false);
    auto finallyUnfixFrame = finally([&frame, this] { bufferManager.unfixPage(frame, false); });
    uint8_t* data = frame.getData();
    uint32_t offset = 0;

    std::vector<RelationSchema> schema;

    uint64_t size;
    while((size = *reinterpret_cast<uint64_t*>(data + offset)) > 0) {
      offset += sizeof(uint64_t);
      if(offset + size > BufferManager::pageSize) {
        throw std::runtime_error("invalid data stored in schema segment");
      }
      Record serialized(size, data + offset);
      schema.push_back(RelationSchema(serialized));
      offset += size;
    }

    return schema;
  }

}
