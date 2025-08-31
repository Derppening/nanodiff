#include <algorithm>
#include <filesystem>
#include <fstream>
#include <vector>

#include <gtest/gtest.h>

#include "../nanodiff.h"

namespace {
const std::filesystem::path test_res_dir{"test_resources"};

struct line_count {
  std::size_t context = 0;
  std::size_t expected_only = 0;
  std::size_t actual_only = 0;
};

auto count_lines(const std::vector<diff_line>& diffs) -> line_count {
  return std::ranges::fold_left(diffs, line_count{}, [](line_count lc, const diff_line& diff) {
    switch (diff.type) {
      case diff_line_type::context:
        ++lc.context;
        break;
      case diff_line_type::expected_only:
        ++lc.expected_only;
        break;
      case diff_line_type::actual_only:
        ++lc.actual_only;
        break;
    }
    return lc;
  });
}

TEST(EagerDiffTest, SameOutput) {
  const auto expected_path = test_res_dir / "testcase_same_output-expected.txt";
  const auto actual_path = test_res_dir / "testcase_same_output-actual.txt";

  std::ifstream expected_file{expected_path};
  ASSERT_TRUE(expected_file) << "Failed to open file: " << expected_path;
  std::ifstream actual_file{actual_path};
  ASSERT_TRUE(actual_file) << "Failed to open file: " << actual_path;

  std::vector<diff_line> diffs{};
  const auto has_diff = diff_file_stdout_eager(std::move(expected_file), std::move(actual_file),
                                               [&diffs](const diff_line& line) { diffs.push_back(line); });
  EXPECT_FALSE(has_diff);

  // TODO(Derppening): Add test suite for full context
  const auto line_count{count_lines(diffs)};
  EXPECT_EQ(0, line_count.context);
  EXPECT_EQ(0, line_count.expected_only);
  EXPECT_EQ(0, line_count.actual_only);
}

TEST(EagerDiffTest, OneLineChanged) {
  const auto expected_path = test_res_dir / "testcase_one_line_changed-expected.txt";
  const auto actual_path = test_res_dir / "testcase_one_line_changed-actual.txt";

  std::ifstream expected_file{expected_path};
  ASSERT_TRUE(expected_file) << "Failed to open file: " << expected_path;
  std::ifstream actual_file{actual_path};
  ASSERT_TRUE(actual_file) << "Failed to open file: " << actual_path;

  std::vector<diff_line> diffs{};
  const auto has_diff = diff_file_stdout_eager(std::move(expected_file), std::move(actual_file),
                                               [&diffs](const diff_line& line) { diffs.push_back(line); });
  EXPECT_TRUE(has_diff);

  const auto line_count{count_lines(diffs)};
  EXPECT_EQ(3, line_count.context);
  EXPECT_EQ(1, line_count.expected_only);
  EXPECT_EQ(1, line_count.actual_only);
}

TEST(EagerDiffTest, LineAdded) {
  const auto expected_path = test_res_dir / "testcase_line_added-expected.txt";
  const auto actual_path = test_res_dir / "testcase_line_added-actual.txt";

  std::ifstream expected_file{expected_path};
  ASSERT_TRUE(expected_file) << "Failed to open file: " << expected_path;
  std::ifstream actual_file{actual_path};
  ASSERT_TRUE(actual_file) << "Failed to open file: " << actual_path;

  std::vector<diff_line> diffs{};
  const auto has_diff = diff_file_stdout_eager(std::move(expected_file), std::move(actual_file),
                                               [&diffs](const diff_line& line) { diffs.push_back(line); });
  EXPECT_TRUE(has_diff);

  const auto line_count{count_lines(diffs)};
  EXPECT_EQ(4, line_count.context);
  EXPECT_EQ(0, line_count.expected_only);
  EXPECT_EQ(1, line_count.actual_only);
}

TEST(EagerDiffTest, LineRemoved) {
  const auto expected_path = test_res_dir / "testcase_line_removed-expected.txt";
  const auto actual_path = test_res_dir / "testcase_line_removed-actual.txt";

  std::ifstream expected_file{expected_path};
  ASSERT_TRUE(expected_file) << "Failed to open file: " << expected_path;
  std::ifstream actual_file{actual_path};
  ASSERT_TRUE(actual_file) << "Failed to open file: " << actual_path;

  std::vector<diff_line> diffs{};
  const auto has_diff = diff_file_stdout_eager(std::move(expected_file), std::move(actual_file),
                                               [&diffs](const diff_line& line) { diffs.push_back(line); });
  EXPECT_TRUE(has_diff);

  const auto line_count{count_lines(diffs)};
  EXPECT_EQ(4, line_count.context);
  EXPECT_EQ(1, line_count.expected_only);
  EXPECT_EQ(0, line_count.actual_only);
}

TEST(EagerDiffTest, CompletelyDifferent) {
  const auto expected_path = test_res_dir / "testcase_completely_different-expected.txt";
  const auto actual_path = test_res_dir / "testcase_completely_different-actual.txt";

  std::ifstream expected_file{expected_path};
  ASSERT_TRUE(expected_file) << "Failed to open file: " << expected_path;
  std::ifstream actual_file{actual_path};
  ASSERT_TRUE(actual_file) << "Failed to open file: " << actual_path;

  std::vector<diff_line> diffs{};
  const auto has_diff = diff_file_stdout_eager(std::move(expected_file), std::move(actual_file),
                                               [&diffs](const diff_line& line) { diffs.push_back(line); });
  EXPECT_TRUE(has_diff);

  const auto line_count{count_lines(diffs)};
  EXPECT_EQ(1, line_count.context);
  EXPECT_EQ(5, line_count.expected_only);
  EXPECT_EQ(5, line_count.actual_only);
}

TEST(EagerDiffTest, EmptyFiles) {
  const auto expected_path = test_res_dir / "testcase_empty-expected.txt";
  const auto actual_path = test_res_dir / "testcase_empty-actual.txt";

  std::ifstream expected_file{expected_path};
  ASSERT_TRUE(expected_file) << "Failed to open file: " << expected_path;
  std::ifstream actual_file{actual_path};
  ASSERT_TRUE(actual_file) << "Failed to open file: " << actual_path;

  std::vector<diff_line> diffs{};
  const auto has_diff = diff_file_stdout_eager(std::move(expected_file), std::move(actual_file),
                                               [&diffs](const diff_line& line) { diffs.push_back(line); });
  EXPECT_FALSE(has_diff);

  const auto line_count{count_lines(diffs)};
  EXPECT_EQ(0, line_count.context);
  EXPECT_EQ(0, line_count.expected_only);
  EXPECT_EQ(0, line_count.actual_only);
}

TEST(LazyDiffTest, SameOutput) {
  const auto expected_path = test_res_dir / "testcase_same_output-expected.txt";
  const auto actual_path = test_res_dir / "testcase_same_output-actual.txt";

  std::ifstream expected_file{expected_path};
  ASSERT_TRUE(expected_file) << "Failed to open file: " << expected_path;
  std::ifstream actual_file{actual_path};
  ASSERT_TRUE(actual_file) << "Failed to open file: " << actual_path;

  std::vector<diff_line> diffs{};
  const auto has_diff = diff_file_stdout(std::move(expected_file), std::move(actual_file),
                                         [&diffs](const diff_line& line) { diffs.push_back(line); });
  EXPECT_FALSE(has_diff);
  EXPECT_TRUE(diffs.empty()
              || std::ranges::all_of(diffs, [](const auto& line) { return line.type == diff_line_type::context; }));
}

TEST(LazyDiffTest, OneLineChanged) {
  const auto expected_path = test_res_dir / "testcase_one_line_changed-expected.txt";
  const auto actual_path = test_res_dir / "testcase_one_line_changed-actual.txt";

  std::ifstream expected_file{expected_path};
  ASSERT_TRUE(expected_file) << "Failed to open file: " << expected_path;
  std::ifstream actual_file{actual_path};
  ASSERT_TRUE(actual_file) << "Failed to open file: " << actual_path;

  std::vector<diff_line> diffs{};
  const auto has_diff = diff_file_stdout(std::move(expected_file), std::move(actual_file),
                                         [&diffs](const diff_line& line) { diffs.push_back(line); });
  EXPECT_TRUE(has_diff);

  const auto line_count{count_lines(diffs)};
  EXPECT_EQ(3, line_count.context);
  EXPECT_EQ(1, line_count.expected_only);
  EXPECT_EQ(1, line_count.actual_only);
}

TEST(LazyDiffTest, LineAdded) {
  const auto expected_path = test_res_dir / "testcase_line_added-expected.txt";
  const auto actual_path = test_res_dir / "testcase_line_added-actual.txt";

  std::ifstream expected_file{expected_path};
  ASSERT_TRUE(expected_file) << "Failed to open file: " << expected_path;
  std::ifstream actual_file{actual_path};
  ASSERT_TRUE(actual_file) << "Failed to open file: " << actual_path;

  std::vector<diff_line> diffs{};
  const auto has_diff = diff_file_stdout(std::move(expected_file), std::move(actual_file),
                                         [&diffs](const diff_line& line) { diffs.push_back(line); });
  EXPECT_TRUE(has_diff);
  EXPECT_TRUE(std::ranges::any_of(diffs, [](const auto& line) { return line.type != diff_line_type::context; }));
}

TEST(LazyDiffTest, LineRemoved) {
  const auto expected_path = test_res_dir / "testcase_line_removed-expected.txt";
  const auto actual_path = test_res_dir / "testcase_line_removed-actual.txt";

  std::ifstream expected_file{expected_path};
  ASSERT_TRUE(expected_file) << "Failed to open file: " << expected_path;
  std::ifstream actual_file{actual_path};
  ASSERT_TRUE(actual_file) << "Failed to open file: " << actual_path;

  std::vector<diff_line> diffs{};
  const auto has_diff = diff_file_stdout(std::move(expected_file), std::move(actual_file),
                                         [&diffs](const diff_line& line) { diffs.push_back(line); });
  EXPECT_TRUE(has_diff);

  const auto line_count{count_lines(diffs)};
  EXPECT_EQ(4, line_count.context);
  EXPECT_EQ(1, line_count.expected_only);
  EXPECT_EQ(0, line_count.actual_only);
}

TEST(LazyDiffTest, CompletelyDifferent) {
  const auto expected_path = test_res_dir / "testcase_completely_different-expected.txt";
  const auto actual_path = test_res_dir / "testcase_completely_different-actual.txt";

  std::ifstream expected_file{expected_path};
  ASSERT_TRUE(expected_file) << "Failed to open file: " << expected_path;
  std::ifstream actual_file{actual_path};
  ASSERT_TRUE(actual_file) << "Failed to open file: " << actual_path;

  std::vector<diff_line> diffs{};
  const auto has_diff = diff_file_stdout(std::move(expected_file), std::move(actual_file),
                                         [&diffs](const diff_line& line) { diffs.push_back(line); });
  EXPECT_TRUE(has_diff);

  const auto line_count{count_lines(diffs)};
  EXPECT_EQ(1, line_count.context);
  EXPECT_EQ(5, line_count.expected_only);
  EXPECT_EQ(5, line_count.actual_only);
}

TEST(LazyDiffTest, EmptyFiles) {
  const auto expected_path = test_res_dir / "testcase_empty-expected.txt";
  const auto actual_path = test_res_dir / "testcase_empty-actual.txt";

  std::ifstream expected_file{expected_path};
  ASSERT_TRUE(expected_file) << "Failed to open file: " << expected_path;
  std::ifstream actual_file{actual_path};
  ASSERT_TRUE(actual_file) << "Failed to open file: " << actual_path;

  std::vector<diff_line> diffs{};
  const auto has_diff = diff_file_stdout(std::move(expected_file), std::move(actual_file),
                                         [&diffs](const diff_line& line) { diffs.push_back(line); });
  EXPECT_FALSE(has_diff);

  const auto line_count{count_lines(diffs)};
  EXPECT_EQ(0, line_count.context);
  EXPECT_EQ(0, line_count.expected_only);
  EXPECT_EQ(0, line_count.actual_only);
}
}  // namespace
