#ifndef USERSERVICE_H
#define USERSERVICE_H

#include <map>
#include <memory>
#include <string>
#include <vector>
#include "EntityService.hpp"
#include "FileHandler.h"
#include "IStreamableFileHandler.h"
#include "User.h"


/**
 * @class UserService
 * @brief Realization of EntityService for users
 */
class UserService : public EntityService<User> {
 public:
  /**
   * Default Constructor for our User Service. It calls the base interface's
   * constructor for initialization.
   **/
  UserService() : EntityService("users.json") {}

  /**
   * Constructor for our User Service. It calls the base interface's constructor
   *for initialization.
   * @param fileHandler a file handler to manage the reading and writing of our
   *streams
   **/
  explicit UserService(std::shared_ptr<IStreamableFileHandler> fileHandler)
      : EntityService(fileHandler) {}
  /**
   * Destructor for our User Service
   **/
  virtual ~UserService() {}

  /**
   * Gets the Users in our JSON file
   * @param queryParams the parameters to filter Entities by. Default is an
   *empty map
   * @return a vector of Users
   **/
  std::vector<User> Get(
      const std::multimap<std::string, std::string> queryParams = {});

  /**
   * Gets a User
   * @param id the User's ID that we want to retrieve from our JSON file
   **/
  User Get(const std::string id);

  /**
   * Creates a User and saves that User to our JSON File
   * @param body the information of the User to create
   **/
  User Create(std::string body);

  /**
   * Updates a User and saves that updated User to our JSON File
   * @param body the information of the User to update
   **/
  User Update(std::string body);

  /**
   * Deletes a User and saves the changes to our JSON file
   * @param id the id of the User to delete
   **/
  bool Delete(std::string id);
};

#endif  // USERSERVICE_H
