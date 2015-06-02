#include <iostream>
#include <vector>
#include <string>
#include <tuple>
#include <fstream>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/format.hpp>

#include "buffer/bufferManager.h"
#include "bTree/bTree.h"

//applies a command to the tree
bool executeCommand(const std::string& command, dbImpl::BTree<int>* tree);

int main(int argc, const char* []) {
  if(argc > 1) {
    std::cerr << "This command does not take arguments" << std::endl;
    return 1;
  }

  dbImpl::BufferManager bm(100);
  dbImpl::BTree<int> tree(bm, std::less<int>(), 6); //for testing, use a maximum node size of 6

  bool running = true;
  while(running && !std::cin.eof()) {
    std::string command;
    std::getline(std::cin, command);
    if(command == "q") {
      running = false;
    } else if(!executeCommand(command, &tree)) {
      std::cout << "ERROR: unknown command" << std::endl;
    }
  }
  return 0;
}

std::vector<std::string> tokenize(std::string line);
boost::optional<std::tuple<int, int, int> > parseRange(std::vector<std::string> tokens);

bool executeCommand(const std::string& command, dbImpl::BTree<int>* tree) {
  //ignore comments && empty lines
  if(command.empty() || command.substr(0,1) == "#") return true;
  //tokenize it
  std::vector<std::string> tokens = tokenize(command);
  std::string cmdName = tokens[0];
  if(cmdName == "i") {
    //parse parameters
    auto range = parseRange(tokens);
    if(!range) {
      std::cout << "unable to interpret the range of keys to be inserted" << std::endl
                << "usage: i <first> [<last> [<stepSize>]]" << std::endl;
    } else {
      int first    = std::get<0>(*range);
      int last     = std::get<1>(*range);
      int stepSize = std::get<2>(*range);
      //insert all the values
      int insertedCnt = 0;
      for(int i = first; i <= last; i+=stepSize) {
        if(tree->insert(i, i*i)) {
          insertedCnt++;
        } else {
          std::cout << "failed insertion: " << i << std::endl;
        }
      }
      std::cout << "inserted " << insertedCnt << " entries" << std::endl;
    }
    return true;
  } else if(cmdName == "l") {
    //parse parameters
    auto range = parseRange(tokens);
    if(!range) {
      std::cout << "unable to interpret the range of keys to be looked up" << std::endl
                << "usage: l <first> [<last> [<stepSize>]]" << std::endl;
    } else {
      int first    = std::get<0>(*range);
      int last     = std::get<1>(*range);
      int stepSize = std::get<2>(*range);
      //delete all the values
      for(int i = first; i <= last; i+=stepSize) {
        auto result = tree->lookup(i);
        if(result) {
          std::cout << i << ": " << *result << std::endl;
        } else {
          std::cout << i << ": unable to lookup" << std::endl;
        }
      }
    }
    return true;
  } else if(cmdName == "d") {
    //parse parameters
    auto range = parseRange(tokens);
    if(!range) {
      std::cout << "unable to interpret the range of keys to be deleted" << std::endl
                << "usage: d <first> [<last> [<stepSize>]]" << std::endl;
    } else {
      int first    = std::get<0>(*range);
      int last     = std::get<1>(*range);
      int stepSize = std::get<2>(*range);
      //delete all the values
      int deletedCnt = 0;
      for(int i = first; i <= last; i+=stepSize) {
        if(tree->erase(i)) {
          deletedCnt++;
        } else {
          std::cout << "failed deletion: " << i << std::endl;
        }
      }
      std::cout << "deleted " << deletedCnt << " entries" << std::endl;
    }
    return true;
  } else if(cmdName == "v") {
    std::string fileName;
    if(tokens.size() == 1) {
      static int fileNr = 0;
      fileName = (boost::format("%1%.dot") % fileNr).str();
      fileNr++;
    } else if(tokens.size() == 2) {
      fileName = tokens[1];
      if(fileName.substr(fileName.length() - 4) != ".dot") {
        fileName += ".dot";
      }
    } else {
      std::cout << "too many arguments" << std::endl
                << "usage: v [<filename>]" << std::endl;
    }
    std::ofstream outFile(fileName);
    tree->exportAsDot(outFile);
    std::cout << "saved dot file to " << fileName << std::endl;
    return true;
  } else {
    return false;
  }
}

boost::optional<std::tuple<int, int, int>> parseRange(std::vector<std::string> tokens) {
  int first, last;
  int stepSize = 1;
  //interpret all the different possibilities to specify a range
  try {
    if(tokens.size() == 2) {
      first = last = boost::lexical_cast<int>(tokens[1]);
      stepSize = 1;
    } else if(tokens.size() == 3) {
      first = boost::lexical_cast<int>(tokens[1]);
      last = boost::lexical_cast<int>(tokens[2]);
    } else if(tokens.size() == 4) {
      first = boost::lexical_cast<int>(tokens[1]);
      last = boost::lexical_cast<int>(tokens[2]);
      stepSize = boost::lexical_cast<int>(tokens[3]);
    } else {
      return boost::optional<std::tuple<int, int, int>>();
    }
    //adjust stepSize's sign
    stepSize = (first <= last ? 1 : -1) * std::abs(stepSize);
    return std::make_tuple(first, last, stepSize);
  } catch(boost::bad_lexical_cast& e) {
    return boost::optional<std::tuple<int, int, int>>();
  }
}

std::vector<std::string> tokenize(std::string line) {
  std::vector<std::string> tokens;
  size_t currSepPos = line.find(' ');
  while (currSepPos != std::string::npos) {
    tokens.push_back(line.substr(0, currSepPos));
    line = line.substr(currSepPos + 1);
    currSepPos = line.find(' ');
  }
  tokens.push_back(line);
  return tokens;
}
