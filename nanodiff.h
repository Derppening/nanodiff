#ifndef NANODIFF_H
#define NANODIFF_H

#include <cstdint>

#include <fstream>
#include <functional>
#include <optional>
#include <string>
#include <string_view>

// IMPORTANT: The members of this header must be kept in sync with `nanodiff.cpp`!!

struct command_line_args {
  std::optional<std::string> expected{std::nullopt};
  std::optional<std::string> actual{std::nullopt};
  // TODO(Derppening): Add option for hiding expected/actual file paths
  // TODO(Derppening): Add option for hiding context lines
  // TODO(Derppening): Add option for custom exit code
  // TODO(Derppening): Add diff options supported by ZINC
  // TODO(Derppening): Add option for treating missing file as empty
};

enum struct diff_line_type : std::uint8_t {
  context,
  expected_only,
  actual_only,
};

struct diff_line {
  std::string_view line;
  diff_line_type type;
};

using diff_line_cb = std::function<void(const diff_line& line)>;

auto diff_file_stdout_eager(std::ifstream expected, std::ifstream actual, const diff_line_cb& line_callback) -> bool;
auto diff_file_stdout(std::ifstream expected, std::ifstream actual, const diff_line_cb& line_callback) -> bool;

#endif  // NANODIFF_H
