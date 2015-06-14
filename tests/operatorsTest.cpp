#include <gtest/gtest.h>
#include <vector>
#include <iostream>
#include <sstream>

#include "operators/register.h"
#include "operators/relation.h"
#include "schema/relationSchema.h"
#include "operators/tableScan.h"
#include "operators/projection.h"
#include "operators/selection.h"
#include "operators/hashJoin.h"
#include "operators/print.h"

using namespace dbImpl;

Relation getTestRelation();
Relation getTestRelation2();
Relation getStudentenRelation();
Relation getPunkteRelation();

TEST(Operators, Register) {
  Register s1("hallo");
  Register s2("welt");
  EXPECT_NE(s1, s2);

  s2.setString("hallo");
  EXPECT_EQ(s1, s2);
  EXPECT_EQ(s1.getString(), "hallo");
  EXPECT_EQ(s1, s2);

  Register i1(5);
  Register i2(10);
  EXPECT_EQ(i1.getInteger(), 5);
  EXPECT_EQ(i2.getInteger(), 10);
  EXPECT_TRUE(i1 < i2);
}

TEST(Operators, Relation) {
  Relation r = getTestRelation();

  EXPECT_EQ(3, r.getNumTuples());

  std::vector<Register> t = r.get(0);
  EXPECT_EQ(Register("Alf"), t[0]);
  EXPECT_EQ(Register(50), t[1]);
  t = r.get(1);
  EXPECT_EQ(Register("Bert"), t[0]);
  EXPECT_EQ(Register(20), t[1]);

  EXPECT_ANY_THROW(r.get(3));
  EXPECT_ANY_THROW(r.addAttribute("Another Attribute", TypeTag::Integer));
}

TEST(Operators, ScanTable) {
  Relation r = getTestRelation();

  TableScanOperator tScan(r);
  tScan.open();
  tScan.next();
  Register* r2 = tScan.getOutput()[0];
  EXPECT_EQ("Alf", r2->getString());
  r2 = tScan.getOutput()[1];
  EXPECT_EQ(50, r2->getInteger());

  tScan.next();
  r2 = tScan.getOutput()[0];
  EXPECT_EQ("Bert", r2->getString());
  r2 = tScan.getOutput()[1];
  EXPECT_EQ(20, r2->getInteger());

  tScan.next();
  r2 = tScan.getOutput()[0];
  EXPECT_EQ("Carl", r2->getString());
  r2 = tScan.getOutput()[1];
  EXPECT_EQ(33, r2->getInteger());

  ASSERT_FALSE(tScan.next());
}

TEST(Operators, PrintTable) {
  Relation r = getTestRelation();
  Operator* tScan = new TableScanOperator(r);
  std::stringstream ss;
  PrintOperator tPrint(tScan, ss);
  tPrint.open();
  while (tPrint.next()) {}

  std::stringstream cmpStream;
  cmpStream << "Alf" << " " << 50 << std::endl;
  cmpStream << "Bert" << " " << 20 << std::endl;
  cmpStream << "Carl" << " " << 33 << std::endl;
  EXPECT_EQ(cmpStream.str(), ss.str());
}

TEST(Operators, ProjectionOperator) {
  Relation r = getTestRelation();
  Operator* tScan = new TableScanOperator(r);

  //Project to Age
  Operator* tProject = new ProjectionOperator(tScan, { 1 });
  std::stringstream ss;
  PrintOperator tPrint(tProject, ss);
  tPrint.open();
  while (tPrint.next())
    ;
  std::stringstream cmpStream;
  cmpStream << 50 << std::endl << 20 << std::endl << 33 << std::endl;

  EXPECT_EQ(cmpStream.str(), ss.str());
}

