#include <memory>
#include <sstream>
#include <string>

#include "Exceptions.h"
#include "FileHandler.h"
#include "MockIStreamFileHandler.h"
#include "gtest/gtest.h"
#include "nlohmann/json.hpp"

using ::testing::Return;
using json = nlohmann::json;

TEST(TestFileHandler, Read_Returns_Valid_Json_User) {
  const std::string testID = "2222";
  std::stringstream ss;
  json fakeJsonData;
  ss << R"(
        {
          "2222":
            {
              "name": "Steven Trinh",
              "role": "Developer",
              "email": "steventrinh0@gmail.com"
             }
        }
     )";

  fakeJsonData = json::parse(ss);
  std::shared_ptr<MockIStreamFileHandler> fileHandler =
      std::make_shared<MockIStreamFileHandler>();
  // We should read the json data once
  EXPECT_CALL(*fileHandler, read()).Times(1).WillOnce(Return(fakeJsonData));
  nlohmann::json testUser = fileHandler->read();

  EXPECT_EQ(testUser.contains(testID), true);
  EXPECT_EQ(testUser[testID]["name"].get<std::string>(), "Steven Trinh");
  EXPECT_EQ(testUser[testID]["role"].get<std::string>(), "Developer");
  EXPECT_EQ(testUser[testID]["email"].get<std::string>(),
            "steventrinh0@gmail.com");
}

TEST(TestFileHandler,
     Read_Returns_BadStreamInput_ExpectInternalServerError_Exception) {
  std::ifstream is;
  std::ofstream os;
  std::string fileName = "McTestyFakeStreamStringy";

  FileHandler* fileHandler = new FileHandler(os, is, fileName);
  EXPECT_THROW(fileHandler->read(), InternalServerError);
}
