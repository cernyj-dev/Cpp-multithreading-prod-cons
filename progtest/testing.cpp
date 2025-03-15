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
#include "solution.cpp"


void test(CWeldingCompany& test, size_t workers, size_t a_producers, size_t s_producers, size_t customers){
    using namespace std::placeholders;
    std::vector<AProducer> producers;
    std::vector<AProducerAsync> producers_async;
  
    for(size_t i = 0; i < a_producers; i++){
      AProducerAsync a = std::make_shared<CProducerAsync>(std::bind(&CWeldingCompany::addPriceList, &test, _1, _2));
      producers_async.push_back(a);
      test.addProducer(producers_async[i]);
      a->start();
    }
  
    for(size_t i = 0; i < s_producers; i++){
      producers.push_back(std::make_shared<CProducerSync>(std::bind(&CWeldingCompany::addPriceList, &test, _1, _2)));
      test.addProducer(producers[i]);
    }
  
    for(size_t i = 0; i < customers; i++){
      std::srand(std::time(nullptr));
      test.addCustomer(std::make_shared<CCustomerTest>(std::rand() % 100));
    }
  
    test.start(workers);
    test.stop();
  
    for(size_t i = 0; i < a_producers; i++){
      AProducerAsync a = producers_async[i];
      a->stop();
    }
  }


int main(){
  CWeldingCompany brambor;
  cout << "Test 1" << endl;
  test(brambor, 1, 1, 10, 1);

  return EXIT_SUCCESS;
}