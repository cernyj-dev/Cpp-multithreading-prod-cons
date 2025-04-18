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
/*
#ifndef sleeper
#define sleeper
inline void randomZpozdeni(){
  std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 100));
}
#endif */




class CMaterialInfo{
  public:
    bool m_has_all_pricelists;
    bool m_is_unified_calculated;
    unsigned m_material_id;
    //mutex m_mutex_mi; 

    vector<APriceList> m_pricelists; 
    CPriceList m_unified_pricelist; 
    unordered_set<AProducer> m_producers_received; 

    CMaterialInfo(): m_has_all_pricelists(false), m_is_unified_calculated(false), m_material_id(0), m_pricelists(vector<APriceList>()), m_unified_pricelist(0), m_producers_received(unordered_set<AProducer>())
    {}
    

    void unifyPriceList(){
      if(m_is_unified_calculated == true){
        return;
      }

      bool duplicity_found = false;
      
      for(size_t i = 0; i < m_pricelists.size(); i++){
        for(size_t j = 0; j < m_pricelists[i]->m_List.size(); j++){
          duplicity_found = false;

          for(size_t k = 0; k < m_unified_pricelist.m_List.size(); k++){

            if((m_unified_pricelist.m_List[k].m_W == m_pricelists[i]->m_List[j].m_W && m_unified_pricelist.m_List[k].m_H == m_pricelists[i]->m_List[j].m_H) || 
              (m_unified_pricelist.m_List[k].m_W == m_pricelists[i]->m_List[j].m_H && m_unified_pricelist.m_List[k].m_H == m_pricelists[i]->m_List[j].m_W)){
              duplicity_found = true;
              m_unified_pricelist.m_List[k].m_Cost = min(m_unified_pricelist.m_List[k].m_Cost, m_pricelists[i]->m_List[j].m_Cost);
              break;
            }
          }

          if(duplicity_found == false){
            m_unified_pricelist.add(m_pricelists[i]->m_List[j]);
          }
        }
      }
      m_is_unified_calculated = true;
    }

  };
// ===================================================================================================================
    
class CPackageInfo{
  public:
    CPackageInfo(ACustomer cust, AOrderList orderList, const unsigned& material_id, const unsigned& number_of_all_orders): 
      cust(cust), orderList(orderList), m_material_id(material_id), m_number_of_completed_orders(0) ,m_number_of_all_orders(number_of_all_orders)
    {}

    ACustomer cust;
    AOrderList orderList;
    unsigned m_material_id;
    unsigned m_number_of_completed_orders;
    unsigned m_number_of_all_orders;

  };

// ===================================================================================================================

class CWeldingCompany{

  public:

    CWeldingCompany(){
      m_Producers = vector<AProducer>();
      m_Customers = vector<ACustomer>();
      m_All_Materials_Info = unordered_map<unsigned, CMaterialInfo>();
      m_Orders_buffer = queue<pair<size_t, shared_ptr<CPackageInfo>>>();
      m_number_of_active_customers = 0;
      m_number_of_workers = 0;


    }

    static bool usingProgtestSolver(){
      return false;
    }

    void catcherFnc(ACustomer cust){
      while(true){
        AOrderList orderList = cust->waitForDemand();

        if(orderList == nullptr){
          break;
        }
        
        unique_lock<mutex> material_lock(m_mutex_wc);
        // do i have the record about the materialID?
        if(m_All_Materials_Info.find(orderList->m_MaterialID) == m_All_Materials_Info.end()){
          // if not then send out requests for pricelists from all producers
          material_lock.unlock();

          for(size_t j = 0; j < m_Producers.size(); j++){
            m_Producers[j]->sendPriceList(orderList->m_MaterialID);
          }

          material_lock.lock();
        }

        // wait for all pricelists
        m_cv_Complete_CPriceList.wait(material_lock, [&](){return m_All_Materials_Info[orderList->m_MaterialID].m_has_all_pricelists;});
        
        m_All_Materials_Info[orderList->m_MaterialID].unifyPriceList();


        material_lock.unlock();
        

        auto pkg_info = make_shared<CPackageInfo>(cust, orderList, orderList->m_MaterialID, orderList->m_List.size());
        {
          lock_guard<mutex> buffer_lock(m_mutex_wc);

          for(size_t idx = 0; idx < orderList->m_List.size(); idx++){
            m_Orders_buffer.push(make_pair(idx, pkg_info));
            m_cv_Orders_buffer_empty.notify_one(); 
          }
        }
      }

      // after customer stops sending orders, decrement the number of active customers
      {
        lock_guard<mutex> buffer_lock(m_mutex_wc);
        m_number_of_active_customers--;

        // lights out
        if(m_number_of_active_customers == 0){
          for (int i = 0; i < m_number_of_workers; i++){
            // dummy light out order has nullptr as customer info
            m_Orders_buffer.push(make_pair(0, nullptr)); 
            m_cv_Orders_buffer_empty.notify_one(); 
          }
        }
      }

    }
    
