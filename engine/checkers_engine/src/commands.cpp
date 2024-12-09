#include "commands.hpp"

#include <optional>
#include <algorithm>
#include <iterator>
#include <cstddef>

#include "error.hpp"

namespace commands {
    static bool token_available(const std::vector<std::string>& tokens, std::size_t index) noexcept {
        return index < tokens.size();
    }

    static std::vector<std::string>::const_iterator token_available(const std::vector<std::string>& tokens, const std::string& token) noexcept {
        return std::find(tokens.cbegin(), tokens.cend(), token);
    }

    static std::vector<std::string>::const_iterator next_token(std::vector<std::string>::const_iterator iter) noexcept {
        return std::next(iter);
    }

    static std::optional<std::vector<std::string>> parse_setup_moves(const std::vector<std::string>& tokens) {
        std::size_t index {2};
        std::vector<std::string> setup_moves;

        while (token_available(tokens, index)) {
            setup_moves.push_back(tokens.at(index));
            index++;
        }

        if (setup_moves.empty()) {
            return std::nullopt;
        } else {
            return std::make_optional(setup_moves);
        }
    }

    void init(engine::Engine& engine, const std::vector<std::string>&) {
        engine.init();
    }

    void newgame(engine::Engine& engine, const std::vector<std::string>& tokens) {
        const auto setup_moves {parse_setup_moves(tokens)};

        if (token_available(tokens, 1)) {
            engine.newgame(std::make_optional(tokens.at(1)), setup_moves);
        } else {
            engine.newgame(std::nullopt, setup_moves);
        }
    }

    void move(engine::Engine& engine, const std::vector<std::string>& tokens) {
        if (!token_available(tokens, 1)) {
            throw error::InvalidCommand();
        }

        engine.move(tokens.at(1));
    }

    void go(engine::Engine& engine, const std::vector<std::string>& tokens) {
        std::optional<std::string> max_depth;
        std::optional<std::string> max_time;
        bool dont_play_move {false};

        if (const auto iter {token_available(tokens, "maxdepth")}; iter != tokens.cend()) {
            if (const auto next_iter {next_token(iter)}; next_iter != tokens.cend()) {
                max_depth = *next_iter;
            }
        }

        if (const auto iter {token_available(tokens, "maxtime")}; iter != tokens.cend()) {
            if (const auto next_iter {next_token(iter)}; next_iter != tokens.cend()) {
                max_time = *next_iter;
            }
        }

        if (const auto iter {token_available(tokens, "dontplaymove")}; iter != tokens.cend()) {
            dont_play_move = true;
        }

        engine.go(max_depth, max_time, dont_play_move);
    }

    void stop(engine::Engine& engine, const std::vector<std::string>&) {
        engine.stop();
    }

    void getparameters(engine::Engine& engine, const std::vector<std::string>&) {
        engine.getparameters();
    }

    void setparameter(engine::Engine& engine, const std::vector<std::string>& tokens) {
        if (!token_available(tokens, 1) || !token_available(tokens, 2)) {
            throw error::InvalidCommand();
        }

        engine.setparameter(tokens.at(1), tokens.at(2));
    }

    void getparameter(engine::Engine& engine, const std::vector<std::string>& tokens) {
        if (!token_available(tokens, 1)) {
            throw error::InvalidCommand();
        }

        engine.getparameter(tokens.at(1));
    }

    void quit(engine::Engine& engine, const std::vector<std::string>&) {
        engine.quit();
    }

    void getname(engine::Engine& engine, const std::vector<std::string>&) {
        engine.getname();
    }

    void board(engine::Engine& engine, const std::vector<std::string>&) {
        engine.board();
    }
}
