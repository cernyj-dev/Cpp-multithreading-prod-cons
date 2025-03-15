/********************************************************************************
  - Producer/consumer problem synchronized by condition variables.
  - There is some problem with program termination. Try to correct it.

    - If the number of producers and consumers is the same, 
      then the proram runs correctly, otherwise it will end by deadlock.
*********************************************************************************/

#include <iostream>
#include <vector>
#include <chrono>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>

#define BUFFER_SIZE 5

using namespace std;

// item of buffer
struct item_t
{
  int  tid;           // tid of the thread that created the item
  int  id;            // sequence number of the item creation
  int  value;         // value of item
};

/*************************************/
class BufferClass
{
private:
  deque<item_t*>       buff;       // shared buffer
  mutex                mtx;        // controls access to share buffer (critical section)
  condition_variable   cv_full;    // protects from inserting items into a full buffer
  condition_variable   cv_empty;   // protects from removing items from an empty buffer

  

  /****************************/
  void PrintBuffer()
  {
    printf("Buffer: ");
    for (deque<item_t*>::iterator it = buff.begin(); it!=buff.end(); ++it)
      printf("[%d, %d, %d] ",(*it)->tid, (*it)->id, (*it)->value);
    printf("\n");
  }

public: 
  /****************************/
  BufferClass() { }
  size_t              active_producers = 0;  // number of active producers
  int                 num_consumers = 0;     // number of active consumers
  mutex               mtx_lights_out;        // mutex for lights_out variable
  /****************************/
  virtual void insert(item_t *item) 
  {
    
    unique_lock<mutex> ul (mtx);
    cv_full.wait(ul, [ this ] () { return ( buff.size() < BUFFER_SIZE ); } );
    buff.push_back(item);

    printf("Producer %d:  item [%d,%d,%d] was inserted\n", item->tid, item->tid, item->id, item->value);
    PrintBuffer();
    

    cv_empty.notify_one();
  }

  /****************************/
  virtual item_t * remove(int tid) 
  {
    item_t *item;

    unique_lock<mutex> ul (mtx);
    cv_empty.wait(ul, [ this ] () { return ( ! buff.empty() ); } );
    item = buff.front();
    buff.pop_front();
   
    printf("Consumer %d:  item [%d,%d,%d] was removed\n", tid, item->tid, item->id, item->value);
    PrintBuffer();

    cv_full.notify_one();
    return item;
  }
  
  /****************************/
  virtual bool empty()
  {
    return buff.empty();
  }
};

/*************************************/
void producer(int tid, int delay, BufferClass &buff)
{
  int count = 0;
  int delay_ms = 100 * delay; 

  // Insert 10 items to the buffer
  for ( int i = 0; i < 10; i++ )
  {
    item_t *item  = new item_t { tid, count, rand() % 100 };
    count++;
    
    buff.insert(item);

    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
  }

  printf("Producer %d: end\n", tid);
  {
    lock_guard<mutex> lg(buff.mtx_lights_out);
    // If all producers are done, then fill the buffer with some NULL items -> lights out
    buff.active_producers--;
    if (buff.active_producers == 0){
      printf("Producer %d: lights out\n", tid);
      // there could be a problem... if i dont generate 10*buff.num_consumers items, then the program will end by deadlock - maybe
      for(int i = 0; i < buff.num_consumers; i++){
        item_t *end_signal  = new item_t { -1, count++, 0 };
        buff.insert(end_signal);
      }
    }
  }
}



/*************************************/
void consumer(int tid, int delay, BufferClass &buff )
{  
  int delay_ms = 100 * delay; 

  while(true){
    // Remove 10 items from the buffer
    // Maybe i should break it if i get a NULL item
    for ( int i = 0; i < 10; i++ ) 
    {
      item_t *item = buff.remove(tid);
      int value = item->tid;
      delete(item);
      std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms)); // this should be here to spot some mistakes
                                                                        // Maybe i should put this in a separate mutex ... just like the for loop in producer - but that might be obsolete
                                                                        // because only single prod does that
      if(value == -1){
        printf("Consumer %d: end\n", tid);
        return;
      }
      
    }
  }
}

/*************************************/
int main ( int argc, char * argv [] )
{
  int            prods;       // Number of producers
  int            prods_delay; // Producer delay
  int            cons;        // Number of consumers
  int            cons_delay;  // Consumer delay
  BufferClass    buffer;      // Shared buffer
  vector<thread> threads;

  // Check arguments
  if ( argc != 5 || sscanf ( argv[1], "%d", &prods )       != 1
                 || sscanf ( argv[2], "%d", &prods_delay ) != 1
                 || sscanf ( argv[3], "%d", &cons )        != 1
                 || sscanf ( argv[4], "%d", &cons_delay )  != 1
                 || prods < 1 || prods_delay < 0 || cons < 1 || cons_delay < 0 )
  {
    printf ( "Usage: %s <# of produsers> <producer delay> <# of consumers> <consumer delay>\n", argv[0] );
    return ( 1 );
  }
  
  buffer.active_producers = prods;
  buffer.num_consumers = cons;
  
  // Create produsers 
  for ( int i = 0; i < prods; i ++ )
    threads.push_back ( thread ( producer, i, prods_delay, ref(buffer) ) );

  // Create consumers
  for ( int i = 0; i < cons; i ++ )
    threads.push_back ( thread ( consumer, i, cons_delay, ref(buffer) ) );

  // Wait for all threads
  for ( auto & t : threads )
    t.join (); 

  return 0;
} 
