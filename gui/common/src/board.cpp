#include "common/board.hpp"

#include <algorithm>
#include <cmath>
#include <regex>
#include <stdexcept>
#include <cassert>
#include <iterator>
#include <sstream>

/*
    branched capture B:W1,3,8,9,10,16,17:B12,20,21,23,26,27,29,31
    longest capture W:WK4:B6,7,8,14,15,16,22,23,24

    winner white W:WK7,K8:B16
    winner black W:W8:BK12,K15
    winner white B:W1,2,K7:B5,6,15,9
    winner black W:W25,6:BK14,29,30

    tie W:WK2:BK30
*/

namespace board {
    wxBEGIN_EVENT_TABLE(CheckersBoard, wxWindow)
        EVT_PAINT(CheckersBoard::on_paint)
        EVT_LEFT_DOWN(CheckersBoard::on_mouse_left_down)
        EVT_RIGHT_DOWN(CheckersBoard::on_mouse_right_down)
    wxEND_EVENT_TABLE()

    static constexpr int ERR {0};

    CheckersBoard::CheckersBoard(wxFrame* parent, int x, int y, int size, const OnPieceMove& on_piece_move)
        : wxWindow(parent, wxID_ANY, wxPoint(x, y), wxSize(size, size)), board_size(size), on_piece_move(on_piece_move) {
        reset_position();
    }

    void CheckersBoard::set_board_size(int size) {
        board_size = std::clamp(size, 200, 2000);

        refresh_canvas();
    }

    void CheckersBoard::reset_position() {
        clear();

        for (Idx i {0}; i < 12; i++) {
            board[i] = Square::White;
        }

        for (Idx i {20}; i < 32; i++) {
            board[i] = Square::Black;
        }

        legal_moves = generate_moves();

        refresh_canvas();
    }

    void CheckersBoard::set_position(const std::string& fen_string) {
        // Don't validate for any stupid things the string might contain
        // Validate only the format
        if (!validate_fen_string(fen_string)) {
            return;
        }

        clear();

        try {
            std::size_t index {0u};

            turn = parse_player(fen_string, index);

            index++;
            index++;

            const Player player1 {parse_player(fen_string, index)};

            index++;

            parse_pieces(fen_string, index, player1);

            index++;

            const Player player2 {parse_player(fen_string, index)};

            index++;

            parse_pieces(fen_string, index, player2);
        } catch (int) {
            clear();
        }

        legal_moves = generate_moves();

        refresh_canvas();
    }

    void CheckersBoard::set_user_input(bool user_input) {
        this->user_input = user_input;
    }

    void CheckersBoard::set_show_indices(bool show_indices) {
        this->show_indices = show_indices;

        refresh_canvas();
    }

    void CheckersBoard::play_move(const Move& move) {
        switch (move.type) {
            case MoveType::Normal:
                play_normal_move(move);
                break;
            case MoveType::Capture:
                play_capture_move(move);
                break;
        }

        legal_moves = generate_moves();

        refresh_canvas();
    }

    void CheckersBoard::play_move(const std::string& move_string) {
        // Validate only the format
        if (!valid_move_string(move_string)) {
            return;
        }

        std::size_t index {0u};
        Idx source {};
        std::vector<Idx> destinations;

        try {
            // These are in the range [1, 32]
            source = parse_source_square(move_string, index);
            destinations = parse_destination_squares(move_string, index);
        } catch (int) {
            return;
        }

        // Construct a move and play it
        Move move;

        if (is_capture_move(source, destinations[0u])) {
            move.type = MoveType::Capture;
            move.capture.source_index = to_0_31(source);
            move.capture.destination_indices_size = destinations.size();

            for (std::size_t i {0u}; i < destinations.size(); i++) {
                move.capture.destination_indices[i] = to_0_31(destinations[i]);
            }
        } else {
            move.type = MoveType::Normal;
            move.normal.source_index = to_0_31(source);
            move.normal.destination_index = to_0_31(destinations[0u]);
        }

        play_move(move);
    }