TEST(Operators, SelectionOperator) {
  Relation r = getTestRelation2();
  Operator* tScan = new TableScanOperator(r);

  //Select Tuples where age = 20
  Operator* tSelect = new SelectionOperator(tScan, 1, Register(20));
  std::stringstream ss;
  PrintOperator tPrint(tSelect, ss);
  tPrint.open();
  while (tPrint.next())
    ;
  std::stringstream cmpStream;
  cmpStream << "Bert" << " " << 20 << std::endl;
  cmpStream << "Berts Twin" << " " << 20 << std::endl;

  EXPECT_EQ(cmpStream.str(), ss.str());
}

TEST(Operators, HashJoinOSelfJoin) {
  Relation r = getTestRelation();
  Operator* tScan = new TableScanOperator(r);
  Operator* tScan2 = new TableScanOperator(r);

  Operator* tHash = new HashJoinOperator(tScan, tScan2, 1, 1);
  std::stringstream ss;
  PrintOperator tPrint(tHash, ss);
  tPrint.open();
  while (tPrint.next())
    ;

  std::stringstream cmpStream;
  cmpStream << "Alf" << " " << 50 << " " << "Alf" << " " << 50 << std::endl;
  cmpStream << "Bert" << " " << 20 << " " << "Bert" << " " << 20 << std::endl;
  cmpStream << "Carl" << " " << 33 << " " << "Carl" << " " << 33 << std::endl;

  EXPECT_EQ(cmpStream.str(), ss.str());
}

TEST(Operators, JoinStudentenHoeren) {
  Relation studenten = getStudentenRelation();
  Relation punkte = getPunkteRelation();
  Operator* tScan = new TableScanOperator(studenten);
  Operator* tScan2 = new TableScanOperator(punkte);

  Operator* tHash = new HashJoinOperator(tScan, tScan2, 0, 0);
  Operator* tProject = new ProjectionOperator(tHash, { 0, 1, 3 });
  std::stringstream ss;
  PrintOperator tPrint(tProject, ss);
  tPrint.open();
  while (tPrint.next())
    ;

  std::stringstream cmpStream;
  cmpStream << 1 << " " << "Alf" << " " << 50 << std::endl;
  cmpStream << 2 << " " << "Bert" << " " << 90 << std::endl;
  cmpStream << 3 << " " << "Carl" << " " << 3 << std::endl;
  cmpStream << 4 << " " << "Dieter" << " " << 90 << std::endl;

  EXPECT_EQ(cmpStream.str(), ss.str());
}

Relation getTestRelation() {
  Relation r;
  r.addAttribute("Name", TypeTag::Char);
  r.addAttribute("Age", TypeTag::Integer);

  r.insert( { Register("Alf"), (Register(50)) });
  r.insert( { Register("Bert"), (Register(20)) });
  r.insert( { Register("Carl"), (Register(33)) });
  return r;
}

Relation getTestRelation2() {
  Relation r;
  r.addAttribute("Name", TypeTag::Char);
  r.addAttribute("Age", TypeTag::Integer);

  r.insert( { Register("Alf"), (Register(50)) });
  r.insert( { Register("Bert"), (Register(20)) });
  r.insert( { Register("Carl"), (Register(33)) });
  r.insert( { Register("Berts Twin"), (Register(20)) });
  return r;
}

Relation getStudentenRelation() {
  Relation r;
  r.addAttribute("MatrNr", TypeTag::Integer);
  r.addAttribute("Name", TypeTag::Char);

  r.insert( { Register(1), (Register("Alf")) });
  r.insert( { Register(2), (Register("Bert")) });
  r.insert( { Register(3), (Register("Carl")) });
  r.insert( { Register(4), (Register("Dieter")) });
  return r;
}

Relation getPunkteRelation() {
  Relation r;
  r.addAttribute("MatrNr", TypeTag::Integer);
  r.addAttribute("Punkte", TypeTag::Integer);

  r.insert( { Register(1), (Register(50)) });
  r.insert( { Register(2), (Register(90)) });
  r.insert( { Register(3), (Register(3)) });
  r.insert( { Register(4), (Register(90)) });
  return r;
}
