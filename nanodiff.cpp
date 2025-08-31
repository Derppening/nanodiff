#include <cassert>
#include <cstdint>
#include <cstdlib>

#include <algorithm>
#include <expected>
#include <filesystem>
#include <fstream>
#include <functional>
#include <optional>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#ifdef NANODIFF_TEST
#include "nanodiff.h"
#else

namespace {
/**
 * @brief Command line arguments structure for the diff tool.
 */
struct command_line_args {
  std::optional<std::string> expected{std::nullopt};
  std::optional<std::string> actual{std::nullopt};
  // TODO(Derppening): Add option for hiding expected/actual file paths
  // TODO(Derppening): Add option for hiding context lines
  // TODO(Derppening): Add option for custom exit code
  // TODO(Derppening): Add diff options supported by ZINC
  // TODO(Derppening): Add option for treating missing file as empty
};

/**
 * @brief Enum representing the type of a diff line.
 */
enum struct diff_line_type : std::uint8_t {
  context,
  expected_only,
  actual_only,
};

/**
 * @brief Structure representing a line in the diff output.
 */
struct diff_line {
  std::string_view line;
  diff_line_type type;
};

/**
 * @brief Callback type for processing diff lines.
 */
using diff_line_cb = std::function<void(const diff_line& line)>;
}  // namespace

#endif  // NANODIFF_TEST

namespace {
/**
 * @brief Result type for command line argument parsing.
 */
using arg_parse_result = std::expected<command_line_args, std::string>;

/**
 * @brief Parses the command line arguments and returns a structure containing the parsed options.
 */
auto parse_cmdline(const std::vector<std::string>& args) -> arg_parse_result;

/**
 * @brief Validates the parsed command line arguments.
 */
auto validate_args(const command_line_args& args) -> arg_parse_result;

/**
 * @brief Validates that the given path exists, and converts it into a canonical, absolute path.
 */
auto normalize_path(const std::string& path_str) -> std::expected<std::filesystem::path, std::string>;

auto parse_cmdline(const std::vector<std::string>& args) -> arg_parse_result {
  command_line_args cmd_args{};

  bool parse_options = true;

  for (auto it = args.begin(); it != args.end(); ++it) {
    // "--" delimits between options and filenames
    if (*it == "--") {
      parse_options = false;
      continue;
    }

    if (parse_options) {
      // no options yet
    } else {
      if (!cmd_args.expected) {
        cmd_args.expected = std::make_optional(*it);
      } else if (!cmd_args.actual) {
        cmd_args.actual = std::make_optional(*it);
      } else {
        return std::unexpected{"Too many arguments"};
      }
    }
  }

  return validate_args(cmd_args);
}

auto validate_args(const command_line_args& args) -> arg_parse_result {
  if (!args.expected) {
    return std::unexpected{"Missing argument for path to expected output"};
  }
  if (!args.actual) {
    return std::unexpected{"Missing argument for path to actual output"};
  }

  return args;
}

auto normalize_path(const std::string& path_str) -> std::expected<std::filesystem::path, std::string> {
  std::filesystem::path path{path_str};

  if (!std::filesystem::exists(path)) {
    return std::unexpected{std::format("'{}': File not found", path_str)};
  }

  if (!std::filesystem::is_regular_file(path)) {
    return std::unexpected{std::format("'{}': Not a file", path_str)};
  }

  return std::filesystem::canonical(path);
}
}  // namespace

