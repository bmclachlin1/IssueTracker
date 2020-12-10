#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <fstream>
#include <string>

#include "IStreamableFileHandler.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

/**
 * @class FileHandler
 * @brief Realization of the IStreamableFileHandler class for reading and
 * writing JSON files to the disk
 */
class FileHandler : public IStreamableFileHandler {
 public:
  /**
   * Constructor. Takes in a sstream or a fstream for easier testing
   * @param ios the iostream to be used for our fileHandler
   */
  explicit FileHandler(std::ofstream& os, std::ifstream& is, std::string file)
      : _os(os), _is(is), fileName(file) {}

  virtual ~FileHandler() {}

  /**
   * Writes the updated JSON object to our file
   * @param updatedJson the updated JSON object
   */
  virtual void write(json updatedJson);

  /**
   * Reads the iostream and parsed the iostream into a JSON object
   * @return a parsed nlohmann::json object from our iostream (sstream or
   * fstream)
   */
  virtual nlohmann::json read();

 private:
  /**
   * The name of the file we are processing
   */
  std::string fileName;

  /**
   * The input file stream for our file
   */
  std::ifstream& _is;

  /**
   * The output file stream for our file
   */
  std::ofstream& _os;
};

#endif