    std::string CheckersBoard::move_to_string(const Move& move) {
        std::ostringstream stream;

        switch (move.type) {
            case MoveType::Normal:
                stream << to_1_32(move.normal.source_index) << 'x' << to_1_32(move.normal.destination_index);

                break;
            case MoveType::Capture:
                stream << to_1_32(move.capture.source_index);

                for (std::size_t i {0u}; i < move.capture.destination_indices_size; i++) {
                    stream << 'x' << to_1_32(move.capture.destination_indices[i]);
                }

                break;
        }

        return stream.str();
    }

    CheckersBoard::Player CheckersBoard::opponent(Player player) {
        return player == Player::Black ? Player::White : Player::Black;
    }

    void CheckersBoard::on_paint(wxPaintEvent&) {
        wxPaintDC dc {this};
        draw(dc);
    }

    void CheckersBoard::on_mouse_left_down(wxMouseEvent& event) {
        if (!user_input) {
            return;
        }

        if (game_over != GameOver::None) {
            return;
        }

        const int square {get_square(event.GetPosition())};

        if (!is_black_square(square)) {
            return;
        }

        const Idx square_index {to_0_31(translate_0_64_to_1_32(square))};

        if (select_piece(square_index)) {
            return;
        }

        if (selected_piece_index == NULL_INDEX) {
            return;
        }

        const bool normal {std::all_of(legal_moves.cbegin(), legal_moves.cend(), [](const Move& move) { return move.type == MoveType::Normal; })};
        const bool capture {std::all_of(legal_moves.cbegin(), legal_moves.cend(), [](const Move& move) { return move.type == MoveType::Capture; })};

        if (normal) {
            for (const Move& move : legal_moves) {
                if (move.type == MoveType::Normal) {
                    if (playable_normal_move(move, square_index)) {
                        play_normal_move(move);

                        legal_moves = generate_moves();
                        selected_piece_index = NULL_INDEX;

                        break;
                    }
                }
            }
        } else if (capture) {
            select_jump_square(square_index);

            for (const Move& move : legal_moves) {
                if (playable_capture_move(move, jump_square_indices)) {
                    play_capture_move(move);

                    legal_moves = generate_moves();
                    selected_piece_index = NULL_INDEX;
                    jump_square_indices.clear();

                    break;
                }
            }
        } else {
            assert(false);
        }

        refresh_canvas();
    }

    void CheckersBoard::on_mouse_right_down(wxMouseEvent& event) {
        if (!user_input) {
            return;
        }

        if (game_over != GameOver::None) {
            return;
        }

        if (selected_piece_index == NULL_INDEX) {
            return;
        }

        const bool capture {std::all_of(legal_moves.cbegin(), legal_moves.cend(), [](const Move& move) { return move.type == MoveType::Capture; })};

        if (!capture) {
            return;
        }

        const int square {get_square(event.GetPosition())};

        if (!is_black_square(square)) {
            return;
        }

        const Idx square_index {to_0_31(translate_0_64_to_1_32(square))};

        deselect_jump_square(square_index);

        for (const Move& move : legal_moves) {
            if (playable_capture_move(move, jump_square_indices)) {
                play_capture_move(move);

                legal_moves = generate_moves();
                selected_piece_index = NULL_INDEX;
                jump_square_indices.clear();

                break;
            }
        }

        refresh_canvas();
    }

    int CheckersBoard::get_square(wxPoint position) const {
        const int SQUARE_SIZE {board_size / 8};

        const int file {position.x / SQUARE_SIZE};
        const int rank {position.y / SQUARE_SIZE};

        return rank * 8 + file;
    }

    std::pair<int, int> CheckersBoard::get_square(int square) {
        const int file {square % 8};
        const int rank {square / 8};

        return std::make_pair(file, rank);
    }

