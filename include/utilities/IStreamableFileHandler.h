#ifndef ISTREAMABLEFILEHANDLER_H
#define ISTREAMABLEFILEHANDLER_H
#include <nlohmann/json.hpp>
using json = nlohmann::json;

/**
 * @class IStreamableFileHandler
 * @brief Provides an interface for reading and writing JSON data to a file
 **/
class IStreamableFileHandler {
 public:
  virtual nlohmann::json read() = 0;
  virtual void write(json updatedJson) = 0;
};

#endif
