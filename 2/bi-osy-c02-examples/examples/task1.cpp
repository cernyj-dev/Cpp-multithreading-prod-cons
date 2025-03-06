#include <iostream>
#include <vector> 
#include <chrono>
#include <thread>                                                          

using namespace std;

//-------------------------------------------
class CounterClass                                                          // <--- #1
{
public: 
  CounterClass(){
    c = 0;
  }
  void Solve(int n)
  {
    for(int i = 1; i <= n; i++){
      c = c + i; // tohle je kriticka sekce - nektere zapisy ztracime, protoze je to prepsano tim druhym v jeden momentik
                 // tisice jsou v poradku - jedno jadro to stihne zatahnout driv, nez se nastartuje to druhy
                 // 
    }
  }

  int getC(){
    return c;
  }

private:
  long long c;
};
 
//-------------------------------------------
int main ( int argc, char * argv [] )
{
  CounterClass    threadObj;    
  vector<thread>  threads;
  int             n1;

  // Check arguments
  if (argc != 3 || sscanf(argv[1], "%d", &n1) != 1)
  {
    //printf("Usage: %s number_of_threads another_argument\n", argv[0]);
    return 1;
  }

  // You can process the second argument here if needed
  int n2;
  if (sscanf(argv[2], "%d", &n2) != 1)
  {
    //printf("Usage: %s number_of_threads another_argument\n", argv[0]);
    return 1;
  }

  //printf("Main:     Start\n");
  
  // Create threads
  /*
  Like telling threadNum number of athletes to start running to a nearby tree and return
  without waiting for them to return. We just iterate through them, add them to the 
  */

  threads.push_back(thread(&CounterClass::Solve, &threadObj, n1));
  threads.push_back(thread(&CounterClass::Solve, &threadObj, n2));

  // Wait for threads
  for ( int i = 0; i < 2; i++ )
    threads[i].join();                                                  

  printf("Result: %d\n", threadObj.getC());
  return 0;
}