    bool CheckersBoard::is_black_square(int square) {
        const auto [x, y] {get_square(square)};

        if ((x + y) % 2 == 0) {
            return false;
        } else {
            return true;
        }
    }

    bool CheckersBoard::select_piece(Idx square_index) {
        if (selected_piece_index == square_index) {
            selected_piece_index = NULL_INDEX;
            jump_square_indices.clear();

            refresh_canvas();

            return false;
        }

        if (board[square_index] != Square::None) {
            selected_piece_index = square_index;
            jump_square_indices.clear();

            refresh_canvas();

            return true;
        }

        return false;
    }

    std::vector<CheckersBoard::Move> CheckersBoard::generate_moves() const {
        std::vector<Move> moves;

        for (Idx i {0}; i < 32; i++) {
            const bool king {static_cast<bool>(static_cast<unsigned int>(board[i]) & (1u << 2))};
            const bool piece {static_cast<bool>(static_cast<unsigned int>(board[i]) & static_cast<unsigned int>(turn))};

            if (piece) {
                generate_piece_capture_moves(moves, i, turn, king);
            }
        }

        // If there are possible captures, force the player to play these moves
        if (!moves.empty()) {
            return moves;
        }

        for (Idx i {0}; i < 32; i++) {
            const bool king {static_cast<bool>(static_cast<unsigned int>(board[i]) & (1u << 2))};
            const bool piece {static_cast<bool>(static_cast<unsigned int>(board[i]) & static_cast<unsigned int>(turn))};

            if (piece) {
                generate_piece_moves(moves, i, turn, king);
            }
        }

        return moves;
    }

    void CheckersBoard::generate_piece_capture_moves(std::vector<Move>& moves, Idx square_index, Player player, bool king) const {
        JumpCtx ctx;
        ctx.board = board;
        ctx.source_index = square_index;

        check_piece_jumps(moves, square_index, player, king, ctx);
    }

    void CheckersBoard::generate_piece_moves(std::vector<Move>& moves, Idx square_index, Player player, bool king) const {
        Direction directions[4u] {};
        std::size_t index {0u};

        if (king) {
            directions[index++] = Direction::NorthEast;
            directions[index++] = Direction::NorthWest;
            directions[index++] = Direction::SouthEast;
            directions[index++] = Direction::SouthWest;
        } else {
            switch (player) {
                case Player::Black:
                    directions[index++] = Direction::NorthEast;
                    directions[index++] = Direction::NorthWest;
                    break;
                case Player::White:
                    directions[index++] = Direction::SouthEast;
                    directions[index++] = Direction::SouthWest;
                    break;
            }
        }

        // Check the squares above or below in diagonal
        for (auto iter {std::begin(directions)}; iter != std::next(std::begin(directions), index); iter++) {
            const Idx target_index {offset(square_index, *iter, Short)};

            if (target_index == NULL_INDEX) {
                continue;
            }

            if (board[target_index] != Square::None) {
                continue;
            }

            Move move;
            move.type = MoveType::Normal;
            move.normal.source_index = square_index;
            move.normal.destination_index = target_index;

            moves.push_back(move);
        }
    }

