// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

#include "src/torque/ast.h"
#include "src/torque/utils.h"

namespace v8 {
namespace internal {
namespace torque {

std::string StringLiteralUnquote(const std::string& s) {
  DCHECK(('"' == s.front() && '"' == s.back()) ||
         ('\'' == s.front() && '\'' == s.back()));
  std::stringstream result;
  for (size_t i = 1; i < s.length() - 1; ++i) {
    if (s[i] == '\\') {
      switch (s[++i]) {
        case 'n':
          result << '\n';
          break;
        case 'r':
          result << '\r';
          break;
        case 't':
          result << '\t';
          break;
        case '\'':
        case '"':
        case '\\':
          result << s[i];
          break;
        default:
          UNREACHABLE();
      }
    } else {
      result << s[i];
    }
  }
  return result.str();
}

std::string StringLiteralQuote(const std::string& s) {
  std::stringstream result;
  result << '"';
  for (size_t i = 0; i < s.length() - 1; ++i) {
    switch (s[i]) {
      case '\n':
        result << "\\n";
        break;
      case '\r':
        result << "\\r";
        break;
      case '\t':
        result << "\\t";
        break;
      case '\'':
      case '"':
      case '\\':
        result << "\\" << s[i];
        break;
      default:
        result << s[i];
    }
  }
  result << '"';
  return result.str();
}

std::string CurrentPositionAsString() {
  return PositionAsString(CurrentSourcePosition::Get());
}

DEFINE_CONTEXTUAL_VARIABLE(LintErrorStatus)

[[noreturn]] void ReportError(const std::string& error) {
  std::cerr << CurrentPositionAsString() << ": Torque error: " << error << "\n";
  std::abort();
}

void LintError(const std::string& error) {
  LintErrorStatus::SetLintError();
  std::cerr << CurrentPositionAsString() << ": Lint error: " << error << "\n";
}

namespace {

bool ContainsUnderscore(const std::string& s) {
  if (s.empty()) return false;
  return s.find("_") != std::string::npos;
}

bool ContainsUpperCase(const std::string& s) {
  if (s.empty()) return false;
  return std::any_of(s.begin(), s.end(), [](char c) { return isupper(c); });
}

}  // namespace

bool IsLowerCamelCase(const std::string& s) {
  if (s.empty()) return false;
  return islower(s[0]) && !ContainsUnderscore(s);
}

bool IsUpperCamelCase(const std::string& s) {
  if (s.empty()) return false;
  return isupper(s[0]) && !ContainsUnderscore(s);
}

bool IsSnakeCase(const std::string& s) {
  if (s.empty()) return false;
  return !ContainsUpperCase(s);
}

std::string CamelifyString(const std::string& underscore_string) {
  std::string result;
  bool word_beginning = true;
  for (auto current : underscore_string) {
    if (current == '_' || current == '-') {
      word_beginning = true;
      continue;
    }
    if (word_beginning) {
      current = toupper(current);
    }
    result += current;
    word_beginning = false;
  }
  return result;
}

std::string DashifyString(const std::string& underscore_string) {
  std::string result = underscore_string;
  std::replace(result.begin(), result.end(), '_', '-');
  return result;
}

void ReplaceFileContentsIfDifferent(const std::string& file_path,
                                    const std::string& contents) {
  std::ifstream old_contents_stream(file_path.c_str());
  std::string old_contents;
  if (old_contents_stream.good()) {
    std::istreambuf_iterator<char> eos;
    old_contents =
        std::string(std::istreambuf_iterator<char>(old_contents_stream), eos);
    old_contents_stream.close();
  }
  if (old_contents.length() == 0 || old_contents != contents) {
    std::ofstream new_contents_stream;
    new_contents_stream.open(file_path.c_str());
    new_contents_stream << contents;
    new_contents_stream.close();
  }
}

}  // namespace torque
}  // namespace internal
}  // namespace v8