#ifndef NANODIFF_TEST
namespace {
#endif

/**
 * @brief Compares two files line by line and outputs the by the @code line_callback @endcode function.
 *
 * This is the reference implementation of the diff algorithm, which eagerly reads both files into memory
 * and compares them line-by-line.
 */
auto diff_file_stdout_eager(std::ifstream expected, std::ifstream actual, const diff_line_cb& line_callback) -> bool {
  bool has_diff{};

  std::vector<std::string> expected_content{};
  std::vector<std::string> actual_content{};

  while (expected) {
    std::string line{};
    std::getline(expected, line);
    expected_content.emplace_back(std::move(line));
  }
  while (actual) {
    std::string line{};
    std::getline(actual, line);
    actual_content.emplace_back(std::move(line));
  }

  // `+`
  std::vector<std::string> only_actual{};

  auto output_diff = [&line_callback, &has_diff, &only_actual]() {
    has_diff |= !only_actual.empty();

    std::ranges::for_each(only_actual, [&line_callback](const auto& l) {
      line_callback(diff_line{.line = l, .type = diff_line_type::actual_only});
    });
    only_actual.clear();
  };

  auto actual_it = actual_content.begin();
  for (auto expected_it = expected_content.begin();
       expected_it != expected_content.end() && actual_it != actual_content.end();) {
    if (const auto it = std::ranges::find(actual_content, *expected_it); it != actual_content.end()) {
      const auto nlines = std::distance(actual_content.begin(), it);

      std::ranges::move(actual_content | std::views::take(nlines), std::back_inserter(only_actual));

      output_diff();
      if (has_diff) {
        line_callback(diff_line{.line = *it, .type = diff_line_type::context});
      }

      // Erase all lines up to and including the matching line from `actual_buffer`
      actual_it = actual_content.erase(actual_content.begin(), it + 1);
    } else {
      has_diff = true;
      line_callback(diff_line{.line = *expected_it, .type = diff_line_type::expected_only});
    }

    expected_it = expected_content.erase(expected_content.begin());
  }

  if (!expected_content.empty()) {
    has_diff = true;
    std::ranges::for_each(expected_content, [&line_callback](const auto& l) {
      line_callback(diff_line{.line = l, .type = diff_line_type::actual_only});
    });
  }
  expected_content.clear();

  if (actual_it != actual_content.end()) {
    has_diff = true;
    std::ranges::for_each(actual_it, actual_content.end(), [&line_callback](const auto& l) {
      line_callback(diff_line{.line = l, .type = diff_line_type::actual_only});
    });
  }
  actual_content.clear();

  return has_diff;
}

/**
 * @brief Compares two files line by line and outputs the by the @code line_callback @endcode function.
 *
 * This diff algorithm is derived from @code diff_file_stdout_eager @endcode, but it uses a lazy approach by lazily
 * looking ahead in the actual file and buffering lines until a match is found in the expected file.
 */
auto diff_file_stdout(std::ifstream expected, std::ifstream actual, const diff_line_cb& line_callback) -> bool {
  bool has_diff{};

  // `+`
  std::vector<std::string> only_actual{};

  auto output_diff = [&line_callback, &has_diff, &only_actual]() {
    has_diff |= !only_actual.empty();

    std::ranges::for_each(only_actual, [&line_callback](const auto& l) {
      line_callback(diff_line{.line = l, .type = diff_line_type::actual_only});
    });
    only_actual.clear();
  };

  // !! Vector of strings containing all lines that are not present in the expected file up to a given point
  std::vector<std::string> actual_buffer;

  while (expected) {
    std::string expected_line{};
    std::getline(expected, expected_line);

    auto matching_actual_it = std::ranges::find(actual_buffer, expected_line);
    while (matching_actual_it == actual_buffer.end()) {
      std::string actual_line{};
      std::getline(actual, actual_line);

      actual_buffer.push_back(std::move(actual_line));

      // matching_actual_it = std::ranges::find(actual_buffer.end() - 1, actual_buffer.end(), expected_line);
      matching_actual_it = std::ranges::find(actual_buffer, expected_line);
    }

    if (matching_actual_it != actual_buffer.end()) {
      // We found a matching line in the actual buffer
      const auto nlines = std::distance(actual_buffer.begin(), matching_actual_it);

      std::ranges::move(actual_buffer | std::views::take(nlines), std::back_inserter(only_actual));

      output_diff();
      if (has_diff) {
        line_callback(diff_line{.line = expected_line, .type = diff_line_type::context});
      }

      // Erase all lines up to and including the matching line from `actual_buffer`
      actual_buffer.erase(actual_buffer.begin(), matching_actual_it + 1);
    } else {
      has_diff = true;
      line_callback(diff_line{.line = expected_line, .type = diff_line_type::expected_only});
    }
  }

  std::ranges::for_each(actual_buffer, [&line_callback](const auto& l) {
    line_callback(diff_line{.line = l, .type = diff_line_type::actual_only});
  });
  actual_buffer.clear();

  while (actual) {
    std::string line{};
    std::getline(actual, line);

    line_callback(diff_line{.line = line, .type = diff_line_type::actual_only});
  }

  return has_diff;
}

#ifndef NANODIFF_TEST
}  // namespace
#endif  // NANODIFF_TEST

#ifndef NANODIFF_TEST

auto main(int argc, char** argv) -> int {
  std::vector<std::string> args{argv, argv + argc};

  auto cmd_args_or_err = parse_cmdline(args);
  if (!cmd_args_or_err) {
    auto err = cmd_args_or_err.error();
    std::print("Error while parsing command-line arguments: {}\n", err);
    return 1;
  }

  const auto& cmd_args = *cmd_args_or_err;

  const auto expected_path_or_err = normalize_path(*cmd_args.expected);
  if (!expected_path_or_err) {
    std::print(stderr, "{}\n", expected_path_or_err.error());
  }
  const auto actual_path_or_err = normalize_path(*cmd_args.actual);
  if (!actual_path_or_err) {
    std::print(stderr, "{}\n", actual_path_or_err.error());
  }

  const auto& expected_path = *expected_path_or_err;
  const auto& actual_path = *actual_path_or_err;

  std::ifstream expected{*expected_path_or_err};
  std::ifstream actual{*actual_path_or_err};

  if (!expected) {
    std::print(stderr, "Unable to open file '{}'\n", expected_path.string());
    return EXIT_FAILURE;
  }
  if (!actual) {
    std::print(stderr, "Unable to open file '{}'\n", actual_path.string());
    return EXIT_FAILURE;
  }

  bool has_diff = diff_file_stdout_eager(std::move(expected), std::move(actual), [](const auto& diff_line) {
    char prefix = ' ';
    switch (diff_line.type) {
      case diff_line_type::context:
        prefix = ' ';
        break;
      case diff_line_type::expected_only:
        prefix = '-';
        break;
      case diff_line_type::actual_only:
        prefix = '+';
        break;
      default:
        assert(false);
    }

    std::print("{}{}\n", prefix, diff_line.line);
  });

  if (has_diff) {
    return EXIT_FAILURE;
  }
}

#endif  // NANODIFF_TEST