    bool CheckersBoard::check_piece_jumps(std::vector<Move>& moves, Idx square_index, Player player, bool king, JumpCtx& ctx) const {
        Direction directions[4u] {};
        std::size_t index {0u};

        if (king) {
            directions[index++] = Direction::NorthEast;
            directions[index++] = Direction::NorthWest;
            directions[index++] = Direction::SouthEast;
            directions[index++] = Direction::SouthWest;
        } else {
            switch (player) {
                case Player::Black:
                    directions[index++] = Direction::NorthEast;
                    directions[index++] = Direction::NorthWest;
                    break;
                case Player::White:
                    directions[index++] = Direction::SouthEast;
                    directions[index++] = Direction::SouthWest;
                    break;
            }
        }

        // We want an enemy piece
        const unsigned int piece_mask {static_cast<unsigned int>(opponent(player))};

        bool sequence_jumps_ended {true};

        for (auto iter {std::begin(directions)}; iter != std::next(std::begin(directions), index); iter++) {
            const Idx enemy_index {offset(square_index, *iter, Short)};
            const Idx target_index {offset(square_index, *iter, Long)};

            if (enemy_index == NULL_INDEX || target_index == NULL_INDEX) {
                continue;
            }

            const bool is_enemy_piece {static_cast<bool>(static_cast<unsigned int>(ctx.board[enemy_index]) & piece_mask)};

            if (!is_enemy_piece || ctx.board[target_index] != Square::None) {
                continue;
            }

            sequence_jumps_ended = false;

            ctx.destination_indices.push_back(target_index);

            // Remove the piece to avoid illegal jumps
            const auto removed_enemy_piece {std::exchange(ctx.board[enemy_index], Square::None)};

            // Jump this piece to avoid other illegal jumps
            std::swap(ctx.board[square_index], ctx.board[target_index]);

            if (check_piece_jumps(moves, target_index, player, king, ctx)) {
                // This means that it reached the end of a sequence of jumps; the piece can't jump anymore

                Move move;
                move.type = MoveType::Capture;
                move.capture.source_index = ctx.source_index;
                move.capture.destination_indices_size = ctx.destination_indices.size();

                for (std::size_t i {0u}; i < ctx.destination_indices.size(); i++) {
                    move.capture.destination_indices[i] = ctx.destination_indices[i];
                }

                moves.push_back(move);
            }

            // Restore jumped piece
            std::swap(ctx.board[square_index], ctx.board[target_index]);

            // Restore removed piece
            ctx.board[enemy_index] = removed_enemy_piece;

            ctx.destination_indices.pop_back();
        }

        return sequence_jumps_ended;
    }

    CheckersBoard::Idx CheckersBoard::offset(Idx square_index, Direction direction, Diagonal diagonal) const {
        Idx result_index {square_index};

        const bool even_row {(square_index / 4) % 2 == 0};

        switch (direction) {
            case Direction::NorthEast:
                result_index -= even_row ? 3 : 4;

                if (diagonal == Diagonal::Long) {
                    result_index -= even_row ? 4 : 3;
                }

                break;
            case Direction::NorthWest:
                result_index -= even_row ? 4 : 5;

                if (diagonal == Diagonal::Long) {
                    result_index -= even_row ? 5 : 4;
                }

                break;
            case Direction::SouthEast:
                result_index += even_row ? 5 : 4;

                if (diagonal == Diagonal::Long) {
                    result_index += even_row ? 4 : 5;
                }

                break;
            case Direction::SouthWest:
                result_index += even_row ? 4 : 3;

                if (diagonal == Diagonal::Long) {
                    result_index += even_row ? 3 : 4;
                }

                break;
        }

        // Check edge cases (literally)
        if (std::abs(square_index / 4 - result_index / 4) != static_cast<int>(diagonal)) {
            return NULL_INDEX;
        }

        // Check out of bounds
        if (result_index < 0 || result_index > 31) {
            return NULL_INDEX;
        }

        return result_index;
    }

    void CheckersBoard::change_turn() {
        turn = opponent(turn);
    }

    void CheckersBoard::check_80_move_rule(bool advancement) {
        if (advancement) {
            plies_without_advancement = 0u;
        } else {
            if (++plies_without_advancement == 80u) {
                game_over = GameOver::Tie;
            }
        }
    }

    void CheckersBoard::check_piece_crowning(Idx square_index) {
        const int row {square_index / 4};

        switch (turn) {
            case Player::Black:
                if (row == 0) {
                    board[square_index] = Square::BlackKing;
                }

                break;
            case Player::White:
                if (row == 7) {
                    board[square_index] = Square::WhiteKing;
                }

                break;
        }
    }

