#include "IssueView.h"
#include "MainView.h"

#include <restbed>

#include <algorithm>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>


#include "ClientAppManager.h"
#include "Comment.h"
#include "Issue.h"
#include "User.h"
#include "UserView.h"
#include "Utilities.h"
#include "Vote.h"
#include "nlohmann/json.hpp"

using nlohmann::json;

using std::string;

std::vector<string> IssueStatus = {"New", "Assigned", "Fixed", "Won't Fix",
                                   "Closed"};

User GetUserById(const std::string& id) {
  auto request = RequestUtilities::CreateGetRequest(
      ClientAppManager::ServerURI(), "/users/" + id);
  auto response = restbed::Http::sync(request);

  User user;
  if (response->get_status_code() == restbed::OK) {
    json responseJson = ResponseUtilities::HandleResponse(response);
    user = responseJson.get<User>();
  } else {
    user.name = "Unknown user";
  }
  return user;
}

Comment GetCommentById(const std::string& id) {
  auto request = RequestUtilities::CreateGetRequest(
      ClientAppManager::ServerURI(), "/comments/" + id);
  auto response = restbed::Http::sync(request);

  Comment comment;
  if (response->get_status_code() == restbed::OK) {
    json responseJson = ResponseUtilities::HandleResponse(response);
    comment = responseJson.get<Comment>();
    comment.createdBy = GetUserById(comment.createdBy.id);
    if (!comment.updatedBy.id.empty()) {
      comment.updatedBy = GetUserById(comment.updatedBy.id);
    }
  }
  return comment;
}

std::string DisplayComments(const std::multiset<Comment>& comments) {
  std::stringstream commentsStream;
  int count = 0;
  for (auto comment : comments) {
    Comment temp = GetCommentById(comment.id);
    User user = GetUserById(temp.createdBy.id);
    commentsStream << "   \u2516" << temp.body << std::endl
                   << "     By: " << user.name << " on "
                   << TimeUtilities::ConvertTimeToString(temp.createdAt)
                   << std::endl;
  }
  return commentsStream.str();
}

std::string DisplayVotesForIssue(const Issue& issue) {
  std::stringstream votesStream;
  auto request = RequestUtilities::CreateGetRequest(
      ClientAppManager::ServerURI(), "/issues/" + issue.id + "/votes");
  auto response = restbed::Http::sync(request);
  if (response->get_status_code() == restbed::OK) {
    json responseJson = ResponseUtilities::HandleResponse(response);
    std::vector<Vote> votes = responseJson;
    User currentUser = ClientAppManager::CurrentUser();
    if (std::find_if(votes.begin(), votes.end(), [currentUser](Vote vote) {
          return vote.createdBy.id == currentUser.id;
        }) != votes.end()) {
      if (votes.size() > 1) {
        votesStream << "\u2516 " << votes.size() << " votes";
        votesStream << " (You and " << votes.size() - 1 << " others)";
      } else {
        votesStream << "\u2516 You voted for this issue";
      }
    } else {
      votesStream << "\u2516 " << votes.size() << " votes";
    }
  }
  return votesStream.str();
}

IssueViewState IssueView::MainMenu() {
  std::cout << std::endl;
  int selection;
  std::vector<std::pair<IssueViewState, string>> options = {
      std::make_pair(IssueViewState::CreateIssue, "Create a new issue"),
      std::make_pair(IssueViewState::ManageOpenIssues, "View open issues"),
      std::make_pair(IssueViewState::ManageAssignedIssues,
                     "View issues assigned to me"),
      std::make_pair(IssueViewState::ManageAllIssues, "View all issues"),
      std::make_pair(IssueViewState::GoBack, "Back to main menu")};

  std::stringstream prompt;
  prompt << "Issues menu\n";
  for (int i = 0; i < options.size(); i++) {
    prompt << i + 1 << ": " << options[i].second << std::endl;
  }
  prompt << "What would you like to do: ";

  selection = PromptUser(
      prompt.str(), Validators::RangeValidator(1, options.size(),
                                               "Please select a valid option"));

  return static_cast<IssueViewState>(options[selection - 1].first);
}

ManageIssueState IssueView::ManageIssueView() {
  std::cout << std::endl;
  int selection;
  std::vector<std::pair<ManageIssueState, string>> options = {
      std::make_pair(ManageIssueState::UpdateIssue, "Update issue details"),
      std::make_pair(ManageIssueState::DeleteIssue, "Delete the issue"),
      std::make_pair(ManageIssueState::AddComment,
                     "Add a comment to the issue"),
      std::make_pair(ManageIssueState::ChangeVote,
                     "Change my vote on the issue"),
      std::make_pair(ManageIssueState::GoBack, "Back to Issues menu")};

  std::stringstream prompt;
  prompt << "Manage Issue\n";
  for (int i = 0; i < options.size(); i++) {
    prompt << i + 1 << ": " << options[i].second << std::endl;
  }
  prompt << "What would you like to do: ";

  selection = PromptUser(
      prompt.str(), Validators::RangeValidator(1, options.size(),
                                               "Please select a valid option"));

  return static_cast<ManageIssueState>(options[selection - 1].first);
}

