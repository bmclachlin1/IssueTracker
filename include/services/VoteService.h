#ifndef VOTESERVICE_H
#define VOTESERVICE_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "EntityService.hpp"
#include "Exceptions.h"
#include "FileHandler.h"
#include "IStreamableFileHandler.h"
#include "UserService.h"
#include "Vote.h"

/**
 * @class VoteService
 * @brief Realization of EntityService for votes
 */
class VoteService : public EntityService<Vote> {
 public:
  /**
   * Constructor. Uses the default EntityService constructor to read data from
   * "votes.json"
   * @param userService the UserService
   */
  VoteService(std::shared_ptr<UserService> userService =
                  std::make_shared<UserService>())
      : EntityService("votes.json"), _userService(userService) {}

  /**
   * Constructor. Specifies the file handler to be used.
   * @param fileHandler FileHandler to be used for reading/writing Votes
   * @param userService internal UserService for handling User data for the
   * Votes
   */
  explicit VoteService(std::shared_ptr<IStreamableFileHandler> fileHandler,
                       std::shared_ptr<UserService> userService)
      : EntityService(fileHandler), _userService(userService) {}
  virtual ~VoteService() {}

  /**
   * Gets the Entities in our JSON file
   * @param queryParams the parameters to filter Entities by. Default is an
   *empty map
   * @return a vector of Users
   */
  virtual std::vector<Vote> Get(
      const std::multimap<std::string, std::string> queryParams = {});

  /**
   * Get a vote by its id
   * @param id the id of the vote
   * @return the vote with that id
   * @throw NotFoundError if no Vote is found with that id
   */
  virtual Vote Get(const std::string id);

  /**
   * Creates a Vote and saves it to the JSON file
   * @param body the information of the Vote to create
   * @throw BadRequestError if the body is invalid
   */
  virtual Vote Create(std::string body);

  /**
   * Not implemented for the Vote model, since there is no concept of updating a
   * vote
   * @throw NotImplementedError alerting the caller that this method isn't valid
   */
  virtual Vote Update(std::string body);

  /**
   * Deletes a Vote and saves the changes to our JSON file
   * @param id the id of the Vote to delete
   * @throw NotFoundError if the Vote
   */
  virtual bool Delete(std::string id);

 protected:
  /**
   * Internal UserService, for handling User data for the Votes
   */
  std::shared_ptr<UserService> _userService;
};

#endif  // VOTESERVICE_H
