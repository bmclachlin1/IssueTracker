#include "FileHandler.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include "Exceptions.h"

using json = nlohmann::json;

void FileHandler::write(nlohmann::json updatedJson) {
  // Clear the iostream
  _os.open(fileName);
  // If the ostream is open
  if (_os) {
    _os << std::setw(4) << updatedJson << std::endl;;
  } else {
    throw InternalServerError(
        "The file stream was not able to be opened for reading. Check if your "
        "file path is correct");
  }

  _os.close();
}

nlohmann::json FileHandler::read() {
  // Clear the iostream
  _is.open(fileName);
  _is.clear();
  _is.seekg(0, _is.beg);
  // If the istream is open
  if (_is) {
    try {
      return json::parse(_is);
    }

    catch (std::exception& e) {
      throw InternalServerError(
          "The file could not be read. Check if your JSON file has valid "
          "syntax.");
    }
  } else {
    throw InternalServerError(
        "The file stream was not able to be opened for reading. Check if your "
        "file path is correct");
  }

  _is.close();
}
