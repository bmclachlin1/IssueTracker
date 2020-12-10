#ifndef IssueSERVICE_H
#define IssueSERVICE_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "CommentService.h"
#include "EntityService.hpp"
#include "Exceptions.h"
#include "FileHandler.h"
#include "IStreamableFileHandler.h"
#include "Issue.h"
#include "UserService.h"
#include "VoteService.h"

/**
 * @class IssueService
 * @brief Realization of EntityService for issues
 */
class IssueService : public EntityService<Issue> {
 public:
  /**
   * Constructor. Uses the default EntityService constructor to read data from
   * "issues.json"
   * @param userService the UserService
   * @param commentService the CommentService
   */
  IssueService(std::shared_ptr<UserService> userService,
               std::shared_ptr<CommentService> commentService,
               std::shared_ptr<VoteService> voteService)
      : EntityService("issues.json"),
        _userService(userService),
        _commentService(commentService),
        _voteService(voteService) {}

  /**
   * Constructor. Specifies the file handler to be used.
   * @param fileHandler, the file handler to be used
   * @param userService the UserService
   * @param commentService the CommentService
   */
  explicit IssueService(std::shared_ptr<IStreamableFileHandler> fileHandler,
                        std::shared_ptr<UserService> userService,
                        std::shared_ptr<CommentService> commentService,
                        std::shared_ptr<VoteService> voteService)
      : EntityService(fileHandler),
        _userService(userService),
        _commentService(commentService),
        _voteService(voteService) {}
  virtual ~IssueService() {}

  /**
   * Gets the Entities in our JSON file
   * @param queryParams the parameters to filter Entities by. Default is an
   *empty map
   * @return a vector of Users
   */
  virtual std::vector<Issue> Get(
      const std::multimap<std::string, std::string> queryParams = {});

  /**
   * Get a Issue by its id
   * @param id the id of the Issue
   * @return the Issue with that id
   * @throw NotFoundError if no Issue is found with that id
   */
  virtual Issue Get(const std::string id);

  /**
   * Creates a Issue and saves it to the JSON file
   * @param body the information of the Issue to create
   * @throw BadRequestError if the body is invalid
   */
  virtual Issue Create(std::string body);

  /**
   * Not implemented for the Issue model, since there is no concept of updating
   * a Issue
   * @throw NotImplementedError alerting the caller that this method isn't valid
   */
  virtual Issue Update(std::string body);

  /**
   * Deletes a Issue and saves the changes to our JSON file
   * @param id the id of the Issue to delete
   * @throw NotFoundError if the Issue
   */
  virtual bool Delete(std::string id);

 protected:
  std::shared_ptr<UserService> _userService;
  std::shared_ptr<CommentService> _commentService;
  std::shared_ptr<VoteService> _voteService;
};

#endif  // IssueSERVICE_H
