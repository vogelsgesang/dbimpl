#include "RelationSchema.h"
#include <string>
#include <sstream>
#include <cstring>

namespace dbImpl {


  static std::string serializeType(const Types::Tag type) {
    switch(type) {
      case Types::Tag::Integer:
         return "Integer";
     
      case Types::Tag::Char: {
         return "Char";
      }
   }
   std::cerr "could not serialize Type" << std::endl;
   return "";
}

  static Types::Tag loadType(std::string typeInfo) {
    if(typeInfo.compare("Integer") == 0)
      return Types::Tag::Integer;
	if(typeInfo.compare("Char") == 0)
		return Types::Tag::Char;
	std::cerr << "Type could not be loaded: " << typeInfo;
	return nullptr;
}

  static RelationSchema RelationSchema::loadFromRecord(Record& record) {

    char* data = record.getData();
    if (data == NULL) {
      std::cerr << "Read data of relation schema is null" << std::endl;
      throw;

    }

    std::stringstream ss(data);
    

   
    //Parse first line (name + Attribute Size + PrimaryKey Size)
 
    std::string name << ss;
    uint64_t size << ss;
    uint64_t segmentID << ss;
    uint64_t attSize << ss;
    uint64_t keySize << ss;
    
    
    //Parse Attributes
    std::vector<AttributeDescriptor> attributes;
    attributes.reserve(attSize);
    for(int i = 0; i < attSize; i++){
      AttributeDescriptor a = new AttributeDescriptor;
      a.name << ss;
      temp << ss;
	  loadType(temp);
      a.len << ss;
      temp << ss;
      if(temp.compare("1") == 0){
        a.notNull = true;
      }
      attributes.push_back(a);
      
    }
    
    //Parse Primary Key
    std::vector<unsigned> primaryKey;
    primaryKey.reserve(keySize);
    for(int i = 0; i < keySize; i++){
      unsigned k << ss;
      primaryKey.push_back(k);
    }
    

    
    //TODO Generate RelationSchema
    RelationSchema schema = new RelationSchema(name);
    //schema.attributes= attributes;
    //schema.primaryKey = primaryKey;
    //schema.size = size;
    //schema.segmentID = segmentID;
    
    return schema;

    


  }

  Record RelationSchema::serializeToRecord() {


    std::stringstream recData;
    recData << name << " " << size << " " << segmentID << " " << attributes.size() << " " << primaryKey.size() << std::endl;
    //Attributes
    for (const AttributeDescriptor& a : attributes) {
      recData << a.name << " " << serializeType(a.type) << " " << a.len << " " << (a.notNull ? "1" : "0") << std::endl;
    }
    for (unsigned key : primaryKey) {
      recData << key << " ";

    }
    recData << std::endl;

    recData.seekg(0, std::ios::end);
    unsigned len = recData.tellg();
    // create new Record
    //TODO convert recData to const char*
    char* data = new char[1];
    return new Record(len, data);
    


  }







} //namespace dbimpl
