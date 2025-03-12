#include <iostream>
#include <vector>
#include <algorithm>
#include <climits>

using namespace std;

struct Tile {
    int width;
    int height;
    int price;
};

void addRotatedTiles(vector<Tile> &tiles) {
    int n = tiles.size();
    for (int i = 0; i < n; ++i) {
        tiles.push_back({tiles[i].height, tiles[i].width, tiles[i].price});
    }
}

vector<vector<int>> memo;
vector<vector<pair<int, int>>> tile_used; // To store the tile used at each position

int minPriceTilesRec(const Tile& last_used ,int w, int h, const vector<Tile>& tiles, const int& max_w, const int& max_h) {
    if (w > max_w || h > max_h) return INT_MAX; // out of bounds
    if (w == max_w && h == max_h) return 0; // filled the grid

    if (memo[w][h] != -1) return memo[w][h];

    int min_price = INT_MAX;
    for (int i = 0; i < tiles.size(); ++i) {
        if(w != 0 || h != 0){ // pridat podminku pro to, ze napravo od sebe muzu dat jenom tiles, ktery stejnou hranu

        }

        const auto& tile = tiles[i];
        // Try placing the tile to the right
        int placed_right_price = minPriceTilesRec(tile ,w + tile.width, h, tiles, max_w, max_h);
        if (placed_right_price != INT_MAX && placed_right_price + tile.price < min_price) {
            min_price = placed_right_price + tile.price;
            tile_used[w][h] = {i, 0}; // Store the tile index and orientation (0 for right)
        }

        // Try placing the tile below
        int placed_below_price = minPriceTilesRec(tile, w, h + tile.height, tiles, max_w, max_h);
        if (placed_below_price != INT_MAX && placed_below_price + tile.price < min_price) {
            min_price = placed_below_price + tile.price;
            tile_used[w][h] = {i, 1}; // Store the tile index and orientation (1 for below)
        }
    }

    memo[w][h] = min_price;
    return min_price;
}

void printUsedTiles(int w, int h, const vector<Tile>& tiles) {
    while (w < tiles[0].width && h < tiles[0].height) {
        auto [tile_index, orientation] = tile_used[w][h];
        const auto& tile = tiles[tile_index];
        cout << "Used tile: " << tile.width << "x" << tile.height << " with price " << tile.price << " at position (" << w << ", " << h << ") ";
        if (orientation == 0) {
            cout << "placed to the right" << endl;
            w += tile.width;
        } else {
            cout << "placed below" << endl;
            h += tile.height;
        }
    }
}

void result_printer(int h, int w, vector<Tile> &tiles) {
    addRotatedTiles(tiles);
    memo.assign(w + 1, vector<int>(h + 1, -1));
    tile_used.assign(w + 1, vector<pair<int, int>>(h + 1, {-1, -1}));
    int result = minPriceTilesRec({0,0,0},0, 0, tiles, w, h);
    if (result != INT_MAX) {
        cout << "The minimum price to fill the grid is: " << result << endl;
        printUsedTiles(0, 0, tiles);
    } else {
        cout << "It is not possible to fill the grid with the given tiles." << endl;
    }
}

int main() {
    int w; 
    int h;

    vector<Tile> tiles; 
    
    /*
    w = 4;
    h = 4;
    
    tiles = {
        {1, 1, 1}
    };

    result_printer(h, w, tiles);
    */

    w = 10;
    h = 9;
    tiles = {
        {4, 2, 3}, // w, h, price
        {4, 7, 5},
        {6, 5, 7},
        {6, 1, 2},
        {6, 3, 1}
    };

    result_printer(h, w, tiles);
    
    return 0;
}