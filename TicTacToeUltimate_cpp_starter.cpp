#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <assert.h>
#include <cstring>
#include <ctime>
#include <vector>
#include <iomanip>

#define PLUS_INF 2147483647
#define MINUS_INF -2147483647

struct settings {
	int timebank; // timpul ramas
	int time_per_move; 
	int botId;
	int round;
	int move;
	int timePool; 
	int expected_move_count;
	int countPerMs = 0;
	std::vector<std::string> playerNames;
	std::string myName;
};

struct board {
	int field[9][9];
	int macroboard[3][3];
	const int Three_in_a_Row[8][3] = {
		{ 0, 1, 2 },
		{ 3, 4, 5 },
		{ 6, 7, 8 },
		{ 0, 3, 6 },
		{ 1, 4, 7 },
		{ 2, 5, 8 },
		{ 0, 4, 8 },
		{ 2, 4, 6 }
	};
	const int Heuristic_Array[4][4] = {
		{ 0, -10, -100, -1000 },
		{ 10, 0, 0, 0 },
		{ 100, 0, 0, 0 },
		{ 1000, 0, 0, 0 }
	};

	const int corners[3] = { 0, 3, 6 };

	board* copy() {
		board* newBoard = new board();
		std::memcpy(&(newBoard->field)[0][0], &field[0][0], sizeof(int) * 9 * 9);
		std::memcpy(&(newBoard->macroboard)[0][0], &macroboard[0][0], sizeof(int) * 3 * 3);
		return newBoard;
	}

	int winner() {
		// pe linii
		for (int y = 0; y < 3; ++y) {
			int player = macroboard[0][y];
			if (player > 0)
				if (macroboard[1][y] == player && macroboard[2][y] == player)
					return player;
		}
		// pe coloane
		for (int x = 0; x < 3; ++x) {
			int player = macroboard[x][0];
			if (player > 0)
				if (macroboard[x][1] == player && macroboard[x][2] == player)
					return player;
		}
		// pe diagonale
		int player = macroboard[1][1];
		if (player > 0) {
			if (macroboard[0][0] == player && macroboard[2][2] == player)
				return player;
			if (macroboard[2][0] == player && macroboard[0][2] == player)
				return player;
		}

		for (int y = 0; y < 3; ++y) 
			for (int x = 0; x < 3; ++x)
				if (macroboard[x][y] != -2 && macroboard[x][y] != 1 && macroboard[x][y] != 2)
					return 0; // inca se mai joaca
		return -1; // e remiza
	}

	int evaluate_stuff(int player) {
		int winplayer = winner();
		int opponent = !(player - 1) + 1;

		if (winplayer == player)
			return PLUS_INF;
		else if (winplayer == opponent)
			return MINUS_INF;
		else if (winplayer != 0)
			return 0;

		int score = 0;
		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j)
				score += score_miniBoard(corners[j], corners[i], player);

		score += score_macroBoard(player);

