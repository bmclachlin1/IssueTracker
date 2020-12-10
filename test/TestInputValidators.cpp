#include <limits>
#include <string>
#include <vector>

#include "UserInputValidators.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using string = std::string;

using ::testing::AnyOf;
using ::testing::MockFunction;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::StrNe;

TEST(TestInputValidators, TestGenericValidator) {
  // Create some mock callbacks to make sure the validator is calling them
  MockFunction<bool(int)> callback1;
  MockFunction<bool(int)> callback2;
  // Create multiple validators
  string message1 = "Error message 1";
  string message2 = "Error message 2";
  Validator<int> validator1 =
      Validator<int>(callback1.AsStdFunction(), message1);
  Validator<int> validator2 =
      Validator<int>(callback2.AsStdFunction(), message2);
  // Combine the validators using the initializer_list constructor
  Validator<int> validator3 = {validator1, validator2};

  int testInt = 5;

  // If the first callback is true, we should check the second one. If the
  // second one is true, we should get no error
  EXPECT_CALL(callback1, Call(testInt)).WillOnce(Return(true));
  EXPECT_CALL(callback2, Call(testInt)).WillOnce(Return(true));
  string message = validator3.Validate(testInt);
  EXPECT_THAT(message, StrEq(""));

  // If the first callback is true, we should check the second one. If the
  // second one is false, we should get the second message back
  EXPECT_CALL(callback1, Call(testInt)).WillOnce(Return(true));
  EXPECT_CALL(callback2, Call(testInt)).WillOnce(Return(false));
  message = validator3.Validate(testInt);
  EXPECT_THAT(message, StrEq(message2));

  // If the first callback is false, we should not check the second one, and we
  // should get the first message back
  EXPECT_CALL(callback1, Call(testInt)).WillOnce(Return(false));
  EXPECT_CALL(callback2, Call(::testing::_)).Times(0);
  message = validator3.Validate(testInt);
  EXPECT_THAT(message, StrEq(message1));
}

TEST(TestInputValidators, TestStringValidator_Default) {
  Validator<string> validator;

  // With a default string validator, any input should provide no error message,
  // and should always evaluate to true
  string input = "some input";
  string message = validator.Validate("some input");
  EXPECT_THAT(message, StrEq(""));
  EXPECT_TRUE(validator(input));

  input = "";
  message = validator.Validate(input);
  EXPECT_THAT(message, StrEq(""));
  EXPECT_TRUE(validator(input));
}

TEST(TestInputValidators, TestIntValidator_Default) {
  Validator<int> validator;
  int input = 0;
  // With a default int validator, any input should provide no error message,
  // and should always evaluate to true
  string message = validator.Validate(input);
  EXPECT_THAT(message, StrEq(""));
  EXPECT_TRUE(validator(input));

  input = std::numeric_limits<int>::min();
  message = validator.Validate(input);
  EXPECT_THAT(message, StrEq(""));
  EXPECT_TRUE(validator(input));

  input = std::numeric_limits<int>::max();
  EXPECT_THAT(message, StrEq(""));
  EXPECT_TRUE(validator(input));
}

TEST(TestInputValidators, TestStringValidator) {
  Validator<string> validator = Validator<string>(
      [](string name) { return name == "Steven"; }, "Name must be Steven");
  string neverSleeps = "Everett";
  string message = validator.Validate(neverSleeps);
  EXPECT_THAT(message, StrNe(""));
  EXPECT_FALSE(validator(neverSleeps));

  neverSleeps = "Steven";
  message = validator.Validate(neverSleeps);
  EXPECT_THAT(message, StrEq(""));
  EXPECT_TRUE(validator(neverSleeps));
}

TEST(TestInputValidators, TestStringValidator_StringLengthValidator) {
  // Test first with default parameters
  auto validator = Validators::StringLengthValidator(1, 13);

  string testString = "thirteen char";
  string message = validator.Validate(testString);
  EXPECT_THAT(message, StrEq(""));

  testString = "fourteen chars";
  message = validator.Validate(testString);
  EXPECT_THAT(message, StrNe(""));

  testString = "";
  message = validator.Validate(testString);
  EXPECT_THAT(message, StrNe(""));

  // Set values for validator
  string customMessage = "Gotta be between 5 and 10, dawg!";
  validator = Validators::StringLengthValidator(5, 10, customMessage);
  testString = "four";
  message = validator.Validate(testString);
  EXPECT_THAT(message, StrEq(customMessage));

  testString = "sooooooo many characters z0MG!";
  message = validator.Validate(testString);
  EXPECT_THAT(message, StrEq(customMessage));

  testString = "Exactly 10";
  message = validator.Validate(testString);
  EXPECT_THAT(message, StrNe(customMessage));

  testString = "Five!";
  message = validator.Validate(testString);
  EXPECT_THAT(message, StrNe(customMessage));
}

