#ifndef ENTITYCONTROLLER_H
#define ENTITYCONTROLLER_H

#include <restbed>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "EntityService.hpp"
#include "Utilities.h"
#include "nlohmann/json.hpp"

using nlohmann::json;

/**
 * @class EntityController
 * Base class for interactions between the service and Entity objects.
 * Utilizes realizations of EntityService to interact with persistent Entity
 * objects and return them to the caller
 * @tparam Entity the Entity the controller is interacting with
 * @tparam Session the type for the Session used in the methods. Defaults to
 * restbed::Session, which is replaced with a mock in testing
 */
template <class Entity, class Session = restbed::Session>
class EntityController {
 public:
  /**
   * Contructor. Sets the endpoint and EntityService for the controller
   * @param endpoint the REST endpoint for the Entity. Must follow conventions
   * used by restbed for specifying resource paths. See
   * [here](https://github.com/Corvusoft/restbed/blob/master/documentation/example/PATH_PARAMETERS.md)
   * for more information
   */
  EntityController(const std::string& endpoint,
                   const std::shared_ptr<EntityService<Entity>>& entityService)
      : _endpoint(endpoint), _entityService(entityService) {}
  virtual ~EntityController() {}

  /**
   * General method for processing GET requests to the resource. Will call the
   * appropriate EntityService method based on the restbed::Request path
   * parameters
   * @param session restbed::Session object passed by the restbed::Service
   */
  virtual void Get(const std::shared_ptr<Session>& session) {
    auto request = session->get_request();
    std::string id;

    int statusCode;
    std::string responseBody;

    if (request != nullptr) {
      id = Utilities::GetEntityIdFromRequestPath(request->get_path(),
                                                 this->_endpoint);
      try {
        if (id.empty()) {
          // Get the query parameters provided in the request
          const StringMap queryParams = request->get_query_parameters();

          // Get all Entities from the service that match the query
          std::vector<Entity> entities = this->_entityService->Get(queryParams);
          json response = entities;

          responseBody = response.dump();
        } else {
          // Get the Entity from the service
          Entity entity = this->_entityService->Get(id);

          json response = entity;
          responseBody = response.dump();
        }

        statusCode = restbed::OK;
      } catch (const NotFoundError& e) {
        // Occues if the Entity could not be retrieved
        statusCode = restbed::NOT_FOUND;

        responseBody = ResponseUtilities::GenerateErrorResponse(
            "Invalid request", statusCode, e);
      } catch (const std::exception& e) {
        // Something else went wrong
        statusCode = restbed::INTERNAL_SERVER_ERROR;

        responseBody = ResponseUtilities::GenerateErrorResponse(
            "Something went wrong while processing your request", statusCode,
            e);
      }
    } else {
      // Create error message
      statusCode = restbed::BAD_REQUEST;
      BadRequestError e = BadRequestError("Could not process your request");
      responseBody = ResponseUtilities::GenerateErrorResponse(
          "An error occurred", statusCode, e);
    }

    // Build response headers
    StringMap contentLengthHeader = {CONTENT_LENGTH(responseBody)};
    StringMap headers =
        ResponseUtilities::BuildResponseHeader(contentLengthHeader);

    // Close session with response
    session->close(statusCode, responseBody, headers);
  }

  /**
   * Allows the creation of an Entity via a POST request to /<endpoint> with an
   * Entity object in the request body
   * @param session restbed::Session object passed by the restbed::Service
   */
  virtual void Create(const std::shared_ptr<Session>& session) {
    auto request = session->get_request();

    std::string responseBody;
    int statusCode;
    std::string requestBody;

    if (request != nullptr) {
      int contentLength = request->get_header("Content-Length", 0);

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
                requestBody = restbed::String::to_string(body);
                Entity entity = this->_entityService->Create(requestBody);

                json response = entity;
                responseBody = response.dump();

                statusCode = restbed::CREATED;
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
    } else {
      BadRequestError e = BadRequestError("No request found");
      statusCode = restbed::BAD_REQUEST;

      responseBody = ResponseUtilities::GenerateErrorResponse("Invalid Request",
                                                              statusCode, e);
    }

    // Create the response headers
    StringMap contentLengthHeader = {CONTENT_LENGTH(responseBody)};
    StringMap headers =
        ResponseUtilities::BuildResponseHeader(contentLengthHeader);

    // Close the session with the response
    session->close(statusCode, responseBody, headers);
  }

