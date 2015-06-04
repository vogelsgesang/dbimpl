#include <gtest/gtest.h>
#include <vector>

#include "operators/register.h"
#include "operators/relation.h"
#include <schema/relationSchema.h>


using namespace dbImpl;

TEST(Operators, Register) {
  Register s1("hallo");
  Register s2("welt");
  ASSERT_FALSE(s1 == s2);

  s2.setString("hallo");
  EXPECT_EQ(s1,s2);
  EXPECT_EQ(s1.getString(),"hallo");
  ASSERT_TRUE(s1 == s2);

  Register i1(5);
  Register i2(10);
  ASSERT_TRUE(i1 < i2);
  EXPECT_EQ(i1.getInteger(),5);
  EXPECT_EQ(i2.getInteger(),10);


}

TEST(Operators, Relation) {
  Relation r1;
  r1.addAttribute("Name",TypeTag::Char);
  r1.addAttribute("Age",TypeTag::Integer);

  r1.insert({Register("Alf"),(Register(50))});
  r1.insert({Register("Bert"),(Register(20))});
  r1.insert({Register("Carl"),(Register(33))});

  EXPECT_EQ(3,r1.getNumTuples());

  std::vector<Register> t = r1.get(0);
  EXPECT_EQ(Register("Alf"),t[0]);
  EXPECT_EQ(Register(50),t[1]);
  t = r1.get(1);
  EXPECT_EQ(Register("Bert"),t[0]);
  EXPECT_EQ(Register(20),t[1]);

  EXPECT_ANY_THROW(r1.addAttribute("Another Attribute",TypeTag::Integer));

}
