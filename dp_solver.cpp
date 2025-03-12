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

void addRotatedTiles(vector<Tile> &tiles) {
    int n = tiles.size();
    for (int i = 0; i < n; ++i) {
        tiles.push_back({tiles[i].height, tiles[i].width, tiles[i].price});
    }
}



// Naive recursive solution
// should i take into account a tile of 0x0 dimensions?
// its a grid, so the dimension are 0, 1, ..., max_w - 1
/*
|  |  |  |  |  |  |  |  |  |  |

4x4 grid
  0  1  2  3
|  |  |  |  |  0
|  |  |  |  |  1
|  |  |  |  |  2
|  |  |  |  |  3

x x x
x x x
x x x
x x x



*/

// each [w][h] stands for the minimum price that the w, h Tile costs
// so basically i will start at 0,0 and start adding Tiles onto it and then add another tile on the right
// or add another tile below
// for example 0,0 i will add a tile 2x2, which will set me to [2][2] and now i can try adding another 2x2 on the right 
// creating a 4x2 ([4][2]) 
vector<vector<int>> memo;

int minPriceTilesRec(const Tile& last_used, int w, int h, const vector<Tile>& tiles, const int& max_w, const int& max_h) {
    
}

void result_printer(int h, int w, vector<Tile> &tiles){
    addRotatedTiles(tiles);
    memo.assign(w + 1, vector<int>(h + 1, -1));
    int result = minPriceTilesRec(0, 0, tiles, w, h);
    if (result != -1) {
        cout << "The minimum price to fill the grid is: " << result << endl;
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