    void CheckersBoard::check_legal_moves() {
        // Generate current player's possible moves
        auto moves {generate_moves()};

        if (moves.empty()) {
            // Either they have no pieces left or they are blocked
            game_over = turn == Player::Black ? GameOver::WinnerWhite : GameOver::WinnerBlack;
        }
    }

    void CheckersBoard::check_repetition(bool advancement) {
        Repetition::Position current;
        current.board = board;
        current.turn = turn;

        if (advancement) {
            repetition.positions.clear();
        } else {
            const auto count {std::count(repetition.positions.cbegin(), repetition.positions.cend(), current)};

            if (count == 2) {
                game_over = GameOver::Tie;
                return;
            }
        }

        // Insert current position even after advancement
        repetition.positions.push_back(current);
    }

    bool CheckersBoard::playable_normal_move(const Move& move, Idx square_index) const {
        if (move.normal.source_index != selected_piece_index) {
            return false;
        }

        if (move.normal.destination_index != square_index) {
            return false;
        }

        return true;
    }

    bool CheckersBoard::playable_capture_move(const Move& move, const std::vector<Idx>& square_indices) const {
        if (move.capture.source_index != selected_piece_index) {
            return false;
        }

        if (!std::equal(
            move.capture.destination_indices,
            move.capture.destination_indices + move.capture.destination_indices_size,
            square_indices.cbegin(),
            square_indices.cend()
        )) {
            return false;
        }

        return true;
    }

    void CheckersBoard::play_normal_move(const Move& move) {
        assert(move.type == MoveType::Normal);

        assert(board[move.normal.destination_index] == Square::None);

        std::swap(board[move.normal.source_index], board[move.normal.destination_index]);

        const bool advancement {
            board[move.normal.destination_index] == Square::Black ||
            board[move.normal.destination_index] == Square::White
        };

        check_piece_crowning(move.normal.destination_index);
        change_turn();
        check_80_move_rule(advancement);
        check_repetition(advancement);
        check_legal_moves();  // This sets game over and has higher precedence

        on_piece_move(move);
    }

    void CheckersBoard::play_capture_move(const Move& move) {
        assert(move.type == MoveType::Capture);

        const Idx destination_index {move.capture.destination_indices[move.capture.destination_indices_size - 1u]};

        assert(board[destination_index] == Square::None);

        remove_jumped_pieces(move);
        std::swap(board[move.capture.source_index], board[destination_index]);

        check_piece_crowning(destination_index);
        change_turn();
        check_80_move_rule(true);
        check_repetition(true);
        check_legal_moves();  // This sets game over and has higher precedence

        on_piece_move(move);
    }

    void CheckersBoard::select_jump_square(Idx square_index) {
        const auto count {std::count(jump_square_indices.cbegin(), jump_square_indices.cend(), square_index)};

        if (count < 2) {
            jump_square_indices.push_back(square_index);
        }
    }

    void CheckersBoard::deselect_jump_square(Idx square_index) {
        const auto iter {std::find(jump_square_indices.cbegin(), jump_square_indices.cend(), square_index)};

        if (iter != jump_square_indices.cend()) {
            jump_square_indices.erase(iter);
        }
    }

    void CheckersBoard::remove_jumped_pieces(const Move& move) {
        assert(board[move.capture.destination_indices[0u]] == Square::None);

        const Idx index {get_jumped_piece_index(
            to_1_32(move.capture.source_index),
            to_1_32(move.capture.destination_indices[0u])
        )};

        assert(board[to_0_31(index)] != Square::None);

        board[to_0_31(index)] = Square::None;

        for (std::size_t i {0u}; i < move.capture.destination_indices_size - 1u; i++) {
            assert(board[move.capture.destination_indices[i + 1u]] == Square::None);

            const Idx index {get_jumped_piece_index(
                to_1_32(move.capture.destination_indices[i]),
                to_1_32(move.capture.destination_indices[i + 1u])
            )};

            assert(board[to_0_31(index)] != Square::None);

            board[to_0_31(index)] = Square::None;
        }
    }

