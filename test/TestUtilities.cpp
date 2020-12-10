#include <string>
#include <utility>

#include "Utilities.h"
#include "gtest/gtest.h"

TEST(TestUtilities, TestBuildHeaders) {
  // Some fake headers
  StringMap fakeHeader = {{"SomeKey", "SomeValue"},
                          {"SomeOtherKey", "SomeOtherValue"}};

  StringMap headers = ResponseUtilities::BuildResponseHeader(fakeHeader);

  // Check to see if the fake headers are in the headers
  EXPECT_NE(headers.find("SomeKey"), headers.end());
  EXPECT_NE(headers.find("SomeOtherKey"), headers.end());

  // Create a pair from the ALLOW_ALL and CLOSE_CONNECTION definitions
  std::pair<std::string, std::string> allowAll = ALLOW_ALL;
  std::pair<std::string, std::string> close = CLOSE_CONNECTION;

  // Expect that these headers are also included
  EXPECT_NE(headers.find(allowAll.first), headers.end());
  EXPECT_NE(headers.find(close.first), headers.end());

  // Expect there to be no other keys
  EXPECT_EQ(headers.find("SomeFakeKey"), headers.end());
}

TEST(TestUtilities, TestGetIdFromPath) {
  std::string endpoint = "/user";
  // Turn off formatter to keep these test easily readible
  // clang-format off
  EXPECT_EQ(Utilities::GetEntityIdFromRequestPath("", endpoint), "");
  EXPECT_EQ(Utilities::GetEntityIdFromRequestPath("user", endpoint), "");
  EXPECT_EQ(Utilities::GetEntityIdFromRequestPath("/user/", endpoint), "");
  EXPECT_EQ(Utilities::GetEntityIdFromRequestPath("/user", endpoint), "");
  EXPECT_EQ(Utilities::GetEntityIdFromRequestPath("/user/123", endpoint), "123");
  EXPECT_EQ(Utilities::GetEntityIdFromRequestPath("/user/123/", endpoint), "123");
  EXPECT_EQ(Utilities::GetEntityIdFromRequestPath("/entity/456/user/123/", endpoint), "123");
  EXPECT_EQ(Utilities::GetEntityIdFromRequestPath("/entity/456/user/123/", endpoint), "123");
  EXPECT_EQ(Utilities::GetEntityIdFromRequestPath("/user/123/entity/456", endpoint), "123");
  EXPECT_EQ(Utilities::GetEntityIdFromRequestPath("/user/123/entity/456/", endpoint), "123");
  // clang-format on
}
