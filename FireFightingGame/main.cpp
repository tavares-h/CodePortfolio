#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

// Define map dimensions
#define mapRows 30
#define mapCols 80

struct Map {
    std::vector<std::vector<char>> map_state;
    std::vector<std::pair<int, int>> fire_location; // Added to store fire coordinates
    int fire_total = 0; // Added to track total fires
    Map() : map_state(mapRows, std::vector<char>(mapCols, ' ')) {}
};

void MapInit(Map& map) {
    for (int i = 0; i < mapRows; i++) {
        for (int j = 0; j < mapCols; j++) {
            map.map_state[i][j] = '.';
        }
    }
}

void MapUpdate(const Map& map) {
    for (int i = 0; i < mapRows; ++i) {
        for (int j = 0; j < mapCols; ++j) {
            std::cout << map.map_state[i][j];
        }
        std::cout << std::endl;
    }
}

int SpotFireChance(int fires, int chance) {
    return fires % chance;
}

void GenFire(Map& map_cur, Map& map_prev) {
    map_prev = map_cur; // Copy current map to previous

    if (map_cur.fire_total < 1) {
        int randomRow = (std::rand() % (mapRows - 10)) + 5; // Adjusted range
        int randomCol = (std::rand() % (mapCols - 10)) + 5; // Adjusted range
        map_cur.map_state[randomRow][randomCol] = '^';
        map_cur.fire_location.push_back({randomRow, randomCol});
        map_cur.fire_total++;
    } else if (map_cur.fire_total >= mapRows * mapCols) {
        return;
    } else {
        int direction = std::rand() % 8;
        int index = std::rand() % map_cur.fire_location.size();
        int row = map_cur.fire_location[index].first;
        int col = map_cur.fire_location[index].second;

        switch (direction) {
            case 0: // North
                if (row + 1 < mapRows) {
                    map_cur.map_state[row + 1][col] = '^';
                    map_cur.fire_location.push_back({row + 1, col});
                    map_cur.fire_total++;
                }
                break;
            case 1: // North-East
                if (row + 1 < mapRows && col + 1 < mapCols) {
                    map_cur.map_state[row + 1][col + 1] = '^';
                    map_cur.fire_location.push_back({row + 1, col + 1});
                    map_cur.fire_total++;
                }
                break;
            case 2: // East
                if (col + 1 < mapCols) {
                    map_cur.map_state[row][col + 1] = '^';
                    map_cur.fire_location.push_back({row, col + 1});
                    map_cur.fire_total++;
                }
                break;
            case 3: // South-East
                if (row - 1 >= 0 && col + 1 < mapCols) {
                    map_cur.map_state[row - 1][col + 1] = '^';
                    map_cur.fire_location.push_back({row - 1, col + 1});
                    map_cur.fire_total++;
                }
                break;
            case 4: // South
                if (row - 1 >= 0) {
                    map_cur.map_state[row - 1][col] = '^';
                    map_cur.fire_location.push_back({row - 1, col});
                    map_cur.fire_total++;
                }
                break;
            case 5: // South-West
                if (row - 1 >= 0 && col - 1 >= 0) {
                    map_cur.map_state[row - 1][col - 1] = '^';
                    map_cur.fire_location.push_back({row - 1, col - 1});
                    map_cur.fire_total++;
                }
                break;
            case 6: // West
                if (col - 1 >= 0) {
                    map_cur.map_state[row][col - 1] = '^';
                    map_cur.fire_location.push_back({row, col - 1});
                    map_cur.fire_total++;
                }
                break;
            case 7: // North-West
                if (row + 1 < mapRows && col - 1 >= 0) {
                    map_cur.map_state[row + 1][col - 1] = '^';
                    map_cur.fire_location.push_back({row + 1, col - 1});
                    map_cur.fire_total++;
                }
                break;
        }
    }
}

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    Map map_cur;
    Map map_prev;

    MapInit(map_cur);

    int loop = 1;
    while (true) {
	if (loop == 3000) {
		GenFire(map_cur, map_prev); // Generate/update fire
		MapUpdate(map_cur); // Display map
		loop = 0;
	}
	loop++;
    }
    return 0;
}
