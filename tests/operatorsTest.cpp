#include <gtest/gtest.h>
#include <vector>
#include <iostream>
#include <sstream>

#include "operators/register.h"
#include "operators/relation.h"
#include "schema/relationSchema.h"
#include "operators/tableScan.h"
#include "operators/print.h"

using namespace dbImpl;

Relation getTestRelation();

TEST(Operators, Register) {
  Register s1("hallo");
  Register s2("welt");
  ASSERT_FALSE(s1 == s2);

  s2.setString("hallo");
  EXPECT_EQ(s1, s2);
  EXPECT_EQ(s1.getString(), "hallo");
  ASSERT_TRUE(s1 == s2);

  Register i1(5);
  Register i2(10);
  ASSERT_TRUE(i1 < i2);
  EXPECT_EQ(i1.getInteger(), 5);
  EXPECT_EQ(i2.getInteger(), 10);

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
  while (tScan.next())
    ;
  Register* r2 = tScan.getOutput()[0];
  EXPECT_EQ("Alf", r2->getString());
  r2 = tScan.getOutput()[1];
  EXPECT_EQ(50, r2->getInteger());

  r2 = tScan.getOutput()[2];
  EXPECT_EQ("Bert", r2->getString());
  r2 = tScan.getOutput()[3];
  EXPECT_EQ(20, r2->getInteger());

  r2 = tScan.getOutput()[4];
  EXPECT_EQ("Carl", r2->getString());
  r2 = tScan.getOutput()[5];
  EXPECT_EQ(33, r2->getInteger());
}

TEST(Operators, PrintTable) {
  Relation r = getTestRelation();
  Operator* tScan = new TableScanOperator(r);
  std::stringstream ss;
  PrintOperator tPrint(tScan, ss);
  tPrint.open();
  while (tPrint.next())
    ;

  std::stringstream cmpStream;
  cmpStream << "Alf" << " " << 50 << std::endl << "Bert" << " " << 20
      << std::endl << "Carl" << " " << 33 << std::endl;
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
