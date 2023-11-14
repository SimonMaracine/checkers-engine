#pragma once

#include <array>
#include <functional>
#include <utility>
#include <vector>
#include <string>
#include <cstddef>

#include <wx/wx.h>

class Board : public wxWindow {
public:
    using Idx = int;

    static constexpr Idx NULL_INDEX = -1;

    enum class MoveType {
        Normal,
        Capture
    };

    struct Move {
        union {
            struct {
                Idx source_index;
                Idx destination_index;
            } normal;

            struct {
                Idx source_index;
                Idx destination_index;
                Idx intermediary_square_indices[9];
                Idx captured_pieces_indices[9];  // FIXME remove; can be deduced
                Idx intermediary_square_indices_size;
                Idx captured_pieces_indices_size;
            } capture;
        };

        MoveType type {};
    };

    enum class Player : unsigned int {
        Black = 0b0001u,
        White = 0b0010u,
    };

    using OnPieceMove = std::function<bool(const Move&)>;

    Board(wxFrame* parent, int x, int y, int size, const OnPieceMove& on_piece_move);

    void set_board_position(int x, int y);
    void set_board_size(int size);

    void reset();
    bool set_position(const std::string& fen_string);
private:
    enum class Direction {
        NorthEast,
        NorthWest,
        SouthEast,
        SouthWest
    };

    enum Diagonal {
        Short = 0,
        Long = 1
    };

    enum class Square : unsigned int {
        None      = 0b0000u,  // I don't usually do this :P
        Black     = 0b0001u,
        BlackKing = 0b0101u,
        White     = 0b0010u,
        WhiteKing = 0b0110u
    };

    struct JumpCtx {
        Idx source_index {};
        std::vector<Idx> intermediary_square_indices;
        std::vector<Idx> captured_pieces_indices;
    };

    void on_paint(wxPaintEvent& event);
    void on_mouse_left_down(wxMouseEvent& event);

    Idx get_square(wxPoint position);
    std::pair<Idx, Idx> get_square(Idx square_index);
    bool select_piece(Idx square_index);
    std::vector<Move> generate_moves();
    void generate_piece_capture_moves(std::vector<Move>& moves, Idx square_index, Player player, bool king);
    void generate_piece_moves(std::vector<Move>& moves, Idx square_index, Player player, bool king);
    bool check_piece_jumps(std::vector<Move>& moves, Idx square_index, Player player, bool king, JumpCtx& ctx);
    Idx offset(Idx square_index, Direction direction, Diagonal diagonal);
    void change_turn();
    void check_piece_crowning(Idx square_index);
    void try_play_normal_move(const Move& move, Idx square_index);
    void try_play_capture_move(const Move& move, Idx square_index);
    static Player opponent(Player player);
    static bool validate_fen_string(const std::string& fen_string);
    static Player parse_player(const std::string& fen_string, std::size_t index);
    void parse_pieces(const std::string& fen_string, std::size_t& index, Player player);
    static std::pair<Idx, bool> parse_piece(const std::string& fen_string, std::size_t& index);
    static Idx translate_index_1_32_to_0_64(Idx index);
    void clear();

    void draw(wxDC& dc);

    int board_size = 0;

    std::array<Square, 64> board {};
    Player turn = Player::Black;
    Idx selected_piece_index = NULL_INDEX;
    std::vector<Move> legal_moves;

    OnPieceMove on_piece_move;

    wxDECLARE_EVENT_TABLE();
};