    void workerFnc(){
      while(true){
        unique_lock<mutex> buffer_lock(m_mutex_wc);
        m_cv_Orders_buffer_empty.wait(buffer_lock, [&](){return !m_Orders_buffer.empty();});

        pair<size_t, shared_ptr<CPackageInfo>> order = m_Orders_buffer.front();
        m_Orders_buffer.pop();
        
        // lights out
        if(order.second == nullptr){                                             
          break;
        }
        
        APriceList priceList = make_shared<CPriceList>(m_All_Materials_Info[order.second->m_material_id].m_unified_pricelist);
        buffer_lock.unlock();

        // seqSolve called without locking - it runs in parallel with the other threads
        seqSolve(priceList, order.second->orderList->m_List[order.first]);

        {
          lock_guard<mutex> package_lock(m_mutex_wc);
          order.second->m_number_of_completed_orders++;

          // are all COrders of that one COrderList done? Then notify the customer
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
      for(unsigned w = 0; w <= max_w; w++){ 
          for(unsigned h = 0; h <= max_h; h++){
              if(memo[w][h] == DBL_MAX){
                  continue;
              }

              // same height
              for(unsigned i = 1; i <= w; i++){
                  if(memo[i][h] != DBL_MAX && w + i <= max_w){
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
    
    // add producers before starting the parallel work
    void addProducer(AProducer prod){
      m_Producers.push_back(prod);
    }

    // add customers before starting the parallel work
    void addCustomer(ACustomer cust){ 
      m_Customers.push_back(cust);
    }

    // called by the producers to send us their pricelist
    void addPriceList(AProducer prod, APriceList priceList){
      lock_guard<mutex> lg(m_mutex_wc);
      
      auto it = m_All_Materials_Info.find(priceList->m_MaterialID);
      
      // there isnt any record about the current material yet                
      if(it == m_All_Materials_Info.end()){                                        
        m_All_Materials_Info[priceList->m_MaterialID] = CMaterialInfo();
        m_All_Materials_Info[priceList->m_MaterialID].m_material_id = priceList->m_MaterialID;
        m_All_Materials_Info[priceList->m_MaterialID].m_pricelists.push_back(priceList);
        m_All_Materials_Info[priceList->m_MaterialID].m_producers_received.insert(prod);

        if(m_All_Materials_Info[priceList->m_MaterialID].m_producers_received.size() == m_Producers.size()){
          m_All_Materials_Info[priceList->m_MaterialID].m_has_all_pricelists = true;
          m_cv_Complete_CPriceList.notify_all();                                                                
        }
        return;
      }
      // if the materialID record was found
      
      // if the producer is not already in the list of producers that sent the price list, add it to the list
      if(it->second.m_producers_received.find(prod) == it->second.m_producers_received.end()){ 
        it->second.m_pricelists.push_back(priceList);
        it->second.m_producers_received.insert(prod);
      }
      
      // did I get pricelists from all the producers?
      if(it->second.m_producers_received.size() == m_Producers.size()){
        // then wake up customer threads waiting for all pricelists
        it->second.m_has_all_pricelists = true; // <- unlocking its condition
        m_cv_Complete_CPriceList.notify_all(); 
      }
    }
    
    void start(unsigned thrCount){
      // all threads = main + threadCount (workers) + customer.size (customer "servers")
      m_number_of_active_customers = m_Customers.size();
      m_number_of_workers = thrCount;

      // start up the customer threads
      for(size_t i = 0; i < m_Customers.size(); i++){
        m_catcher_threads.push_back(thread(&CWeldingCompany::catcherFnc, this, m_Customers[i]));
      }

      // start up the worker threads
      for(unsigned i = 0; i < thrCount; i++){
      m_worker_threads.push_back(thread(&CWeldingCompany::workerFnc, this));
      }

    }

    // stop method called to finalize the parallel work
    void stop(){
      for ( auto & t : m_catcher_threads )
        t.join (); 

      for ( auto & t : m_worker_threads )
        t.join (); 
    }


  
    vector<AProducer> m_Producers;                                  // producers
    vector<ACustomer> m_Customers;                                  // customers

    vector<thread> m_catcher_threads;                               // customer threads
    vector<thread> m_worker_threads;                                // worker threads

    unordered_map<unsigned, CMaterialInfo> m_All_Materials_Info;    // for each material there is a material info object

    queue<pair<size_t, shared_ptr<CPackageInfo>>> m_Orders_buffer;  // shared buffer      
    mutex m_mutex_wc;                                               // controls access to share buffer (critical section)

    condition_variable m_cv_Orders_buffer_empty;                    // protects from removing items from an empty buffer

    condition_variable m_cv_Complete_CPriceList;                    // protects from unifying before having all pricelists

    size_t m_number_of_active_customers;                            // counter for active customer threads in order 
                                                                        // to properly shut down the program afterwards
    int m_number_of_workers;                                        // variable to use for "lights out" afterwards
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