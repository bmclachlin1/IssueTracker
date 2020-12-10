#ifndef MOCKISTREAMFILEHANDLER_H
#define MOCKISTREAMFILEHANDLER_H

#include "IStreamableFileHandler.h"

#include <iostream>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

/**
 * This class provides a mock implementaton of our File Handler interface
 */
class MockIStreamFileHandler : public IStreamableFileHandler {
 public:
  /**
   * Constructor for our Mock File Handler
   */
  MockIStreamFileHandler() {}

  /**
   * Destructor for our Mock File Handler
   */
  virtual ~MockIStreamFileHandler() {}

  MOCK_METHOD1(write, void(json));
  MOCK_METHOD0(read, json());
};

#endif