TEST(TestInputValidators, TestStringValidator_MultipleValidators) {
  Validator<string> multiValidator;
  // Create a validator to check a list of "bad words"
  std::vector<string> badWords = {"poop", "toilet", "Unit testing"};
  string badWordsMessage = "Please, no bad words";
  auto noBadWords = Validator<string>(
      [badWords](string input) {
        bool inputIsClean = true;
        for (auto word : badWords)
          inputIsClean &= input.find(word) == string::npos;
        return inputIsClean;
      },
      badWordsMessage);

  // Create a validator to check the length of an input string
  string lengthMessage = "The input has too many characters";
  auto lengthValidator = Validator<string>(
      [](string input) { return input.size() <= 30; }, lengthMessage);

  // Add both validators to another validator object
  multiValidator.Add(noBadWords);
  multiValidator.Add(lengthValidator);

  // All validators should evaluate to true
  string testString = "This is a nice, short string!";
  string message1 = lengthValidator.Validate(testString);
  string message2 = noBadWords.Validate(testString);
  string message3 = multiValidator.Validate(testString);

  EXPECT_THAT(message1 + message2 + message3, StrEq(""));

  // The multi-validator and bad words validator should evaluate to false, but
  // the length validator should be true
  testString = "Unit testing makes me poop";
  message1 = lengthValidator.Validate(testString);
  message2 = noBadWords.Validate(testString);
  message3 = multiValidator.Validate(testString);
  EXPECT_THAT(message1, StrEq(""));
  EXPECT_THAT(message2, StrEq(badWordsMessage));
  EXPECT_THAT(message3, StrEq(badWordsMessage));

  // The multivalidator and length validator should evaluate to false, but the
  // bad words validator should be true
  testString = "This is a longer string that contains no naughty words.";
  message1 = lengthValidator.Validate(testString);
  message2 = noBadWords.Validate(testString);
  message3 = multiValidator.Validate(testString);
  EXPECT_THAT(message1, StrEq(lengthMessage));
  EXPECT_THAT(message2, StrEq(""));
  EXPECT_THAT(message3, StrEq(lengthMessage));

  testString =
      "This is a long string talking about a toilet and unit testing. Such a "
      "naughty string ;)";
  message1 = lengthValidator.Validate(testString);
  message2 = noBadWords.Validate(testString);
  message3 = multiValidator.Validate(testString);
  EXPECT_THAT(message1, StrEq(lengthMessage));
  EXPECT_THAT(message2, StrEq(badWordsMessage));
  EXPECT_THAT(message3, AnyOf(StrEq(lengthMessage), StrEq(badWordsMessage)));
}

TEST(TestInputValidators, TestStringValidators_IdValidator) {
  auto idValidator = Validators::IdValidator();
  string testString = "abcde12345";
  string message = idValidator.Validate(testString);
  EXPECT_THAT(message, StrEq(""));
  EXPECT_TRUE(idValidator(testString));

  // Testing these by manually specifying the expected error message seems
  // frail, but :shrug:
  string lengthMessage = "Id must be 10 characters long";
  string alphanumericMessage =
      "Id must only contain lowercase alphanumeric characters";

  // We should get an error if we put in uppercase characters
  testString = "UPPERCASE0";
  message = idValidator.Validate(testString);
  EXPECT_THAT(message, StrEq(alphanumericMessage));
  EXPECT_FALSE(idValidator(testString));

  // Shouldn't work with non alphanumeric characters
  testString = R"({"abc": 1})";
  message = idValidator.Validate(testString);
  EXPECT_THAT(message, StrEq(alphanumericMessage));
  EXPECT_FALSE(idValidator(testString));

  // We should get an error if we put in more than 10 characters
  testString = "wayover10characters";
  message = idValidator.Validate(testString);
  EXPECT_THAT(message, StrEq(lengthMessage));
  EXPECT_FALSE(idValidator(testString));

  // We should also get an error if there are less than 10 characters
  testString = "2few";
  message = idValidator.Validate(testString);
  EXPECT_THAT(message, StrEq(lengthMessage));
  EXPECT_FALSE(idValidator(testString));

  // We should get some errors for too long and non-alphanumeric
  testString = "toolong&has$ymb0ls";
  message = idValidator.Validate(testString);
  EXPECT_THAT(message, AnyOf(StrEq(alphanumericMessage), StrEq(lengthMessage)));
  EXPECT_FALSE(idValidator(testString));

  // The empty string shouldn't work
  testString = "";
  message = idValidator.Validate(testString);
  EXPECT_THAT(message, AnyOf(StrEq(alphanumericMessage), StrEq(lengthMessage)));
  EXPECT_FALSE(idValidator(testString));
}

