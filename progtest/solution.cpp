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


// printf("Producer %d: lights out\n", tid);

class CMaterialInfo{
  public:
    bool m_has_all_pricelists = false;
    unsigned m_material_id;
    mutex m_mutex_mi; 

    CMaterialInfo(unsigned material_id): m_material_id(material_id){}

    void unifyPriceList(){
      lock_guard<mutex> lg(m_mutex_mi);
      bool duplicity_found = false;
      // projedu vsechny pricelisty a kazdy cProd z nich zkusim pridat do unified_pricelist
      for(size_t i = 0; i < m_pricelists.size(); i++){
        for(size_t j = 0; j < m_pricelists[i]->m_List.size(); j++){
          // projedu cely pricelist a pokud uz mam stejne rozmery, tak si necham tu s mensi cenou
          for(size_t k = 0; k < m_unified_pricelist.size(); k++){
            // maji stejne vysky a sirky nebo je vyska se sirkou stejna a sirka s vyskou stejna ... rotace
            if((m_unified_pricelist[k].m_W == m_pricelists[i]->m_List[j].m_W && m_unified_pricelist[k].m_H == m_pricelists[i]->m_List[j].m_H) || 
              (m_unified_pricelist[k].m_W == m_pricelists[i]->m_List[j].m_H && m_unified_pricelist[k].m_H == m_pricelists[i]->m_List[j].m_W)){
              duplicity_found = true;
              m_unified_pricelist[k].m_Cost = min(m_unified_pricelist[k].m_Cost, m_pricelists[i]->m_List[j].m_Cost);
              break;
            }
          }
          // pokud jsem projel cely unified_pricelist a nenasel jsem tam tenhle CProd, tak ho tam pridam ... nema tam duplicitu
          if(duplicity_found == false){
            m_unified_pricelist.push_back(m_pricelists[i]->m_List[j]);
          }
        }
      }
      // zamek se odemkne a ja mam unifikovany cenik
    }

    vector<APriceList> m_pricelists; // <- postupne sem budu strkat ceniky dodavatelu
    vector<CProd> m_unified_pricelist; // <- jakmile mam vsechny ceniky, tak do tohodle dam ten unifikovany
    set<AProducer> m_producers_received; // <- abych vedel, jestli uz jsem od kazdeho dostal cenik



    // co kdyz ale dva ruzni customers najednou poslou COrderList se stejnym materialID?
    // pak prece budu zaplnovat ten stejny vector dvakrat a naplnim to mnohem driv a hlavne bez vsech ceniku
    // takze pamatovat si ty AProducer musim a pak podle velikosti tohodle zjistit, jestli mam vsechny

  };
  using AMaterialInfo = std::shared_ptr<CMaterialInfo>;
// ===================================================================================================================
    
class CCustomerInfo{
  public:
    CCustomerInfo(ACustomer cust, AOrderList orderList, AMaterialInfo materialInfo): cust(cust), orderList(orderList), materialInfo(materialInfo){
      m_number_of_all_orders = orderList->m_List.size();
    }

    ACustomer cust;
    AOrderList orderList;
    AMaterialInfo materialInfo;
    unsigned m_number_of_completed_orders = 0;
    unsigned m_number_of_all_orders;

  };
  using ACustomerInfo = std::shared_ptr<CCustomerInfo>;

// ===================================================================================================================

class CWeldingCompany{

  public:
    static bool usingProgtestSolver(){
      return false;
    }

    void catcherFnc(ACustomer cust){
      while(true){
        AOrderList orderList = cust->waitForDemand();

        if(orderList == nullptr){
          //printf("Customer ended\n");
          break;
        }

        bool has_all_pricelists = false;

        {
          lock_guard<mutex> lg(m_mutex_wc);
          // mam uz zaznam o tomhle materialID? Pokud ne... tak si ho vytvorim
          if(m_Materials.find(orderList->m_MaterialID) == m_Materials.end()){
            m_Materials[orderList->m_MaterialID] = make_shared<CMaterialInfo>(orderList->m_MaterialID);
          }

          has_all_pricelists = m_Materials[orderList->m_MaterialID]->m_has_all_pricelists;
        }

        // mam k tomuhle materialID uz kompletni cenik?
        if(has_all_pricelists == false){
          // pokud ne, tak projdi vsechny dodavatele a zavolej na nich sendPriceList
          for(size_t j = 0; j < m_Producers.size(); j++){
            m_Producers[j]->sendPriceList(orderList->m_MaterialID);
          }
          // pockej na to, nez budes mit vsechny ceniky
          unique_lock<mutex> ul(m_mutex_wc);
          m_cv_Complete_CPriceList.wait(ul, [this, orderList](){return m_Materials[orderList->m_MaterialID]->m_has_all_pricelists;});
        }

        // ted musim zavolat funkci na unifikovani tech pricelists
        // a pak uz muzu zacit plnit tu frontu

        m_Materials[orderList->m_MaterialID]->unifyPriceList();

        ACustomerInfo cust_info = make_shared<CCustomerInfo>(cust, orderList, m_Materials[orderList->m_MaterialID]);
        
        {
          lock_guard<mutex> lg(m_mutex_wc);
          for(auto& order: orderList->m_List){
            m_Orders_buffer.push(make_pair(&order, cust_info));
            m_cv_Orders_buffer_empty.notify_one();
          }
        }
      }
      {
        lock_guard<mutex> lg(m_mutex_wc);
        m_number_of_active_customers--;

        // lights out
        if(m_number_of_active_customers == 0){
          for (int i = 0; i < m_number_of_workers; i++){
            // dummy light out order has nullptr as customer info
            m_Orders_buffer.push(make_pair(nullptr, nullptr));
            m_cv_Orders_buffer_empty.notify_one();
          }
        }
      }

    }
    
