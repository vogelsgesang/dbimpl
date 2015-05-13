#include "RelationSchema.h"
#include <string>
#include <sstream>

namespace dbimpl {

  static RelationSchema RelationSchema::loadFromRecord(Record& record) {

    char* data = record.getData();
    if (data == NULL) {
      std::cerr << "Read data of relation schema is null" << std::endl;
      return NULL;

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
      a.type << ss;
      a.len << ss;
      std::string temp << ss;
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
    

    
    
    RelationSchema schema = new RelationSchema(name);
    schema.attributes= attributes;
    schema.primaryKey = primaryKey;
    schema.size = size;
    schema.segmentID = segmentID;

    


  }

  Record RelationSchema::serializeToRecord() {


    std::stringstream recData;
    recData << name << " " << size << " " << segmentID << " " << attributes.size() << " " << primaryKey.size() << std::endl;
    //Attributes
    for (const AttributeDescriptor& a : attributes) {
      recData << a.name << " " << a.type << " " << a.len << " " << (a.notNull ? "1" : "0") << std::endl;
    }
    for (unsigned key : primaryKey) {
      recData << key << " ";

    }
    recData << std::endl;

    recData.seekg(0, ios::end);
    int len = recData.tellg();
    // create new Record
    return new Record(len, recData);


  }







} //namespace dbimpl
