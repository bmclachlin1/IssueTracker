#ifndef ISSUE_VIEW_H
#define ISSUE_VIEW_H

#include <string>

#include "BaseView.h"
#include "Comment.h"
#include "Issue.h"
#include "Utilities.h"
#include "Vote.h"

enum class IssueViewState {
  MainMenu = 1,
  CreateIssue,
  ManageOpenIssues,
  ManageAssignedIssues,
  ManageAllIssues,
  GoBack,
};

enum class ManageIssueState {
  UpdateIssue = 1,
  DeleteIssue,
  AddComment,
  UpdateComment,
  DeleteComment,
  ChangeVote,
  GoBack,
};

/**
 * @class IssueView
 * @brief Handles user input related to viewing and manipulating Issue objects
 */
class IssueView : public BaseView {
 public:
  IssueView() {}
  virtual ~IssueView() {}

  IssueViewState MainMenu();
  ManageIssueState ManageIssueView();

  Issue IssuesView(const StringMap& query);

  void CreateIssueView();
  Issue UpdateIssueView(const Issue& issue);
  bool DeleteIssueView(const Issue& issue);
  void AddCommentView(const Issue& issue);
  void UpdateCommentView(const Issue& issue, const Comment& comment);
  void DeleteCommentView(const Issue& issue, const Comment& comment);
  Issue ChangeVoteView(const Issue& issue);

  void RunIssueView(IssueViewState choice);

  static std::string DisplayIssueBrief(const Issue& issue);

  /**
   * Displays an issue in a nice, formatted manner
   */
  static void DisplayIssue(const Issue& issue);
};

#endif  // ISSUE_VIEW_H
