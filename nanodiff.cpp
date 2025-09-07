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
  // TODO(Derppening): Add option for showing/hiding all context lines
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

class file_differ {
 public:
  file_differ() = default;
  file_differ(const file_differ&) = default;
  file_differ(file_differ&&) noexcept = default;

  virtual ~file_differ() = default;

  auto operator=(const file_differ&) -> file_differ& = default;
  auto operator=(file_differ&&) noexcept -> file_differ& = default;

  virtual auto do_diff(const diff_line_cb& line_callback) -> bool {
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

    auto expected_line{read_expected_line()};
    while (expected_line) {
      auto matching_actual_it = std::ranges::find(actual_buffer, expected_line);
      while (matching_actual_it == actual_buffer.end()) {
        auto actual_line = read_actual_line();
        if (!actual_line) {
          break;
        }

        actual_buffer.push_back(std::move(*actual_line));

        matching_actual_it = std::ranges::find(actual_buffer.end() - 1, actual_buffer.end(), expected_line);
      }

      if (matching_actual_it != actual_buffer.end()) {
        // We found a matching line in the actual buffer
        const auto nlines = std::distance(actual_buffer.begin(), matching_actual_it);

        std::ranges::move(actual_buffer | std::views::take(nlines), std::back_inserter(only_actual));

        output_diff();
        if (has_diff) {
          line_callback(diff_line{.line = *expected_line, .type = diff_line_type::context});
        }

        // Erase all lines up to and including the matching line from `actual_buffer`
        actual_buffer.erase(actual_buffer.begin(), matching_actual_it + 1);
      } else {
        has_diff = true;
        line_callback(diff_line{.line = *expected_line, .type = diff_line_type::expected_only});
      }

      expected_line = read_expected_line();
    };

    std::ranges::for_each(actual_buffer, [&line_callback](const auto& l) {
      line_callback(diff_line{.line = l, .type = diff_line_type::actual_only});
    });
    actual_buffer.clear();

    auto actual_line{read_actual_line()};
    while (actual_line) {
      line_callback(diff_line{.line = *actual_line, .type = diff_line_type::actual_only});

      actual_line = read_actual_line();
    }

    return has_diff;
  }

 protected:
  virtual auto read_expected_line() -> std::optional<std::string> = 0;
  virtual auto read_actual_line() -> std::optional<std::string> = 0;
};

class eager_file_differ final : public file_differ {
 public:
  eager_file_differ(const eager_file_differ&) = delete;
  eager_file_differ(eager_file_differ&&) noexcept = default;

  ~eager_file_differ() override = default;

  auto operator=(const eager_file_differ&) -> eager_file_differ& = delete;
  auto operator=(eager_file_differ&&) noexcept -> eager_file_differ& = default;

  eager_file_differ(std::ifstream expected, std::ifstream actual) {
    while (expected) {
      std::string line{};
      std::getline(expected, line);
      _expected_content.emplace_back(std::move(line));
    }
    _expected_it = _expected_content.cbegin();

    while (actual) {
      std::string line{};
      std::getline(actual, line);
      _actual_content.emplace_back(std::move(line));
    }
    _actual_it = _actual_content.cbegin();
  }

 private:
  auto read_expected_line() -> std::optional<std::string> override {
    if (_expected_it == _expected_content.cend()) {
      return std::nullopt;
    }
    return std::make_optional(*_expected_it++);
  }
  auto read_actual_line() -> std::optional<std::string> override {
    if (_actual_it == _actual_content.cend()) {
      return std::nullopt;
    }
    return std::make_optional(*_actual_it++);
  }

  std::vector<std::string> _expected_content;
  decltype(_expected_content)::const_iterator _expected_it;
  std::vector<std::string> _actual_content;
  decltype(_actual_content)::const_iterator _actual_it;
};

class lazy_file_differ final : public file_differ {
 public:
  lazy_file_differ(const lazy_file_differ&) = delete;
  lazy_file_differ(lazy_file_differ&&) noexcept = default;

  ~lazy_file_differ() override = default;

  auto operator=(const lazy_file_differ&) -> lazy_file_differ& = delete;
  auto operator=(lazy_file_differ&&) noexcept -> lazy_file_differ& = default;

  lazy_file_differ(std::ifstream expected, std::ifstream actual) :
      _expected{std::move(expected)}, _actual{std::move(actual)} {}

 private:
  auto read_expected_line() -> std::optional<std::string> override {
    std::optional<std::string> line{};

    if (_expected) {
      line = std::string{};
      std::getline(_expected, *line);
    }

    return line;
  }
  auto read_actual_line() -> std::optional<std::string> override {
    std::optional<std::string> line{};

    if (_actual) {
      line = std::string{};
      std::getline(_actual, *line);
    }

    return line;
  }

  std::ifstream _expected;
  std::ifstream _actual;
};

/**
 * @brief Compares two files line by line and outputs the by the @code line_callback @endcode function.
 *
 * This is the reference implementation of the diff algorithm, which eagerly reads both files into memory
 * and compares them line-by-line.
 */
auto diff_file_stdout_eager(std::ifstream expected, std::ifstream actual, const diff_line_cb& line_callback) -> bool {
  eager_file_differ differ{std::move(expected), std::move(actual)};

  return differ.do_diff(line_callback);
}

/**
 * @brief Compares two files line by line and outputs the by the @code line_callback @endcode function.
 *
 * This diff algorithm is derived from @code diff_file_stdout_eager @endcode, but it uses a lazy approach by lazily
 * looking ahead in the actual file and buffering lines until a match is found in the expected file.
 */
auto diff_file_stdout(std::ifstream expected, std::ifstream actual, const diff_line_cb& line_callback) -> bool {
  lazy_file_differ differ{std::move(expected), std::move(actual)};

  return differ.do_diff(line_callback);
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
    return EXIT_FAILURE;
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
