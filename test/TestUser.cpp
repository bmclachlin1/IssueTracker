#include <string>

#include "User.h"
#include "gtest/gtest.h"

using json = nlohmann::json;

TEST(TestUser, Constructor) {
  User user;
  // Need mocking to test ID here
  EXPECT_EQ(std::string(""), user.name);
  EXPECT_EQ(std::string("Developer"), user.role);

  user.name = "John";
  EXPECT_EQ(std::string("John"), user.name);
  user.role = "Admin";
  EXPECT_EQ(std::string("Admin"), user.role);
  user.id = "00001";
  EXPECT_EQ(std::string("00001"), user.id);
}

TEST(TestUser, Serializer) {
  std::string jsonText =
      "{ \"id\":\"0123456789\", \"name\":\"Jeffrey\", \"role\":\"Admin\" }";
  User user = json::parse(jsonText).get<User>();
  EXPECT_EQ("0123456789", user.id);
  EXPECT_EQ("Jeffrey", user.name);
  EXPECT_EQ("Admin", user.role);
}

TEST(TestUser, Deserializer) {
  User user;
  user.id = "abcdefgh23";
  user.name = "BoDiddly";
  json j = user;
  std::string jsonText = j.dump();
  std::string expected_val =
      "{\"id\":\"abcdefgh23\",\"name\":\"BoDiddly\",\"role\":\"Developer\"}";
  EXPECT_EQ(expected_val, jsonText);
}
