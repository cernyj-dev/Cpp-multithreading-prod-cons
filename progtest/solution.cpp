#ifndef __PROGTEST__
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <climits>
#include <cfloat>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <vector>
#include <set>
#include <list>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <stack>
#include <deque>
#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <semaphore>
#include <atomic>
#include <condition_variable>
#include "progtest_solver.h"
#include "sample_tester.h"
#endif /* __PROGTEST__ */

using namespace std;

// materialID je unsigned int
// m_Cost je double, stejne tak m_WeldingStrength

/*
  Scenar: 
  catcher ziska objednavku
  catcher se podiva do m_Materials jestli pro zadany material ma ten unifikovany pricelist
    
    pokud ne, tak projde kazdeho producenta a zavola na nem sendPriceList
    potom pocka na cv_Complete_CPriceList - teda na to, ze vsechny pricelisty jsou nacteny

- synchronni... catcherFnc -> sendPriceList -> addPriceList(lock) -> sendPriceList -> catcherFnc
- asynchronni... catcherFnc -> sendPriceList(poznamena si) -> catcherFnc ... a potom nekdy se zavola addPriceList
v obou pripadech ale proste dojede ten for cyklus a po nem si musi pockat na wait, nez bude mit vsechny ceniky
  wait si jeste odemkne ten mutex

  po tomhle uz muze catcher naplnit queue temi COrders a workers si je budou postupne brat a volat s nimi solve

  po solve bude potreba jeste potreba poznamenat do spolecneho wrapperu, ze je hotova tahle COrder z COrderList
  a pokud je to posledni, tak ji worker asi muze i odeslat pomoci complete()

  wrappery:
    MaterialInfo - o cenikach a tak
    ActiveOrder - wrapper okolo COrderListu
    


*/ 
class CMaterialInfo{
  public:
    bool m_has_all_pricelists = false;
    unsigned m_material_id;
    unsigned m_price_list_count = 0;
    CMaterialInfo(unsigned material_id): m_material_id(material_id){}



  private:

    vector<APriceList> m_pricelists; // <- postupne sem budu strkat ceniky dodavatelu
    vector<CProd> m_unified_pricelist; // <- jakmile mam vsechny ceniky, tak do tohodle dam ten unifikovany


  };

class CWeldingCompany{
  private:
    vector<AProducer> m_Producers;
    vector<ACustomer> m_Customers;
    vector<thread> m_Threads;

    map<unsigned, CMaterialInfo> m_Materials;

    queue<COrder> m_Orders_buffer;                // shared buffer      
    mutex m_buffer_mutex;                         // controls access to share buffer (critical section)
    condition_variable m_cv_Orders_buffer_full;   // protects from inserting items into a full buffer
    condition_variable m_cv_Orders_buffer_empty;  // protects from removing items from an empty buffer

    condition_variable m_cv_Complete_CPriceList;  // 


    bool m_Stop = false;
    int m_number_of_customers = 0;
    int m_number_of_workers = 0;

  public:
    static bool usingProgtestSolver(){
      return false;
    }
    // maybe i should lock this? -> cvicici rikal ze ne


    static void seqSolve(APriceList priceList, COrder& order){
      int max_w = order.m_W;
      int max_h = order.m_H;
      vector<vector<double>> memo(max_w + 1, vector<double>(max_h + 1, DBL_MAX));
      for(int i = 0; i < priceList->m_List.size(); i++){
          // fill out the memoization table with alread existing tiles - the normal ones and rotated ones as well
          // only add tiles that are actually smaller or equal to the max_w and max_h
          // cant cut down the tiles, so it only makes sense to add the ones that i can weld together (and are therefore smaller or equal to the tasked plate)
          if(priceList->m_List[i].m_W <= max_w && priceList->m_List[i].m_H <= max_h){
              memo[priceList->m_List[i].m_W][priceList->m_List[i].m_H] = priceList->m_List[i].m_Cost;    
          }
          if(priceList->m_List[i].m_H <= max_w && priceList->m_List[i].m_W <= max_h){
              memo[priceList->m_List[i].m_H][priceList->m_List[i].m_W] = priceList->m_List[i].m_Cost;
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
    
    void addProducer(AProducer prod){
      m_Producers.push_back(prod);
    }
    
    void addCustomer(ACustomer cust){
      m_Customers.push_back(cust);
    }
    
    void addPriceList(AProducer prod, APriceList priceList){
      
      m_Materials[priceList->m_MaterialID].m_price_list_count++;
      if(m_Materials[priceList->m_MaterialID].m_price_list_count == m_Producers.size()){
        m_Materials[priceList->m_MaterialID].m_has_all_pricelists = true;
        m_cv_Complete_CPriceList.notify_all();
      }
    }
    
    void start(unsigned thrCount){
      // 
    }
    
    void stop(){}
};

//-------------------------------------------------------------------------------------------------
#ifndef __PROGTEST__
int main(){
  using namespace std::placeholders;
  CWeldingCompany test;

  AProducer p1 = std::make_shared<CProducerSync>(std::bind(&CWeldingCompany::addPriceList, &test, _1, _2));
  AProducerAsync p2 = std::make_shared<CProducerAsync>(std::bind(&CWeldingCompany::addPriceList, &test, _1, _2));
  test.addProducer(p1);
  test.addProducer(p2);
  test.addCustomer(std::make_shared<CCustomerTest>(2));
  p2->start();
  test.start(3);
  test.stop();
  p2->stop();
  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
