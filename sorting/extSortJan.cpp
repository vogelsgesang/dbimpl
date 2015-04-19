#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

using namespace std;

int main() {

cout << "Test";


}

struct prioQueueElem {
std::vector<uint64_t>::iterator ptr;
uint64_t chunkNumber;
};

class Compare {
    public:
    bool operator()(prioQueueElem& p1, prioQueueElem& p2) 
    {
       return *p1.ptr > *p2.ptr;
    }
};


bool comparePrioQueueElem (prioQueueElem a, prioQueueElem b) {
return *a.ptr > *b.ptr;
}

void externalSort(int fdInput, uint64_t size, int fdOutput, uint64_t memSize) {
  uint64_t chunkSize = memSize;
  uint64_t runs = 1+ ((size * sizeof(uint64_t) - 1)/memSize);

  vector<uint64_t> chunkInMem;
  uint64_t totalNumbersRead = 0;

  vector<int> chunkFiles;
  vector<uint64_t> elementsInChunk;
  
  for(uint64_t i = 0; i < runs; i++) {
    uint64_t numbersToRead = min(chunkSize, size-totalNumbersRead);
    chunkInMem.resize(numbersToRead);
    elementsInChunk.push_back(numbersToRead);
    read(fdInput, &chunkInMem[0], numbersToRead); //TODO Check if file was read (properly)
    totalNumbersRead += numbersToRead;

    //Sort chunk in-memory
    sort(chunkInMem.begin(), chunkInMem.end());

    // Write on disc
    string fileName = "tempChunk";
    fileName += i;
    int chunkFile = open(fileName.c_str(), O_WRONLY); //TODO Check if file was opened properly
    chunkFiles.push_back(chunkFile);
    write(chunkFile, chunkInMem.data(), chunkInMem.size() * sizeof(uint64_t));
    close(chunkFile);


    // Clear Memory
    chunkInMem.clear();

  }

// -------------------- Merge Phase -------------------------

  // Determine size of "memory parts"
    // 1 part for output buffer and n-1 parts for (part-)Chunks

  uint64_t memPartSize = memSize / (runs + 1); 
  vector<vector<uint64_t> >partChunks;
  vector<uint64_t> outputBuf;

  // Read chunks

  for(uint64_t i = 0; i < runs; i++) {
    string fileName = "tempChunk";
    fileName += i;
    chunkFiles[i] = open(fileName.c_str(), O_RDONLY);
    uint64_t numbersToRead = min(memPartSize, elementsInChunk[i]);
    elementsInChunk[i] -= numbersToRead;
    vector<uint64_t> inChunk;
    inChunk.resize(numbersToRead);
    read(chunkFiles[i], &inChunk[0], numbersToRead * sizeof(uint64_t));

    partChunks.push_back(inChunk);



  }

// Build Priority Queue

std::priority_queue<prioQueueElem, std::vector<prioQueueElem>, Compare> prioQueue;

for(uint64_t i = 0; i < runs; i++) {
	prioQueue.push({partChunks[i].begin(),i});
}


while(!prioQueue.empty()) {
prioQueueElem e = prioQueue.top();
prioQueue.pop();


outputBuf.push_back(*e.ptr);
if(outputBuf.size() == memPartSize) {
	write(fdOutput, outputBuf.data(), outputBuf.size() * sizeof(uint64_t));
	outputBuf.clear();
  
}
e.ptr++;

if(partChunks[e.chunkNumber].end() == e.ptr) {
  partChunks[e.chunkNumber].clear();
  uint64_t numbersToRead = min(memPartSize, elementsInChunk[e.chunkNumber]);
  elementsInChunk[e.chunkNumber] -= numbersToRead;
  read(chunkFiles[e.chunkNumber], &partChunks[e.chunkNumber][0], numbersToRead * sizeof(uint64_t));
  e.ptr = partChunks[e.chunkNumber].begin();

}

if(partChunks[e.chunkNumber].size() > 0) {
  prioQueue.push(e);
} else {
  close(chunkFiles[e.chunkNumber]);
  string fileName = "tempChunk";
  fileName += e.chunkNumber;
  remove(fileName.c_str());

}

}

write(fdOutput, outputBuf.data(), outputBuf.size() * sizeof(uint64_t));
cout << "Finished external sorting";








}
