#include <cstdio>
#include <cmath>
#include <vector>
#include "sample_tester.h"
#include "common.h"


struct Plate{
  size_t width, height;
  double price;
};

double min(const double a, const double b){
  return a < b ? a : b;
}

double min_cost(size_t width, size_t height, double price, std::vector<Plate>& plates){
  std::vector<std::vector<double>> dp(width + 1, std::vector<double>(height + 1, __DBL_MAX__));

  dp[0][0] = 0.0;

  for(size_t w = 0; w <= width; w++){
      for(size_t h = 0; h <= height; h++){
          if(w == 0 || h == 0){
              dp[w][h] = 0.0;
              continue;
          }

          for(const auto& plate : plates){
              if((plate.width == w && plate.height == h) || (plate.width == h && plate.height == w)){
                  dp[w][h] = min(dp[w][h], plate.price);
              }
          }

          for(size_t k = 1; k < w; k++){
              if(dp[k][h] == __DBL_MAX__ || dp[w - k][h] == __DBL_MAX__){
                continue;
              }
              dp[w][h] = min(dp[w][h], dp[k][h] + dp[w - k][h] + h * price);
          }

          for(size_t k = 1; k < h; k++){
              if(dp[w][k] == __DBL_MAX__ || dp[w][h - k] == __DBL_MAX__){
                continue;
              }
              dp[w][h] = min(dp[w][h], dp[w][k] + dp[w][h - k] + w * price);
          }
      }
  }

  return dp[width][height];
}

void initRandom() {
  std::srand(std::time(nullptr)); // Seed once at the start
}

size_t getRandomSizeT(size_t max) {
  initRandom();
  return 0 + std::rand() % max; // Random integer from 1 to 200
}

double getRandomDouble(double max) {
  initRandom();
  return 0.0 + (std::rand() / (RAND_MAX / max)); // Random double from 1.0 to 200.0
}

//=============================================================================================================================================================
void CProducerSync::sendPriceList(unsigned materialID){
  if(materialID > 2){return;}
    APriceList l = std::make_shared<CPriceList>(materialID);
    for(const auto& x : c_Prod)
      l->add(x);
    m_Receiver(shared_from_this(), l);
}

std::vector<CProd> generate(size_t num){
  std::vector<CProd> v;
  initRandom();
  for(size_t i = 0; i < num; i++){
    v.push_back(CProd(getRandomSizeT(200), getRandomSizeT(200), getRandomDouble(500)));
  }

  return v;
}

std::vector<CProd> async_v = generate(std::rand() % 100);
std::vector<CProd> sync_v = generate(std::rand() % 100);

std::vector<std::pair<COrder, double>> generate_sol(size_t num){
  std::vector<std::pair<COrder, double>> v;
  initRandom();
  for(size_t i = 0; i < num; i++){
    size_t width = getRandomSizeT(200);
    size_t height = getRandomSizeT(200);
    double price = getRandomDouble(500);

    async_v.insert(async_v.end(), sync_v.begin(), sync_v.end());

    std::vector<Plate> plates;

    for(auto& element : async_v){
      plates.push_back({element.m_W, element.m_H, element.m_Cost});
    }

    COrder order = COrder(width, height, price);
    double sol = min_cost(width, height, price, plates);

    auto a = std::make_pair(order, sol);

    v.push_back(a);
  }
  return v;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// std::vector<CProd> CProducerSync::c_Prod = {
//   CProd(1, 1, 10),
//   CProd(2, 7, 120),
//   CProd(11, 8, 950)
// };

std::vector<CProd> CProducerSync::c_Prod = sync_v;


//=============================================================================================================================================================
void CProducerAsync::start(){
  m_Thr = std::move(std::thread(&CProducerAsync::prodThr, this));
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void CProducerAsync::stop(){
  std::unique_lock locker(m_Mtx);
  m_Stop = true;
  m_Cond.notify_one();
  locker.unlock();
  m_Thr.join();
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void CProducerAsync::sendPriceList(unsigned materialID){
  if(materialID == 1){
    std::lock_guard locker(m_Mtx);
    m_Req++;
    m_Cond.notify_one();
  }
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void CProducerAsync::prodThr(){
  while(true){
    std::unique_lock locker(m_Mtx);
    m_Cond.wait(locker, [this](void){
      return m_Stop || m_Req > 0;
    });

    if(m_Stop)
      break;
    if(m_Req > 0){
      m_Req--;
      locker.unlock ();
      APriceList l = std::make_shared<CPriceList>(1);
      for(const auto& x : c_Prod)
        l->add(x);
      m_Receiver(shared_from_this(), l);
      APriceList m = std::make_shared<CPriceList>(2);
      for(const auto& x : c_Prod)
        m->add(x);
      m_Receiver(shared_from_this(), m);
    }
  }
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// std::vector<CProd> CProducerAsync::c_Prod = {
//   CProd(2, 7, 125),
//   CProd(3, 5, 150),
//   CProd(7, 3, 240),
//   CProd(4, 4, 155),
// };
std::vector<CProd> CProducerAsync::c_Prod = async_v;
//=============================================================================================================================================================
AOrderList CCustomerTest::waitForDemand(){
  if(!m_Count)
    return AOrderList();
  m_Count--;
  AOrderList req = std::make_shared<COrderList>(1);
  for(const auto& x : c_Orders)
    req->add(x.first);
  return req;
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void CCustomerTest::completed(AOrderList x){
  bool mismatch = false;

  for(size_t i = 0; i < c_Orders.size(); i++)
    if(fabs(c_Orders[i].second - x->m_List[i].m_Cost) > 1e-5 * c_Orders[i].second){
      printf("is: %f, should be: %f: ERROR\n", x->m_List[i].m_Cost, c_Orders[i].second);
      mismatch = true;
      break;
    }
  printf("CCustomerTest::completed, status = %s\n", mismatch ? "fail" : "OK");
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// std::vector<std::pair<COrder, double>> CCustomerTest::c_Orders = {
//   std::make_pair(COrder(1, 1, 0.0), 10.0),
//   std::make_pair(COrder(7, 12, 1.0), 755.0),
//   std::make_pair(COrder(8,  4, 10.0), 350.0),
//   std::make_pair(COrder(25, 11, 0.1), 2399.8)
// };
std::vector<std::pair<COrder, double>> CCustomerTest::c_Orders = generate_sol(std::rand() % 100);
//=============================================================================================================================================================