void IssueView::RunIssueView(IssueViewState choice) {
  bool quit = choice == IssueViewState::GoBack;
  while (!quit) {
    if (choice == IssueViewState::GoBack) {
      quit = true;
    } else if (choice == IssueViewState::CreateIssue) {
      CreateIssueView();
    } else {
      StringMap query;
      if (choice == IssueViewState::ManageOpenIssues) {
        query = {{"status", "New"},
                 {"status", "Assigned"},
                 {"status", "Fixed"},
                 {"status", "Won't Fix"}};
      } else if (choice == IssueViewState::ManageAssignedIssues) {
        query = {{"assignedTo", ClientAppManager::CurrentUser().id}};
      } else {
        // Leave the query as nothing, and just get them all
      }
      Issue selectedIssue = IssuesView(query);

      if (!selectedIssue.id.empty()) {
        DisplayIssue(selectedIssue);

        ManageIssueState manageIssueChoice = ManageIssueView();

        bool quitManagingIssue = manageIssueChoice == ManageIssueState::GoBack;
        while (!quitManagingIssue) {
          switch (manageIssueChoice) {
            default:
              quitManagingIssue = true;
              break;
            case ManageIssueState::UpdateIssue:
              selectedIssue = UpdateIssueView(selectedIssue);
              DisplayIssue(selectedIssue);
              break;
            case ManageIssueState::DeleteIssue:
              quitManagingIssue = DeleteIssueView(selectedIssue);
              break;
            case ManageIssueState::AddComment:
              AddCommentView(selectedIssue);
              DisplayIssue(selectedIssue);
              break;
            case ManageIssueState::ChangeVote:
              selectedIssue = ChangeVoteView(selectedIssue);
              DisplayIssue(selectedIssue);
              break;
          }
          if (!quitManagingIssue) manageIssueChoice = ManageIssueView();
        }
      }
    }

    if (!quit) choice = MainMenu();
  }
}

Issue IssueView::IssuesView(const StringMap& query) {
  std::cout << std::endl;

  auto request = RequestUtilities::CreateGetRequest(_serverUri, "/issues");
  request->set_query_parameters(query);

  auto response = restbed::Http::sync(request);
  json jsonResponse = ResponseUtilities::HandleResponse(response);

  Issue selectedIssue;
  if (response->get_status_code() != restbed::OK) {
    std::cout << "Something went wrong: " << jsonResponse.dump() << std::endl;
  } else {
    std::vector<Issue> issues = jsonResponse;
    if (issues.empty()) {
      std::cout << "No issues to display" << std::endl;
    } else {
      std::stringstream prompt;
      prompt << "Available Issues:\n";
      for (int i = 1; i <= issues.size(); i++) {
        prompt << i << ": " << DisplayIssueBrief(issues[i - 1]) << "\n";
      }
      prompt << issues.size() + 1 << ": Go Back\n";
      prompt << "Select an issue to view more details: ";

      int selection =
          PromptUser(prompt, Validators::RangeValidator(1, issues.size() + 1));

      if (selection >= 1 && selection <= issues.size()) {
        selectedIssue = issues[selection - 1];
      }
    }
  }
  return selectedIssue;
}

