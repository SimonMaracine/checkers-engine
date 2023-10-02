#pragma once

#include <array>
#include <functional>
#include <optional>
#include <utility>
#include <vector>

#include <wx/wx.h>

class Board : public wxWindow {
public:
    static constexpr int NULL_INDEX = -1;

    struct Move {
        int source_index = NULL_INDEX;
        int destination_index = NULL_INDEX;
    };

    enum class Player {
        Black,
        White
    };

    using OnPieceMove = std::function<bool(Move)>;

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

    void on_paint(wxPaintEvent& event);
    void on_mouse_move(wxMouseEvent& event);
    void on_mouse_left_down(wxMouseEvent& event);
    void on_mouse_right_down(wxMouseEvent& event);

    int get_square(wxPoint position);
    std::pair<int, int> get_square(int square_index);
    bool select_piece(int square_index);
    std::vector<Move> generate_moves();
    void generate_piece_moves(std::vector<Move>& moves, int square_index, Player player, bool king);
    void check_piece_jumps(std::vector<Move>& moves, int square_index, Player player, bool king);
    int offset(int square_index, Direction direction, Diagonal diagonal);
    void change_turn();

    void draw(wxDC& dc);

    int board_size = 0;

    std::array<Square, 64> board {};
    Player turn = Player::Black;
    int selected_piece_index = NULL_INDEX;

    OnPieceMove on_piece_move;

    wxDECLARE_EVENT_TABLE();
};
