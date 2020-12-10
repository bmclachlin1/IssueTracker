#ifndef ENTITYSERVICE_H
#define ENTITYSERVICE_H

#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "FileHandler.h"
#include "IStreamableFileHandler.h"

/**
 * @class EntityService
 * This class provides a template interface for handling Entity data stored in
 * JSON files in persistent storage
 * @tparam T Entity class the service is handling
 */
template <typename T>
class EntityService {
 public:
  /**
   * Constructor for building an EntityService from an existing
   * IStreamableFileHandler
   * @param fileHandler a file handler to manage the reading and writing of our
   * streams
   */
  explicit EntityService<T>(std::shared_ptr<IStreamableFileHandler> fileHandler)
      : _fileHandler(fileHandler) {}

  /**
   * Constructor for building an EntityService based on the name of the file
   * where the Entity information is stored
   * @param fileName the name of the file corresponding to the Entity data
   */
  explicit EntityService<T>(const std::string& fileName) {
    _fileHandler = std::make_shared<FileHandler>(os, is, fileName);
  }

  /**
   * Destructor for our User Service
   */
  virtual ~EntityService<T>() {}

  /**
   * Gets Entities in our JSON file corresponding to a query
   * @param queryParams the parameters to filter Entities by. Default is an
   * empty map
   * @return a vector of entities of type T
   */
  virtual std::vector<T> Get(
      const std::multimap<std::string, std::string> queryParams = {}) = 0;

  /**
   * Gets an Entity
   * @param id the Entity's ID that we want to retrieve from our JSON file
   * @return the entity of type T
   */
  virtual T Get(const std::string id) = 0;

  /**
   * Creates a Entity and saves it to the JSON file
   * @param body the information of the Entity to create
   * @return the successfully created Entity of type T
   */
  virtual T Create(std::string body) = 0;

  /**
   * Updates an Entity and saves that updated Entity to our JSON File
   * @param body the information of the Entity to update
   * @return the successfully updated Entity of type T
   */
  virtual T Update(std::string body) = 0;

  /**
   * Deletes an Entity and saves the changes to our JSON file
   * @param id the id of the Entity to delete
   * @returns whether or not the Entity was successfully deleted
   */
  virtual bool Delete(std::string id) = 0;

  /**
   * @return the file handler for the service
   */
  virtual std::shared_ptr<IStreamableFileHandler> getFileHandler() {
    return _fileHandler;
  }

  /**
   * Filters our json data file to find a specific value. The json data should
   * be an array.
   * @param data the json file data we are trying to search
   * @param searchParameters a key value pair where the key is the field and the
   * value is the value we want to filter from our json file.
   * @return the filtered json data
   */
  json Filter(json data,
              std::multimap<std::string, std::string> searchParameters) {
    json filtered;
    std::copy_if(data.begin(), data.end(), std::back_inserter(filtered),
                 [&](const json& item) {
                   bool found = true;
                   for (auto& param : searchParameters) {
                     found = found && item.contains(param.first) &&
                             item[param.first] == param.second;
                   }

                   return found;
                 });

    return filtered;
  }

  /**
   * Generates unique ID of 10 lowercase characters and numbers for an Entity
   * object
   */
  std::string GenerateId() {
    std::string result = "";

    for (int i = 0; i < 10; i++) {
      int num = std::rand() % 36;
      if (num < 26) {
        result += num + 'a';
      } else {
        result += num + '0' - 26;
      }
    }

    return result;
  }

 protected:
  /**
   * The File Handler
   */
  std::shared_ptr<IStreamableFileHandler> _fileHandler;

  /**
  * Output file stream containing the JSON data corresponding to the service
  */
  std::ofstream os;

  /**
  * Input file stream containing the JSON data corresponding to the service
  */
  std::ifstream is;
};

#endif  // ENTITYSERVICE_H
