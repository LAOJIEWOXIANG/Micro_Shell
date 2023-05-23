#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
  int gNumber;
  char* water_craft;
  int vest_num;
  int useTime; 
  int status; //  0 means created, 1 means exited, 2 means joined
  pthread_cond_t cond;
} Group;

typedef struct node {
  Group* g;
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
  return (count >= 5);
}

void queue_init(queue *q) {
  q->head = NULL;
  q->tail = NULL;
}

void queue_print(queue *q) {
  node *temp;
  temp = q->head;
  printf("Queue: [ ");
  while (temp != NULL) {
    printf("%d ", temp->g->gNumber);
    temp = temp->next;
  }
  printf("]");
  printf("\n");
}

bool queue_isEmpty(queue *q) {
  return q->head == NULL;
}

void queue_insert(queue *q, Group* group) {
  struct node *tmp = malloc(sizeof(struct node));
  if (tmp == NULL) {
    fputs ("malloc failed\n", stderr);
    exit(1);
  }

  /* create the node */
  tmp->g = group; 
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
    retval = tmp->g->gNumber;
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
  printf("Group number: %d, requested: %s, needs %d jackets\n",
   g->gNumber, g->water_craft, g->vest_num);
  if (!queue_isEmpty(&q) || g->vest_num > vest) {
    if (!queue_isFull(&q)) {
      queue_insert(&q, g);
      printf("Group %d waiting in queue for %d jackets\n", g->gNumber, g->vest_num);
      queue_print(&q);
      pthread_cond_wait(&(g->cond), &mutex1);
    } else {
      printf("Group %d leaves due to long wait\n", g->gNumber);
      if (pthread_mutex_unlock(&mutex1)) { fatal(g->gNumber); }
      g->status = 1;
      pthread_exit(NULL);
    }
  }
  if (!queue_isEmpty(&q)) {
    int removed_group = queue_remove(&q);
    printf("  Removed Group %d from queue\n", removed_group);
  }
  vest -= g->vest_num;
  printf("    Group %d issued %d jackets, %d remaining\n", g->gNumber, g->vest_num, vest);

  // if the queue is not empty after removal, check the new head's availability
  if (!queue_isEmpty(&q) && q.head->g->vest_num <= vest) {
    pthread_cond_signal(&(q.head->g->cond));
  }
  if (pthread_mutex_unlock(&mutex1)) { fatal(g->gNumber); } 
    
}

void * thread_body (void *group) {
  Group* g = (Group*)group;
  int choose = random() % 3;
  g->water_craft = water_crafts[choose];
  g->vest_num = need_vest[choose];
  g->useTime = (random() % 8) + 1;

  getJacket(g);
  sleep(g->useTime);
  if (pthread_mutex_lock(&mutex1)) { fatal(g->gNumber); }
  vest += g->vest_num;
  printf("Group %d returned %d jackets, now have %d\n", g->gNumber, g->vest_num, vest);
  if (!queue_isEmpty(&q)) {
    int need = q.head->g->vest_num;
    printf("Group %d needs %d jackets, remaining %d jackets\n", q.head->g->gNumber, need, vest);
    if (need <= vest) {
      pthread_cond_signal(&(q.head->g->cond));
    } else {
      printf("  Don't have enough jackets yet!\n");
    }
  }
  if (pthread_mutex_unlock(&mutex1)) { fatal(g->gNumber); }
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
  } else if(strcmp(argv[3], "r") == 0) {
    srandom(0);
  } else {
    srandom(atoi(argv[3]));
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
    // for (int j = 0; j < i; j++) {
    //   if (groups[j].status != 1) {
    //     pthread_join(ids[j], NULL);
    //     groups[j].status = 2;
    //   }
    // }
  }

  void *retval;

  for (i=0; i < N; i++) {
    if (groups[i].status != 2) {
      pthread_join(ids[i], &retval);
    }
  }

  // printf ("global_group is %ld,  final_group is %ld\n", global_group, final_group);

  pthread_mutex_destroy(&mutex1);  // Not needed, but here for completeness
  return 0;
}