    CheckersBoard::Idx CheckersBoard::get_jumped_piece_index(Idx index1, Idx index2) {
        // This works with indices in the range [1, 32]

        const auto sum {index1 + index2};

        assert(sum % 2 == 1);

        if (((to_0_31(index1)) / 4) % 2 == 0) {
            return (sum + 1) / 2;
        } else {
            return (sum - 1) / 2;
        }
    }

    bool CheckersBoard::validate_fen_string(const std::string& fen_string) {
        const std::regex pattern {"(W|B)(:(W|B)K?[0-9]+(,K?[0-9]+){0,11}){2}"};

        return std::regex_match(fen_string, pattern);
    }

    CheckersBoard::Player CheckersBoard::parse_player(const std::string& fen_string, std::size_t index) {
        switch (fen_string[index]) {
            case 'B':
                return Player::Black;
            case 'W':
                return Player::White;
        }

        throw ERR;
    }

    void CheckersBoard::parse_pieces(const std::string& fen_string, std::size_t& index, Player player) {
        while (fen_string[index] != ':' && index != fen_string.size()) {
            // Squares are in the range [1, 32]
            const auto [square, king] {parse_piece(fen_string, index)};

            if (player == Player::White) {
                if (king) {
                    board[to_0_31(square)] = Square::WhiteKing;
                } else {
                    board[to_0_31(square)] = Square::White;
                }
            } else {
                if (king) {
                    board[to_0_31(square)] = Square::BlackKing;
                } else {
                    board[to_0_31(square)] = Square::Black;
                }
            }
        }
    }

    std::pair<CheckersBoard::Idx, bool> CheckersBoard::parse_piece(const std::string& fen_string, std::size_t& index) {
        bool scanning {true};
        bool king {false};
        std::string result_number;

        while (scanning) {
            if (index == fen_string.size()) {
                // Also stop scanning when going past the string
                break;
            }

            switch (fen_string[index]) {
                case 'K':
                    king = true;
                    index++;

                    break;
                case ',':
                    scanning = false;
                    index++;

                    break;
                case ':':
                    scanning = false;

                    break;
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    result_number.push_back(fen_string[index]);
                    index++;

                    break;
            }
        }

        unsigned long result {0u};

        try {
            result = std::stoul(result_number);
        } catch (const std::invalid_argument&) {
            throw ERR;
        } catch (const std::out_of_range&) {
            throw ERR;
        }

        return std::make_pair(static_cast<Idx>(result), king);
    }

    int CheckersBoard::translate_1_32_to_0_64(Idx index) {
        if (((index - 1) / 4) % 2 == 0) {
            return index * 2 - 1;
        } else {
            return (index - 1) * 2;
        }
    }

    CheckersBoard::Idx CheckersBoard::translate_0_64_to_1_32(int index) {
        if (index % 2 == 1) {
            return (index + 1) / 2;
        } else {
            return (index / 2) + 1;
        }
    }

    CheckersBoard::Idx CheckersBoard::to_0_31(Idx index) {
        return index - 1;
    }

    CheckersBoard::Idx CheckersBoard::to_1_32(Idx index) {
        return index + 1;
    }

    bool CheckersBoard::valid_move_string(const std::string& move_string) {
        const std::regex pattern {"([0-9]+x)+[0-9]+"};

        return std::regex_match(move_string, pattern);
    }

    unsigned int CheckersBoard::parse_number(const std::string& move_string, std::size_t& index) {
        std::string result_number;

        while (true) {
            if (index == move_string.size()) {
                // Also stop scanning when going past the string
                break;
            }

            if (move_string[index] >= '0' && move_string[index] <= '9') {
                result_number.push_back(move_string[index]);
                index++;
            } else if (move_string[index] == 'x') {
                index++;
                break;
            } else {
                throw ERR;
            }
        }

        unsigned long result {0u};

        try {
            result = std::stoul(result_number);
        } catch (const std::invalid_argument&) {
            throw ERR;
        } catch (const std::out_of_range&) {
            throw ERR;
        }

        return static_cast<Idx>(result);
    }

