#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <boost/asio.hpp>
#include <charconv>
#include "cmd.hpp"
#include "json.hpp" // nlohmann/json library
#include "base.hpp"
#include "helper.hpp"

using boost::asio::ip::tcp;
using json = nlohmann::json;
namespace fs = std::filesystem;
// Ensure at least `n` bytes are available in `buf`.
// Returns with `ec` set on error.
static void ensure_bytes(tcp::socket& sock,
                         boost::asio::streambuf& buf,
                         std::size_t n,
                         boost::system::error_code& ec)
{
    while (!ec && buf.size() < n) {
        // Read at least one byte; avoid huge reads to keep latency low
        boost::asio::read(sock, buf, boost::asio::transfer_at_least(1), ec);
    }
}

std::string getUpgradeText(int level) {
    if (level >= 1 && level <= 7) {
        switch (level) {
            case 1: return "🌱 A tiny spark ignites. Lv.1 reached!";
            case 2: return "💡 You feel a flicker of energy. Lv.2!";
            case 3: return "🌿 Growth begins... Lv.3 unlocked!";
            case 4: return "✨ Your steps shine brighter. Lv.4!";
            case 5: return "🔥 A small flame awakens. Lv.5!";
            case 6: return "⚔️ Your grip tightens with strength. Lv.6!";
            case 7: return "🛡️ A new aura surrounds you. Lv.7!";
        }
    } else if (level >= 8 && level <= 14) {
        switch (level) {
            case 8: return "🔥 Your spirit blazes brighter! Lv.8!";
            case 9: return "⚡ Energy surges through your veins. Lv.9!";
            case 10: return "🌟 Power awakens in full bloom! Lv.10!";
            case 11: return "💥 You strike harder than ever! Lv.11!";
            case 12: return "🧭 Destiny calls your name. Lv.12!";
            case 13: return "🎯 Precision sharpens your skills. Lv.13!";
            case 14: return "⚔️🔥 An unstoppable warrior rises! Lv.14!";
        }
    } else if (level >= 15 && level <= 20) {
        switch (level) {
            case 15: return "⚡⚔️💎 Lv.15 – Your aura dominates!";
            case 16: return "🔥🌪️ Storms obey your command. Lv.16!";
            case 17: return "🛡️⚔️ Nothing can pierce your defense. Lv.17!";
            case 18: return "⚡⚔️💎 Lv.18 – You radiate unstoppable force!";
            case 19: return "🌌🔥 The battlefield bends to you. Lv.19!";
            case 20: return "👑⚔️ Supreme champion crowned! Lv.20!";
        }
    } else if (level == 21) {
        return "🌌✨ LEGENDARY ASCENSION! ✨🌌\n"
               "👑 Lv.2 – You are no longer playing the game...\n"
               "    The game now plays you.";
    } else if (level > 21) {
        return "👑 Lv." + std::to_string(level) + " - I can't believe you really did it" +  "...\n";
    } else {
        return "❓ Invalid level.";
    }
    return "";
}

