#ifndef VOTE_CONTROLLER_H
#define VOTE_CONTROLLER_H

#include <restbed>

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "EntityController.hpp"
#include "Utilities.h"
#include "VoteService.h"

using std::placeholders::_1;

/**
 * @class VoteController
 * @brief Implementation of the EntityController class to deal with Vote
 * entities
 *
 * Votes can only be created, deleted, or queried. Thus, the update method for
 * the controller has been deleted.
 */
template <class Session = restbed::Session>
class VoteController : public EntityController<Vote, Session> {
 public:
  /**
   * Default constructor. Sets the endpoint to "/votes" and leaves the data
   * members uninitialized
   */
  VoteController()
      : EntityController<Vote, Session>("/votes",
                                        std::make_shared<VoteService>()) {}

  /**
   * Constructor. Sets the endpoint to "/votes" and the VoteService, and
   * initializes the internal data members to the appropriate values
   * @param voteService the VoteService used by the controller
   */
  explicit VoteController(const std::shared_ptr<VoteService>& voteService)
      : EntityController<Vote, Session>("/votes", voteService) {
    // Initialize the resource paths to /votes and /issue/:issueId/votes
    std::string issuePath = "/issues/{issueId:[a-z0-9]*}/";
    std::string idPath = "/{id:[a-z0-9]*}";
    this->resource->set_paths({
        this->_endpoint,                     /* /votes */
        this->_endpoint + idPath,            /* /votes/:id */
        issuePath + this->_endpoint,         /* /issues/:issueId/votes */
        issuePath + this->_endpoint + idPath /* /issues/:issueId/votes/:id */
    });
    // Set the method handler for GET requests
    this->resource->set_method_handler(
        "GET", std::bind(&VoteController::Get, this, _1));
    // Set the method handler for POST requests
    this->resource->set_method_handler(
        "POST", std::bind(&VoteController::Create, this, _1));
  }
  virtual ~VoteController() {}

  /**
   * Overloaded Create method, which handles creating and deleting Votes. If a
   * Vote exists for the corresponding User and Issue, we will call
   * VoteService::Delete to remove it. Else, we will call VoteService::Create to
   * create one
   * @param session the restbed::Session containing the request
   */
  void Create(const std::shared_ptr<Session>& session) override {
    auto request = session->get_request();

    std::string responseBody;
    int statusCode;

    if (request == nullptr) {
      statusCode = restbed::BAD_REQUEST;
      BadRequestError e("No request found");
      responseBody = ResponseUtilities::GenerateErrorResponse("Invalid request",
                                                              statusCode, e);
    } else {
      int contentLength = request->get_header("Content-Length", 0);
      std::string path = request->get_path();

      std::string issueId =
          Utilities::GetEntityIdFromRequestPath(path, "/issues");

      std::string voteId =
          Utilities::GetEntityIdFromRequestPath(path, this->_endpoint);
      if (!voteId.empty()) {
        BadRequestError e =
            BadRequestError(std::string("Invalid path provided. POST requests "
                                        "must not contain a Vote id")
                                .c_str());
        statusCode = restbed::BAD_REQUEST;
        responseBody = ResponseUtilities::GenerateErrorResponse(
            "Invalid Request", statusCode, e);
      } else {
        if (contentLength == 0) {
          BadRequestError e = BadRequestError("No request body found");
          statusCode = restbed::BAD_REQUEST;
          responseBody = ResponseUtilities::GenerateErrorResponse(
              "Invalid Request", statusCode, e);
        } else {
          // Get the content from the session and process the request
          session->fetch(
              contentLength, [&](const std::shared_ptr<Session>& session,
                                 const restbed::Bytes& body) {
                try {
                  std::string requestBody = restbed::String::to_string(body);
                  json requestJson = json::parse(requestBody);
                  std::string userId = requestJson.value("createdBy", "");

                  // Create the query for existing votes
                  StringMap queryParams = {{"issueId", issueId},
                                           {"createdBy", userId}};
                  std::vector<Vote> existingVotes =
                      this->_entityService->Get(queryParams);

                  // If there are no votes by that user for the issue, we create
                  // one Else, we delete the one that exists
                  if (existingVotes.empty()) {
                    Vote vote = this->_entityService->Create(requestBody);
                    json response = vote;
                    responseBody = response.dump();
                    statusCode = restbed::CREATED;
                  } else {
                    bool deleted =
                        this->_entityService->Delete(existingVotes.front().id);
                    statusCode = restbed::NO_CONTENT;
                  }
                } catch (const NotFoundError& e) {
                  // The service couldn't find the user trying to vote on the
                  // issue
                  statusCode = restbed::NOT_FOUND;
                  responseBody = ResponseUtilities::GenerateErrorResponse(
                      "Not found", statusCode, e);
                } catch (const BadRequestError& e) {
                  // The request was bad (i.e. missing required fields on the
                  // Entity)
                  statusCode = restbed::BAD_REQUEST;
                  responseBody = ResponseUtilities::GenerateErrorResponse(
                      "Invalid request", statusCode, e);
                } catch (const std::exception& e) {
                  // Something went wrong, likely processing the file
                  statusCode = restbed::INTERNAL_SERVER_ERROR;

                  responseBody = ResponseUtilities::GenerateErrorResponse(
                      "Something went wrong while processing your request",
                      statusCode, e);
                }
              });
        }
      }
    }
    // Create the response headers
    StringMap contentLengthHeader = {CONTENT_LENGTH(responseBody)};
    StringMap headers =
        ResponseUtilities::BuildResponseHeader(contentLengthHeader);

    // Close the session with the response
    session->close(statusCode, responseBody, headers);
  }

  /**
   * Votes cannot be updated, so this method overloaded to do nothing
   */
  void Update(const std::shared_ptr<Session>& session) override {}

  /**
   * Votes will be deleted by the user making a POST request. Thus, this method
   * is not needed, so it is overloaded to do nothing
   */
  void Delete(const std::shared_ptr<Session>& session) override {}
};

#endif  // VOTE_CONTROLLER_H
