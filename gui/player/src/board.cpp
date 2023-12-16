#include <array>
#include <functional>
#include <utility>
#include <vector>
#include <string>
#include <cstddef>
#include <algorithm>
#include <cmath>
#include <regex>
#include <stdexcept>
#include <cassert>

#include <wx/wx.h>

#include "board.hpp"

/*
    B:W1,3,8,9,10,16,17:B12,20,21,23,26,27,29,31
    W:WK4:B6,7,8,14,15,16,22,23,24

    winner white W:WK7,K8:B16
    winner black W:W8:BK12,K15
    winner white B:W1,2,K7:B5,6,15,9
    winner black W:W25,6:BK14,29,30

    tie W:WK2:BK30
*/

wxBEGIN_EVENT_TABLE(Board, wxWindow)
    EVT_PAINT(Board::on_paint)
    EVT_LEFT_DOWN(Board::on_mouse_left_down)
    EVT_RIGHT_DOWN(Board::on_mouse_right_down)
wxEND_EVENT_TABLE()

Board::Board(wxFrame* parent, int x, int y, int size, const OnPieceMove& on_piece_move)
    : wxWindow(parent, wxID_ANY, wxPoint(x, y), wxSize(size, size)), board_size(size), on_piece_move(on_piece_move) {
    reset();
}

void Board::set_board_size(int size) {
    board_size = std::clamp(size, 200, 2000);

    refresh_canvas();
}

void Board::reset() {
    clear();

    for (Idx i {0}; i < 24; i++) {
        const auto [file, rank] {get_square(i)};

        if ((file + rank) % 2 == 1) {
            board[i] = Square::White;
        }
    }

    for (Idx i {40}; i < 64; i++) {
        const auto [file, rank] {get_square(i)};

        if ((file + rank) % 2 == 1) {
            board[i] = Square::Black;
        }
    }

    legal_moves = generate_moves();

    refresh_canvas();
}

bool Board::set_position(const std::string& fen_string) {
    if (!validate_fen_string(fen_string)) {
        return false;
    }

    clear();

    std::size_t index {0};

    turn = parse_player(fen_string, index);

    index++;
    index++;

    const Player player1 {parse_player(fen_string, index)};

    index++;

    parse_pieces(fen_string, index, player1);

    index++;

    const Player player2 {parse_player(fen_string, index)};  // TODO check

    index++;

    parse_pieces(fen_string, index, player2);

    legal_moves = generate_moves();

    refresh_canvas();

    return true;
}

void Board::on_paint(wxPaintEvent&) {
    wxPaintDC dc {this};
    draw(dc);
}

