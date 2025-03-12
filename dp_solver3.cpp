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



// moznosti: udelat si memo tabulku, vyplnit ji MAX_INT hodnotama, pak projet tu tabulku a nastavit kazde uz existujici Tile jeji cenu
// myslenka: v te tabulce muzu pridavat hodnoty bud napravo od sebe - protoze budu mit stejnou sirku (row)
// nebo pod sebe - protoze budu mit stejnou vysku (column) - takze vlastne pro soucasnou bunku projedu vsechny bunky vlevo od sebe (n) a tim muzu pridat napravo od sebe az nejakych nekolik hodnot
// pak se podivam nad sebe (n) a zase pod sebou muzu pridat nekolik hodnot - moje soucasna cena + cena te mensi stejno vyskove bunky na bunce soucasna_sirka + mensi_bunka_sirka 

/*

        {4, 2, 3}, // w, h, price
        {4, 7, 5},
        {6, 5, 7},
        {6, 1, 2},
        {6, 3, 1}



     0  1  2  3  4  5  6  7  8  9 10 11
0- |  |  |  |  |  |  |  |  |  |  |  |  |
1- |  |  |  |  |  |  |  |  |  |  |  |  |
2- |  |  |  |  | 3|  |  |  |  |  |  |  |
3- |  |  |  |  |  |  |  |  |  |  |  |  |
4- |  |  |  |  |  |  |  |  |  |  |  |  |
5- |  |  |  |  |  |  |  |  |  |  |  |  |
6- |  | 2|  | 1|  |  |  |  |  |  |  |  |
7- |  |  |  |  | 7|  |  |  |  |  |  |  |
8- |  |  |  |  |  |  |  |  |  |  |  |  |
9- |  |  |  |  |  |  |  |  |  |  |  |  |



     0  1  2  3  4  5  6  7  8  9 10 11
0- |  |  |  |  |  |  |  |  |  |  |  |  |
1- |  |  |  |  |  |  |  |  |  |  |  |  |
2- |  |  |  |  | 3|  |  |  |  |  |  |  |
3- |  |  |  |  |  |  |  |  |  |  |  |  |
4- |  |  |  |  |  |  |  |  |  |  |  |  |
5- |  |  |  |  |  |  |  |  |  |  |  |  |
6- |  | 2| 2| 1|  |  |  |  |  |  |  |  |
7- |  |  |  |  | 7|  |  |  |  |  |  |  |
8- |  |  |  |  |  |  |  |  |  |  |  |  |
9- |  |  |  |  |  |  |  |  |  |  |  |  |

*/


int solver(const vector<Tile>& tiles, const int& max_w, const int& max_h) {
    vector<vector<int>> memo(max_w + 1, vector<int>(max_h + 1, INT_MAX));
    for(int i = 0; i < tiles.size(); i++){
        // fill out the memoization table with alread existing tiles - the normal ones and rotated ones as well
        memo[tiles[i].width][tiles[i].height] = tiles[i].price;
        memo[tiles[i].height][tiles[i].width] = tiles[i].price;
    }
    // i can shut this down early if i need
    if(memo[max_w][max_h] != INT_MAX){
        return memo[max_w][max_h];
    }


    // go over the table and iterate left of "me" to look at all the tiles with the same height - whenever that tile isnt INT_MAX, then i can add it to the right of me (within bounds)
    // and iterate above of "me" to look at all the tiles with the same width - whenever that tile isnt INT_MAX, then i can add it below of me (within bounds)
    for(int w = 0; w <= max_w; w++){ // desky muzu pouze svaret a ne rezat ... dava smysl jit pouze zleva doprava a shora dolu
        for(int h = 0; h <= max_h; h++){
            if(memo[w][h] == INT_MAX){
                continue;
            }

            // same height
            for(int i = 0; i <= w; i++){
                if(memo[i][h] != INT_MAX && w + i <= max_w){
                    memo[w + i][h] = min(memo[w + i][h], memo[w][h] + memo[i][h]);
                }
            }

            // same width
            for(int j = 0; j <= h; j++){
                if(memo[w][j] != INT_MAX && h + j <= max_h){
                    memo[w][h + j] = min(memo[w][h + j], memo[w][h] + memo[w][j]);
                }
            }
        }
    }
    return memo[max_w][max_h];

}

void result_printer(int h, int w, vector<Tile> &tiles){
    int result = solver(tiles, w, h);
    if (result != INT_MAX) {
        cout << "The minimum price to fill the grid is: " << result << endl;
    } else {
        cout << "It is not possible to fill the grid with the given tiles." << endl;
    }
}

int main() {
    int w; 
    int h;

    vector<Tile> tiles; 
    
    
    w = 4;
    h = 4;
    
    tiles = {
        {1, 1, 1}
    };

    result_printer(h, w, tiles);
    
    w = 4;
    h = 4;
    
    tiles = {
        {2, 2, 1}
    };

    result_printer(h, w, tiles);



    w = 10;
    h = 9;
    tiles = {
        {4, 2, 1}, // w, h, price
        {4, 7, 1},
        {6, 5, 1},
        {6, 1, 1},
        {6, 3, 1}
    };

    result_printer(h, w, tiles);
    

    return 0;
}