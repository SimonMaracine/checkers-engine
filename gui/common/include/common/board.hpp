#pragma once

#include <array>
#include <functional>
#include <utility>
#include <vector>
#include <string>
#include <cstddef>

#include <common/wx.hpp>

namespace board {
    class CheckersBoard : public wxWindow {
    public:
        static constexpr int NULL_INDEX {-1};

        enum class MoveType {
            Normal,
            Capture
        };

        struct Move {
            // Always in the range [0, 31]
            union {
                struct {
                    int source_index;
                    int destination_index;
                } normal;

                struct {
                    int source_index;
                    int destination_indices[9];
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
        void reset_position();
        void set_position(const std::string& fen_string);
        void set_user_input(bool user_input);
        void set_show_indices(bool show_indices);
        void play_move(const Move& move);
        void play_move(const std::string& move_string);

        static std::string move_to_string(const Move& move);
        static Player opponent(Player player);

        GameOver get_game_over() const { return m_game_over; }
        Player get_player() const { return m_turn; }
        const std::vector<Move>& get_legal_moves() const { return m_legal_moves; }
        unsigned int get_plies_without_advancement() const { return m_plies_without_advancement; }
        std::size_t get_repetition_size() const { return m_repetition_positions.size(); }
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

        using Board = std::array<Square, 32>;

        struct JumpCtx {
            Board board {};  // Use a copy of the board
            int source_index {};
            std::vector<int> destination_indices;
        };

        void on_paint(wxPaintEvent&);
        void on_mouse_left_down(wxMouseEvent& event);
        void on_mouse_right_down(wxMouseEvent&);

        int get_square(wxPoint position) const;
        static std::pair<int, int> get_square(int square);
        static bool is_black_square(int square);
        bool select_piece(int square_index);
        std::vector<Move> generate_moves() const;
        void generate_piece_capture_moves(std::vector<Move>& moves, int square_index, Player player, bool king) const;
        void generate_piece_moves(std::vector<Move>& moves, int square_index, Player player, bool king) const;
        bool check_piece_jumps(std::vector<Move>& moves, int square_index, Player player, bool king, JumpCtx& ctx) const;
        int offset_index(int square_index, Direction direction, Diagonal diagonal) const;
        void change_turn();
        void check_80_move_rule(bool advancement);
        void check_piece_crowning(int square_index);
        void check_legal_moves();
        void check_repetition(bool advancement);
        bool playable_normal_move(const Move& move, int square_index) const;
        bool playable_capture_move(const Move& move, const std::vector<int>& square_indices) const;
        void play_normal_move(const Move& move);
        void play_capture_move(const Move& move);
        void select_jump_square(int square_index);
        void deselect_jump_square(int square_index);
        void remove_jumped_pieces(const Move& move);
        void remember_move(const Move& move);
        static int get_jumped_piece_index(int index1, int index2);
        static bool validate_fen_string(const std::string& fen_string);
        static Player parse_player(const std::string& fen_string, std::size_t index);
        void parse_pieces(const std::string& fen_string, std::size_t& index, Player player);
        static std::pair<int, bool> parse_piece(const std::string& fen_string, std::size_t& index);
        static int translate_1_32_to_0_64(int index);
        static int translate_0_64_to_1_32(int index);
        static int to_0_31(int index);
        static int to_1_32(int index);
        static bool valid_move_string(const std::string& move_string);
        static unsigned int parse_number(const std::string& move_string, std::size_t& index);
        static int parse_source_square(const std::string& move_string, std::size_t& index);
        static std::vector<int> parse_destination_squares(const std::string& move_string, std::size_t& index);
        static bool is_capture_move(int source, int destination);
        void clear();
        void refresh_canvas();
        void draw(wxDC& dc);

        // Used for mouse cursor detection
        int m_board_size {0};

        // Called every time a move has been made
        OnPieceMove m_on_piece_move;

        // Used to differentiate between human and computer turns
        bool m_user_input {true};

        // Used for quicker analysis
        bool m_show_indices {false};

        // GUI data
        int m_selected_piece_index {NULL_INDEX};
        std::vector<Move> m_legal_moves;
        std::vector<Move> m_last_moves;
        std::vector<int> m_jump_square_indices;
        GameOver m_game_over {GameOver::None};

        struct Position {
            Board board {};
            Player turn {Player::Black};

            bool operator==(const Position& other) const {
                return board == other.board && turn == other.turn;
            }
        };

        // Game data
        Board m_board {};
        Player m_turn {Player::Black};
        unsigned int m_plies_without_advancement {0};
        std::vector<Position> m_repetition_positions;

        wxDECLARE_EVENT_TABLE();
    };
}
