#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <utility>
#include <queue>
#include <vector>
#include <cstring>

extern int rows;         // The count of rows of the game map.
extern int columns;      // The count of columns of the game map.
extern int total_mines;  // The count of mines of the game map.

// You MUST NOT use any other external variables except for rows, columns and total_mines.

// Custom global variables for client
char client_map[35][35];     // The map as seen by client
bool client_visited[35][35]; // Whether grid has been visited
bool client_marked[35][35];  // Whether grid has been marked
int client_mine_count[35][35]; // Mine count for visited grids
int client_visited_count;    // Number of visited grids
int client_marked_count;     // Number of marked grids

/**
 * @brief The definition of function Execute(int, int, bool)
 *
 * @details This function is designed to take a step when player the client's (or player's) role, and the implementation
 * of it has been finished by TA. (I hope my comments in code would be easy to understand T_T) If you do not understand
 * the contents, please ask TA for help immediately!!!
 *
 * @param r The row coordinate (0-based) of the block to be visited.
 * @param c The column coordinate (0-based) of the block to be visited.
 * @param type The type of operation to a certain block.
 * If type == 0, we'll execute VisitBlock(row, column).
 * If type == 1, we'll execute MarkMine(row, column).
 * If type == 2, we'll execute AutoExplore(row, column).
 * You should not call this function with other type values.
 */
void Execute(int r, int c, int type);

/**
 * @brief The definition of function InitGame()
 *
 * @details This function is designed to initialize the game. It should be called at the beginning of the game, which
 * will read the scale of the game map and the first step taken by the server (see README).
 */
void InitGame() {
  // Initialize all custom global variables
  client_visited_count = 0;
  client_marked_count = 0;
  
  for (int i = 0; i < 35; i++) {
    for (int j = 0; j < 35; j++) {
      client_map[i][j] = '?';
      client_visited[i][j] = false;
      client_marked[i][j] = false;
      client_mine_count[i][j] = 0;
    }
  }
  
  int first_row, first_column;
  std::cin >> first_row >> first_column;
  Execute(first_row, first_column, 0);
}

/**
 * @brief The definition of function ReadMap()
 *
 * @details This function is designed to read the game map from stdin when playing the client's (or player's) role.
 * Since the client (or player) can only get the limited information of the game map, so if there is a 3 * 3 map as
 * above and only the block (2, 0) has been visited, the stdin would be
 *     ???
 *     12?
 *     01?
 */
void ReadMap() {
  for (int i = 0; i < rows; i++) {
    std::string line;
    std::cin >> line;
    for (int j = 0; j < columns; j++) {
      client_map[i][j] = line[j];
      
      // Update state based on what we read
      if (line[j] >= '0' && line[j] <= '8') {
        client_visited[i][j] = true;
        client_mine_count[i][j] = line[j] - '0';
      } else if (line[j] == '@') {
        client_marked[i][j] = true;
      } else if (line[j] == 'X') {
        // Game over - hit a mine or marked wrong
        client_visited[i][j] = true;
      }
    }
  }
}

/**
 * @brief The definition of function Decide()
 *
 * @details This function is designed to decide the next step when playing the client's (or player's) role. Open up your
 * mind and make your decision here! Caution: you can only execute once in this function.
 */
