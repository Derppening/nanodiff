#include <cassert>
#include <cstdint>
#include <cstdlib>

#include <expected>
#include <filesystem>
#include <fstream>
#include <optional>
#include <print>
#include <string>
#include <vector>

namespace {
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
  std::string line;
  diff_line_type type;
};

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

// TODO(Derppening): Refactor this into accepting function pointers on different cases
auto diff_file_stdout(std::ifstream expected, std::ifstream actual) -> bool {
  bool has_diff{};
  std::vector<std::string> expected_absent{};
  std::vector<std::string> actual_absent{};

  auto output_diff = [&expected_absent, &actual_absent]() {
    for (const auto& l : actual_absent) {
      std::print("-{}\n", l);
    }
    actual_absent.clear();

    for (const auto& l : expected_absent) {
      std::print("+{}\n", l);
    }
    expected_absent.clear();
  };

  while (expected || actual) {
    std::optional<std::string> expected_line{};
    std::optional<std::string> actual_line{};

    if (expected) {
      expected_line = std::make_optional<std::string>();
      std::getline(expected, *expected_line);
    }
    if (actual) {
      actual_line = std::make_optional<std::string>();
      std::getline(expected, *actual_line);
    }

    if (expected_line && actual_line) {
      if (*expected_line == *actual_line) {
        output_diff();
        std::print(" {}\n", *expected_line);
      } else {
        actual_absent.emplace_back(*expected_line);
        expected_absent.emplace_back(*actual_line);
      }
    } else if (expected_line) {
      actual_absent.emplace_back(*expected_line);
    } else if (actual_line) {
      expected_absent.emplace_back(*actual_line);
    } else {
      assert(false);
    }
  }

  output_diff();

  return has_diff;
}
}  // namespace

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

  bool has_diff = diff_file_stdout(std::move(expected), std::move(actual));

  if (has_diff) {
    return EXIT_FAILURE;
  }
}
