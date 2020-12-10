#include "Utilities.h"

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

using string = std::string;

namespace Utilities {
std::string GetEntityIdFromRequestPath(const std::string& path,
                                       const std::string& endpoint) {
  std::string id;
  std::size_t start_pos = path.find(endpoint);
  if (start_pos != std::string::npos) {
    std::string subpath = path.substr(start_pos + endpoint.length());
    if (subpath.front() == '/') {
      subpath.erase(subpath.begin());
    }
    std::size_t end_pos = subpath.find('/');
    if (end_pos != std::string::npos) {
      id = subpath.substr(0, end_pos);
    } else {
      id = subpath;
    }
  }
  return id;
}

int DigitCount(int i) {
  if (i == 0) return 1;
  int count = 0;
  i = abs(i);
  while (i != 0) {
    count++;
    i /= 10;
  }
  return count;
}
}  // namespace Utilities

namespace TimeUtilities {
tm ConvertStringToTime(const std::string& stringTime) {
  std::tm time;
  std::istringstream stream(stringTime);
  // Attempt to read the input time into the expected format
  stream >> std::get_time(&time, "%a %b %d %H:%M:%S %Y");

  if (stream.fail()) {
    std::string errorMessage =
        "Unable to parse the following date string: " + stringTime;
    throw InternalServerError(errorMessage.c_str());
  }

  return time;
}

std::string ConvertTimeToString(struct tm time) {
  char buffer[80];
  strftime(buffer, 80, "%a %b %d %H:%M:%S %Y", &time);

  return std::string(buffer);
}

bool DateComparator(struct tm tm1, struct tm tm2) {
  return difftime(mktime(&tm1), mktime(&tm2)) < 0.0;
}

struct tm CurrentTimeUTC() {
  time_t t = time(0);
  return *gmtime(&t);
}

struct tm NullTimeUTC() {
  struct tm nullTime;
  nullTime.tm_hour = 0;
  nullTime.tm_min = 0;
  nullTime.tm_sec = 0;
  nullTime.tm_mday = 1;
  nullTime.tm_mon = 0;
  nullTime.tm_wday = 0;
  nullTime.tm_yday = 0;
  nullTime.tm_year = 0;
  nullTime.tm_isdst = 0;
  return nullTime;
}
}  // namespace TimeUtilities

namespace ResponseUtilities {
StringMap BuildResponseHeader(const StringMap& parameters) {
  StringMap headers = {ALLOW_ALL, CLOSE_CONNECTION};
  for (auto param : parameters) {
    headers.insert(param);
  }
  return headers;
}

std::string GenerateErrorResponse(const std::string& title,
                                  const int& statusCode,
                                  const std::exception& exception) {
  json errorResponse;
  ServerErrorResponse error(title, exception.what(), statusCode);
  errorResponse["error"] = error;
  return errorResponse.dump();
}

json HandleResponse(std::shared_ptr<restbed::Response> response) {
  int status_code = response->get_status_code();
  auto length = response->get_header("Content-Length", 0);
  json result;

  if (status_code >= 100 && status_code < 200) {
    // Informational codes. Do nothing?
  } else if (status_code >= 200 && status_code < 300) {
    restbed::Http::fetch(length, response);
    std::string responseStr(
        reinterpret_cast<char*>(response->get_body().data()), length);

    if (!responseStr.empty() && length > 0)
      result = nlohmann::json::parse(responseStr);
  } else if (status_code >= 300 && status_code < 400) {
    // Redirects. Do nothing?
  } else if (status_code >= 400 && status_code < 600) {
    restbed::Http::fetch(length, response);
    std::string responseStr(
        reinterpret_cast<char*>(response->get_body().data()), length);

    try {
      json responseJson = nlohmann::json::parse(responseStr);
      if (responseJson.find("error") != responseJson.end()) {
        ServerErrorResponse error = responseJson["error"];
        result = error;
        // Print the error message like this:
        // <title>
        //    Response code: <code>
        //    Details: <details>
        // fprintf(stderr, "%s\n\tResponse Code: %d\n\tDetails: %s\n",
        //         error.title.c_str(), error.statusCode, error.detail.c_str());
      }
    } catch (const std::exception& e) {
      // If the above fails, we fall through to the default reponse below
      // fprintf(stderr, "Response: %s\n", responseStr.c_str());
    }
  } else {
    // fprintf(stderr, "An error occurred with the service. Status Code: %d\n",
    //         status_code);
  }

  return result;
}

StringMap BuildResponseHeaders(const std::string& body) {
  StringMap headers = {ALLOW_ALL, CLOSE_CONNECTION, CONTENT_LENGTH(body)};
  return headers;
}
}  // namespace ResponseUtilities

namespace RequestUtilities {
std::shared_ptr<restbed::Request> CreateGenericRequest(
    const std::string& baseUri, const std::string& endpoint,
    const std::string& method) {
  // Create the URI string
  std::string uri = baseUri;
  uri.append(endpoint);

  // Configure request headers
  auto request = std::make_shared<restbed::Request>(restbed::Uri(uri));
  request->set_header("Accept", "*/*");
  request->set_header("Content-Type", "text/json");
  request->set_method(method);

  return request;
}

std::shared_ptr<restbed::Request> CreateGetRequest(
    const std::string& baseUri, const std::string& endpoint) {
  // Build and return request
  return CreateGenericRequest(baseUri, endpoint, "GET");
}

std::shared_ptr<restbed::Request> CreatePostRequest(const std::string& baseUri,
                                                    const std::string& endpoint,
                                                    const StringMap& body) {
  // Build request
  auto request = CreateGenericRequest(baseUri, endpoint, "POST");

  // Create the body
  json bodyJson = body;
  std::string bodyString = bodyJson.dump();

  // Set the message
  request->set_header("Content-Length", std::to_string(bodyString.length()));
  request->set_body(bodyString);

  return request;
}

std::shared_ptr<restbed::Request> CreatePutRequest(const std::string& baseUri,
                                                   const std::string& endpoint,
                                                   const StringMap& body) {
  // Build request
  auto request = CreateGenericRequest(baseUri, endpoint, "PUT");

  // Create the body
  json bodyJson = body;
  std::string bodyString = bodyJson.dump();

  // Set the message
  request->set_header("Content-Length", std::to_string(bodyString.length()));
  request->set_body(bodyString);

  return request;
}

std::shared_ptr<restbed::Request> CreateDeleteRequest(
    const std::string& baseUri, const std::string& endpoint) {
  // Build and return request
  return CreateGenericRequest(baseUri, endpoint, "DELETE");
}
}  // namespace RequestUtilities
