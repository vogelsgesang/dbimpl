#include <iostream>
#include <vector>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>


bool isSorted(uint64_t size, int fdTest, uint64_t memSize) {
  uint64_t chunkSize = memSize;


  std::vector<uint64_t> inMemory;
  uint64_t totalNumbersRead = 0;
  
  uint64_t controlNum = 0;

    
while(totalNumbersRead < size)
{
    uint64_t numbersToRead = std::min(chunkSize, size-totalNumbersRead);
    inMemory.resize(numbersToRead);
    read(fdTest, &inMemory[0], numbersToRead); //TODO Check if file was read (properly)
    totalNumbersRead += numbersToRead;
    
    //Check data
    for(uint64_t i = 0; i < inMemory.size(); i++) {
      if(controlNum > inMemory[i]) {
        std::cout << "Error in sorting";
        return false;
      }
      controlNum = inMemory[i];

    } 
    
    


 }

std::cout << "Finished checking: File was sorted successfully";
return true;


}
