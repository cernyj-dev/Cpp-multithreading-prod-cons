#include <iostream>
#include <vector>
#include <algorithm>
#include <climits>
#include <unordered_set>
#include <cfloat>

using namespace std;

struct Tile {
    int width;
    int height;
    int price;
};

class COrder{
    public:
      unsigned m_W;
      unsigned m_H;
      double m_WeldingStrength;
      double m_Cost = 0;
  
      COrder(unsigned w, unsigned h, double weldingStrength): m_W(w), m_H (h), m_WeldingStrength(weldingStrength){}
  };

void solver(const vector<Tile>& tiles, COrder& order) {
    int max_w = order.m_W;
    int max_h = order.m_H;
    vector<vector<double>> memo(max_w + 1, vector<double>(max_h + 1, DBL_MAX));
    for(int i = 0; i < tiles.size(); i++){
        // fill out the memoization table with alread existing tiles - the normal ones and rotated ones as well
        // only add tiles that are actually smaller or equal to the max_w and max_h
        // cant cut down the tiles, so it only makes sense to add the ones that i can weld together (and are therefore smaller or equal to the tasked plate)
        if(tiles[i].width <= max_w && tiles[i].height <= max_h){
            memo[tiles[i].width][tiles[i].height] = tiles[i].price;    
        }
        if(tiles[i].height <= max_w && tiles[i].width <= max_h){
            memo[tiles[i].height][tiles[i].width] = tiles[i].price;
        }
    }
    // i can shut this down early if i need
    if(memo[max_w][max_h] != DBL_MAX){
        order.m_Cost = memo[max_w][max_h];
        return;
    }


    // go over the table and iterate left of "me" to look at all the tiles with the same height - whenever that tile isnt INT_MAX, then i can add it to the right of me (within bounds)
    // and iterate above of "me" to look at all the tiles with the same width - whenever that tile isnt INT_MAX, then i can add it below of me (within bounds)
    for(int w = 0; w <= max_w; w++){ // desky muzu pouze svaret a ne rezat ... dava smysl jit pouze zleva doprava a shora dolu
        for(int h = 0; h <= max_h; h++){
            if(memo[w][h] == DBL_MAX){
                continue;
            }

            // same height
            for(int i = 1; i <= w; i++){
                if(memo[i][h] != DBL_MAX && w + i <= max_w){
                    // chcu si nechat, co uz tam je, nebo novou price? ... nova price = memo[w][h] + memo[i][h] + order.m_WeldingStrength*h ... maji stejnou vejsku, takze spoj je h*m_WeldingStrength
                    memo[w + i][h] = min(memo[w + i][h], memo[w][h] + memo[i][h] + order.m_WeldingStrength*h);
                }
            }

            // same width
            for(int j = 1; j <= h; j++){
                if(memo[w][j] != DBL_MAX && h + j <= max_h){
                    memo[w][h + j] = min(memo[w][h + j], memo[w][h] + memo[w][j] + order.m_WeldingStrength*w);
                }
            }
        }
    }
    order.m_Cost = memo[max_w][max_h];
    return;
}

void result_printer(COrder& order, vector<Tile> &tiles){
    solver(tiles, order);
    if (order.m_Cost != DBL_MAX) {
        cout << "The minimum price to fill the grid is: " << order.m_Cost << endl;
    } else {
        cout << "It is not possible to fill the grid with the given tiles." << endl;
    }
}

int main() {
    int w; 
    int h;

    vector<Tile> tiles; 
    
    
    COrder order(4, 4, 1.0);
    
    tiles = {
        {1, 1, 1}
    };

    result_printer(order, tiles);
    
    COrder order2(4, 4, 1.0);
    
    tiles = {
        {2, 2, 1}
    };

    result_printer(order2, tiles);



    COrder order3(10, 9, 1.0);
    tiles = {
        {4, 2, 1}, // w, h, price
        {4, 7, 1},
        {6, 5, 1},
        {6, 1, 1},
        {6, 3, 1}
    };

    result_printer(order3, tiles);
    

    w = 3,
    h = 3;
    tiles = {

        {1, 2, 1}
    };
    COrder order4(w, h, 1.0);
    result_printer(order4, tiles);
    return 0;
}