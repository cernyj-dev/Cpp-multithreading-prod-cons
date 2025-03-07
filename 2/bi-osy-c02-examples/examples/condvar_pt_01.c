/********************************************************************************
  - Main thread creates 4 working threads (T0, T1, T2, T3).
    - Fist shift:   T0 and T1 should be working,  T2 and T3 should be sleeping.
    - Second shift: T0 and T1 should be sleeping, T2 and T3 should be working.
  - There is some problem with synchonization. Try to correct it.
*********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <unistd.h>

int              g_Cnt = 0;
pthread_mutex_t  g_Mtx1, g_Mtx2;
pthread_cond_t   g_Cond1, g_Cond2;

/*************************************/
void * thr1 ( void * arg )
{
  int val;

  while ( 1 )
  {
    pthread_mutex_lock ( &g_Mtx1 );
    if ( g_Cnt < 10000 ) 
      g_Cnt ++;
    val = g_Cnt;
    pthread_mutex_unlock ( &g_Mtx1 );
    if ( val >= 10000 ) 
      break;
  }

  sleep( rand() % 2 );           /* <--- #2 */
  pthread_mutex_lock ( &g_Mtx2 );
  pthread_cond_broadcast ( &g_Cond2 ); /* mame dve podminky, musime mit druhou promennou!! */
  while( g_Cnt < 50000) pthread_cond_wait ( &g_Cond1, &g_Mtx2 ); /*opet potrebeme dat podminku - zase bychom zbytecne cekali, prestoze uz ta potrebna podminka neni aktualni*/
                                                                 /*v C++ tohle resi uz uvnitr a ten while tam je*/
  pthread_mutex_unlock ( &g_Mtx2 );
  printf ( "Counter1 = %d\n", g_Cnt );
  return NULL;
}

/*************************************/
void * thr2 ( void * arg )
{
  int val;
   
  sleep(1);                       /* <--- #1 */
  pthread_mutex_lock ( &g_Mtx2 );
  while(g_Cnt < 10000) pthread_cond_wait ( &g_Cond2, &g_Mtx2 ); /*TADY TEN WHILE JE POTREBA - abychom zbytecne necekali, kdyz uz ta podminka neplati*/
  pthread_mutex_unlock ( &g_Mtx2 );
  while ( 1 )
  {
    pthread_mutex_lock ( &g_Mtx1 );
    if ( g_Cnt < 50000 ) 
      g_Cnt ++;
    val = g_Cnt;
    pthread_mutex_unlock ( &g_Mtx1 );
    if ( val == 50000 ) 
      break;
  }
  pthread_mutex_lock ( &g_Mtx2 );
  pthread_cond_broadcast ( &g_Cond1 );
  pthread_mutex_unlock ( &g_Mtx2 );
  printf ( "Counter = %d\n", g_Cnt );
  return NULL;
}

/*************************************/
int main ( int argc, char * argv [] )
{
  int              i;
  pthread_t        thrID[4];
  pthread_attr_t   attr;
   
  pthread_attr_init ( &attr );
  pthread_attr_setdetachstate ( &attr, PTHREAD_CREATE_JOINABLE );

  pthread_mutex_init ( &g_Mtx1, NULL );
  pthread_mutex_init ( &g_Mtx2, NULL );
  pthread_cond_init  ( &g_Cond1, NULL );
  pthread_cond_init  ( &g_Cond2, NULL );

  for ( i = 0; i < 4; i ++ )
    if ( pthread_create ( &thrID[i], &attr, i < 2 ? thr1 : thr2, NULL ) )
    {
      perror ( "pthread_create" );
      return 1;    
    }
  pthread_attr_destroy ( &attr );  
    
  for ( i = 0; i < 4; i ++ )
    pthread_join ( thrID[i], NULL );  

  pthread_cond_destroy ( &g_Cond1 );
  pthread_cond_destroy ( &g_Cond2 );
  pthread_mutex_destroy ( &g_Mtx1 );
  pthread_mutex_destroy ( &g_Mtx2 );
  return 0;
} 
