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
  
  // Strategy 2: Mark obvious mines (when unknown = mine count)
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
        
        // If remaining unknown equals remaining mines, mark them all
        if (unknown_neighbors > 0 && unknown_neighbors == mine_count - marked_neighbors) {
          Execute(first_unknown_r, first_unknown_c, 1);
          return;
        }
      }
    }
  }
  
  // Strategy 3: Visit safe cells (when all mines are marked)
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
  
  // Strategy 4: Comprehensive constraint propagation with all neighbors
  // Try all combinations of constraints to deduce safe/mine cells
  for (int i1 = 0; i1 < rows; i1++) {
    for (int j1 = 0; j1 < columns; j1++) {
      if (!client_visited[i1][j1] || client_map[i1][j1] < '1' || client_map[i1][j1] > '8') continue;
      
      int mc1 = client_mine_count[i1][j1];
      int marked1 = 0;
      std::vector<std::pair<int,int>> unknown1;
      
      for (int d = 0; d < 8; d++) {
        int ni = i1 + dr[d];
        int nj = j1 + dc[d];
        if (ni >= 0 && ni < rows && nj >= 0 && nj < columns) {
          if (client_marked[ni][nj]) marked1++;
          else if (!client_visited[ni][nj] && client_map[ni][nj] == '?') {
            unknown1.push_back({ni, nj});
          }
        }
      }
      int remaining1 = mc1 - marked1;
      if (unknown1.empty()) continue;
      
      // Compare with all neighboring numbered cells
      for (int i2 = 0; i2 < rows; i2++) {
        for (int j2 = 0; j2 < columns; j2++) {
          if (i1 == i2 && j1 == j2) continue;
          if (!client_visited[i2][j2] || client_map[i2][j2] < '1' || client_map[i2][j2] > '8') continue;
          
          int mc2 = client_mine_count[i2][j2];
          int marked2 = 0;
          std::vector<std::pair<int,int>> unknown2;
          
          for (int d = 0; d < 8; d++) {
            int ni = i2 + dr[d];
            int nj = j2 + dc[d];
            if (ni >= 0 && ni < rows && nj >= 0 && nj < columns) {
              if (client_marked[ni][nj]) marked2++;
              else if (!client_visited[ni][nj] && client_map[ni][nj] == '?') {
                unknown2.push_back({ni, nj});
              }
            }
          }
          int remaining2 = mc2 - marked2;
          if (unknown2.empty()) continue;
          
          // Find cells only in unknown1, only in unknown2, and shared
          std::vector<std::pair<int,int>> only1, only2, shared;
          for (auto &p : unknown1) {
            bool found = false;
            for (auto &q : unknown2) {
              if (p.first == q.first && p.second == q.second) {
                found = true;
                break;
              }
            }
            if (found) shared.push_back(p);
            else only1.push_back(p);
          }
          for (auto &p : unknown2) {
            bool found = false;
            for (auto &q : shared) {
              if (p.first == q.first && p.second == q.second) {
                found = true;
                break;
              }
            }
            if (!found) only2.push_back(p);
          }
          
          // Case 1: unknown1 is subset of unknown2
          if (only1.empty() && !shared.empty()) {
            int mines_in_only2 = remaining2 - remaining1;
            if (mines_in_only2 == (int)only2.size() && only2.size() > 0) {
              // All cells in only2 are mines
              Execute(only2[0].first, only2[0].second, 1);
              return;
            } else if (mines_in_only2 == 0 && only2.size() > 0) {
              // All cells in only2 are safe
              Execute(only2[0].first, only2[0].second, 0);
              return;
            }
          }
          
          // Case 2: unknown2 is subset of unknown1
          if (only2.empty() && !shared.empty()) {
            int mines_in_only1 = remaining1 - remaining2;
            if (mines_in_only1 == (int)only1.size() && only1.size() > 0) {
              // All cells in only1 are mines
              Execute(only1[0].first, only1[0].second, 1);
              return;
            } else if (mines_in_only1 == 0 && only1.size() > 0) {
              // All cells in only1 are safe
              Execute(only1[0].first, only1[0].second, 0);
              return;
            }
          }
        }
      }
    }
  }
  
  // Strategy 5: Probability-based guessing
  // Calculate mine probability for each unknown cell
  int best_r = -1, best_c = -1;
  double min_prob = 2.0;  // Higher than any valid probability
  
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (!client_visited[i][j] && client_map[i][j] == '?') {
        // Calculate probability this cell is a mine
        double prob_sum = 0.0;
        int constraint_count = 0;
        
        for (int d = 0; d < 8; d++) {
          int ni = i + dr[d];
          int nj = j + dc[d];
          if (ni >= 0 && ni < rows && nj >= 0 && nj < columns) {
            if (client_visited[ni][nj] && client_map[ni][nj] >= '0' && client_map[ni][nj] <= '8') {
              int mc = client_mine_count[ni][nj];
              int marked = 0;
              int unknown = 0;
              
              for (int d2 = 0; d2 < 8; d2++) {
                int nni = ni + dr[d2];
                int nnj = nj + dc[d2];
                if (nni >= 0 && nni < rows && nnj >= 0 && nnj < columns) {
                  if (client_marked[nni][nnj]) marked++;
                  else if (!client_visited[nni][nnj] && client_map[nni][nnj] == '?') unknown++;
                }
              }
              
              if (unknown > 0) {
                double prob = (double)(mc - marked) / unknown;
                prob_sum += prob;
                constraint_count++;
              }
            }
          }
        }
        
        if (constraint_count > 0) {
          double avg_prob = prob_sum / constraint_count;
          if (avg_prob < min_prob) {
            min_prob = avg_prob;
            best_r = i;
            best_c = j;
          }
        }
      }
    }
  }
  
  if (best_r != -1 && min_prob < 0.5) {
    // Only guess if probability is reasonably low
    Execute(best_r, best_c, 0);
    return;
  }
  
  // Strategy 6: Visit cell adjacent to lowest number (prefer 0 or 1)
  best_r = -1;
  best_c = -1;
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
  
  // Strategy 7: Visit any unvisited cell
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