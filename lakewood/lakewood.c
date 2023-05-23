#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>

typedef struct {
  int gNumber;
  char* water_craft;
  int vest_num;
  int useTime; 
  int status; //  0 means created, 1 means exited, 2 means joined, 3 means signaled
  pthread_cond_t cond;
} Group;

typedef struct node {
  int group;
  int needed_vests;
  pthread_cond_t cond;
  int status;
  struct node *next;
} node;

typedef struct queue {
  node *head;
  node *tail;
} queue;

int N;
long global_group = 0;
char* water_crafts[3] = {"kayak", "canoe", "sailboat"};
int need_vest[3] = {1, 2, 4};
int vest = 10;
pthread_mutex_t mutex1;
pthread_cond_t condition;
queue q;

bool queue_isFull(queue *q) {
  int count = 0;
  node *temp;
  temp = q->head;
  while (temp != NULL) {
    count++;
    temp = temp->next;
  }
  return (count == 5);
}

void queue_init(queue *q) {
  q->head = NULL;
  q->tail = NULL;
}

void queue_print(queue *q) {
  node *temp;
  temp = q->head;
  printf("Queue: [");
  while (temp != NULL) {
    printf("%d ", temp->group);
    temp = temp->next;
  }
  printf("]");
  printf("\n");
}

bool queue_isEmpty(queue *q) {
  return q->head == NULL;
}

void queue_insert(queue *q, int groupNum, int vestNum, pthread_cond_t cond) {
  struct node *tmp = malloc(sizeof(struct node));if (!queue_isEmpty(&q) && q.head->status != 3) { 
      queue_insert(&q, g->gNumber, g->vest_num, g->cond);
      printf("Group %d waiting in queue for %d jackets\n", g->gNumber, g->vest_num);
      queue_print(&q);
      pthread_cond_wait(&g->cond, &mutex1);
    }
  if (tmp == NULL) {
    fputs ("malloc failed\n", stderr);
    exit(1);
  }

  /* create the node */
  tmp->group = groupNum;
  tmp->needed_vests = vestNum;
  tmp->cond = cond;
  tmp->next = NULL;

  if (q->head == NULL) {
    q->head = tmp;
  } else {
    q->tail->next = tmp;
  }
  q->tail = tmp;
}

int queue_remove(queue *q) {
  int retval = 0;
  node *tmp;
  
  if (!queue_isEmpty(q)) {
    tmp = q->head;
    retval = tmp->group;
    q->head = tmp->next;
    free(tmp);
  }
  return retval;
}

void fatal (long n) {
  printf ("Fatal error, lock or unlock error, thread %ld.\n", n);
  exit(n);
}

void getJacket(Group* g) {
  if (pthread_mutex_lock(&mutex1)) { fatal(g->gNumber); }
    while (g->vest_num > vest) {
      if (!queue_isFull(&q)) {
        queue_insert(&q, g->gNumber, g->vest_num, g->cond);
        printf("Group %d waiting in queue for %d jackets\n", g->gNumber, g->vest_num);
        queue_print(&q);
      } else {
        printf("Group %d leaves due to long wait\n", g->gNumber);
        pthread_exit(NULL);
      }
      // pthread_cond_t condition = g->cond;
      pthread_cond_wait(&(g->cond), &mutex1);
    }
    // printf("here\n");
    // if the queue is not empty, and the group is not signaled, wait.
    if (!queue_isEmpty(&q) && q.head->status != 3) { 
      queue_insert(&q, g->gNumber, g->vest_num, g->cond);
      printf("Group %d waiting in queue for %d jackets\n", g->gNumber, g->vest_num);
      queue_print(&q);
      pthread_cond_wait(&g->cond, &mutex1);
    }
    vest -= g->vest_num;
    printf("Group %d issued %d jackets, %d remaining\n", g->gNumber, g->vest_num, vest);
    if (pthread_mutex_unlock(&mutex1)) { fatal(g->gNumber); } 
    sleep(g->useTime);
}

void * thread_body (void *group) {
  Group* g = (Group*)group;
  int choose = random() % 3;
  g->water_craft = water_crafts[choose];
  g->vest_num = need_vest[choose];
  g->useTime = (random() % 8) + 1;

  printf("Group number: %d, requested: %s, needs %d jackets\n",
   g->gNumber, g->water_craft, g->vest_num);

  getJacket(g);
  if (pthread_mutex_lock(&mutex1)) { fatal(g->gNumber); }
  vest += g->vest_num;
  printf("Group %d returned %d jackets, now have %d\n", g->gNumber, g->vest_num, vest);
  if (pthread_mutex_unlock(&mutex1)) { fatal(g->gNumber); }

  node *temp;
  temp = q.head;
  if (temp != NULL) {
    int need = temp->needed_vests;
    printf("Group %d needs %d jackets, remaining %d jackets\n", temp->group, need, vest);
    if (pthread_mutex_lock(&mutex1)) { fatal(g->gNumber); }
    temp->status = 3;
    pthread_cond_signal(&(temp->cond));
    // Enough vests are available for this group.
    // printf("Group %d issued %d jackets, %d remaining\n", temp->group, temp->needed_vests, vest);
    // Remove group from queue.
    int removed_group = queue_remove(&q);
    printf("Removed Group %d from queue\n", removed_group);
    // Signal the group to continue.
    
    // vest -= need;
    if (pthread_mutex_unlock(&mutex1)) { fatal(g->gNumber); }
    //  not enough vests are available for this group.
  }
  g->status = 1;
  pthread_exit(NULL);
}


int main (int argc, char** argv) {
  N = atoi(argv[1]);
  pthread_t ids[N];
  Group groups[N];
  int err;
  long i;
  long final_group = 0;
  int next_group = 10 / 2;
  queue_init(&q);

  if (argv[3] == NULL) {
    srandom(time(NULL));
  } else {
    srandom(0);
  }

  if (argv[2]) {
    next_group= atoi(argv[2]) / 2;  
  }
  
  pthread_mutex_init(&mutex1, NULL);
  
  for (i = 0; i < N; i++) {
    groups[i].status = 0;
    groups[i].gNumber = i;
    pthread_cond_init(&(groups[i].cond), NULL);
    err = pthread_create(&ids[i], NULL, thread_body, (void *)&groups[i]);
    sleep(random() % next_group);
    if (err) {
      fprintf (stderr, "Can't create thread %ld\n", i);
      exit (1);
    }
  }

  // printids("main");

  void *retval;

  for (i=0; i < N; i++) {
    pthread_cond_destroy(&(groups[i].cond));
    pthread_join(ids[i], &retval);
    final_group += (long)retval;
  }

  // printf ("global_group is %ld,  final_group is %ld\n", global_group, final_group);

  pthread_mutex_destroy(&mutex1);  // Not needed, but here for completeness
  return 0;
}