TEST(TestInputValidators, TestStringValidators_StringNotEmptyValidator) {
  auto notEmptyValidator = Validators::StringNotEmptyValidator();
  string testString = "";
  string message = notEmptyValidator.Validate(testString);
  EXPECT_THAT(message, StrNe(""));
  EXPECT_FALSE(notEmptyValidator(testString));

  testString = "something else";
  message = notEmptyValidator.Validate(testString);
  EXPECT_THAT(message, StrEq(""));
  EXPECT_TRUE(notEmptyValidator(testString));

  // Make sure setting the message works properly
  string errorMessage = "Your string is empty, ya dingus";
  notEmptyValidator = Validators::StringNotEmptyValidator(errorMessage);
  testString = "";
  message = notEmptyValidator.Validate(testString);
  EXPECT_THAT(message, StrEq(errorMessage));
  EXPECT_FALSE(notEmptyValidator(testString));

  testString = "a not empty string";
  message = notEmptyValidator.Validate(testString);
  EXPECT_THAT(message, StrEq(""));
  EXPECT_TRUE(notEmptyValidator(testString));
}

TEST(TestInputValidators, TestIntValidators_MinValidator) {
  auto minValidator = Validators::MinValidator(10);
  int testInt = 5;
  string message = minValidator.Validate(testInt);
  EXPECT_THAT(message, StrNe(""));
  EXPECT_FALSE(minValidator(testInt));

  testInt = 10;
  message = minValidator.Validate(testInt);
  EXPECT_THAT(message, StrEq(""));
  EXPECT_TRUE(minValidator(testInt));

  testInt = std::numeric_limits<int>::max();
  message = minValidator.Validate(testInt);
  EXPECT_THAT(message, StrEq(""));
  EXPECT_TRUE(minValidator(testInt));

  // Make sure setting the messge works properly
  string errorMessage = "ur number is too damn low!";
  minValidator = Validators::MinValidator(10, errorMessage);
  testInt = 5;
  message = minValidator.Validate(testInt);
  EXPECT_THAT(message, StrEq(errorMessage));
  EXPECT_FALSE(minValidator(testInt));
}

TEST(TestInputValidator, TestIntValidators_MaxValidator) {
  auto maxValidator = Validators::MaxValidator(10);
  int testInt = 15;
  string message = maxValidator.Validate(testInt);
  EXPECT_THAT(message, StrNe(""));
  EXPECT_FALSE(maxValidator(testInt));

  testInt = 10;
  message = maxValidator.Validate(testInt);
  EXPECT_THAT(message, StrEq(""));
  EXPECT_TRUE(maxValidator(testInt));

  testInt = std::numeric_limits<int>::min();
  message = maxValidator.Validate(testInt);
  EXPECT_THAT(message, StrEq(""));
  EXPECT_TRUE(maxValidator(testInt));

  // Make sure setting the messge works properly
  string errorMessage = "ur number is too damn high!";
  maxValidator = Validators::MaxValidator(10, errorMessage);
  testInt = std::numeric_limits<int>::max();
  message = maxValidator.Validate(testInt);
  EXPECT_THAT(message, StrEq(errorMessage));
  EXPECT_FALSE(maxValidator(testInt));
}

TEST(TestInputValidator, TestIntValidators_RangeValidator) {
  auto rangeValidator = Validators::RangeValidator(0, 10);
  int testInt = 5;
  string message = rangeValidator.Validate(testInt);
  EXPECT_THAT(message, StrEq(""));
  EXPECT_TRUE(rangeValidator(testInt));

  // Check lower boundary value
  testInt = 0;
  message = rangeValidator.Validate(testInt);
  EXPECT_THAT(message, StrEq(""));
  EXPECT_TRUE(rangeValidator(testInt));

  // Check upper boundary value
  testInt = 10;
  message = rangeValidator.Validate(testInt);
  EXPECT_THAT(message, StrEq(""));
  EXPECT_TRUE(rangeValidator(testInt));

  // Check outside the range
  testInt = 15;
  message = rangeValidator.Validate(testInt);
  EXPECT_THAT(message, StrNe(""));
  EXPECT_FALSE(rangeValidator(testInt));
  testInt = -5;
  message = rangeValidator.Validate(testInt);
  EXPECT_THAT(message, StrNe(""));
  EXPECT_FALSE(rangeValidator(testInt));

  // Make sure a custom message works
  string errorMessage = "gotta be in that range, dawg";
  rangeValidator = Validators::RangeValidator(0, 10, errorMessage);
  testInt = -5;
  message = rangeValidator.Validate(testInt);
  EXPECT_THAT(message, StrEq(errorMessage));
  testInt = 15;
  message = rangeValidator.Validate(testInt);
  EXPECT_THAT(message, StrEq(errorMessage));
}
