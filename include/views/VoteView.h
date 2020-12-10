
#ifndef VOTEVIEW_H
#define VOTEVIEW_H

#include <string>

#include "BaseView.h"

enum VoteViewState { CreateVote = 1, GetVotes, GetVote, DeleteVote };

/**
 * @class VoteView
 * @brief Handles user input relating to votes
 */
class VoteView : public BaseView {
 public:
  VoteView() {}
  virtual ~VoteView() {}

  /**
   * Displays a menu of choices for the user to pick from
   * @returns the integer representing their choice
   */
  VoteViewState MainMenu();

  /**
   * Prompts the user for information to create a new user
   * @return the vote's inputted values
   */
  std::string CreateVoteView();

  /**
   * Prompts the user for information to retrieve a user
   * @return the vote's login credentials
   */
  std::string GetVoteView();

  /**
   * Displays all the Votes in the system to the user
   */
  void GetVotesView();

  /**
   * Prompts the user for the information required to delete a Vote from the
   * system
   * @return std::string the vote's input
   */
  std::string DeleteVoteView();

  inline void RunVoteView(VoteViewState choice) {
    bool quit = false;
    while (!quit) {
      switch (choice) {
        case VoteViewState::CreateVote:
          CreateVoteView();
          break;
        case VoteViewState::GetVote:
          GetVoteView();
          break;
        case VoteViewState::GetVotes:
          GetVotesView();
          break;
        case VoteViewState::DeleteVote:
          DeleteVoteView();
          break;
        default:
          quit = true;
          break;
      }
      if (!quit) choice = MainMenu();
    }
  }
};

#endif  // VOTEVIEW_H
