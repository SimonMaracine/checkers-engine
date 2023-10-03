#pragma once

#include <array>
#include <functional>
#include <optional>
#include <utility>
#include <vector>
#include <stack>

#include <wx/wx.h>

class Board : public wxWindow {
public:
    static constexpr int NULL_INDEX = -1;

    enum class MoveType {
        Normal,
        Capture
    };

    struct Move {
        union {
            struct {
                int source_index;
                int destination_index;
            } normal;

            struct {
                int source_index;
                int destination_index;
                int intermediary_square_indices[9];
                int captured_pieces_indices[9];
            } capture;
        };

        MoveType type {};
    };

    enum class Player : unsigned int {
        Black = 0b0001u,
        White = 0b0010u,
    };

    using OnPieceMove = std::function<bool(const Move&)>;

    Board(wxFrame* parent, int x, int y, int size, OnPieceMove on_piece_move);

    void set_position(int x, int y);
    void set_size(int size);

    void reset();
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
        White     = 0b0001u,
        WhiteKing = 0b0101u,
        Black     = 0b0010u,
        BlackKing = 0b0110u
    };

    struct JumpCtx {
        unsigned int jumps = 0;
        int source_index {};
        std::stack<int, std::vector<int>> intermediary_square_indices;
        std::stack<int, std::vector<int>> captured_pieces_indices;
    };

    void on_paint(wxPaintEvent& event);
    void on_mouse_move(wxMouseEvent& event);
    void on_mouse_left_down(wxMouseEvent& event);
    void on_mouse_right_down(wxMouseEvent& event);

    int get_square(wxPoint position);
    std::pair<int, int> get_square(int square_index);
    bool select_piece(int square_index);
    std::vector<Move> generate_moves();
    void generate_piece_capture_moves(std::vector<Move>& moves, int square_index, Player player, bool king);
    void generate_piece_moves(std::vector<Move>& moves, int square_index, Player player, bool king);
    bool check_piece_jumps(std::vector<Move>& moves, int square_index, Player player, bool king, JumpCtx& ctx);
    int offset(int square_index, Direction direction, Diagonal diagonal);
    void change_turn();
    void try_play_normal_move(const Move& move, int square_index);
    void try_play_capture_move(const Move& move, int square_index);

    void draw(wxDC& dc);

    int board_size = 0;

    std::array<Square, 64> board {};
    Player turn = Player::Black;
    int selected_piece_index = NULL_INDEX;

    OnPieceMove on_piece_move;

    wxDECLARE_EVENT_TABLE();
};
