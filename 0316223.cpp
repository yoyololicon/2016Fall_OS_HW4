#include <unistd.h>
#include <semaphore.h>
#include <ctime>
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <sys/time.h>
using namespace std;

#define NUM_THREADS 8

struct job{
    vector<int>::iterator left, right;
    bool sort_or_not = false;
    int level;
};

vector<int>::iterator QuickSort_Buildthread(const vector<int>::iterator, const vector<int>::iterator, int);
void *threader(void*);

sem_t       sem, job_lock, next_round;
queue<job>  job_list;

int main()
{
    vector<int> matrix_1, matrix_2 ;
    int n, rc, data_n;
    pthread_t Thread[NUM_THREADS];
    struct timeval start, end;
    string file_name;

    ifstream data("input_100000_PN.txt", fstream::in);
    ofstream MT;

    data >> data_n;

    while(data >> n)
        matrix_1.push_back(n);

    matrix_2 = matrix_1;

    sem_init(&sem, 0, 0);
    sem_init(&job_lock, 0, 1);
    sem_init(&next_round, 0, 0);

    struct job first;
    first.left = matrix_1.begin();
    first.right = matrix_1.end();
    first.level = 3;

    for(int i = 0; i < NUM_THREADS; i++)
    {
        rc = pthread_create(&Thread[i], NULL, threader, NULL);
        if (rc){
            cout << "ERROR; return code from pthread_create(" << i+1 << ")" << endl;
            return 1;
        }
    }

    for(int i = 1; i <= 8; i++)
    {
        file_name = "output_" + to_string(i) + ".txt";
        MT.open(file_name.c_str(), fstream::out);

        sem_post(&sem);
        sem_wait(&job_lock);
        gettimeofday(&start, 0);

        job_list.push(first);
        sem_post(&job_lock);
        for(int j = 0; j < 8; j++)
            sem_wait(&next_round);

        gettimeofday(&end, 0);

        cout << i << "-thread" << endl;
        int sec = end.tv_sec - start.tv_sec;
        int usec = end.tv_usec - start.tv_usec;
        cout << "elapsed " << sec*1000+(usec/1000.0) << " ms" << endl;

        for(vector<int>::iterator i = matrix_1.begin() ; i != matrix_1.end() ; i++)
            MT << *i << " ";

        MT.close();
        matrix_1 = matrix_2;
    }

    sem_destroy(&sem);
    sem_destroy(&job_lock);
    sem_destroy(&next_round);
    data.close();
    return 0 ;
}

void *threader(void*)
{
    struct job current_job;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    while (1) {
        sem_wait(&sem);
        sem_wait(&job_lock);
        if(!job_list.empty()) {
            current_job = job_list.front();
            job_list.pop();
        }
        else{
            current_job.level = -1;
            current_job.sort_or_not = false;
        }
        sem_post(&job_lock);

        if (current_job.sort_or_not) {
            QuickSort_Buildthread(current_job.left, current_job.right, 0);
            sem_post(&next_round);
        }
        else if(current_job.level >= 0)
        {
            struct job child1, child2;
            child1.left = current_job.left;
            child2.right = current_job.right;
            child1.right = child2.left = QuickSort_Buildthread(current_job.left, current_job.right, 1);
            child1.level = child2.level = current_job.level - 1;
            if (current_job.level == 1)
                child1.sort_or_not = child2.sort_or_not = true;

            sem_wait(&job_lock);
            job_list.push(child1);
            job_list.push(child2);
            sem_post(&job_lock);
        }
        sem_post(&sem);
    }
#pragma clang diagnostic pop
}

vector<int>::iterator QuickSort_Buildthread(const vector<int>::iterator left , const vector<int>::iterator right, int build)
{
    if(left < right-1)
    {
        vector<int>::iterator i = left ;
        vector<int>::iterator j = right ;
        int pivot = *i ;
        do
        {
            do
                i++;
            while(*i < pivot) ;
            do
                j--;
            while(*j > pivot) ;
            if(i<j)
                swap(*i , *j) ;
        }
        while(i < j) ;
        swap(*left , *j) ;
        if(build)
            return i;

        QuickSort_Buildthread(left , j, 0);
        QuickSort_Buildthread(i , right, 0);
    }
}