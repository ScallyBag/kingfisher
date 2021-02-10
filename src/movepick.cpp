#include "attacks.h"
#include "bitboard.h"
#include "board.h"
#include "move.h"
#include "movegen.h"
#include "movepick.h"
#include "search.h"
#include "types.h"

Move pickNextMove(const Board& b, const Move& hashMove, int& stage, std::vector<Move>& moves, const int& ply, int& movesTried) {

	switch (stage) {
		
		case START_PICK: {
			stage = TT_PICK;
			[[fallthrough]];
		}

		case TT_PICK: { // We try move from hash table first
			stage = NORMAL_GEN;
			if (hashMove != NO_MOVE && moveIsPsuedoLegal(b, hashMove)) {
				movesTried++;
				return hashMove;
			}
			
			[[fallthrough]];
		}

		case NORMAL_GEN: { // We generate, score and sort the rest of the moves
			stage = NORMAL_PICK;
			bool ageHistory = false;
			moves.clear();

			auto unscoredMoves = genAllMoves(b);
			moves = scoreMoves(b, unscoredMoves, ply, hashMove, ageHistory);
			if (ageHistory) reduceHistory();
			std::sort(moves.begin(), moves.end(), [](const auto& a, const auto& b) { return a.score > b.score; });

			[[fallthrough]];
		}
		
		case NORMAL_PICK: {
			REPICK:

			movesTried++;

			// Check if out of bounds
			if (movesTried > moves.size()) {
				assert(movesTried == moves.size() + 1);
				movesTried--;
				return NO_MOVE;
			}

			Move m = moves[movesTried - 1];
			if (!moveIsPsuedoLegal(b, m)) {
				printBoard(b);
				std::cout << toNotation(m) << "\n";
				assert(false);
			}

			// We skip the hash move
			if (m == hashMove) {
				movesTried--;
				goto REPICK; // FIXME: Find a way to do this without goto
			}

			return m;
		}

		default: {
			return NO_MOVE;
		}

	}

}