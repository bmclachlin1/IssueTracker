#ifndef CommentSERVICE_H
#define CommentSERVICE_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Comment.h"
#include "EntityService.hpp"
#include "Exceptions.h"
#include "FileHandler.h"
#include "IStreamableFileHandler.h"
#include "UserService.h"

/**
 * @class CommentService
 * @brief realization of EntityService for comments
 */
class CommentService : public EntityService<Comment> {
 public:
  /**
   * Constructor. Uses the default EntityService constructor to read data from
   * "Comments.json"
   * @param userService the UserService
   */
  CommentService(std::shared_ptr<UserService> userService)
      : EntityService("comments.json"), _userService(userService) {}

  /**
   * Constructor. Specifies the FileHandler to be used.
   * @param fileHandler FileHandler to be used for reading/writing Comments
   * @param userService the UserService
   */
  explicit CommentService(std::shared_ptr<IStreamableFileHandler> fileHandler,
                          std::shared_ptr<UserService> userService)
      : EntityService(fileHandler), _userService(userService) {}
  virtual ~CommentService() {}

  /**
   * Gets the Entities in our JSON file
   * @param queryParams the parameters to filter Entities by. Default is an
   *empty map
   * @return a vector of Users
   */
  virtual std::vector<Comment> Get(
      const std::multimap<std::string, std::string> queryParams = {});

  /**
   * Get a Comment by its id
   * @param id the id of the Comment
   * @return the Comment with that id
   * @throw NotFoundError if no Comment is found with that id
   */
  virtual Comment Get(const std::string id);

  /**
   * Creates a Comment and saves it to the JSON file
   * @param body the information of the Comment to create
   * @throw BadRequestError if the body is invalid
   */
  virtual Comment Create(std::string body);

  /**
   * Updates a Comment and saves it to the JSON file
   * @param body the updated Comment information
   * @throw BadRequestError if the body is invalid
   * @throw NotFoundError if the Comment could not be found
   */
  virtual Comment Update(std::string body);

  /**
   * Deletes a Comment and saves the changes to our JSON file
   * @param id the id of the Comment to delete
   * @throw NotFoundError if the Comment could not be found
   */
  virtual bool Delete(std::string id);

 protected:
  /**
   * Internal UserService, for handling User data for the Comments
   */
  std::shared_ptr<UserService> _userService;
};

#endif  // CommentSERVICE_H