		return score;
	}
	
	int score_miniBoard(int x, int y, int player) {
		int opponent = (player == 2) ? 1 : 2, piece;
		int players, others, score = 0, i, j, counter = 0;

		int board[9];
		for (int col = y; col < y + 3; ++col) 
			for (int row = x; row < x + 3; ++row) 
				board[counter++] = field[row][col];		

		for (i = 0; i < 8; ++i)  {
			players = others = 0;
			for (j = 0; j < 3; ++j)  {
				piece = board[Three_in_a_Row[i][j]];
				if (piece == player)
					players++;
				else if (piece == opponent)
					others++;
			}
			score += Heuristic_Array[players][others];
		}
		return score;
	}

	int score_macroBoard(int player) {
		int opponent = (player == 2) ? 1 : 2, piece;
		int players, others, score = 0, i, j, counter = 0;

		int board[9];
		for (int col = 0; col < 3; ++col) 
			for (int row = 0; row < 3; ++row) 
				board[counter++] = macroboard[row][col];		

		for (i = 0; i < 8; ++i)  {
			players = others = 0;
			for (j = 0; j < 3; ++j)  {
				piece = board[Three_in_a_Row[i][j]];
				if (piece == player)
					players++;
				else if (piece == opponent)
					others++;
			}
			score += Heuristic_Array[players][others] * 100;
		}
		return score;
	}

	// numarul de patrate in care am voie sa pun
	int get_nr_free_macroboard() {
		int count = 0;
		for (int y = 0; y < 3; ++y)
			for (int x = 0; x < 3; ++x)
				if (macroboard[x][y] == -1)
					count++;
		return count;
	}

	// numarul de mutari disponibile
	int get_nr_free_field() {
		int free_field_cells = 0;
		for (int macro_y = 0; macro_y < 3; ++macro_y)
			for (int macro_x = 0; macro_x < 3; ++macro_x)
				if (macroboard[macro_x][macro_y] == -1)
					for (int y = 0; y < 3; ++y)
						for (int x = 0; x < 3; ++x)
							if (field[3 * macro_x + x][3 * macro_y + y] == 0)
								free_field_cells++;
		return free_field_cells;
	}

	std::pair<int, int> get_free_cells_status() {
		int free_macro_cells = 0, free_field_cells = 0;
		for (int macro_y = 0; macro_y < 3; ++macro_y)
			for (int macro_x = 0; macro_x < 3; ++macro_x)
				if (macroboard[macro_x][macro_y] == -1) {
					free_macro_cells++;
					for (int y = 0; y < 3; ++y)
						for (int x = 0; x < 3; ++x)
							if (field[3 * macro_x + x][3 * macro_y + y] == 0)
								free_field_cells++;
				}
			
		return std::make_pair(free_macro_cells, free_field_cells);
	}

	std::vector<std::pair<int, int> > get_available_moves() {
		std::vector<std::pair<int, int> > am;

		for (int macro_y = 0; macro_y < 3; ++macro_y)
			for (int y = 0; y < 3; ++y)
				for (int macro_x = 0; macro_x < 3; ++macro_x)
					for (int x = 0; x < 3; ++x)
						if (macroboard[macro_x][macro_y] == -1)
							if (field[3 * macro_x + x][3 * macro_y + y] == 0)
								am.push_back(std::make_pair(3 * macro_x + x, 3 * macro_y + y));
		return am;
	}

	void make_move_and_update_macroboard(int player, std::pair<int, int> move) {
		int x = move.first;
		int y = move.second;
		int macro_x = (int)(x / 3);
		int macro_y = (int)(y / 3);
		int next_macro_x = x % 3;
		int next_macro_y = y % 3;
		int this_row = 3 * macro_x;
		int first_row_after = this_row + 1;
		int second_row_after = this_row + 2;
		int this_column = 3 * macro_y;
		int first_column_after = this_column + 1;
		int second_column_after = this_column + 2;

		field[x][y] = player;		
		bool line_win = field[this_row][y] == field[first_row_after][y] && field[first_row_after][y] == field[second_row_after][y];
		bool column_win = field[x][this_column] == field[x][first_column_after] && field[x][first_column_after] == field[x][second_column_after];
		bool first_diagonal_win = field[this_row][this_column] == player && field[first_row_after][first_column_after] == player && field[second_row_after][second_column_after] == player;
		bool second_diagonal_win = field[second_row_after][this_column] == player && field[first_row_after][first_column_after] == player && field[this_row][second_column_after] == player;

		// marchez castigul jucatorului
		if (line_win || column_win || first_diagonal_win || second_diagonal_win)
			macroboard[macro_x][macro_y] = player;

		for (int macro_y = 0; macro_y < 3; ++macro_y)
			for (int macro_x = 0; macro_x < 3; ++macro_x) {
				// nu se mai poate juca in fostele patratele active
				if (macroboard[macro_x][macro_y] == -1)
					macroboard[macro_x][macro_y] = 0;
				if (macroboard[macro_x][macro_y] == 0) { // verific daca este remiza in acel patratel
					macroboard[macro_x][macro_y] = -2; // daca da marchez cu -2
					bool cont = true;

					for (int y = 0; y < 3 && cont; ++y)
						for (int x = 0; x < 3 && cont; ++x)
							// daca am gasit o casuta libera inseamna ca nu e remiza si marchez ca nu se poate juca acolo
							if (field[this_row + x][this_column + y] == 0) {
								macroboard[macro_x][macro_y] = 0;
								cont = false;
								break;
							}
				}
			}
		
		if (macroboard[next_macro_x][next_macro_y] == 0)
			macroboard[next_macro_x][next_macro_y] = -1;
		else {
			for (int i = 0; i < 3; ++i)
				for (int j = 0; j < 3; ++j)
					if (macroboard[j][i] == 0)
						macroboard[j][i] = -1;
		}
	}
};

