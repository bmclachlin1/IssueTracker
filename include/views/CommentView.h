#ifndef CommentVIEW_H
#define CommentVIEW_H

#include <string>

#include "BaseView.h"
#include "Comment.h"

enum CommentViewState {
  CreateComment = 1,
  UpdateComment,
  GetComments,
  GetComment,
  DeleteComment
};

/**
 * @class CommentView
 * @brief Handles user input relating to comments
 */
class CommentView : public BaseView {
 public:
  CommentView() {}
  virtual ~CommentView() {}

  /**
   * Displays a menu of choices for the user to pick from
   * @returns the integer representing their choice
   */
  CommentViewState MainMenu();

  /**
   * Prompts the user for information to create a new user
   * @return the comment's inputted values
   */
  std::string CreateCommentView();

  /**
   * Prompts the user for information to update a user
   * @return the comment's input
   */
  std::string UpdateCommentView();

  /**
   * Prompts the user for information to retrieve a user
   * @return the comment's login credentials
   */
  std::string GetCommentView();

  /**
   * Displays all the Comments in the system to the user
   */
  void GetCommentsView();

  /**
   * Prompts the user for the information required to delete a Comment from the
   * system
   * @return std::string the comment's input
   */
  std::string DeleteCommentView();

  inline void RunCommentView(CommentViewState choice) {
    bool quit = false;
    while (!quit) {
      switch (choice) {
        case CommentViewState::CreateComment:
          CreateCommentView();
          break;
        case CommentViewState::UpdateComment:
          UpdateCommentView();
          break;
        case CommentViewState::GetComment:
          GetCommentView();
          break;
        case CommentViewState::GetComments:
          GetCommentsView();
          break;
        case CommentViewState::DeleteComment:
          DeleteCommentView();
          break;
        default:
          quit = true;
          break;
      }
      if (!quit) choice = MainMenu();
    }
  }

  static std::string DisplayComment(const Comment& comment);
};

#endif  // CommentVIEW_H
