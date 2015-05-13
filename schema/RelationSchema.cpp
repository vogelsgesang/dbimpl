#include "RelationSchema.h"

#include <sstream>

namespace dbimpl{
    
    static RelationSchema RelationSchema::loadFromRecord(Record& record){
      
      //TODO
      
      
      
    }
    
    Record RelationSchema::serializeToRecord(){
      
      /*
       std::string name;
    std::vector<AttributeDescriptor> attributes;
    std::vector<unsigned> primaryKey;*/
      
      std::stringstream recData;
      recData << name << ";" << attributes.size() << std::endl;
      //Attributes
      for(const AttributeDescriptor& a : attributes){
        recData << a.name << ";" << a.type << ";" << a.len << ";" << a.notNull << std::endl;
                
     std::string name;
     Types::Tag type;
     unsigned len;
     bool notNull;
      }
      
      
    }
    
   
    
    
    
    
    
} //namespace dbimpl