void IssueView::CreateIssueView() {
  std::cout << std::endl << "Create new issue\n";

  string titlePrompt = "Issue title: ";
  string title = PromptUser(
      titlePrompt, Validators::StringLengthValidator(
                       1, 256, "Title must be between 1 and 256 characters"));

  string descriptionPrompt = "Issue description: ";
  string description = PromptUser(
      descriptionPrompt,
      Validators::StringLengthValidator(
          1, 1000, "Description must be between 1 and 1000 characters"));

  std::stringstream statusPrompt;
  statusPrompt << "Issue Status:\n";

  for (int i = 1; i < 5; i++) {
    statusPrompt << i << ": " << IssueStatus[i - 1] << std::endl;
  }

  statusPrompt << "Choose a status: ";
  int selection = PromptUser(
      statusPrompt,
      Validators::RangeValidator(1, 4, "Please select a valid option"));

  // Get the current user
  User currentUser = ClientAppManager::CurrentUser();

  // Validate that the User we want to assign the issue to exists
  string assignToPrompt = "Assign issue to: ";
  string assignedToId = UserView::GetUserIdFromName(assignToPrompt);
  while (assignedToId.empty()) {
    std::cout << "\nCouldn't find a user by that name. Please try again\n";
    assignedToId = UserView::GetUserIdFromName(assignToPrompt);
  }

  // Validate that the Reporter we want to assign exists
  string reporterPrompt = "Reporter: ";
  string reporterId = UserView::GetUserIdFromName(reporterPrompt);
  while (reporterId.empty()) {
    std::cout << "\nCouldn't find a user by that name. Please try again\n";
    reporterId = UserView::GetUserIdFromName(reporterPrompt);
  }

  json newComment;

  StringMap issue = {{"title", title},
                     {"createdBy", currentUser.id},
                     {"assignedTo", assignedToId},
                     {"description", description},
                     {"reporter", reporterId},
                     {"status", IssueStatus[selection - 1]}};

  auto request =
      RequestUtilities::CreatePostRequest(_serverUri, "/issues", issue);
  auto response = restbed::Http::sync(request);

  json issueResponse = ResponseUtilities::HandleResponse(response);
  if (response->get_status_code() == restbed::CREATED) {
    std::cout << std::endl << "Issue successfully created!" << std::endl;
  } else {
    std::cout << "Could not create the Issue. Server responded with: "
              << issueResponse.dump() << std::endl;
  }
  std::cout << std::endl;
}

Issue IssueView::UpdateIssueView(const Issue& issue) {
  std::cout << std::endl << "Update issue" << std::endl;

  DisplayIssue(issue);
  std::cout << std::endl;

  string titlePrompt = "Issue title (leave blank to keep the same): ";
  string title = PromptUser(
      titlePrompt, Validators::StringLengthValidator(
                       0, 256, "Title must be between at most 256 characters"));

  std::stringstream statusPrompt;
  statusPrompt << "Issue Status:\n";

  for (int i = 1; i < 5; i++) {
    statusPrompt << i << ": " << IssueStatus[i - 1] << std::endl;
  }

  statusPrompt << "Choose a status: ";
  int selection = PromptUser(
      statusPrompt,
      Validators::RangeValidator(1, 4, "Please select a valid option"));

  string assignToPrompt = "Assign issue to: ";
  string assignedToId = UserView::GetUserIdFromName(assignToPrompt);
  while (assignedToId.empty()) {
    std::cout << "\nCouldn't find a user by that name. Please try again\n";
    assignedToId = UserView::GetUserIdFromName(assignToPrompt);
  }

  Issue updatedIssue = issue;
  if (!title.empty()) {
    updatedIssue.title = title;
  }
  updatedIssue.status = IssueStatus[selection - 1];
  updatedIssue.assignedTo.id = assignedToId;
  updatedIssue.updatedBy = ClientAppManager::CurrentUser();

  auto request =
      RequestUtilities::CreatePutRequest(_serverUri, "/issues/" + issue.id);
  json updatedIssueJson = updatedIssue;
  string body = updatedIssueJson.dump();
  std::cout << std::endl << body << std::endl;
  request->set_body(body);
  request->set_header("Content-Length", std::to_string(body.length()));

  auto response = restbed::Http::sync(request);
  json issueResponse = ResponseUtilities::HandleResponse(response);
  if (response->get_status_code() == restbed::OK) {
    Issue updatedIssue = issueResponse.get<Issue>();
    std::cout << std::endl << "Issue successfully updated!" << std::endl;
    return updatedIssue;
  } else {
    std::cout << std::endl
              << "Could not update the Issue. Server responded with: "
              << issueResponse.dump() << std::endl;
    return issue;
  }
}

bool IssueView::DeleteIssueView(const Issue& issue) {
  std::cout << std::endl << "Delete issue" << std::endl;

  string prompt = "Are you sure you want to delete this issue? (y/n): ";
  auto validator = Validator<string>(
      [](string input) { return input == "y" || input == "n"; },
      "Please enter y or n");
  string confirm = PromptUser(prompt, validator);
  if (confirm == "n") {
    return false;
  }

  auto request =
      RequestUtilities::CreateDeleteRequest(_serverUri, "/issues/" + issue.id);
  auto response = restbed::Http::sync(request);
  if (response->get_status_code() < 400) {
    std::cout << std::endl << "Issue successfully delete" << std::endl;
    return true;
  } else {
    json responseJson = ResponseUtilities::HandleResponse(response);
    std::cout << std::endl
              << "Unable to delete the issue. Server responsded with: "
              << responseJson.dump() << std::endl;
    return false;
  }
}