std::pair<int, int> get_random_free_cell(board* gameBoard) {
	std::vector<std::pair<int, int> > available_moves = gameBoard->get_available_moves();
	return available_moves[rand() % available_moves.size()];
}

// comparator pentru scor
bool score_comparator(std::pair<std::pair<int, int>, int> a, std::pair<std::pair<int, int>, int> b) {
	return (a.second > b.second); 
}

// obtine cea mai buna urmatoare mutare
// verifica doar cu un pas in fata
std::pair<int, int> get_greedy_cell(settings* gameSettings, board* gameBoard) {
	std::vector<std::pair<std::pair<int, int>, int> > moves;
	for (int macro_y = 0; macro_y < 3; ++macro_y)
		for (int y = 0; y < 3; ++y)
			for (int macro_x = 0; macro_x < 3; ++macro_x)
				for (int x = 0; x < 3; ++x)
					if (gameBoard->macroboard[macro_x][macro_y] == -1)
						if (gameBoard->field[3 * macro_x + x][3 * macro_y + y] == 0) {
							board* newBoard = gameBoard->copy();
							newBoard->make_move_and_update_macroboard(gameSettings->botId, std::make_pair(3 * macro_x + x, 3 * macro_y + y));

							int score_per_move = newBoard->evaluate_stuff(gameSettings->botId);
							moves.push_back(std::make_pair(std::make_pair(3 * macro_x + x, 3 * macro_y + y), score_per_move));
						}

	std::sort(moves.begin(), moves.end(), score_comparator);

	return moves[0].first;
}

