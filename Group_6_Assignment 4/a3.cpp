#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>

using namespace std;

const int EVENTS = 100;
const int SEATS = 500;
const int NUM_THREADS = 20;
const int MAX = 5;
const int RUN_TIME = 60;
const int minTickets = 5;
const int maxTickets = 10;

struct queryInfo {
  int eId;  // Event number
  int qType;    // Query type. 0 - INQUIRE, 1 - BOOK, 2 - CANCEL
  int tId; // Thread making the query
};
typedef struct queryInfo queryInfo;

queryInfo sharedTable[MAX];

vector<int> availableSeats(EVENTS + 1, SEATS);

pthread_mutex_t tableMutex;
pthread_cond_t tableCondition = PTHREAD_COND_INITIALIZER;
pthread_mutex_t seatMutex;

bool canRead(int eId) {
  for (int i = 0; i < MAX; i++) {
    if (sharedTable[i].eId == eId && sharedTable[i].qType != 0)
      return false;
  }
  
  return true;
}

bool canWrite(int eId) {
  for (int i = 0; i < MAX; i++) {
    if (sharedTable[i].eId == eId)
      return false;
  }
  
  return true;
}

int findBlankEntry() {
  for (int i = 0; i < MAX; i++) {
    if (sharedTable[i].eId == 0) {
      return i;
    }
  }
  return -1;
}

// Inquiry for an event
void inquireEvent(int eId, int tId) {
  pthread_mutex_lock(&tableMutex);
  int tableEntry = findBlankEntry();

  if (tableEntry == -1 || !canRead(eId)) {
    pthread_cond_wait(&tableCondition, &tableMutex);
  }

  tableEntry = findBlankEntry();
  if (tableEntry == -1)
    return;
  
  sharedTable[tableEntry].eId = eId;
  sharedTable[tableEntry].qType = 0;
  sharedTable[tableEntry].tId = tId;
  pthread_mutex_unlock(&tableMutex);
  
  pthread_mutex_lock(&seatMutex);
  if (availableSeats[eId] == 0)
    printf("\n\nInquire, Thread - %d, Event - %d : The event is housefull..!", tId, eId);
  else
    printf("\n\nInquire, Thread - %d, Event - %d : %d seats available.", tId, eId, availableSeats[eId]);
  pthread_mutex_unlock(&seatMutex);
  
  pthread_mutex_lock(&tableMutex);
  sharedTable[tableEntry].eId = 0;
  pthread_mutex_unlock(&tableMutex);
  
  pthread_cond_signal(&tableCondition);
}

// Booking the seats if available for the event specified
int bookEvent(int eId, int seatsToBook, int tId) {
  pthread_mutex_lock(&tableMutex);
  int tableEntry = findBlankEntry();

  if (tableEntry == -1 || !canRead(eId)) {
    pthread_cond_wait(&tableCondition, &tableMutex);
  }

  tableEntry = findBlankEntry();
  if (tableEntry == -1)
    return -1;

  sharedTable[tableEntry].eId = eId;
  sharedTable[tableEntry].qType = 1;
  sharedTable[tableEntry].tId = tId;
  pthread_mutex_unlock(&tableMutex);

  pthread_mutex_lock(&seatMutex);
  if (seatsToBook > availableSeats[eId]) {
    printf("\n\nBooking, Thread - %d, Event - %d : Not enough seats to book %d seats!", tId, eId, seatsToBook);
    pthread_mutex_unlock(&seatMutex); 
    return -1;                        
  }

  printf("\n\nBooking, Thread - %d, Event - %d : Booked %d seats", tId, eId, seatsToBook);

  availableSeats[eId] -= seatsToBook;
  pthread_mutex_unlock(&seatMutex);

  pthread_mutex_lock(&tableMutex);
  sharedTable[tableEntry].eId = 0;
  pthread_mutex_unlock(&tableMutex);

  pthread_cond_signal(&tableCondition);

  return 1; 
}

// Cancel the booking for specific event
int cancelEvent(int tId, vector<pair<int, int>> &bookings) {

  int totalBookings = bookings.size();
  if (totalBookings < 1) {
    printf("\n\nCancel, Thread - %d : No bookings found ", tId);
    return -1;
  }
  int m = 0, n = totalBookings - 1;
  int pos = rand() % (n - m + 1) + m;
  m = 1, n = bookings[pos].first;
  int numOfSeats = rand() % (n - m + 1) + m; 
  int eId = bookings[pos].second; 

  pthread_mutex_lock(&tableMutex);
  int tableEntry = findBlankEntry();

  if (tableEntry == -1 || !canRead(eId)) {
    pthread_cond_wait(&tableCondition, &tableMutex);
  }

  tableEntry = findBlankEntry();
  if (tableEntry == -1)
    return -1;

  sharedTable[tableEntry].eId = eId;
  sharedTable[tableEntry].qType = 2;
  sharedTable[tableEntry].tId = tId;
  pthread_mutex_unlock(&tableMutex);

  pthread_mutex_lock(&seatMutex);
  printf("\n\nCancel, Thread - %d, Event - %d : Canceled %d seats for the event.", tId, eId, numOfSeats);

  availableSeats[eId] += numOfSeats;

  bookings.erase(bookings.begin() + pos);
  pthread_mutex_unlock(&seatMutex);

  pthread_mutex_lock(&tableMutex);
  sharedTable[tableEntry].eId = 0;
  pthread_mutex_unlock(&tableMutex);
  
  pthread_cond_signal(&tableCondition);

  return 1;
}

void *workerThread(void *tid) {
  int tId = *((int *)tid);
  srand(time(NULL) + tId); 

  vector<pair<int, int>> bookings;

  while (true) {
    int queryType = rand() % (3 - 0 + 1) + 0; 
    int eventNum = rand() % (EVENTS - 1 + 1) + 1; 

    if (queryType == 0) { 
      inquireEvent(eventNum, tId);
      sleep(rand() % (3 - 0 + 1) + 0); 
    } 
    else if (queryType == 1) {     
      int seatsToBook = rand() % (maxTickets - minTickets + 1) + minTickets; 
      int success = bookEvent(eventNum, seatsToBook, tId);

      if (success == 1)
        bookings.push_back(make_pair(seatsToBook, eventNum));

      sleep(rand() % (3 - 1 + 1) + 1);
    } 
    else if (queryType == 2) {
      cancelEvent(tId, bookings);
      sleep(rand() % (3 - 1 + 1) + 1);
    }
  }
}

// The master thread that handles the initialization and thread creation part.
int main(int argc, char const *argv[]) {
  cout << endl;

  pthread_mutex_init(&tableMutex, NULL);
  pthread_mutex_init(&seatMutex, NULL);

  pthread_t tIds[NUM_THREADS]; 
  for (int i = 0; i < NUM_THREADS; i++) {
    void *t = &i;
    pthread_create(&tIds[i], NULL, workerThread, t);
  }

  sleep(RUN_TIME);

  for (int i = 0; i < NUM_THREADS; i++)
    pthread_cancel(tIds[i]);

  for (int i = 0; i < NUM_THREADS; i++)
    pthread_join(tIds[i], NULL);

  cout << "\n\nPrinting reservation status\n" << endl;
  for (int i = 1; i <= EVENTS; i++) {
    float perc = (SEATS - availableSeats[i]) / (float)SEATS * 100;
    printf("Event - %d : %.2f %% booked with %d seats leftover.\n", i, perc, availableSeats[i]);
  }

  pthread_mutex_destroy(&tableMutex);
  pthread_mutex_destroy(&seatMutex);
  pthread_cond_destroy(&tableCondition);

  return 0;
}
