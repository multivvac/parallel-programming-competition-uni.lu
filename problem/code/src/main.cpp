#include <iomanip>   // for std::setprecision, std::fixed
#include <map>
#include "client.hpp"
#include "helper.hpp"
#include "cmd.hpp"

int main(int argc, char** argv) {
  try {
    Cmd cmd = Cmd::parse(argc, argv);
    if (cmd.mode == Cmd::Mode::Debug) {
      const std::map<int, std::pair<int,int>> l2result = {
          {1,  {287, 144}},
          {12, {2193,2327}},
      };

      Grid art_data;
      Grid pattern_data;

      art_data = parse_grid_from_bin_file("./data/art_" + std::to_string(cmd.level) + ".bin", 1);
      pattern_data = parse_grid_from_bin_file("./data/pattern_" + std::to_string(cmd.level) + ".bin", 1);

      const auto it = l2result.find(cmd.level);
      if (it == l2result.end()) {
          std::cerr << "\033[1;31m[ERROR]\033[0m No expected result for level "
                    << cmd.level << ".\n";
          return 1;
      }
      const auto expected_pair = it->second;
      const std::string expected_str = pair2str(expected_pair);

      std::pair<int, int> result;
      const double result_perf = benchmark([&](){
          result = compute(art_data.data, art_data.w(),
                          pattern_data.data, pattern_data.w());
      }, 1);

      if (result != expected_pair) {
          std::cerr << "\033[1;31m[ERROR]\033[0m Wrong result! Expected "
                    << expected_str << " but got '" << pair2str(result) << "'.\n";
          return 1;
      }

      std::cout << "\033[1;32m[SUCCESS]\033[0m "
                << "Result = " << pair2str(result)
                << " | Time = " << std::fixed << std::setprecision(6) << result_perf << " s\n";

    } else {
      boost::asio::io_context io_context;
      tcp::resolver resolver(io_context);
      auto endpoints = resolver.resolve(cmd.host, std::to_string(cmd.port));

      tcp::socket socket(io_context);
      boost::asio::connect(socket, endpoints);
      std::cout << "[Client] Connected to server " << cmd.host << ":" << cmd.port << ".\n";

      start_computation(socket);
    }
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