void receive_file(tcp::socket& socket,
                  boost::asio::streambuf& rx,   // <-- persistent buffer
                  boost::system::error_code& ec,
                  Grid& buffer)
{
    // 1) header size (4 bytes)
    ensure_bytes(socket, rx, 4, ec);
    if (ec) return;

    // Copy 4 bytes from rx into a uint32_t
    uint32_t header_size_net = 0;
    {
        auto seq = rx.data(); // may be split; copy safely
        std::array<char,4> tmp{};
        auto it = boost::asio::buffers_begin(seq);
        for (int i=0;i<4;++i) { tmp[i] = *(it++); }
        rx.consume(4);
        std::memcpy(&header_size_net, tmp.data(), 4);
    }
    uint32_t header_size = ntohl(header_size_net);
    if (header_size == 0 || header_size > 4096) {
        throw std::runtime_error("invalid header size");
    }

    // 2) header JSON
    ensure_bytes(socket, rx, header_size, ec);
    if (ec) return;

    std::string header_str(header_size, '\0');
    {
        std::istream is(&rx);               // consumes from rx
        is.read(header_str.data(), header_size);
    }

    std::string filename;
    std::size_t file_size = 0;
    try {
        json j = json::parse(header_str);
        filename  = j.at("filename").get<std::string>();
        file_size = j.at("filesize").get<std::size_t>();
    } catch (const json::exception& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        ec = boost::asio::error::invalid_argument;
        return;
    }

    std::cout << "[Client] Receiving '" << filename << "' (" << file_size << " bytes).\n";

    // 3) file payload
    std::stringstream text_content_stream;
    std::size_t remaining = file_size;

    while (remaining > 0 && !ec) {
        // Make sure at least 1 byte is available; then consume up to remaining
        ensure_bytes(socket, rx, 1, ec);
        if (ec) break;

        std::size_t chunk = std::min<std::size_t>(rx.size(), remaining);
        std::vector<char> tmp(chunk);

        {
            std::istream is(&rx);
            is.read(tmp.data(), static_cast<std::streamsize>(chunk));
        }
        text_content_stream.write(tmp.data(), static_cast<std::streamsize>(chunk));
        remaining -= chunk;
    }
    if (ec) return;

    buffer.clear();
    text_content_stream.seekg(0);
    buffer = parse_grid_from_bin(text_content_stream, 1);
    if (buffer.height == 0 || buffer.width == 0) {
        std::cerr << "No data parsed from stream.\n";
        ec = boost::asio::error::invalid_argument;
        return;
    }
}

void wait_for_server_reply(tcp::socket& socket, boost::asio::streambuf& rx) {
    boost::system::error_code ec;
    boost::asio::read_until(socket, rx, '\n', ec);
    if (ec) {
        std::cerr << "[Client] Error reading server reply: " << ec.message() << "\n";
        return;
    }

    std::istream is(&rx);  // consumes only the line, leaves any over-read bytes in rx
    std::string msg;
    std::getline(is, msg);

    if (msg == "TIMEOUT") {
        std::cout << "[Client] TIMED OUT, TOO SLOW 😛. Exiting.\n";
        socket.close();
    } else if (msg == "WRONG") {
        std::cout << "[Client] WRONG RESULT 🥲, DEBUG!. Exiting.\n";
        socket.close();
    } else if (msg == "OK") {
        std::cout << "[Client] Advancing...\n";
    } else {
        std::cout << "[Client] Unknown reply: " << msg << "\n";
        socket.close();
    }
}

void start_computation(tcp::socket& socket) {
    boost::asio::streambuf rx;        // persistent receive buffer
    boost::system::error_code ec;
    int level = 0;
    while (true) {
        std::cout << "------------------------------------------\n";
        if (level > 0) std::cout << getUpgradeText(level) << "\n";
        std::cout << "[Client] Waiting for next level's data.\n";

        Grid artData;
        receive_file(socket, rx, ec, artData);
        if (ec) { /* handle & break */ break; }
        std::cout << "[Client] Receive art data from server.\n";

        Grid patternData;
        receive_file(socket, rx, ec, patternData);
        if (ec) { /* handle & break */ break; }
        std::cout << "[Client] Receive pattern data from server.\n";

        // handshake + answer
        std::cout << "[Client] Acknowledge server start computing.\n";
        boost::asio::write(socket, boost::asio::buffer(std::string("parsed\n")));
        std::cout << "[Client] Computing start.\n";
        std::pair<int, int> result;
            result = compute(artData.data, artData.w(),
                                            patternData.data, patternData.w());

        std::string line = pair2str(result) + "\n";
        std::cout << "[Client] Computing finish, get result: " << line;
        std::cout << "------------------------------------------\n";
        boost::asio::write(socket, boost::asio::buffer(line), ec);
        if (ec) { /* handle & break */ break; }

        // This may over-read; that's fine because rx is persistent.
        wait_for_server_reply(socket, rx);
        if (!socket.is_open()) break;
        level++;
    }
    // After loop, check why we exited
    if (ec) {
        if (ec == boost::asio::error::eof) {
            std::cout << "[Client] Server closed the connection.\n";
        } else {
            std::cerr << "[Client] Connection error: " << ec.message() << "\n";
        }
    } else if (!socket.is_open()) {
        std::cout << "[Client] Socket closed by server.\n";
    } else {
        std::cout << "[Client] Exiting computation loop cleanly.\n";
    }
}

