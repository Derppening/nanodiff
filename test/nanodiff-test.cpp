#include <cstdlib>
#include <algorithm>
#include <filesystem>
#include <format>
#include <fstream>
#include <optional>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

#include "../nanodiff.h"

using std::literals::operator""sv;

namespace {
const std::filesystem::path test_res_dir{"test_resources"};
const std::string_view stdout_tmpfile_name{".nanodiff-test.stdout"sv};
const std::string_view stderr_tmpfile_name{".nanodiff-test.stderr"sv};

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

#if defined(__linux__)

struct exec_output {
  int exit_code;
  std::string stdout;
  std::string stderr;
};

class PorcelainStdoutTest : public testing::Test {
 protected:
  void SetUp() override {
    remove_tmpfile_if_exists(stdout_path);
    remove_tmpfile_if_exists(stderr_path);
  }

  // NOLINTBEGIN(concurrency-mt-unsafe)
  static auto run_cmd(const std::filesystem::path& expected_path,
                      const std::filesystem::path& actual_path,
                      const std::string_view args = ""sv) -> exec_output {
    std::filesystem::path exec_path{};
    PorcelainStdoutTest::exec_path(exec_path);

    const auto cmd = std::format("{} {} -- {} {} >{} 2>{}", std::string{exec_path}, args, std::string{expected_path},
                                 std::string{actual_path}, std::string{stdout_path}, std::string{stderr_path});
    const auto exit_code = std::system(cmd.c_str());

    std::string stdout{};
    read_to_string(stdout_path, stdout);

    std::string stderr{};
    read_to_string(stderr_path, stderr);

    return exec_output{
        .exit_code = exit_code,
        .stdout = std::move(stdout),
        .stderr = std::move(stderr),
    };
  }
  // NOLINTEND(concurrency-mt-unsafe)

  void TearDown() override {
    remove_tmpfile_if_exists(stdout_path);
    remove_tmpfile_if_exists(stderr_path);
  }

  static void SetUpTestSuite() {
    const auto exec_path{std::filesystem::absolute(std::filesystem::current_path() / ".." / "nanodiff")};

    _exec_path = std::filesystem::is_regular_file(exec_path) ? std::make_optional(exec_path) : std::nullopt;

    const auto tmp_path{std::filesystem::temp_directory_path()};
    stdout_path = std::filesystem::absolute(tmp_path / stdout_tmpfile_name);
    stderr_path = std::filesystem::absolute(tmp_path / stderr_tmpfile_name);

    remove_tmpfile_if_exists(stdout_path);
    remove_tmpfile_if_exists(stderr_path);
  }

  static void exec_path(std::filesystem::path& exec_path) {
    if (!_exec_path) {
      GTEST_SKIP() << "Test requires `nanodiff` executable to be compiled";
    }
    exec_path = *_exec_path;
  }

  static void read_to_string(const std::filesystem::path& path, std::string& str) {
    if (!std::filesystem::exists(path)) {
      str.clear();
      return;
    }

    const auto size = std::filesystem::file_size(path);
    str.resize(size);

    std::ifstream ifs{path};
    ASSERT_TRUE(ifs) << "Unable to open " << path << " for reading";

    ifs.read(str.data(), static_cast<std::streamsize>(size));
  }

  static std::filesystem::path stdout_path;
  static std::filesystem::path stderr_path;

 private:
  static void remove_tmpfile_if_exists(const std::filesystem::path& path) {
    if (std::filesystem::exists(path)) {
      ASSERT_TRUE(std::filesystem::is_regular_file(path))
          << path << "already exists as a directory and needs to be removed manually";
      ASSERT_TRUE(std::filesystem::remove(path)) << "Failed to remove stdout temporary file " << path;
    }
  }

  static std::optional<std::filesystem::path> _exec_path;
};

std::optional<std::filesystem::path> PorcelainStdoutTest::_exec_path{};
std::filesystem::path PorcelainStdoutTest::stdout_path{};
std::filesystem::path PorcelainStdoutTest::stderr_path{};

TEST_F(PorcelainStdoutTest, SameOutput) {
  const auto expected_path = test_res_dir / "testcase_same_output-expected.txt";
  const auto actual_path = test_res_dir / "testcase_same_output-actual.txt";

  const auto exec_result = PorcelainStdoutTest::run_cmd(expected_path, actual_path);
  EXPECT_EQ(exec_result.exit_code, 0);

  EXPECT_EQ(exec_result.stdout, R"()"sv);
  EXPECT_EQ(exec_result.stderr, R"()"sv);
}

TEST_F(PorcelainStdoutTest, OneLineChanged) {
  const auto expected_path = test_res_dir / "testcase_one_line_changed-expected.txt";
  const auto actual_path = test_res_dir / "testcase_one_line_changed-actual.txt";

  const auto exec_result = PorcelainStdoutTest::run_cmd(expected_path, actual_path);
  EXPECT_NE(exec_result.exit_code, 0);

  EXPECT_EQ(exec_result.stdout, R"(-3
+X
 4
 5

)"sv);
  EXPECT_EQ(exec_result.stderr, R"()"sv);
}

TEST_F(PorcelainStdoutTest, LineAdded) {
  const auto expected_path = test_res_dir / "testcase_line_added-expected.txt";
  const auto actual_path = test_res_dir / "testcase_line_added-actual.txt";

  const auto exec_result = PorcelainStdoutTest::run_cmd(expected_path, actual_path);
  EXPECT_NE(exec_result.exit_code, 0);

  EXPECT_EQ(exec_result.stdout, R"(+extra line
 4
 5
 6

)"sv);
  EXPECT_EQ(exec_result.stderr, R"()"sv);
}

TEST_F(PorcelainStdoutTest, LineRemoved) {
  const auto expected_path = test_res_dir / "testcase_line_removed-expected.txt";
  const auto actual_path = test_res_dir / "testcase_line_removed-actual.txt";

  const auto exec_result = PorcelainStdoutTest::run_cmd(expected_path, actual_path);
  EXPECT_NE(exec_result.exit_code, 0);

  EXPECT_EQ(exec_result.stdout, R"(-extra line
 4
 5
 6

)"sv);
  EXPECT_EQ(exec_result.stderr, R"()"sv);
}

TEST_F(PorcelainStdoutTest, CompletelyDifferent) {
  const auto expected_path = test_res_dir / "testcase_completely_different-expected.txt";
  const auto actual_path = test_res_dir / "testcase_completely_different-actual.txt";

  const auto exec_result = PorcelainStdoutTest::run_cmd(expected_path, actual_path);
  EXPECT_NE(exec_result.exit_code, 0);

  EXPECT_EQ(exec_result.stdout, R"(-A
-B
-C
-D
-E
+Apple
+Banana
+Carrot
+Dog
+Eggplant

)"sv);
  EXPECT_EQ(exec_result.stderr, R"()"sv);
}

TEST_F(PorcelainStdoutTest, EmptyFiles) {
  const auto expected_path = test_res_dir / "testcase_empty-expected.txt";
  const auto actual_path = test_res_dir / "testcase_empty-actual.txt";

  const auto exec_result = PorcelainStdoutTest::run_cmd(expected_path, actual_path);
  EXPECT_EQ(exec_result.exit_code, 0);

  EXPECT_EQ(exec_result.stdout, R"()"sv);
  EXPECT_EQ(exec_result.stderr, R"()"sv);
}

#endif  // defined(__linux__)
}  // namespace
