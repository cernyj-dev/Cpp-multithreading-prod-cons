#include <iostream>
#include <vector>
#include <algorithm>
#include <climits>
#include <unordered_set>

using namespace std;

struct Tile {
    int width;
    int height;
    int price;
};


int minPriceToFitTiles(int w, int h, vector<Tile>& tiles) {
    vector<vector<int>> dp(w + 1, vector<int>(h + 1, INT_MAX));
    dp[0][0] = 0;
    
    for (int i = 0; i <= w; i++) {
        for (int j = 0; j <= h; j++) {
            if (dp[i][j] == INT_MAX) continue;
            for (const auto& tile : tiles) {
                for (int rot = 0; rot < 2; rot++) {
                    int tw = (rot == 0) ? tile.width : tile.height;
                    int th = (rot == 0) ? tile.height : tile.width;
                    if (i + tw <= w && j + th <= h) {
                        dp[i + tw][j + th] = min(dp[i + tw][j + th], dp[i][j] + tile.price);
                    }
                }
            }
        }
    }
    
    return (dp[w][h] == INT_MAX) ? -1 : dp[w][h];
}

int main() {
    int w = 10; 
    int h = 9;
    vector<Tile> tiles = {
        {4, 2, 3}, // w, h, price
        {4, 7, 5},
        {6, 5, 7},
        {6, 1, 2},
        {6, 3, 1}
    };

    int result = minPriceToFitTiles(w, h, tiles);
    if (result != -1) {
        cout << "The minimum price to fill the grid is: " << result << endl;
    } else {
        cout << "It is not possible to fill the grid with the given tiles." << endl;
    }

    return 0;
}