int alpha_beta(board* b, int currPlayer, int scorePlayer, bool maximizing, int alpha, int beta, int* count, int* totalCount, bool getIndex, int startTime) {
	if ((*count) <= 1 || b->winner() != 0) {
		(*count) = 0; (*totalCount)++;
		return b->evaluate_stuff(scorePlayer);
	}

	int best_score = maximizing ? MINUS_INF : PLUS_INF;
	std::pair<int, int> best_move = std::make_pair(-1, -1);
	std::pair<int, int> free_status = b->get_free_cells_status();
	int free_macro_cells = free_status.first;
	int free_field_cells = free_status.second;

	for (int macro_y = 0; macro_y < 3; ++macro_y)
		for (int y = 0; y < 3; ++y)
			for (int macro_x = 0; macro_x < 3; ++macro_x)
				for (int x = 0; x < 3; ++x)
					if (b->macroboard[macro_x][macro_y] == -1)
						if (b->field[3 * macro_x + x][3 * macro_y + y] == 0) {
							board* newBoard = b->copy();
							newBoard->make_move_and_update_macroboard(currPlayer, std::make_pair(3 * macro_x + x, 3 * macro_y + y));

							int nextCount = (int)round(*count / free_field_cells);
							if (nextCount == 0)
								nextCount = 1;
							// daca am mai putin de 2 macro_cells libere, inseamna ca as avea maxim 18 casute de incercat
							// fiind putin, imi permit sa dublez numarul de noduri de cautat
							if (newBoard->macroboard[macro_x][macro_y] == currPlayer && free_macro_cells <= 2)
								nextCount *= 2;

							(*count) -= nextCount;
							int v = alpha_beta(newBoard, !(currPlayer - 1) + 1, scorePlayer, !maximizing, alpha, beta, &nextCount, totalCount, false, startTime);
							(*count) += nextCount;
							free_field_cells--;

							delete newBoard;
							if (maximizing) {
								if (getIndex && (v > best_score || v == PLUS_INF))
									best_move = std::make_pair(3 * macro_x + x, 3 * macro_y + y);

								best_score = std::max(best_score, v);
								alpha = std::max(alpha, best_score);

								if (best_score == MINUS_INF && getIndex)
									best_move = std::make_pair(3 * macro_x + x, 3 * macro_y + y);

							}
							else {
								if (getIndex && (v < best_score || v == MINUS_INF))
									best_move = std::make_pair(3 * macro_x + x, 3 * macro_y + y);

								best_score = std::min(best_score, v);
								beta = std::min(beta, best_score);

								if (best_score == PLUS_INF && getIndex)
									best_move = std::make_pair(3 * macro_x + x, 3 * macro_y + y);

							}

							if (beta <= alpha) { // prune...
								if (getIndex)
									return 9 * best_move.second + best_move.first;
								return beta;
							}

						}
	if (getIndex)
		return 9 * best_move.second + best_move.first;
	return best_score;
}

/**
* Implement this function.
* type is always "move"
*
* return value must be position in x,y presentation
*      (use std::make_pair(x, y))
*/
std::pair<int, int> action(settings* gameSettings, board* gameBoard, const std::string &type, int timeleft) {
	int start_time = clock();
	gameSettings->timePool = timeleft; // modific timpul actual
	int time = gameSettings->timePool / gameSettings->expected_move_count; // timpul alocat pentru o mutare
	// modific depth-ul pentru alpha-beta dinamic

	// prima mutare hard-code
	// am observat ca aceasta mutare e cea mai buna
	if (gameSettings->move == 1)
		return std::make_pair(4, 4);

	// verific daca pot sa fac mutarea castigatoare imediat, pentru a nu mai prelungi meciul
	if (gameSettings->move >= 26) {
		for (int macro_y = 0; macro_y < 3; ++macro_y)
			for (int y = 0; y < 3; ++y)
				for (int macro_x = 0; macro_x < 3; ++macro_x)
					for (int x = 0; x < 3; ++x)
						if (gameBoard->macroboard[macro_x][macro_y] == -1)
							if (gameBoard->field[3 * macro_x + x][3 * macro_y + y] == 0) {
								board* newBoard = gameBoard->copy();
								newBoard->make_move_and_update_macroboard(gameSettings->botId, std::make_pair(3 * macro_x + x, 3 * macro_y + y));

								if (newBoard->winner() == gameSettings->botId)
									return std::make_pair(3 * macro_x + x, 3 * macro_y + y);
							}
	}
		
	try {
		// daca am resetat nr nodurilor de analizat, fac o noua verificare pentru a vedea in cat timp ar trebui
		// sa ma incadrez in urmatoarea verificare
		if (gameSettings->countPerMs == 0) {
			int currTime = clock();
			int testTotalCount = 0;

			while (clock() / double(CLOCKS_PER_SEC) * 1000 < currTime / double(CLOCKS_PER_SEC) * 1000 + 10) {
				int testCount = 10000; // reprezinta cate noduri de test incerc cu un alpha-beta mai chior
				alpha_beta(gameBoard, gameSettings->botId, gameSettings->botId, true, MINUS_INF, PLUS_INF, &testCount, &testTotalCount, false, 0);
			}

			gameSettings->countPerMs = testTotalCount / 10; // numarul de noduri analizate per ms
		}

		int count = gameSettings->countPerMs * time; // cate noduri am de gand sa analizez la mutarea aceasta
		int totalCount = 0;
		int startT = clock();
		int index = alpha_beta(gameBoard, gameSettings->botId, gameSettings->botId, true, MINUS_INF, PLUS_INF, &count, &totalCount, true, clock());
		int stopT = clock();
		int x = index % 9;
		int y = (int)((index - x) / 9);
		int stop_time = clock();
		int timeInMs = (stopT - startT) / double(CLOCKS_PER_SEC) * 1000;

		if (timeInMs > 0) // actualizez nr nodurilor in functie de cat a durat ultima executie
			gameSettings->countPerMs = totalCount / timeInMs;
		else 
			gameSettings->countPerMs = 0;
		// incerc si mai adanc...
		if (gameSettings->expected_move_count > 2)
			gameSettings->expected_move_count--;

		return std::make_pair(x, y);
	}
	catch (...) {
		// in cazul in care raman fara timp, pentru a nu pierde la failed to parse input
		// macar sa pierd/castig pe noroc :))
		return get_random_free_cell(gameBoard);
	}
}

