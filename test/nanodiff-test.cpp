#include <algorithm>
#include <filesystem>
#include <fstream>
#include <vector>

#include <gtest/gtest.h>

#include "../nanodiff.h"

namespace {
const std::filesystem::path test_res_dir{"test_resources"};

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
  EXPECT_TRUE(diffs.empty()
              || std::ranges::all_of(diffs, [](const auto& line) { return line.type == diff_line_type::context; }));
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
}  // namespace
