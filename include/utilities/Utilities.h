#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <math.h>
#include <restbed>

#include <ctime>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>

#include "Exceptions.h"
#include "ServerErrorResponse.h"
#include "User.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;
using string = std::string;

/**
 * Typedef of commonly used multimap objects to improve readbility of controller
 * code
 */
typedef std::multimap<std::string, std::string> StringMap;

/**
 * HTTP response header to allow all origins
 */
#define ALLOW_ALL \
  { "Access-Control-Allow-Origin", "*" }

/**
 * HTTP response header to notify the client to close the connection
 */
#define CLOSE_CONNECTION \
  { "Connection", "close" }

#define CONTENT_LENGTH(content) \
  { "Content-Length", std::to_string(content.length()) }

namespace HotTicket {
// clang-format off
static std::string AppInfo =
R"(
    )                                         
 ( /(        )    *   )          )         )  
 )\())    ( /(  ` )  /((      ( /(   (  ( /(  
((_)\  (  )\())  ( )(_))\  (  )\()) ))\ )\()) 
 _((_) )\(_))/  (_(_()|(_) )\((_)\ /((_|_))/  
| || |((_) |_   |_   _|(_)((_) |(_|_)) | |_   
| __ / _ \  _|    | |  | / _|| / // -_)|  _|  
|_||_\___/\__|    |_|  |_\__||_\_\\___| \__|  

        Issue Tracking, made spicy

            © Team Jalapeno, 2020
            
  By: Everett Blakley, Blake McLachlin, and 
               Steven Trinh
)";
// clang-format on
}  // namespace HotTicket

/**
 * @namespace Utilities
 * @brief General utility functions used throughout the application
 */
namespace Utilities {
/**
 * Parses a request path and returns the id, assumed to be between the endpoint
 * parameter and the end next /, or the end of the path
 * @param path the request path to be parsed
 * @param endpoint the name of the endpoint to being parsing the path from
 * @return the id, or and empty string if an id could not be found
 */
std::string GetEntityIdFromRequestPath(const std::string& path,
                                       const std::string& endpoint);

/**
 * @returns the number of digits in i
 */
int DigitCount(int i);
}  // namespace Utilities

/**
 * @namespace TimeUtilities
 * @brief Collection of utility methods related to time manipulation
 */
namespace TimeUtilities {
/**
 * Converts a string formatted time from our JSON file to a Time object
 * Expected Format: Www Mmm dd hh:mm:ss yyyy (e.g. Thu Aug 23 14:55:02 2001)
 * To parse back into an object, we use the std::get_time specifiers, defined
 * [here](https://en.cppreference.com/w/cpp/io/manip/get_time)
 * @param stringTime the string formatted time to be converted
 */
tm ConvertStringToTime(const std::string& stringTime);

/**
 * Converts a struct tm object to a string. Similar to std::asctime
 * The output format is Www Mmm dd hh:mm:ss yyyy
 * @param time the time object to convert
 */
std::string ConvertTimeToString(struct tm time);

/**
 * Compares two time structs
 * @param tm1 first time to compare
 * @param tm2 second time to compare
 * @return true if tm1 < tm2, false otherwise
 */
bool DateComparator(struct tm tm1, struct tm tm2);

/**
 * @returns a struct tm with the current time in UTC
 */
struct tm CurrentTimeUTC();

/**
 * @returns a struct tm object with the "null date", i.e.Sunday January 1 1900
 * 00:00:00
 */
struct tm NullTimeUTC();
}  // namespace TimeUtilities

/**
 * @namespace ResponseUtilities
 * @brief Collection of utility methods for handling and creating responses from
 * the server
 */
namespace ResponseUtilities {
/**
 * Method to build the response header to send back to the client
 * @return a key-value pair of header parameters
 */
StringMap BuildResponseHeader(const StringMap& parameters);

/**
 * Creates a good error response (as described
 * [here](https://www.baeldung.com/rest-api-error-handling-best-practices))
 * when an error is encountered. Output will be a JSON object of the form
 * \verbatim
 * {
 *   "error": {
 *      "title": A generic title of the error (i.e. Not Found),
 *      "detail": A more specific description of the error (i.e. A User could
 * not be found with the following id: 12),
 *      "status": the [HTTP Status
 * Code](https://developer.mozilla.org/en-US/docs/Web/HTTP/Status) for the
 * error
 *    }
 * }
 * \endverbatim
 * @param title the title of the error
 * @param statusCode the HTTP Status Code for the error
 * @param exception the exception containing the details of the error
 * @return a string representation of the JSON error response
 */
std::string GenerateErrorResponse(const std::string& title,
                                  const int& statusCode,
                                  const std::exception& exception);

/**
 * Handle the response from the service.
 * @param response The response object from the server.
 */
json HandleResponse(std::shared_ptr<restbed::Response> response);

/**
 * Creates default headers for the server response
 */
StringMap BuildResponseHeaders(const std::string& body = "");
}  // namespace ResponseUtilities

/**
 * @namespace RequestUtilities
 * @brief Collection of utilities used for forming properly structured HTTP
 * requests
 */
namespace RequestUtilities {
/**
 * Create a generic request based on the information provided
 * @param baseUri the base uri being requested (i.e. http://localhost:8080)
 * @param endpoint the endpoint being requested (i.e. /users)
 * @param method the HTTP method (i.e. GET, POST, PUT, or DELETE)
 */
std::shared_ptr<restbed::Request> CreateGenericRequest(
    const std::string& baseUri, const std::string& endpoint,
    const std::string& method);

/**
 * Create a GET request
 * @param baseUri the base uri being requested (i.e. http://localhost:8080)
 * @param endpoint the endpoint being requested (i.e. /users)
 * @return the fully assembled restbed::Request object
 */
std::shared_ptr<restbed::Request> CreateGetRequest(const std::string& baseUri,
                                                   const std::string& endpoint);

/**
 * Create a POST request
 * @param baseUri the base uri being requested (i.e. http://localhost:8080)
 * @param endpoint the endpoint being requested (i.e. /users)
 * @param body the body of the post request (default is empty)
 * @return the fully assembled restbed::Request object
 */
std::shared_ptr<restbed::Request> CreatePostRequest(const std::string& baseUri,
                                                    const std::string& endpoint,
                                                    const StringMap& body = {});

/**
 * Create a PUT request
 * @param baseUri the base uri being requested (i.e. http://localhost:8080)
 * @param endpoint the endpoint being requested (i.e. /users)
 * @param body the body of the post request (default is empty)
 * @return the fully assembled restbed::Request object
 */
std::shared_ptr<restbed::Request> CreatePutRequest(const std::string& baseUri,
                                                   const std::string& endpoint,
                                                   const StringMap& body = {});

/**
 * Create a DELETE request
 * @param baseUri the base uri being requested (i.e. http://localhost:8080)
 * @param endpoint the endpoint being requested (i.e. /users)
 * @param body the body of the post request (default is empty)
 * @return the fully assembled restbed::Request object
 */
std::shared_ptr<restbed::Request> CreateDeleteRequest(
    const std::string& baseUri, const std::string& endpoint);

}  // namespace RequestUtilities

#endif  // UTILITIES_HPP