void Board::on_mouse_left_down(wxMouseEvent& event) {
    if (game_over != GameOver::None) {
        return;
    }

    const Idx square_index {get_square(event.GetPosition())};

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

void Board::on_mouse_right_down(wxMouseEvent& event) {
    if (game_over != GameOver::None) {
        return;
    }

    const Idx square_index {get_square(event.GetPosition())};

    if (selected_piece_index == NULL_INDEX) {
        return;
    }

    const bool capture {std::all_of(legal_moves.cbegin(), legal_moves.cend(), [](const Move& move) { return move.type == MoveType::Capture; })};

    if (!capture) {
        return;
    }

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

Board::Idx Board::get_square(wxPoint position) const {
    const Idx SQUARE_SIZE {board_size / 8};

    const Idx file {position.x / SQUARE_SIZE};
    const Idx rank {position.y / SQUARE_SIZE};

    return rank * 8 + file;
}

std::pair<Board::Idx, Board::Idx> Board::get_square(Idx square_index) const {
    const Idx file {square_index % 8};
    const Idx rank {square_index / 8};

    return std::make_pair(file, rank);
}

bool Board::select_piece(Idx square_index) {
    for (Idx i {0}; i < 64; i++) {
        if (i != square_index) {
            continue;
        }

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
    }

    return false;
}

std::vector<Board::Move> Board::generate_moves() const {
    std::vector<Move> moves;

    for (Idx i {0}; i < 64; i++) {
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

    for (Idx i {0}; i < 64; i++) {
        const bool king {static_cast<bool>(static_cast<unsigned int>(board[i]) & (1u << 2))};
        const bool piece {static_cast<bool>(static_cast<unsigned int>(board[i]) & static_cast<unsigned int>(turn))};

        if (piece) {
            generate_piece_moves(moves, i, turn, king);
        }
    }

    return moves;
}

void Board::generate_piece_capture_moves(std::vector<Move>& moves, Idx square_index, Player player, bool king) const {
    JumpCtx ctx;
    ctx.board = board;
    ctx.source_index = square_index;

    check_piece_jumps(moves, square_index, player, king, ctx);
}

void Board::generate_piece_moves(std::vector<Move>& moves, Idx square_index, Player player, bool king) const {
    Direction directions[4] {};
    std::size_t index {0};

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
    for (std::size_t i {0}; i < index; i++) {
        const Idx target_index {offset(square_index, directions[i], Short)};

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

bool Board::check_piece_jumps(std::vector<Move>& moves, Idx square_index, Player player, bool king, JumpCtx& ctx) const {
    Direction directions[4] {};
    std::size_t index {0};

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

    for (std::size_t i {0}; i < index; i++) {
        const Idx enemy_index {offset(square_index, directions[i], Short)};
        const Idx target_index {offset(square_index, directions[i], Long)};

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
        const Square enemy_piece {ctx.board[enemy_index]};
        ctx.board[enemy_index] = Square::None;

        // Jump this piece to avoid illegal jumps
        std::swap(ctx.board[square_index], ctx.board[target_index]);

        if (check_piece_jumps(moves, target_index, player, king, ctx)) {
            // This means that it reached the end of a sequence of jumps; the piece can't jump anymore

            Move move;
            move.type = MoveType::Capture;
            move.capture.source_index = ctx.source_index;
            move.capture.destination_indices_size = ctx.destination_indices.size();

            for (std::size_t i {0}; i < ctx.destination_indices.size(); i++) {
                move.capture.destination_indices[i] = ctx.destination_indices[i];
            }

            moves.push_back(move);
        }

        // Restore removed piece
        ctx.board[enemy_index] = enemy_piece;

        // Restore jumped piece
        std::swap(ctx.board[square_index], ctx.board[target_index]);

        ctx.destination_indices.pop_back();
    }

    return sequence_jumps_ended;
}

Board::Idx Board::offset(Idx square_index, Direction direction, Diagonal diagonal) const {
    static constexpr int OFFSET[2] { 1, 2 };

    Idx result_index {square_index};

    switch (direction) {
        case Direction::NorthEast:
            result_index -= 7 * OFFSET[diagonal];
            break;
        case Direction::NorthWest:
            result_index -= 9 * OFFSET[diagonal];
            break;
        case Direction::SouthEast:
            result_index += 9 * OFFSET[diagonal];
            break;
        case Direction::SouthWest:
            result_index += 7 * OFFSET[diagonal];
            break;
    }

    // Check edge cases (literally)
    if (std::abs(square_index / 8 - result_index / 8) != OFFSET[diagonal]) {
        return NULL_INDEX;
    }

    // Check out of bounds
    if (result_index < 0 || result_index > 63) {
        return NULL_INDEX;
    }

    return result_index;
}

void Board::change_turn() {
    turn = opponent(turn);
}

void Board::check_80_move_rule(bool advancement) {
    if (advancement) {
        plies_without_advancement = 0;
    } else {
        if (++plies_without_advancement == 80) {
            game_over = GameOver::Tie;
        }
    }
}

void Board::check_piece_crowning(Idx square_index) {
    const Idx index {square_index / 8};

    switch (turn) {
        case Player::Black:
            if (index == 0) {
                board[square_index] = Square::BlackKing;
            }

            break;
        case Player::White:
            if (index == 7) {
                board[square_index] = Square::WhiteKing;
            }

            break;
    }
}

void Board::check_legal_moves() {
    // Generate current player's possible moves
    auto moves {generate_moves()};

    if (moves.empty()) {
        // Either they have no pieces left or they are blocked
        game_over = turn == Player::Black ? GameOver::WinnerWhite : GameOver::WinnerBlack;
    }
}

void Board::check_repetition(bool advancement) {
    Repetition::Position current;
    current.board = board;
    current.turn = turn;

    if (advancement) {
        repetition.positions.clear();
    } else {
        auto count {std::count(repetition.positions.cbegin(), repetition.positions.cend(), current)};

        if (count == 2) {
            game_over = GameOver::Tie;
            return;
        }
    }

    // Insert current position even after advancement
    repetition.positions.push_back(current);
}

bool Board::playable_normal_move(const Move& move, Idx square_index) const {
    if (move.normal.source_index != selected_piece_index) {
        return false;
    }

    if (move.normal.destination_index != square_index) {
        return false;
    }

    return true;
}

bool Board::playable_capture_move(const Move& move, const std::vector<Idx>& square_indices) const {
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

void Board::play_normal_move(const Move& move) {
    assert(move.type == MoveType::Normal);

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

void Board::play_capture_move(const Move& move) {
    assert(move.type == MoveType::Capture);

    const Idx destination_index {move.capture.destination_indices[move.capture.destination_indices_size - 1]};

    std::swap(board[move.capture.source_index], board[destination_index]);

    remove_jumped_pieces(move);

    check_piece_crowning(destination_index);
    change_turn();
    check_80_move_rule(true);
    check_repetition(true);
    check_legal_moves();  // This sets game over and has higher precedence

    on_piece_move(move);
}

void Board::select_jump_square(Idx square_index) {
    const auto [x, y] {get_square(square_index)};

    if ((x + y) % 2 == 0) {
        return;
    }

    const auto count {std::count(jump_square_indices.cbegin(), jump_square_indices.cend(), square_index)};

    if (count < 2) {
        jump_square_indices.push_back(square_index);
    }
}

void Board::deselect_jump_square(Idx square_index) {
    const auto [x, y] {get_square(square_index)};

    if ((x + y) % 2 == 0) {
        return;
    }

    const auto iter {std::find(jump_square_indices.cbegin(), jump_square_indices.cend(), square_index)};

    if (iter != jump_square_indices.cend()) {
        jump_square_indices.erase(iter);
    }
}

void Board::remove_jumped_pieces(const Move& move) {  // FIXME :P
    const auto index {get_jumped_piece_index(
        translate_index_0_64_to_1_32(move.capture.source_index),
        translate_index_0_64_to_1_32(move.capture.destination_indices[0])
    )};
    board[translate_index_1_32_to_0_64(index)] = Square::None;

    for (Idx i {0}; i < move.capture.destination_indices_size - 1; i++) {
        const auto index {get_jumped_piece_index(
            translate_index_0_64_to_1_32(move.capture.destination_indices[i]),
            translate_index_0_64_to_1_32(move.capture.destination_indices[i + 1])
        )};
        board[translate_index_1_32_to_0_64(index)] = Square::None;
    }
}

Board::Idx Board::get_jumped_piece_index(Idx index1, Idx index2) {
    const auto sum {index1 + index2};

    assert(sum % 2 == 1);

    if (((index1 - 1) / 4) % 2 == 0) {
        return (sum + 1) / 2;
    } else {
        return (sum - 1) / 2;
    }
}

Board::Player Board::opponent(Player player) {
    return player == Player::Black ? Player::White : Player::Black;
}

bool Board::validate_fen_string(const std::string& fen_string) {
    const std::regex pattern {"(W|B)(:(W|B)K?[0-9]+(,K?[0-9]+){0,11}){2}"};

    return std::regex_match(fen_string, pattern);
}

Board::Player Board::parse_player(const std::string& fen_string, std::size_t index) {
    switch (fen_string[index]) {
        case 'B':
            return Player::Black;
        case 'W':
            return Player::White;
    }

    return {};
}

void Board::parse_pieces(const std::string& fen_string, std::size_t& index, Player player) {
    while (fen_string[index] != ':' && index != fen_string.size()) {
        // Here, square is based on the formal indexing of the board: [1, 32]
        const auto [square, king] {parse_piece(fen_string, index)};

        // Translate to [0, 64]
        const Idx this_square {translate_index_1_32_to_0_64(square)};

        if (player == Player::White) {
            if (king) {
                board[this_square] = Square::WhiteKing;
            } else {
                board[this_square] = Square::White;
            }
        } else {
            if (king) {
                board[this_square] = Square::BlackKing;
            } else {
                board[this_square] = Square::Black;
            }
        }
    }
}

std::pair<Board::Idx, bool> Board::parse_piece(const std::string& fen_string, std::size_t& index) {
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

    unsigned long result {0};

    try {
        result = std::stoul(result_number);
    } catch (const std::invalid_argument&) {
        return std::make_pair(NULL_INDEX, false);
    } catch (const std::out_of_range&) {
        return std::make_pair(NULL_INDEX, false);
    }

    return std::make_pair(static_cast<Idx>(result), king);
}

Board::Idx Board::translate_index_1_32_to_0_64(Idx index) {
    if (((index - 1) / 4) % 2 == 0) {
        return index * 2 - 1;
    } else {
        return (index - 1) * 2;
    }
}

Board::Idx Board::translate_index_0_64_to_1_32(Idx index) {
    if (index % 2 == 1) {
        return (index + 1) / 2;
    } else {
        return (index / 2) + 1;
    }
}

void Board::clear() {
    std::fill(std::begin(board), std::end(board), Square::None);
    turn = Player::Black;
    selected_piece_index = NULL_INDEX;
    legal_moves.clear();
    jump_square_indices.clear();
    game_over = GameOver::None;
    repetition.positions.clear();
}

void Board::refresh_canvas() {
    Refresh();
    Update();
}

void Board::draw(wxDC& dc) {
    const auto ORANGE {wxColour(240, 180, 80)};
    const auto REDDISH {wxColour(255, 140, 60)};
    const auto DARKER_REDDISH {wxColour(255, 100, 40)};
    const auto GOLD {wxColour(160, 160, 10)};
    const auto WHITE {wxColour(200, 200, 200)};
    const auto BLACK {wxColour(80, 60, 40)};
    const int SQUARE_SIZE {board_size / 8};

    for (int x {0}; x < 8; x++) {
        for (int y {0}; y < 8; y++) {
            if ((x + y) % 2 == 0) {
                dc.SetBrush(wxBrush(WHITE));
                dc.SetPen(wxPen(WHITE));
            } else {
                dc.SetBrush(wxBrush(BLACK));
                dc.SetPen(wxPen(BLACK));
            }

            dc.DrawRectangle(wxPoint(SQUARE_SIZE * x, SQUARE_SIZE * y), wxSize(SQUARE_SIZE, SQUARE_SIZE));
        }
    }

    if (selected_piece_index != NULL_INDEX) {
        dc.SetBrush(wxBrush(ORANGE));
        dc.SetPen(wxPen(ORANGE));

        for (const Move& move : legal_moves) {
            switch (move.type) {
                case MoveType::Normal: {
                    if (move.normal.source_index == selected_piece_index) {
                        const auto [x, y] {get_square(move.normal.destination_index)};

                        dc.DrawRectangle(wxPoint(SQUARE_SIZE * x, SQUARE_SIZE * y), wxSize(SQUARE_SIZE, SQUARE_SIZE));
                    }

                    break;
                }
                case MoveType::Capture: {
                    if (move.capture.source_index == selected_piece_index) {
                        for (Idx i {0}; i < move.capture.destination_indices_size; i++) {
                            const auto [x, y] {get_square(move.capture.destination_indices[i])};

                            dc.DrawRectangle(wxPoint(SQUARE_SIZE * x, SQUARE_SIZE * y), wxSize(SQUARE_SIZE, SQUARE_SIZE));
                        }
                    }

                    break;
                }
            }
        }

        for (std::size_t i {0}; i < jump_square_indices.size(); i++) {
            const auto count {std::count(jump_square_indices.cbegin(), jump_square_indices.cend(), jump_square_indices[i])};

            if (count < 2) {
                dc.SetBrush(wxBrush(REDDISH));
                dc.SetPen(wxPen(REDDISH));
            } else {
                dc.SetBrush(wxBrush(DARKER_REDDISH));
                dc.SetPen(wxPen(DARKER_REDDISH));
            }

            const auto [x, y] {get_square(jump_square_indices[i])};

            dc.DrawRectangle(wxPoint(SQUARE_SIZE * x, SQUARE_SIZE * y), wxSize(SQUARE_SIZE, SQUARE_SIZE));
        }

        std::vector<Idx> indices_visited;

        for (std::size_t i {0}; i < jump_square_indices.size(); i++) {
            const auto iter {std::find(indices_visited.cbegin(), indices_visited.cend(), jump_square_indices[i])};

            int alignment {0};

            if (iter == indices_visited.cend()) {
                alignment = wxALIGN_LEFT | wxALIGN_TOP;
            } else {
                alignment = wxALIGN_RIGHT | wxALIGN_TOP;
            }

            const auto [x, y] {get_square(jump_square_indices[i])};

            dc.DrawLabel(std::to_string(i + 1), wxRect(SQUARE_SIZE * x, SQUARE_SIZE * y, SQUARE_SIZE, SQUARE_SIZE), alignment);

            indices_visited.push_back(jump_square_indices[i]);
        }
    }

    const auto PIECE_WHITE {wxColour(140, 10, 10)};
    const auto PIECE_BLACK {wxColour(10, 10, 10)};
    const int PIECE_SIZE {static_cast<int>(static_cast<float>(SQUARE_SIZE) / 2.5f)};
    const int OFFSET {SQUARE_SIZE / 2};

    for (Idx i {0}; i < 64; i++) {
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

        const auto [x, y] {get_square(i)};
        const auto position {wxPoint(SQUARE_SIZE * x + OFFSET, SQUARE_SIZE * y + OFFSET)};

        dc.DrawCircle(position, PIECE_SIZE);

        if (king) {
            dc.SetBrush(wxBrush(GOLD));
            dc.SetPen(wxPen(GOLD));
            dc.DrawCircle(position, PIECE_SIZE / 3);
        }
    }
}
