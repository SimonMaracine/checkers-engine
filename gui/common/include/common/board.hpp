#pragma once

#include <array>
#include <functional>
#include <utility>
#include <vector>
#include <string>
#include <cstddef>

#include <wx/wx.h>

namespace board {
    class CheckersBoard : public wxWindow {
    public:
        using Idx = int;

        static constexpr Idx NULL_INDEX {-1};

        enum class MoveType {
            Normal,
            Capture
        };

        struct Move {
            union {
                // Indices are in the range [0, 63]
                struct {
                    Idx source_index;
                    Idx destination_index;
                } normal;

                struct {
                    Idx source_index;
                    Idx destination_indices[9u];
                    std::size_t destination_indices_size;
                } capture;
            };

            MoveType type {};
        };

        enum class Player : unsigned int {
            Black = 0b0001u,
            White = 0b0010u
        };

        enum class GameOver {
            None,
            WinnerBlack,
            WinnerWhite,
            Tie
        };

        using OnPieceMove = std::function<void(const Move&)>;

        CheckersBoard(wxFrame* parent, int x, int y, int size, const OnPieceMove& on_piece_move);

        void set_board_size(int size);
        void reset();
        void set_position(const std::string& fen_string);
        void set_user_input(bool user_input);
        void play_move(const Move& move);
        void play_move(const std::string& move_string);

        static std::string move_to_string(const Move& move);

        GameOver get_game_over() const { return game_over; }
        Player get_player() const { return turn; }
        const std::vector<Move>& get_legal_moves() const { return legal_moves; }
        unsigned int get_plies_without_advancement() const { return plies_without_advancement; }
        std::size_t get_repetition_size() const { return repetition.positions.size(); }
    private:
        enum class Direction {
            NorthEast,
            NorthWest,
            SouthEast,
            SouthWest
        };

        enum Diagonal {
            Short = 1,
            Long = 2
        };

        enum class Square : unsigned int {
            None      = 0b0000u,  // I don't usually align like this :P
            Black     = 0b0001u,
            BlackKing = 0b0101u,
            White     = 0b0010u,
            WhiteKing = 0b0110u
        };

        using Board = std::array<Square, 64u>;

        struct JumpCtx {
            Board board {};  // Use a copy of the board
            Idx source_index {};
            std::vector<Idx> destination_indices;
        };

        void on_paint(wxPaintEvent&);
        void on_mouse_left_down(wxMouseEvent& event);
        void on_mouse_right_down(wxMouseEvent& event);

        Idx get_square(wxPoint position) const;
        std::pair<Idx, Idx> get_square(Idx square_index) const;
        bool select_piece(Idx square_index);
        std::vector<Move> generate_moves() const;
        void generate_piece_capture_moves(std::vector<Move>& moves, Idx square_index, Player player, bool king) const;
        void generate_piece_moves(std::vector<Move>& moves, Idx square_index, Player player, bool king) const;
        bool check_piece_jumps(std::vector<Move>& moves, Idx square_index, Player player, bool king, JumpCtx& ctx) const;
        Idx offset(Idx square_index, Direction direction, Diagonal diagonal) const;
        void change_turn();
        void check_80_move_rule(bool advancement);
        void check_piece_crowning(Idx square_index);
        void check_legal_moves();
        void check_repetition(bool advancement);
        bool playable_normal_move(const Move& move, Idx square_index) const;
        bool playable_capture_move(const Move& move, const std::vector<Idx>& square_indices) const;
        void play_normal_move(const Move& move);
        void play_capture_move(const Move& move);
        void select_jump_square(Idx square_index);
        void deselect_jump_square(Idx square_index);
        void remove_jumped_pieces(const Move& move);
        static Idx get_jumped_piece_index(Idx index1, Idx index2);
        static Player opponent(Player player);
        static bool validate_fen_string(const std::string& fen_string);
        static Player parse_player(const std::string& fen_string, std::size_t index);
        void parse_pieces(const std::string& fen_string, std::size_t& index, Player player);
        static std::pair<Idx, bool> parse_piece(const std::string& fen_string, std::size_t& index);
        static Idx translate_index_1_32_to_0_64(Idx index);
        static Idx translate_index_0_64_to_1_32(Idx index);
        static bool valid_move_string(const std::string& move_string);
        static unsigned int parse_number(const std::string& move_string, std::size_t& index);
        static Idx parse_source_square(const std::string& move_string, std::size_t& index);
        static std::pair<std::array<Idx, 9u>, std::size_t> parse_destination_squares(const std::string& move_string, std::size_t& index);
        static bool is_capture_move(Idx source, const std::array<Idx, 9u>& destinations, std::size_t count);
        void clear();
        void refresh_canvas();
        void draw(wxDC& dc);

        // Used for mouse cursor detection
        int board_size {0};

        // Called every time a move has been made
        OnPieceMove on_piece_move;

        // Used to differentiate between human and computer turns
        bool user_input {true};

        // Game data
        Board board {};
        Player turn {Player::Black};
        Idx selected_piece_index {NULL_INDEX};
        std::vector<Move> legal_moves;
        std::vector<Idx> jump_square_indices;
        unsigned int plies_without_advancement {0u};
        GameOver game_over {GameOver::None};

        struct Repetition {
            struct Position {
                Board board {};
                Player turn {Player::Black};

                bool operator==(const Position& other) const {
                    return board == other.board && turn == other.turn;
                }
            };

            std::vector<Position> positions;
        } repetition;

        wxDECLARE_EVENT_TABLE();
    };
}