    CheckersBoard::Idx CheckersBoard::parse_source_square(const std::string& move_string, std::size_t& index) {
        const auto number {parse_number(move_string, index)};

        if (number > 32u) {
            throw ERR;
        }

        return static_cast<Idx>(number);
    }

    std::vector<CheckersBoard::Idx> CheckersBoard::parse_destination_squares(const std::string& move_string, std::size_t& index) {
        std::vector<Idx> destinations;

        while (true) {
            if (index == move_string.size()) {
                break;
            }

            const auto number {parse_number(move_string, index)};

            if (number > 32u) {
                throw ERR;
            }

            destinations.push_back(static_cast<Idx>(number));
        }

        return destinations;
    }

    bool CheckersBoard::is_capture_move(Idx source, Idx destination) {
        // Indices must be in the range [1, 32]

        const auto distance {std::abs(to_0_31(source) - to_0_31(destination))};

        if (distance >= 3 && distance <= 5) {
            return false;
        } else if (distance == 7 || distance == 9) {
            return true;
        } else {
            assert(false);
        }
    }

    void CheckersBoard::clear() {
        std::fill(std::begin(board), std::end(board), Square::None);
        turn = Player::Black;
        selected_piece_index = NULL_INDEX;
        legal_moves.clear();
        jump_square_indices.clear();
        plies_without_advancement = 0u;
        game_over = GameOver::None;
        repetition.positions.clear();
    }

    void CheckersBoard::refresh_canvas() {
        Refresh();
        Update();
    }

