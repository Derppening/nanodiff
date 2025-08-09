#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

namespace {
const std::filesystem::path test_res_dir{"test_resources"};

TEST(SampleTest, AlwaysTrue) {
  EXPECT_TRUE(true);
}

TEST(SampleTest, ReadFile) {
  auto path = test_res_dir / "testcase_same_output-expected.txt";
  EXPECT_TRUE(std::filesystem::exists(path)) << "Expected file does not exist: " << path;

  std::ifstream file{path};
  EXPECT_TRUE(file.is_open()) << "Failed to open file: " << path;
}
}  // namespace
