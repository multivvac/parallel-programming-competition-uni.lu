#pragma once
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#define HOST "iris-069"
#define PORT "8013"

struct Cmd {
  enum class Mode { Debug, Client };

  Mode mode;
  // Debug fields
  std::string art_path;
  std::string pattern_path;
  uint16_t level{0};

  // Client fields
  std::string host;
  uint16_t port{0};

  static constexpr std::string_view kUsage =
      "Usage:\n"
      "  DEBUG:  ppc --debug <level> \n"
      "  CLIENT: ppc <hostname> <port>\n"
      "          (or set PPC_HOSTNAME and PPC_PORT)\n";

  static Cmd parse(int argc, char** argv) {
    // if (argc < 2) {
    //   throw std::runtime_error(std::string(kUsage));
    // }

    std::vector<std::string> args;
    args.reserve(static_cast<size_t>(argc - 1));
    for (int i = 1; i < argc; ++i) args.emplace_back(argv[i]);

    if (!args.empty() && (args[0] == "--debug" || args[0] == "-d")) {
      if (args.size() < 2) {
        throw std::runtime_error(
            std::string(kUsage));
      }
      if (args[1] != "1" && args[1] != "12") {
        throw std::runtime_error(
            "DEBUG mode only support level 1(for validation) and level 12(for performance)\n"
            "e.g. ppc --debug 12 or ppc --debug 1\n" +
            std::string(kUsage));
      }
      Cmd cmd;
      cmd.mode = Cmd::Mode::Debug;
      cmd.level = args[1] == "1" ? 1 : 12;
      return cmd;
    }

    // CLIENT MODE
    Cmd cmd;
    cmd.mode = Mode::Client;

    // Try positional first
    std::optional<std::string> host_arg;
    std::optional<std::string> port_arg;

    if (args.size() >= 1) host_arg = args[0];
    if (args.size() >= 2) port_arg = args[1];

    // Fallback to env if missing
    if (!host_arg) {
      if (const char* env = std::getenv("PPC_HOSTNAME")) host_arg = std::string(env);
      else host_arg = std::string(HOST);
    }
    if (!port_arg) {
      if (const char* env = std::getenv("PPC_PORT")) port_arg = std::string(env);
      else port_arg = std::string(PORT);
    }

    if (!host_arg || !port_arg || port_arg->empty() || host_arg->empty()) {
      throw std::runtime_error(
          "CLIENT mode requires <hostname> and <port>, or PPC_HOSTNAME and PPC_PORT.\n" +
          std::string(kUsage));
    }

    // Validate and store
    cmd.host = *host_arg;
    cmd.port = parse_port_or_throw(*port_arg);
    return cmd;
  }

private:
  static uint16_t parse_port_or_throw(const std::string& s) {
    if (s.empty()) throw std::runtime_error("Empty port string.\n" + std::string(kUsage));
    for (unsigned char c : s) {
      if (c < '0' || c > '9') {
        throw std::runtime_error("Port must be a positive integer: '" + s + "'\n" + std::string(kUsage));
      }
    }
    try {
      unsigned long v = std::stoul(s);
      if (v == 0 || v > 65535) {
        throw std::runtime_error("Port out of range (1..65535): '" + s + "'\n" + std::string(kUsage));
      }
      return static_cast<uint16_t>(v);
    } catch (const std::exception&) {
      throw std::runtime_error("Invalid port: '" + s + "'\n" + std::string(kUsage));
    }
  }
};
