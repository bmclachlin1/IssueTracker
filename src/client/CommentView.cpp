#include "CommentView.h"

#include <restbed>

#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "Comment.h"
#include "Utilities.h"
#include "nlohmann/json.hpp"

using nlohmann::json;

using std::string;

CommentViewState CommentView::MainMenu() {
  int selection;

  std::map<int, string> options = {
      {CommentViewState::CreateComment, "Create a comment"},
      {CommentViewState::GetComments, "Get all comments"},
      {CommentViewState::GetComment, "Get a comment"},
      {CommentViewState::UpdateComment, "Update a comment"},
      {CommentViewState::DeleteComment, "Delete a comment"},
      {0, "Exit"}};

  std::stringstream prompt;
  prompt << "Please select from the following options:\n";
  for (auto option : options) {
    prompt << option.first << ": " << option.second << std::endl;
  }
  prompt << "What would you like to do: ";

  selection = PromptUser(
      prompt.str(), Validators::RangeValidator(
                        0, static_cast<int>(CommentViewState::DeleteComment),
                        "Please select a valid option"));

  return static_cast<CommentViewState>(selection);
}

string CommentView::CreateCommentView() {
  string name;

  string namePrompt =
      "Please enter the name of the comment you want to create: ";

  name = PromptUser(namePrompt,
                    Validators::StringLengthValidator(
                        1, 256, "Name must be between 1 and 256 characters"));

  StringMap comment = {{"name", name}, {"role", "Developer"}};

  auto request =
      RequestUtilities::CreatePostRequest(_serverUri, "/comments", comment);
  auto response = restbed::Http::sync(request);

  json commentResponse = ResponseUtilities::HandleResponse(response);

  if (response->get_status_code() == restbed::CREATED) {
    std::cout << "Newly created comment: \n";
    for (auto& it : commentResponse.items()) {
      std::cout << it.key() << ": " << it.value() << ", ";
    }
    std::cout << "\n\n";
  }

  return name;
}

string CommentView::UpdateCommentView() {
  string commentInfo;

  string idPrompt = "Please enter the id of the comment you want to update: ";
  string id = PromptUser(idPrompt, Validators::IdValidator());

  auto request =
      RequestUtilities::CreateGetRequest(_serverUri, "/comments/" + id);
  auto response = restbed::Http::sync(request);

  json commentResponse = ResponseUtilities::HandleResponse(response);

  if (response->get_status_code() == restbed::OK) {
    std::cout << "Requested comment: \n";
    for (auto& it : commentResponse.items()) {
      std::cout << it.key() << ": " << it.value() << std::endl;
    }

    string updatePrompt = "What would you like their new name to be: ";
    string newName =
        PromptUser(updatePrompt, Validators::StringLengthValidator(1, 256));

    commentResponse["name"] = newName;
    StringMap body = {{"id", commentResponse["id"]},
                      {"name", commentResponse["name"]},
                      {"role", commentResponse["role"]}};

    auto newRequest = RequestUtilities::CreatePutRequest(
        _serverUri, "/comments/" + commentResponse["id"].get<std::string>(),
        body);
    auto newResponse = restbed::Http::sync(newRequest);

    json j = ResponseUtilities::HandleResponse(newResponse);
  } else if (response->get_status_code() == restbed::NOT_FOUND) {
    std::cout << "Could not find that comment. Please try again later\n";
  }

  return commentInfo;
}

void CommentView::GetCommentsView() {
  auto request = RequestUtilities::CreateGetRequest(_serverUri, "/comments");
  auto response = restbed::Http::sync(request);
  json comments = ResponseUtilities::HandleResponse(response);

  if (comments.empty()) {
    std::cout << "No comments to display\n\n";
  } else {
    std::cout << "All comments: " << std::endl;
    for (auto comment : comments) {
      json u = json(comment);
      for (auto& attribute : u.items()) {
        std::cout << attribute.key() << ": " << attribute.value() << ", ";
      }
      std::cout << std::endl;
    }
  }
}

string CommentView::GetCommentView() {
  string id;

  string idPrompt = "Please enter the id of the comment you want to retrieve: ";
  id = PromptUser(idPrompt, Validators::IdValidator());

  auto request =
      RequestUtilities::CreateGetRequest(_serverUri, "/comments/" + id);
  auto response = restbed::Http::sync(request);

  json commentResponse = ResponseUtilities::HandleResponse(response);

  if (response->get_status_code() == restbed::OK) {
    std::cout << "The comment you requested: \n";
    for (auto& it : commentResponse.items()) {
      std::cout << it.key() << ": " << it.value() << ", ";
    }
    std::cout << "\n\n";
  }

  return id;
}

string CommentView::DeleteCommentView() {
  string id;

  string idPrompt = "Please enter the id of the comment you want to delete: ";
  id = PromptUser(idPrompt, Validators::IdValidator());

  auto request =
      RequestUtilities::CreateDeleteRequest(_serverUri, "/comments/" + id);
  auto response = restbed::Http::sync(request);

  json commentResponse = ResponseUtilities::HandleResponse(response);
  if (response->get_status_code() == restbed::OK) {
    std::cout << "Comment successfully deleted!\n";
  }

  return id;
}
