#include <gtest/gtest.h>
#include <vector>
#include <sstream>

#include "operators/inMemoryScan.h"
#include "operators/tupleCollector.h"
#include "operators/relation.h"
#include "schema/relationSchema.h"
#include "operators/projection.h"
#include "operators/selection.h"
#include "operators/hashJoin.h"
#include "operators/print.h"

using namespace dbImpl;

typedef std::vector<std::vector<Register>> Table;

const Table studentsTable{
  //MatrNr      , Name                    , Age
  { Register(1) , Register("Alf")         , Register(50) } ,
  { Register(2) , Register("Bert")        , Register(20) } ,
  { Register(3) , Register("Bert's twin") , Register(20) } ,
  { Register(4) , Register("Carl")        , Register(33) } ,
};

const Table pointsTable{
  //MatrNr     , Points
  { Register(1), Register(50) },
  { Register(3), Register( 3) },
  { Register(4), Register(90) }
};

TEST(InMemoryScanOperator, scansATable) {
  InMemoryScanOperator scan(studentsTable);
  scan.open();

  ASSERT_TRUE(scan.next());
  EXPECT_EQ(3     , scan.getOutput().size());
  EXPECT_EQ(1     , scan.getOutput()[0]->getInteger());
  EXPECT_EQ("Alf" , scan.getOutput()[1]->getString());
  EXPECT_EQ(50    , scan.getOutput()[2]->getInteger());

  ASSERT_TRUE(scan.next());
  EXPECT_EQ(3      , scan.getOutput().size());
  EXPECT_EQ(2      , scan.getOutput()[0]->getInteger());
  EXPECT_EQ("Bert" , scan.getOutput()[1]->getString());
  EXPECT_EQ(20     , scan.getOutput()[2]->getInteger());

  ASSERT_TRUE(scan.next());
  EXPECT_EQ(3             , scan.getOutput().size());
  EXPECT_EQ(3             , scan.getOutput()[0]->getInteger());
  EXPECT_EQ("Bert's twin" , scan.getOutput()[1]->getString());
  EXPECT_EQ(20            , scan.getOutput()[2]->getInteger());

  ASSERT_TRUE(scan.next());
  EXPECT_EQ(3      , scan.getOutput().size());
  EXPECT_EQ(4      , scan.getOutput()[0]->getInteger());
  EXPECT_EQ("Carl" , scan.getOutput()[1]->getString());
  EXPECT_EQ(33     , scan.getOutput()[2]->getInteger());

  ASSERT_FALSE(scan.next());
}

TEST(TupleCollector, collectsAllTuples) {
  InMemoryScanOperator scan(studentsTable);
  TupleCollector collector(&scan);
  Table collectedTable = collector.collect();
  EXPECT_EQ(studentsTable, collectedTable);
}

TEST(PrintTableOperator, printsTheTable) {
  InMemoryScanOperator scan(studentsTable);
  std::stringstream ss;
  PrintOperator tPrint(&scan, ss);
  tPrint.open();
  while (tPrint.next()) {}

  std::stringstream cmpStream;
  cmpStream << 1 << " | " << "Alf"         << " | " << 50 << std::endl;
  cmpStream << 2 << " | " << "Bert"        << " | " << 20 << std::endl;
  cmpStream << 3 << " | " << "Bert's twin" << " | " << 20 << std::endl;
  cmpStream << 4 << " | " << "Carl"        << " | " << 33 << std::endl;
  EXPECT_EQ(cmpStream.str(), ss.str());
}

TEST(ProjectionOperator, projectsOnColumns) {
  InMemoryScanOperator scan(studentsTable);
  ProjectionOperator tProject(&scan, { 2 }); //project on third column
  TupleCollector collector(&tProject);

  Table collectedTable = collector.collect();
  Table expectedResult = {
    { Register(50) },
    { Register(20) },
    { Register(20) },
    { Register(33) },
  };
  EXPECT_EQ(expectedResult, collectedTable);
}

TEST(ProjectionOperator, duplicatesColumns) {
  InMemoryScanOperator scan(studentsTable);
  ProjectionOperator tProject(&scan, { 2, 2 }); //duplicate third column
  TupleCollector collector(&tProject);

  Table collectedTable = collector.collect();
  Table expectedResult = {
    { Register(50), Register(50) },
    { Register(20), Register(20) },
    { Register(20), Register(20) },
    { Register(33), Register(33) },
  };
  EXPECT_EQ(expectedResult, collectedTable);
}

TEST(ProjectionOperator, reorderColumns) {
  InMemoryScanOperator scan(pointsTable);
  ProjectionOperator tProject(&scan, { 1, 0 }); //swap first and second column
  TupleCollector collector(&tProject);

  Table collectedTable = collector.collect();
  Table expectedResult = {
    { Register(50) , Register(1) } ,
    { Register( 3) , Register(3) } ,
    { Register(90) , Register(4) }
  };
  EXPECT_EQ(expectedResult, collectedTable);
}

TEST(SelectionOperator, selectsTheCorrectTuples) {
  InMemoryScanOperator scan(studentsTable);
  //Select Tuples where age = 20
  SelectionOperator tSelect(&scan, 2, Register(20));
  TupleCollector collector(&tSelect);

  Table collectedTable = collector.collect();
  Table expectedResult = {
    { Register(2) , Register("Bert")        , Register(20) },
    { Register(3) , Register("Bert's twin") , Register(20) },
  };
  EXPECT_EQ(expectedResult, collectedTable);
}

TEST(HashJoinOperator, joinsStudentsWithPoints) {
  InMemoryScanOperator scan1(studentsTable);
  InMemoryScanOperator scan2(pointsTable);
  HashJoinOperator tHash(&scan1, &scan2, 0, 0);
  ProjectionOperator tProject (&tHash, { 0, 1, 4 }); //eliminate age and duplicated MatrNr
  TupleCollector collector(&tProject);

  Table collectedTable = collector.collect();
  Table expectedResult = {
    { Register(1) , Register("Alf")         , Register(50)},
    { Register(3) , Register("Bert's twin") , Register( 3)},
    { Register(4) , Register("Carl")        , Register(90)}
  };
  EXPECT_EQ(expectedResult, collectedTable);
}

TEST(HashJoinOperator, supportsSelfJoins) {
  InMemoryScanOperator scan1(studentsTable);
  InMemoryScanOperator scan2(studentsTable);
  HashJoinOperator tHash(&scan1, &scan2, 2, 2);
  TupleCollector collector(&tHash);

  Table collectedTable = collector.collect();
  Table expectedResult = {
    { Register(1) , Register("Alf")         , Register(50) , Register(1) , Register("Alf")         , Register(50) },
    { Register(2) , Register("Bert")        , Register(20) , Register(2) , Register("Bert")        , Register(20) },
    { Register(3) , Register("Bert's twin") , Register(20) , Register(2) , Register("Bert")        , Register(20) },
    { Register(2) , Register("Bert")        , Register(20) , Register(3) , Register("Bert's twin") , Register(20) },
    { Register(3) , Register("Bert's twin") , Register(20) , Register(3) , Register("Bert's twin") , Register(20) },
    { Register(4) , Register("Carl")        , Register(33) , Register(4) , Register("Carl")        , Register(33) },
  };
  EXPECT_EQ(expectedResult, collectedTable);
}