  /**
   * Allows the update of an Entity via a PUT request to /<endpoint>/:id, where
   * id refers to the id of the Entity in request body
   * @param session restbed::Session object passed by the restbed::Service
   */
  virtual void Update(const std::shared_ptr<Session>& session) {
    auto request = session->get_request();

    std::string responseBody;
    std::string requestBody;
    int statusCode;

    if (request != nullptr) {
      int contentLength = request->get_header("Content-Length", 0);

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
                requestBody = restbed::String::to_string(body);
                Entity entity = this->_entityService->Update(requestBody);
                json response = entity;

                responseBody = response.dump();

                statusCode = restbed::OK;
              } catch (const NotFoundError& e) {
                // Can't find the Entity to be updated
                statusCode = restbed::NOT_FOUND;

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
    } else {
      BadRequestError e = BadRequestError("No request found");
      statusCode = restbed::BAD_REQUEST;

      responseBody = ResponseUtilities::GenerateErrorResponse("Invalid Request",
                                                              statusCode, e);
    }

    // Create the response headers
    StringMap contentLengthHeader = {CONTENT_LENGTH(responseBody)};
    StringMap headers =
        ResponseUtilities::BuildResponseHeader(contentLengthHeader);

    session->close(statusCode, responseBody, headers);
  }

  /**
   * Allows the deletion of an Entity via a DELETE request to /<endpoint>/:id,
   * where id refers to the id of the Entity to be deleted
   */
  virtual void Delete(const std::shared_ptr<Session>& session) {
    auto request = session->get_request();

    std::string id;
    std::string responseBody;
    std::stringstream responseStream;
    int statusCode;

    if (request != nullptr) {
      id = Utilities::GetEntityIdFromRequestPath(request->get_path(),
                                                 this->_endpoint);
    }

    if (id.empty()) {
      responseBody = "Unable to process your request";

      statusCode = restbed::BAD_REQUEST;
    } else {
      try {
        bool deleted = this->_entityService->Delete(id);

        statusCode = restbed::OK;
      } catch (const NotFoundError& e) {
        // Couldn't find the Entity
        statusCode = restbed::NOT_FOUND;

        responseBody = ResponseUtilities::GenerateErrorResponse(
            "Invalid request", statusCode, e);
      } catch (const std::exception& e) {
        // Something went wrong, likely processing the file
        statusCode = restbed::INTERNAL_SERVER_ERROR;

        responseBody = ResponseUtilities::GenerateErrorResponse(
            "Something went wrong while processing your request", statusCode,
            e);
      }
    }

    // Create the response headers
    StringMap contentLengthHeader = {CONTENT_LENGTH(responseBody)};
    StringMap headers =
        ResponseUtilities::BuildResponseHeader(contentLengthHeader);

    session->close(statusCode, responseBody, headers);
  }

  /**
   * The restbed::Resource related to the Entity
   */
  std::shared_ptr<restbed::Resource> resource =
      std::make_shared<restbed::Resource>();

  /**
   * @return the endpoint for the EntityController
   */
  std::string GetEndpoint() { return _endpoint; }

  /**
   * Allow for setter injection of the EntityService
   * @param userService pointer to the new EntityService
   */
  virtual void SetEntityService(
      const std::shared_ptr<EntityService<Entity>>& entityService) {
    _entityService = entityService;
  }

 protected:
  /**
   * The EntityService corresponding to the Entity class. Handles interacting
   * with Entity objects in persistent storage
   */
  std::shared_ptr<EntityService<Entity>> _entityService;

  /**
   * The REST endpoint for the Entity
   */
  std::string _endpoint;
};

#endif  // ENTITYCONTROLLER_H