    void workerFnc(){
      while(true){
        pair<COrder*, ACustomerInfo> order (nullptr, nullptr);
        {
          unique_lock<mutex> ul(m_mutex_wc);
          m_cv_Orders_buffer_empty.wait(ul, [this](){return !m_Orders_buffer.empty();});
          order = m_Orders_buffer.front();
          m_Orders_buffer.pop();
        }
        // lights out
        if(order.second == nullptr){
          break;
        }

        APriceList priceList = make_shared<CPriceList>(order.second->materialInfo->m_material_id);
        priceList->m_List = order.second->materialInfo->m_unified_pricelist;

        seqSolve(priceList, *(order.first));

        {
          lock_guard<mutex> lg(m_mutex_wc);
          order.second->m_number_of_completed_orders++;

          if(order.second->m_number_of_completed_orders == order.second->m_number_of_all_orders){
            order.second->cust->completed(order.second->orderList);
          }
        }
      }
    }

    static void seqSolve(APriceList priceList, COrder& order){
      unsigned max_w = order.m_W;
      unsigned max_h = order.m_H;
      vector<vector<double>> memo(max_w + 1, vector<double>(max_h + 1, DBL_MAX));
      for(size_t i = 0; i < priceList->m_List.size(); i++){
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
      for(unsigned w = 0; w <= max_w; w++){ // desky muzu pouze svaret a ne rezat ... dava smysl jit pouze zleva doprava a shora dolu
          for(unsigned h = 0; h <= max_h; h++){
              if(memo[w][h] == DBL_MAX){
                  continue;
              }

              // same height
              for(unsigned i = 1; i <= w; i++){
                  if(memo[i][h] != DBL_MAX && w + i <= max_w){
                      // chcu si nechat, co uz tam je, nebo novou price? ... nova price = memo[w][h] + memo[i][h] + order.m_WeldingStrength*h ... maji stejnou vejsku, takze spoj je h*m_WeldingStrength
                      memo[w + i][h] = min(memo[w + i][h], memo[w][h] + memo[i][h] + order.m_WeldingStrength*h);
                  }
              }

              // same width
              for(unsigned j = 1; j <= h; j++){
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
      lock_guard<mutex> lg(m_mutex_wc);
      // neni ten producer jeste zaznamenan?
      if(m_Materials[priceList->m_MaterialID]->m_producers_received.find(prod) == m_Materials[priceList->m_MaterialID]->m_producers_received.end()){ // iterator muze zrychlit pokud si drzim iterator na m_Material[priceList->m_MaterialID]->m_producers_received
        m_Materials[priceList->m_MaterialID]->m_producers_received.insert(prod);
        m_Materials[priceList->m_MaterialID]->m_pricelists.push_back(priceList);
        
        // dostal jsem uz vsechny pricelisty?
        if(m_Materials[priceList->m_MaterialID]->m_producers_received.size() == m_Producers.size()){
          // tak probud catchery, kteri cekaji na vsechny pricelisty
          m_Materials[priceList->m_MaterialID]->m_has_all_pricelists = true; // <- odemceni te podminky
          m_cv_Complete_CPriceList.notify_all();
        }
      }
      // mutex se unlockne
    }
    
    void start(unsigned thrCount){
      // vsechny threads = main + threadCount (workers) + customer.size (podpurny)
      m_number_of_active_customers = m_Customers.size();
      m_number_of_workers = thrCount;

      
      for(size_t i = 0; i < m_Customers.size(); i++){
        m_catcher_threads.push_back(thread(&CWeldingCompany::catcherFnc, this, m_Customers[i]));
      }

      for(unsigned i = 0; i < thrCount; i++){
        m_worker_threads.push_back(thread(&CWeldingCompany::workerFnc, this));
      }

    }
    
    void stop(){
      for ( auto & t : m_catcher_threads )
        t.join (); 

      for ( auto & t : m_worker_threads )
        t.join (); 
    }


  
    vector<AProducer> m_Producers;
    vector<ACustomer> m_Customers;

    vector<thread> m_catcher_threads;
    vector<thread> m_worker_threads;

    map<unsigned, AMaterialInfo> m_Materials;     // ke kazdemu materialu si drzi optimalni cenik

    queue<pair<COrder*, ACustomerInfo>> m_Orders_buffer;                // shared buffer      
    mutex m_mutex_wc;                         // controls access to share buffer (critical section)
    //condition_variable m_cv_Orders_buffer_full;   // protects from inserting items into a full buffer
    condition_variable m_cv_Orders_buffer_empty;  // protects from removing items from an empty buffer, not needed

    condition_variable m_cv_Complete_CPriceList;  // 

    size_t m_number_of_active_customers = 0;
    int m_number_of_workers = 0;
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
