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
}  // namespace

auto main(int argc, char** argv) -> int {
  std::vector<std::string> args{argv, argv + argc};

  auto cmd_args_or_err = parse_cmdline(args);
  if (!cmd_args_or_err) {
    auto err = cmd_args_or_err.error();
    std::print("Error while parsing command-line arguments: {}\n", err);
    return 1;
  }

  auto cmd_args = *cmd_args_or_err;

  std::vector<std::string> expected_absent{};
  std::vector<std::string> actual_absent{};

  std::filesystem::path expected_file{*cmd_args.expected};
  std::filesystem::path actual_file{*cmd_args.actual};

  std::ifstream expected{expected_file};
  std::ifstream actual{actual_file};

  if (!expected) {
    std::print(stderr, "Unable to open file '{}'", expected_file.string());
    return EXIT_FAILURE;
  }
  if (!actual) {
    std::print(stderr, "Unable to open file '{}'", actual_file.string());
    return EXIT_FAILURE;
  }
}
