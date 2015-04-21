#include <gtest/gtest.h>
#include <cstring>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <memory>
#include <vector>
#include <algorithm>
#include <stdlib.h>

#include "sorting/externalSort.h"

void testSorting(std::vector<uint64_t> values, uint64_t memSize) {
  char inFileName[] = "unsortedXXXXXX";
  char outFileName[] = "sortedXXXXXX";
  //create temporary input and output files
  int fdIn = mkstemp(inFileName);
  ASSERT_GE(fdIn, 0) << "unable to create input file: " << strerror(errno);
  int fdOut = mkstemp(outFileName);
  ASSERT_GE(fdOut, 0) << "unable to create output file: " << strerror(errno);

  //store the input numbers into the input file
  uint64_t bytesWritten = 0;
  uint64_t bytesToBeWritten = values.size()*sizeof(uint64_t);
  while(bytesWritten < bytesToBeWritten) {
    int pwriteRet = pwrite(fdIn, &values[0], bytesToBeWritten - bytesWritten, bytesWritten);
    ASSERT_GE(pwriteRet, 0) << "unable to write values to input file: " << strerror(errno);
    bytesWritten += pwriteRet;
  }

  //sort the values
  dbImpl::externalSort(fdIn, values.size(), fdOut, memSize);

  //check the file size of the output file
  struct stat outStat;
  ASSERT_EQ(0, fstat(fdOut, &outStat)) << strerror(errno);
  EXPECT_EQ(values.size()*sizeof(uint64_t), outStat.st_size);

  //read back the results
  std::unique_ptr<uint64_t[]> results(new uint64_t[values.size()]);
  uint64_t bytesRead = 0;
  uint64_t bytesToBeRead = values.size()*sizeof(uint64_t);
  while(bytesRead < bytesToBeRead) {
    int preadRet = pread(fdOut, &results[0], bytesToBeRead - bytesRead, bytesRead);
    ASSERT_GE(preadRet, 0) << "unable to read values from output file: " << strerror(errno);
    bytesRead += preadRet;
  }
  //they should be sorted now...
  ASSERT_TRUE(std::is_sorted(&results[0], &results[values.size()]));

  //close both file descriptors
  //since they were created using O_TMPFILE, the files will be
  //deleted automatically
  ASSERT_EQ(0, close(fdOut)) << strerror(errno);
  ASSERT_EQ(0, close(fdIn)) << strerror(errno);
  //unlink the files
  ASSERT_EQ(0, unlink(inFileName)) << strerror(errno);
  ASSERT_EQ(0, unlink(outFileName)) << strerror(errno);
}

TEST(ExternalSortTest, handlesEmptyInput) {
  testSorting({}, 1<<20); //no values at all
}

TEST(ExternalSortTest, handlesOneValue) {
  testSorting({42}, 1<<20); //only one elements
}

TEST(ExternalSortTest, sortsValues) {
  testSorting({1, 2, 3, 12, 33}, 1<<20); //already sorted
  testSorting({10, 8, 23, 12, 43}, 1<<20); //unsorted
}

TEST(ExternalSortTest, keepsDuplicate) {
  testSorting({1, 1, 3, 3}, 1<<20); //already sorted
  testSorting({10, 8, 10, 8, 7}, 1<<20); //unsorted
}

TEST(DISABLED_ExternalSortTest, sortValuesExternally) {
  //TODO: implement
}

TEST(DISABLED_ExternalSortTest, sorts5GB) {
  //TODO: implement
}

TEST(DISABLED_ExternalSortTest, sorts20GB) {
  //TODO: implement
}