void debug(const std::string &s) {
	std::cerr << s << "\n" << std::flush;
}

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
	std::stringstream ss(s);
	std::string item;
	elems.clear();
	while (std::getline(ss, item, delim))
		elems.push_back(item);
	return elems;
}

void update(settings* s, board* gameBoard, const std::string& player, const std::string& type, const std::string& value) {
	if (player != "game" && player != s->myName)
		return;
	if (type == "round")
		s->round = std::stoi(value);
	else if (type == "move")
		s->move = std::stoi(value);
	else if (type == "macroboard") {
		std::stringstream ss(value);
		for (int y = 0; y < 3; ++y)
			for (int x = 0; x < 3; ++x) {
				ss >> gameBoard->macroboard[x][y];
				if (ss.peek() == ',')
					ss.ignore();
			}
	}
	else if (type == "field") {
		std::stringstream ss(value);
		for (int y = 0; y < 9; ++y)
			for (int x = 0; x < 9; ++x) {
				ss >> gameBoard->field[x][y];
				if (ss.peek() == ',')
					ss.ignore();
			}
	}
	else
		debug("Unknown update <" + type + ">.");
}

void setting(settings* s, const std::string& type, const std::string& value) {
	if (type == "timebank")
		s->timebank = std::stoi(value);
	else if (type == "time_per_move")
		s->time_per_move = std::stoi(value);
	else if (type == "player_names")
		split(value, ',', s->playerNames);
	else if (type == "your_bot")
		s->myName = value;
	else if (type == "your_botid")
		s->botId = std::stoi(value);
	else
		debug("Unknown setting <" + type + ">.");
}

void processCommand(settings* s, board* gameBoard, const std::vector<std::string> &command) {
	if (command[0] == "action") {
		auto point = action(s, gameBoard, command[1], std::stoi(command[2]));
		std::cout << "place_move " << point.first << " " << point.second << "\n" << std::flush;
	}
	else if (command[0] == "update") 
		update(s, gameBoard, command[1], command[2], command[3]);
	else if (command[0] == "settings") 
		setting(s, command[1], command[2]);
	else 
		debug("Unknown command <" + command[0] + ">.");
}

void loop() {
	srand(static_cast<unsigned int>(time(0)));
	std::string line;
	std::vector<std::string> command;
	command.reserve(256);
	settings* gameSettings = new settings();
	gameSettings->expected_move_count = 10;
	board* gameBoard = new board();

	while (std::getline(std::cin, line)) {
		processCommand(gameSettings, gameBoard, split(line, ' ', command));
	}
}

int main() {

	loop();
	
}