    void CheckersBoard::draw(wxDC& dc) {
        const auto ORANGE {wxColour(240, 180, 80)};
        const auto REDDISH {wxColour(255, 140, 60)};
        const auto DARKER_REDDISH {wxColour(255, 100, 40)};
        const auto GOLD {wxColour(160, 160, 10)};
        const auto WHITE {wxColour(200, 200, 200)};
        const auto BLACK {wxColour(80, 60, 40)};
        const int SQUARE_SIZE {board_size / 8};

        {
            unsigned int index {0u};

            for (int y {0}; y < 8; y++) {
                for (int x {0}; x < 8; x++) {
                    if ((x + y) % 2 == 0) {
                        dc.SetBrush(wxBrush(WHITE));
                        dc.SetPen(wxPen(WHITE));
                    } else {
                        dc.SetBrush(wxBrush(BLACK));
                        dc.SetPen(wxPen(BLACK));
                    }

                    dc.DrawRectangle(wxPoint(SQUARE_SIZE * x, SQUARE_SIZE * y), wxSize(SQUARE_SIZE, SQUARE_SIZE));

                    if (show_indices) {
                        if ((x + y) % 2 != 0) {
                            dc.DrawLabel(
                                std::to_string(++index),
                                wxRect(SQUARE_SIZE * x, SQUARE_SIZE * y, SQUARE_SIZE, SQUARE_SIZE),
                                wxALIGN_LEFT | wxALIGN_TOP
                            );
                        }
                    }
                }
            }
        }

        if (selected_piece_index != NULL_INDEX) {
            dc.SetBrush(wxBrush(ORANGE));
            dc.SetPen(wxPen(ORANGE));

            for (const Move& move : legal_moves) {
                switch (move.type) {
                    case MoveType::Normal: {
                        if (move.normal.source_index == selected_piece_index) {
                            const auto [x, y] {get_square(translate_1_32_to_0_64(to_1_32(move.normal.destination_index)))};

                            dc.DrawRectangle(wxPoint(SQUARE_SIZE * x, SQUARE_SIZE * y), wxSize(SQUARE_SIZE, SQUARE_SIZE));
                        }

                        break;
                    }
                    case MoveType::Capture: {
                        if (move.capture.source_index == selected_piece_index) {
                            for (std::size_t i {0u}; i < move.capture.destination_indices_size; i++) {
                                const auto [x, y] {get_square(translate_1_32_to_0_64(to_1_32(move.capture.destination_indices[i])))};

                                dc.DrawRectangle(wxPoint(SQUARE_SIZE * x, SQUARE_SIZE * y), wxSize(SQUARE_SIZE, SQUARE_SIZE));
                            }
                        }

                        break;
                    }
                }
            }

            for (std::size_t i {0u}; i < jump_square_indices.size(); i++) {
                const auto count {std::count(jump_square_indices.cbegin(), jump_square_indices.cend(), jump_square_indices[i])};

                if (count < 2) {
                    dc.SetBrush(wxBrush(REDDISH));
                    dc.SetPen(wxPen(REDDISH));
                } else {
                    dc.SetBrush(wxBrush(DARKER_REDDISH));
                    dc.SetPen(wxPen(DARKER_REDDISH));
                }

                const auto [x, y] {get_square(translate_1_32_to_0_64(to_1_32(jump_square_indices[i])))};

                dc.DrawRectangle(wxPoint(SQUARE_SIZE * x, SQUARE_SIZE * y), wxSize(SQUARE_SIZE, SQUARE_SIZE));
            }

            std::vector<Idx> indices_visited;

            for (std::size_t i {0u}; i < jump_square_indices.size(); i++) {
                const auto iter {std::find(indices_visited.cbegin(), indices_visited.cend(), jump_square_indices[i])};

                const int alignment {wxALIGN_TOP | (iter == indices_visited.cend() ? wxALIGN_LEFT : wxALIGN_RIGHT)};
                const auto [x, y] {get_square(translate_1_32_to_0_64(to_1_32(jump_square_indices[i])))};

                dc.DrawLabel(std::to_string(i + 1u), wxRect(SQUARE_SIZE * x, SQUARE_SIZE * y, SQUARE_SIZE, SQUARE_SIZE), alignment);

                indices_visited.push_back(jump_square_indices[i]);
            }
        }

        const auto PIECE_WHITE {wxColour(140, 10, 10)};
        const auto PIECE_BLACK {wxColour(10, 10, 10)};
        const int PIECE_SIZE {static_cast<int>(static_cast<float>(SQUARE_SIZE) / 2.5f)};
        const int OFFSET {SQUARE_SIZE / 2};

        for (Idx i {0}; i < 32; i++) {
            wxColour brush_color;
            wxColour pen_color;

            bool king {false};

            switch (board[i]) {
                case Square::None:
                    continue;
                case Square::White:
                    brush_color = PIECE_WHITE;
                    pen_color = PIECE_WHITE;
                    break;
                case Square::Black:
                    brush_color = PIECE_BLACK;
                    pen_color = PIECE_BLACK;
                    break;
                case Square::WhiteKing:
                    brush_color = PIECE_WHITE;
                    pen_color = PIECE_WHITE;
                    king = true;
                    break;
                case Square::BlackKing:
                    brush_color = PIECE_BLACK;
                    pen_color = PIECE_BLACK;
                    king = true;
                    break;
            }

            if (selected_piece_index == i) {
                pen_color = ORANGE;
            }

            dc.SetBrush(wxBrush(brush_color));
            dc.SetPen(wxPen(pen_color));

            const auto [x, y] {get_square(translate_1_32_to_0_64(to_1_32(i)))};
            const auto position {wxPoint(SQUARE_SIZE * x + OFFSET, SQUARE_SIZE * y + OFFSET)};

            dc.DrawCircle(position, PIECE_SIZE);

            if (king) {
                dc.SetBrush(wxBrush(GOLD));
                dc.SetPen(wxPen(GOLD));
                dc.DrawCircle(position, PIECE_SIZE / 3);
            }
        }
    }
}