void IssueView::AddCommentView(const Issue& issue) {
  std::cout << std::endl << "Add a comment" << std::endl;

  string commentBodyPrompt = "Enter the body of the comment: ";
  string body = PromptUser(
      commentBodyPrompt,
      Validators::StringLengthValidator(
          1, 1000, "Comment must be between 1 and 1000 characters long"));

  StringMap comment = {{"body", body},
                       {"createdBy", ClientAppManager::CurrentUser().id},
                       {"issueId", issue.id}};
  auto request =
      RequestUtilities::CreatePostRequest(_serverUri, "/comments", comment);
  auto response = restbed::Http::sync(request);
  std::cout << std::endl;
  if (response->get_status_code() == restbed::CREATED) {
    std::cout << std::endl << "Comment successfully created" << std::endl;
  } else {
    json responseJson = ResponseUtilities::HandleResponse(response);
    std::cout << std::endl
              << "Unable to create your comment. Server responded with: "
              << responseJson.dump();
  }
  std::cout << std::endl;
}

Issue IssueView::ChangeVoteView(const Issue& issue) {
  std::cout << std::endl << "Change vote" << std::endl;

  auto request = RequestUtilities::CreateGetRequest(
      _serverUri, "/issues/" + issue.id + "/votes");
  StringMap query = {{"createdBy", ClientAppManager::CurrentUser().id}};
  request->set_query_parameters(query);

  auto response = restbed::Http::sync(request);
  if (response->get_status_code() == restbed::OK) {
    json responseJson = ResponseUtilities::HandleResponse(response);
    auto votes = responseJson.get<std::vector<Vote>>();

    string prompt;
    auto validator = Validator<string>(
        [](string input) { return input == "y" || input == "n"; },
        "Please enter y or n");
    if (votes.size() == 1) {
      prompt = "Remove your vote from this issue? (y/n): ";
    } else {
      prompt = "Vote for this issue? (y/n): ";
    }
    string confirm = PromptUser(prompt, validator);

    if (confirm == "y") {
      StringMap vote = {{"issueId", issue.id},
                        {"createdBy", ClientAppManager::CurrentUser().id}};
      request = RequestUtilities::CreatePostRequest(
          _serverUri, "/issues/" + issue.id + "/votes", vote);
      response = restbed::Http::sync(request);
      bool updated = false;
      if (response->get_status_code() == restbed::CREATED) {
        std::cout << std::endl << "Your vote has been cast" << std::endl;
        updated = true;
      } else if (response->get_status_code() == restbed::NO_CONTENT) {
        std::cout << std::endl << "Your vote has been removed" << std::endl;
        updated = true;
      } else {
        json responseJson = ResponseUtilities::HandleResponse(response);
        std::cout << std::endl
                  << "Unable to create your comment. Server responded with: "
                  << responseJson.dump();
      }
      if (updated) {
        request = RequestUtilities::CreateGetRequest(_serverUri,
                                                     "/issues/" + issue.id);
        response = restbed::Http::sync(request);
        if (response->get_status_code() == restbed::OK) {
          json updatedIssueJson = ResponseUtilities::HandleResponse(response);
          Issue updatedIssue = updatedIssueJson.get<Issue>();
          return updatedIssue;
        }
      }
    }
  }
  return issue;
}

std::string IssueView::DisplayIssueBrief(const Issue& issue) {
  User reporter = GetUserById(issue.reporter.id);
  std::stringstream issueStream;
  issueStream << "Title: " << issue.title.substr(0, 50);
  if (issue.title.size() > 50) issueStream << "...";
  issueStream << ", Reported By: " << reporter.name << ", Created on "
              << TimeUtilities::ConvertTimeToString(issue.createdAt);
  if (issue.comments.size() > 0) {
    issueStream << " (" << issue.comments.size() << " comments)";
  }
  return issueStream.str();
}

void IssueView::DisplayIssue(const Issue& issue) {
  std::cout << std::endl << "Issue details\n";

  // Display the title
  std::cout << "Title: " << issue.title << std::endl;

  // Display the reporter and timestamp
  User reporter = GetUserById(issue.reporter.id);
  std::cout << "\u2516 Reported by: " << reporter.name << " on "
            << TimeUtilities::ConvertTimeToString(issue.createdAt) << std::endl;

  if (!issue.assignedTo.id.empty()) {
    User assignedTo = GetUserById(issue.assignedTo.id);
    std::cout << "\u2516 Assigned to: " << assignedTo.name << std::endl;
  }

  // If updated, display updated information
  if (!issue.updatedBy.id.empty()) {
    User updater = GetUserById(issue.updatedBy.id);
    std::cout << "\u2516 Updated by: " << updater.name << " on "
              << TimeUtilities::ConvertTimeToString(issue.updatedAt)
              << std::endl;
  }

  if (issue.votes.size() > 0) {
    std::cout << DisplayVotesForIssue(issue) << std::endl;
  }

  if (issue.comments.size() > 0) {
    std::cout << "\u2516 Comments:" << std::endl;
    std::cout << DisplayComments(issue.comments);
  }
}
