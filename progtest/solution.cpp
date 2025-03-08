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

class CWeldingCompany
{
  public:
    static bool                        usingProgtestSolver                     ( )
    {
      return true;
    }
    static void                        seqSolve                                ( APriceList                            priceList,
                                                                                 COrder                              & order )
    {
      // empty, using progtest solver
    }
    void                               addProducer                             ( AProducer                             prod );
    void                               addCustomer                             ( ACustomer                             cust );
    void                               addPriceList                            ( AProducer                             prod,
                                                                                 APriceList                            priceList );
    void                               start                                   ( unsigned                              thrCount );
    void                               stop                                    ( );
};

// TODO: CWeldingCompany implementation goes here

//-------------------------------------------------------------------------------------------------
#ifndef __PROGTEST__
int                                    main                                    ()
{
  using namespace std::placeholders;
  CWeldingCompany  test;

  AProducer p1 = std::make_shared<CProducerSync> ( std::bind ( &CWeldingCompany::addPriceList, &test, _1, _2 ) );
  AProducerAsync p2 = std::make_shared<CProducerAsync> ( std::bind ( &CWeldingCompany::addPriceList, &test, _1, _2 ) );
  test . addProducer ( p1 );
  test . addProducer ( p2 );
  test . addCustomer ( std::make_shared<CCustomerTest> ( 2 ) );
  p2 -> start ();
  test . start ( 3 );
  test . stop ();
  p2 -> stop ();
  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
