#include "VoteView.h"

#include <restbed>

#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "Vote.h"
#include "Utilities.h"
#include "nlohmann/json.hpp"

using nlohmann::json;

using std::string;

VoteViewState VoteView::MainMenu() {
  int selection;

  std::map<int, string> options = {{VoteViewState::CreateVote, "Create a vote"},
                                   {VoteViewState::GetVotes, "Get all votes"},
                                   {VoteViewState::GetVote, "Get a vote"},
                                   {VoteViewState::DeleteVote, "Delete a vote"},
                                   {0, "Exit"}};

  std::stringstream prompt;
  prompt << "Please select from the following options:\n";
  for (auto option : options) {
    prompt << option.first << ": " << option.second << std::endl;
  }
  prompt << "What would you like to do: ";

  selection = PromptUser(
      prompt.str(),
      Validators::RangeValidator(0, static_cast<int>(VoteViewState::DeleteVote),
                                 "Please select a valid option"));

  return static_cast<VoteViewState>(selection);
}

string VoteView::CreateVoteView() {
  string issueId;

  string issuePrompt = "Please enter the id of the issue you want to vote on: ";

  issueId = PromptUser(issuePrompt, Validators::IdValidator());

  StringMap vote = {{"issueId", issueId}};

  auto request =
      RequestUtilities::CreatePostRequest(_serverUri, "/votes", vote);
  auto response = restbed::Http::sync(request);

  json voteResponse = ResponseUtilities::HandleResponse(response);

  if (response->get_status_code() == restbed::CREATED) {
    std::cout << "Newly created vote: \n";
    for (auto& it : voteResponse.items()) {
      std::cout << it.key() << ": " << it.value() << ", ";
    }
    std::cout << "\n\n";
  }

  return issueId;
}


void VoteView::GetVotesView() {
  auto request = RequestUtilities::CreateGetRequest(_serverUri, "/votes");
  auto response = restbed::Http::sync(request);
  json votes = ResponseUtilities::HandleResponse(response);

  if (votes.empty()) {
    std::cout << "No votes to display\n\n";
  } else {
    std::cout << "All votes: " << std::endl;
    for (auto vote : votes) {
      json u = json(vote);
      for (auto& attribute : u.items()) {
        std::cout << attribute.key() << ": " << attribute.value() << ", ";
      }
      std::cout << std::endl;
    }
  }
}

string VoteView::GetVoteView() {
  string id;

  string idPrompt = "Please enter the id of the vote you want to retrieve: ";
  id = PromptUser(idPrompt, Validators::IdValidator());

  auto request = RequestUtilities::CreateGetRequest(_serverUri, "/votes/" + id);
  auto response = restbed::Http::sync(request);

  json voteResponse = ResponseUtilities::HandleResponse(response);

  if (response->get_status_code() == restbed::OK) {
    std::cout << "The vote you requested: \n";
    for (auto& it : voteResponse.items()) {
      std::cout << it.key() << ": " << it.value() << ", ";
    }
    std::cout << "\n\n";
  }

  return id;
}

string VoteView::DeleteVoteView() {
  string id;

  string idPrompt = "Please enter the id of the vote you want to delete: ";
  id = PromptUser(idPrompt, Validators::IdValidator());

  auto request =
      RequestUtilities::CreateDeleteRequest(_serverUri, "/votes/" + id);
  auto response = restbed::Http::sync(request);

  json voteResponse = ResponseUtilities::HandleResponse(response);
  if (response->get_status_code() == restbed::OK) {
    std::cout << "Vote successfully deleted!\n";
  }

  return id;
}