void Decide() {
  int dr[] = {-1, -1, -1, 0, 0, 1, 1, 1};
  int dc[] = {-1, 0, 1, -1, 1, -1, 0, 1};
  
  // Strategy 1: Auto-explore on visited grids where all mines are marked
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (client_visited[i][j] && client_map[i][j] >= '0' && client_map[i][j] <= '8') {
        int mine_count = client_mine_count[i][j];
        int marked_neighbors = 0;
        bool has_unknown = false;
        
        for (int d = 0; d < 8; d++) {
          int ni = i + dr[d];
          int nj = j + dc[d];
          if (ni >= 0 && ni < rows && nj >= 0 && nj < columns) {
            if (client_marked[ni][nj]) {
              marked_neighbors++;
            } else if (!client_visited[ni][nj] && client_map[ni][nj] == '?') {
              has_unknown = true;
            }
          }
        }
        
        // If all mines around this grid are marked and there are unknown neighbors, auto-explore
        if (marked_neighbors == mine_count && mine_count > 0 && has_unknown) {
          Execute(i, j, 2);
          return;
        }
      }
    }
  }
  
  // Strategy 2: Mark obvious mines
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (client_visited[i][j] && client_map[i][j] >= '1' && client_map[i][j] <= '8') {
        int mine_count = client_mine_count[i][j];
        int marked_neighbors = 0;
        int unknown_neighbors = 0;
        int first_unknown_r = -1, first_unknown_c = -1;
        
        for (int d = 0; d < 8; d++) {
          int ni = i + dr[d];
          int nj = j + dc[d];
          if (ni >= 0 && ni < rows && nj >= 0 && nj < columns) {
            if (client_marked[ni][nj]) {
              marked_neighbors++;
            } else if (!client_visited[ni][nj] && client_map[ni][nj] == '?') {
              unknown_neighbors++;
              if (first_unknown_r == -1) {
                first_unknown_r = ni;
                first_unknown_c = nj;
              }
            }
          }
        }
        
        // If remaining unknown equals remaining mines, mark them
        if (unknown_neighbors > 0 && unknown_neighbors == mine_count - marked_neighbors) {
          Execute(first_unknown_r, first_unknown_c, 1);
          return;
        }
      }
    }
  }
  
  // Strategy 3: Visit safe cells
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (client_visited[i][j] && client_map[i][j] >= '0' && client_map[i][j] <= '8') {
        int mine_count = client_mine_count[i][j];
        int marked_neighbors = 0;
        int first_unknown_r = -1, first_unknown_c = -1;
        
        for (int d = 0; d < 8; d++) {
          int ni = i + dr[d];
          int nj = j + dc[d];
          if (ni >= 0 && ni < rows && nj >= 0 && nj < columns) {
            if (client_marked[ni][nj]) {
              marked_neighbors++;
            } else if (!client_visited[ni][nj] && client_map[ni][nj] == '?') {
              if (first_unknown_r == -1) {
                first_unknown_r = ni;
                first_unknown_c = nj;
              }
            }
          }
        }
        
        // If all mines are marked, visit unknown neighbors
        if (marked_neighbors == mine_count && first_unknown_r != -1) {
          Execute(first_unknown_r, first_unknown_c, 0);
          return;
        }
      }
    }
  }
  
  // Strategy 4: Visit cell adjacent to lowest number
  int best_r = -1, best_c = -1;
  int best_score = 100;
  
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (!client_visited[i][j] && client_map[i][j] == '?') {
        int min_neighbor = 100;
        bool has_visited_neighbor = false;
        
        for (int d = 0; d < 8; d++) {
          int ni = i + dr[d];
          int nj = j + dc[d];
          if (ni >= 0 && ni < rows && nj >= 0 && nj < columns) {
            if (client_visited[ni][nj] && client_map[ni][nj] >= '0' && client_map[ni][nj] <= '8') {
              has_visited_neighbor = true;
              int count = client_mine_count[ni][nj];
              if (count < min_neighbor) {
                min_neighbor = count;
              }
            }
          }
        }
        
        if (has_visited_neighbor && min_neighbor < best_score) {
          best_score = min_neighbor;
          best_r = i;
          best_c = j;
        }
      }
    }
  }
  
  if (best_r != -1) {
    Execute(best_r, best_c, 0);
    return;
  }
  
  // Strategy 5: Visit any unvisited cell
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (!client_visited[i][j] && client_map[i][j] == '?') {
        Execute(i, j, 0);
        return;
      }
    }
  }
}

#endif