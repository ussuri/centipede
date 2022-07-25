// Copyright 2022 The Centipede Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef THIRD_PARTY_CENTIPEDE_COMMAND_H_
#define THIRD_PARTY_CENTIPEDE_COMMAND_H_

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace centipede {
class Command final {
 public:
  // Move-constructible only.
  Command(const Command& other) = delete;
  Command& operator=(const Command& other) = delete;
  Command& operator=(Command&& other) = delete;

  // Move constructor, ensures the moved-from object doesn't own the pipes.
  // TODO(kcc): [impl] add a test, other than multi_sanitizer_test.sh, for this.
  Command(Command&& other)
      : path_(std::move(other.path_)),
        args_(std::move(other.args_)),
        env_(std::move(other.env_)),
        out_(std::move(other.out_)),
        err_(std::move(other.err_)),
        full_command_string_(std::move(other.full_command_string_)),
        fifo_path_{std::move(other.fifo_path_[0]),
                   std::move(other.fifo_path_[1])},
        pipe_{other.pipe_[0], other.pipe_[1]} {
    // If we don't do this, the moved-from object will close these pipes.
    other.pipe_[0] = -1;
    other.pipe_[1] = -1;
  }

  // Constructs a command:
  // `path`: path to the binary.
  // `args`: arguments.
  // `env`: environment variables/values (in the form "KEY=VALUE").
  // `out`: stdout redirect path (empty means none).
  // `err`: stderr redirect path (empty means none).
  // If `out` == `err` and both are non-empty, stdout/stderr are combined.
  Command(std::string_view path, const std::vector<std::string> &args = {},
          const std::vector<std::string> &env = {}, std::string_view out = "",
          std::string_view err = "")
      : path_(path), args_(args), env_(env), out_(out), err_(err) {}

  // Cleans up the fork server, if that was created.
  ~Command();

  // Returns a string representing the command, e.g. like this
  // "ENV1=VAL1 path arg1 arg2 > out 2>& err"
  std::string ToString() const;
  // Executes the command, returns the exit status.
  // Can be called more than once.
  // If iterrrupted, may call RequestEarlyExit().
  int Execute();

  // Attempts to start a fork server, returns true on success.
  // Pipe files for the fork server are created in `temp_dir_path`
  // with prefix `prefix`.
  // See runner_fork_server.cc for details.
  bool StartForkServer(std::string_view temp_dir_path, std::string_view prefix);

  // Accessors.
  const std::string& path() const { return path_; }

 private:
  const std::string path_;
  const std::vector<std::string> args_;
  const std::vector<std::string> env_;
  const std::string out_;
  const std::string err_;
  std::string full_command_string_ = ToString();
  // Pipe paths and file descriptors for the fork server.
  std::string fifo_path_[2];
  int pipe_[2] = {-1, -1};
};

}  // namespace centipede

#endif  // THIRD_PARTY_CENTIPEDE_COMMAND_H_
