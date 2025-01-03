#include <iostream> //STD INPUT OUTPUT
#include <algorithm> // for std::sort
#include <queue> //DATA STRUC
#include <iomanip> // FOR SETW IN THE PRINT SHEDULE FUNC
#include <pthread.h> //THREADING AND MUTEX
#include <string> //STRING OPERATIONS
#include <fcntl.h> //SHM
#include <sys/stat.h> //SHM
#include <sys/types.h> //SHM
#include <sys/wait.h> //WAIT SYSTEM CALL
#include <cstring> //STRING MANIPULATION
#include <fstream> //FILING
#include <sstream> //CSV DATA SPLITING
#include <string>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <sys/mman.h> //SHM
#include <sys/stat.h> //SHM
#include <unistd.h>

using namespace std;

#define NUMTHREADS 5
#define NUMPROCESS 10
#define MAX_BUFFER_SIZE 10000
 int cnt=0;//global
 int flag=-1;
 bool same=false;
string global22="";
class process;
// FUNCTION DECLARATION/
bool sortByShortestJob(const process &a, const process &b);
bool sortByArrivalTime(const process &a, const process &b);
bool sortByShortestRemainingJob(const process &a, const process &b);
void *sumturnaround(void *arg);
int calculateturnaroundtime();
void *sumResponse(void *arg);
int calculateResponsetime();
void *sumwaiting(void *arg);
int calculatewaitingtime();
void printSchedule(process processes[], int numProcesses);
void *FCFS(void *p1);
void *shortestjobfirst(void *p1);
void *roundrobin(void *p1);
void *shortestremainingtime(void *p1);

class process   //MAIN PROCESS CLASS
{
	//ATTRIBUTES
    string name;
    int arrivalTime;
    int burstTime;
    int dupburstTime;

public:
    bool processing;
    bool done;
    int starttime;
    int endtime;
    static int cnt;
    process(string n, int a, int b)
    {
        name = n;
        arrivalTime = a;
        burstTime = b;
        done = false;
        processing = false;
        starttime = 999;
        endtime = 999;
        dupburstTime = b;
        cnt++;
    }
    process()
    {
    }
    void setprocess(string n, int a, int b)
    {
        name = n;
        arrivalTime = a;
        burstTime = b;
        done = false;
        processing = false;
        starttime = 999;
        endtime = 999;
        dupburstTime = b;
        cnt++;
    }

    int getArrivalTime() const
    {
        return arrivalTime;
    }

    int getBurstTime() const
    {
        return burstTime;
    }
    int getdupBurstTime() const
    {
        return dupburstTime;
    }

    string getname()
    {
        return name;
    }
    void decrementBurstTime(int k)
    {
        dupburstTime = dupburstTime - k;
    }
    int getTurnaroundTime() const
    {
        return (endtime - arrivalTime);
    }

    int getWaitingTime() const
    {
        return (getTurnaroundTime() - burstTime);
    }

    int getResponseTime() const
    {
        return (starttime - arrivalTime);
    }
};
// allocation
process arr[NUMPROCESS];
int process::cnt = 0;

bool sortByShortestJob(const process &a, const process &b)
{
    return a.getBurstTime() < b.getBurstTime();
}
bool sortByArrivalTime(const process &a, const process &b)
{
    return a.getArrivalTime() < b.getArrivalTime();
}
bool sortByShortestRemainingJob(const process &a, const process &b)
{
    return a.getdupBurstTime() < b.getdupBurstTime();
}

//******* multi threading task parallelism *******
//
pthread_mutex_t turn;
void *sumturnaround(void *arg)
{
    int x = 10;
    int tid = *((int *)arg);
    int sum = 0;
    int start = tid * (x / NUMTHREADS);
    int end = start + (x / NUMTHREADS);
    if (tid == NUMTHREADS - 1)
    {
        end = x;
    }
    for (int i = start; i < end; i++)
    {
    	pthread_mutex_lock(&turn);   //MAKING SURE THAT ALL TH DATA IS ADDED CORRECTED LY AND 2 PROCESS IS NOT MAKING MESS WITH SUM VARIABLE
        sum += arr[i].getTurnaroundTime(); // Corrected access to process object
    	pthread_mutex_unlock(&turn);
	}
    int *result = new int(sum); // Dynamically allocate memory for result
    pthread_exit((void *)result);
    return (arg);
}

int calculateturnaroundtime()
{
    int sum1 = 0;
    pthread_t tid[NUMTHREADS];
    int thread_ids[NUMTHREADS];
    for (int i = 0; i < NUMTHREADS; i++)
    {
        thread_ids[i] = i;
        pthread_create(&tid[i], NULL, sumturnaround, (void *)&thread_ids[i]);
    }

    for (int i = 0; i < NUMTHREADS; i++)
    {
        int *result;
        pthread_join(tid[i], (void **)&result);
        if (result != nullptr)
        {
            // chcking when working on it
            //   cout << "CHECKING PURPOSE Result FROM THREAD " << i << ": " << *result << endl; // Print the result value
            sum1 += *result;
            delete result;
        }
        else
        {
            // Handle error or exception
            cout << "Error: Thread " << i << " failed to return a result." << endl;
        }
    }
    return sum1 / NUMPROCESS;
}
// %%%%%%%%%%%%%%%%%%%%5 RESPONSE TIME %%%%%%%%%%%%%%%%
pthread_mutex_t res;
void *sumResponse(void *arg)
{
    int x = 10;
    int tid = *((int *)arg);
    int sum = 0;
    int start = tid * (x / NUMTHREADS);
    int end = start + (x / NUMTHREADS);
    if (tid == NUMTHREADS - 1)
    {
        end = x;
    }
    for (int i = start; i < end; i++)
    {
    	pthread_mutex_lock(&res);   //MAKING SURE THAT ALL TH DATA IS ADDED CORRECTED LY AND 2 PROCESS IS NOT MAKING MESS WITH SUM VARIABLE
        sum += arr[i].getResponseTime(); // Corrected access to process object
   		pthread_mutex_unlock(&res);
    }
    int *result = new int(sum); // Dynamically allocate memory for result
    pthread_exit((void *)result);
    return (arg);
}

int calculateResponsetime()
{
    int sum1 = 0;
    pthread_t tid[NUMTHREADS];
    int thread_ids[NUMTHREADS];
    for (int i = 0; i < NUMTHREADS; i++)
    {
        thread_ids[i] = i;
        pthread_create(&tid[i], NULL, sumResponse, (void *)&thread_ids[i]);
    }

    for (int i = 0; i < NUMTHREADS; i++)
    {
        int *result;
        pthread_join(tid[i], (void **)&result);
        if (result != nullptr)
        {
            sum1 += *result;
            delete result;
        }
        else
        {
            // Handle error or exception
            cout << "Error: Thread " << i << " failed to return a result." << endl;
        }
    }
    return sum1 / NUMPROCESS;
}
// %%%%%%%%%%%%%%%%%%%%5 WAITING TIME %%%%%%%%%%%%%%%%
pthread_mutex_t wai;
void *sumwaiting(void *arg)
{
    int x = 10;
    int tid = *((int *)arg);
    int sum = 0;
    int start = tid * (x / NUMTHREADS);
    int end = start + (x / NUMTHREADS);
    if (tid == NUMTHREADS - 1)
    {
        end = x;
    }
    for (int i = start; i < end; i++)
    {
    	    	pthread_mutex_lock(&wai);   //MAKING SURE THAT ALL TH DATA IS ADDED CORRECTED LY AND 2 PROCESS IS NOT MAKING MESS WITH SUM VARIABLE
        sum += arr[i].getWaitingTime(); // Corrected access to process object
    pthread_mutex_unlock(&wai);
	}
    int *result = new int(sum); // Dynamically allocate memory for result
    pthread_exit((void *)result);
    return (arg);
}

int calculatewaitingtime()
{
    int sum1 = 0;
    pthread_t tid[NUMTHREADS];
    int thread_ids[NUMTHREADS];
    for (int i = 0; i < NUMTHREADS; i++)
    {
        thread_ids[i] = i;
        pthread_create(&tid[i], NULL, sumwaiting, (void *)&thread_ids[i]);
    }

    for (int i = 0; i < NUMTHREADS; i++)
    {
        int *result;
        pthread_join(tid[i], (void **)&result);
        if (result != nullptr)
        {
            // chcking when working on it
            //   cout << "CHECKING PURPOSE Result FROM THREAD " << i << ": " << *result << endl; // Print the result value
            sum1 += *result;
            delete result;
        }
        else
        {
            // Handle error or exception
            cout << "Error: Thread " << i << " failed to return a result." << endl;
        }
    }
    return sum1 / NUMPROCESS;
}

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% FIRST COME FIRST SERVE ALGORITHM %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void *FCFS(void *p1)
{
    flag=0;
    process *p = (process *)p1;
    sort(p, p + p->cnt, sortByArrivalTime);
    int time = 0;
    for (int i = 0; i < p->cnt; i++)
    {

        if (p[i].getArrivalTime() > time)
        {
            time = p[i].getArrivalTime();
        }
        int ttime = time + p[i].getBurstTime();
        cout << endl
             << "PROCESS NUM " << i + 1 << " NAME = " << p[i].getname() << " EXECUTING TIME " << time << " TERMINATING time " << ttime;
        p[i].starttime = time;
        p[i].endtime = ttime;
        time = ttime;
    }
    pthread_exit(0);
    return p;
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% SHORTEST REMAINING JOB FIRST ALGORITHM %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void *shortestjobfirst(void *p1)
{
    flag=1;
    process *p = (process *)p1;
    cout << endl
         << "SHORTEST JOB FIRST";
    sort(p, p + p->cnt, sortByShortestJob);
    int time = 0;
    int completed = 0;
    int k = 0;
    while (completed < p->cnt)
    {
        k = 100;
        for (int i = 0; i < p->cnt; i++)
        {

            if ((p[i].getArrivalTime() <= time) && !p[i].done)
            {
                int ttime = time + p[i].getBurstTime();
                cout << endl
                     << "PROCESS NUM " << i + 1 << " NAME = " << p[i].getname() << " EXECUTING TIME " << time << " TERMINATING time " << ttime;

                p[i].starttime = time;
                time = ttime;
                time++;
                p[i].endtime = ttime;
                p[i].done = true;
                sort(p, p + p->cnt, sortByShortestJob);
                //   printSchedule(arr,n);
                i = -1;
                k++;
                completed++;
            }
        }
        if (k == 100)
        {
            for (int t = 0; t < p->cnt; t++)
                if (!p[t].done)
                {
                    time = p[t].getArrivalTime();
                    break;
                }
        }
    }
    pthread_exit(0);
    return p;
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% ROUND ROBIN %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void *roundrobin(void *p1)
{
    flag=2;
    process *p = (process *)p1;
    int time = 0;
    queue<process> RR;
    int completed = 0;
    int k = 0;
    while (completed < p->cnt)
    {
        k = 100;
        for (int i = 0; i < p->cnt; i++)
        {
            if ((p[i].getArrivalTime() <= time) && !p[i].done)
            {
                cout << endl
                     << " PROCESS " << p[i].getname() << " STARTED AT " << time << " REMAINING BURST " << p[i].getdupBurstTime() << endl;
                k++;
                if (!p[i].processing)
                {
                    p[i].processing = true;
                    p[i].starttime = time;
                }
                time += 3;
                p[i].decrementBurstTime(3);
                if (p[i].getdupBurstTime() > 0)
                {
                    RR.push(p[i]);
                }
                else
                {
                    completed++;
                    p[i].done = true;
                    p[i].endtime = time;
                    cout << endl
                         << " PROCESS " << p[i].getname() << " ENDED AT " << time << " REMAINING BURST " << p[i].getdupBurstTime() << endl;
                }
            }
        }

        if (k == 100)
        {
            sort(p, p + p->cnt, sortByArrivalTime);
            for (int t = 0; t < p->cnt; t++)
                if (!p[t].done)
                {
                    time = p[t].getArrivalTime();
                    RR.push(p[t]);
                    break;
                }
        }
        if (RR.empty())
        {
            break;
        }
    }
    pthread_exit(0);
    return p;
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% SHORTEST REMAINING TIME ALGORITHM %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void *shortestremainingtime(void *p1)
{
    flag=3;
    process *p = (process *)p1;
    int time = 0;
    int k = 0;
    queue<process> SRT;
    int completed = 0;
    const int bufferSize = 100;
    char str[bufferSize];
    sort(p, p + p->cnt, sortByShortestRemainingJob);
    while (completed < p->cnt)
    {
        k = 100;
        for (int i = 0; i < p->cnt; i++)
        {
            if ((p[i].getArrivalTime() <= time) && !p[i].done)
            {

                cout << endl
                     << " PROCESS " << p[i].getname() << " STARTED AT " << time << " REMAINING BURST " << p[i].getdupBurstTime() << endl;
                k++;
                if (!p[i].processing)
                {
                    p[i].processing = true;
                    p[i].starttime = time;
                }
                time++;
                p[i].decrementBurstTime(1);

                if (p[i].getdupBurstTime() > 0)
                {
                    SRT.push(p[i]);
                }
                else
                {
                    completed++;
                    p[i].done = true;
                    p[i].endtime = time;
                    cout << endl
                         << " PROCESS " << p[i].getname() << " ENDED AT " << time << " REMAINING BURST " << p[i].getdupBurstTime() << endl;
                }
                sort(p, p + p->cnt, sortByShortestRemainingJob);
                i--;
            }
        }

        if (k == 100)
        {
            sort(p, p + p->cnt, sortByArrivalTime);
            for (int t = 0; t < p->cnt; t++)
                if (!p[t].done)
                {
                    SRT.push(p[t]);
                    time = p[t].getArrivalTime();
                    break;
                }
        }
        if (SRT.empty())
        {
            break;
        }
    }
    pthread_exit(0);
    return p;
}
void split(const string &s, char delimiter, string tokens[]);

void csvfile() // the data in csv file has been generated syntheticly using randbetween function
{

    ifstream file("data.csv");
    if (!file.is_open())
    {
        cerr << "Error opening file!" << endl;
    }
same=true;
    string cells[NUMPROCESS][2];
    int numRows = 0;

    string line;
    while (getline(file, line))
    {
        string tokens[2];
        split(line, ',', tokens);
        int numCols = sizeof(tokens) / sizeof(tokens[0]);
        for (int i = 0; i < numCols; ++i)
        {
            cells[numRows][i] = tokens[i];
        }
        ++numRows;
        if (numRows == NUMPROCESS)
        {
            break;
        }
    }

    file.close();

    for (int k = 0; k < NUMPROCESS; k++)

    {
        arr[k].setprocess(" P " + to_string(k), atoi(cells[k][0].c_str()), atoi(cells[k][1].c_str()));  //.c_str is used to make the atoi work correctly
    }
}
void split(const string &s, char delimiter, string tokens[])
{ // FUNCTION FOR THE CSV FILE TO TOKENIZE THE CELL
    stringstream ss(s);
    string token;
    int i = 0;
    while (getline(ss, token, delimiter))
    {
        tokens[i++] = token;
    }
}
void getinput()
{
    int choice = 999;
    cout << endl
         << "PLEASE CHOOSE WHICH INPUT METHOD YOU WANT:\n1) CSV FILE READING\n2) RANDOM INPUT\n3) USER INPUT\n";
    cin >> choice;

    while (1)
    {
    	if(choice==456)
    	{
    		cout << endl
         << "PLEASE CHOOSE CORRECT INPUT METHOD YOU WANT:\n1) CSV FILE READING\n2) RANDOM INPUT\n3) USER INPUT\n";
    cin >> choice;
		}
        if (choice == 2)
        {
            for (int k = 0; k < NUMPROCESS; k++)

            {
                arr[k].setprocess(" P " + to_string(k), rand() % 1000, rand() % 1000);
            }
            break;
        }
        else if (choice == 1)
        {
            csvfile();
            break;
        }
        else if (choice == 3)
        {
            string name;
            int b, v;

            for (int k = 0; k < NUMPROCESS; k++)

            {
                cout << endl
                     << " ENTER THE PROCESS NAME :";
                cin >> name;
                cout << endl
                     << " ENTER THE BURST TIME : ";
                cin >> b;
                cout << endl
                     << " ENTER THE ARRIVAL TIME : ";
                cin >> v;
                arr[k].setprocess(name, v, b);
            }

            break;
        }
        choice=456;
    }
}

//%%%%%%%%%%%%%%%%%%% SHM WORKING %%%%%%%%%%%%%%%%%%%%%%%
void* createMemory() {
shm_unlink("/my_shared_memory3");
    int fd = shm_open("/my_shared_memory3", O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open");
        exit(1);
    }
    
    if (ftruncate(fd, MAX_BUFFER_SIZE) == -1) {
        perror("ftruncate");
        exit(1);
    }
    
    void *ptr = mmap(0, MAX_BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    
    close(fd);
    
    return ptr;
}

void destroyMemory(void *ptr) {
    if (munmap(ptr, MAX_BUFFER_SIZE) == -1) {
        perror("munmap");
        exit(1);
    }
}

void writeToSharedMemory(void *ptr, const char *message) {
    if (strlen(message) >= MAX_BUFFER_SIZE) {
        cerr << "Message too large for shared memory" << endl;
        return;
    }
    
    strcpy((char*)ptr, message);
}

string readFromSharedMemory(void *ptr) {
    return string((char*)ptr);
}

void printSchedule(process processes[], int numProcesses, void *ptr)
{
    cout << "Process Name | Arrival Time | Burst Time | Start Time | End Time | Waiting Time | Response Time | Turnaround Time" << endl;
    cout << "-----------------------------------------------------------------------------------------------------------------" << endl;
    for (int i = 0; i < numProcesses; i++)
    {
        cout << setw(13) << left << processes[i].getname() << "| ";
        cout << setw(13) << left << processes[i].getArrivalTime() << "| ";
        cout << setw(11) << left << processes[i].getBurstTime() << "| ";
        cout << setw(11) << left << processes[i].starttime << "| ";
        cout << setw(9) << left << processes[i].endtime << "| ";
        cout << setw(13) << left << processes[i].getWaitingTime() << "| ";
        cout << setw(14) << left << processes[i].getResponseTime() << "| ";
        cout << processes[i].getTurnaroundTime() << endl;
    }
    cout << "-----------------------------------------------------------------------------------------------------------------" << endl;

//LOADING MESSAGE TO THE SHM AS A REPORT OF THE EXECUTION
string msg=readFromSharedMemory(ptr);

    if(flag==0)
    {msg+="\nTHE PROCEDURE FIRST COME FIRST SERVE \n";}
    else if(flag==1)
    {msg+="\nTHE PROCEDURE SHORTEST JOB FIRST  \n";}
    else if(flag==2)
    {msg+="\nTHE PROCEDURE IS ROUND ROBIN  \n";}
    else if(flag==3)
    {msg+="\nTHE PROCEDURE IS SHORTEST RUNTIME FIRST  \n";}
    if(!same)
    {
    msg+="\n RANDOM DATA IS USED \n";
    }
    else
    {
    msg+="\n THE DATA FROM THE CSV FILE IS USED \n";
    }

    cout << "Average Turnaround Time = " << fixed << setprecision(1) << calculateturnaroundtime() << endl;
    msg+="Average Turnaround Time :  ";
    int yy= calculateturnaroundtime();
    msg+=to_string(yy);
    msg+="\n";
    cout << "Average Waiting Time = " << fixed << setprecision(1) << calculatewaitingtime() << endl;
    msg+="Average Waiting Time :  ";
    int yy2= calculatewaitingtime();
    msg+=to_string(yy2);
    msg+="\n";
    cout << "Average Response Time = " << fixed << setprecision(1) << calculateResponsetime() << endl;
    msg+="Average Response Time :  ";
    int yy3= calculateResponsetime();
    msg+=to_string(yy3);
    msg+="\n";

const char *msgg3 = msg.c_str();
cnt++;
writeToSharedMemory(ptr, msgg3);
}



int main_flow(void *ptr)
{

    int n1 = 0;
    int choice = 999;
    pthread_t tid[40];

    int n = sizeof(arr) / sizeof(arr[0]);
    n = NUMPROCESS;

    getinput();

    cout << "Choose a scheduling algorithm:" << endl;
    cout << "0. First Come First Serve (FCFS)" << endl;
    cout << "1. Shortest Job First (SJF)" << endl;
    cout << "2. Shortest Remaining Time (SRT)" << endl;
    cout << "3. Round Robin (RR)" << endl;
    cout << "4. Exit" << endl;
    cout << "Enter your choice: ";
    cin >> choice;

    switch (choice)
    {
    case 0:
        // FCFS(&arr[0]);
        pthread_create(&tid[n1], NULL, FCFS, (void *)(&arr[0]));
        pthread_join(tid[n1], NULL);
        n1++;
        break;
    case 1:
        // shortestjobfirst(&arr[0]);
        pthread_create(&tid[n1], NULL, shortestjobfirst, (void *)(&arr[0]));
        pthread_join(tid[n1], NULL);
        n1++;
        break;
    case 2:
        // shortestremainingtime(&arr[0]);
        pthread_create(&tid[n1], NULL, shortestremainingtime, (void *)(&arr[0]));
        pthread_join(tid[n1], NULL);
        n1++;
        break;
    case 3:
        // roundrobin(&arr[0]);
        pthread_create(&tid[n1], NULL, roundrobin, (void *)(&arr[0]));
        pthread_join(tid[n1], NULL);
        n1++;
        break;
    case 4:
        cout << "Ending the program..." << endl;
        break;
    default:
        cout << "Invalid choice. Please enter a number between 0 and 4." << endl;
    }

    if (choice != 4)
    {
        cout << endl
             << "Schedule after scheduling algorithm execution:-" << endl;
        printSchedule(arr, n, ptr);
        cout << endl;
    }

    return 0;
}

int main()
{
        void *sharedMemory = createMemory();
    pthread_t pid,pid2,pid3;
    pid=fork();
    if(pid==0)
    {
        pid2=fork();
        if(pid2==0)
        {
            pid3=fork();
            if(pid3==0)
            {
                main_flow(sharedMemory);
            }
            else
            {
                wait(NULL);
                main_flow(sharedMemory);
               string receivedMessage = readFromSharedMemory(sharedMemory);
          //  cout<<endl << "Received message from shared memory: " <<endl<< receivedMessage <<endl;
            printf("\x1B[93m\nReceived message from shared memory:\033[0m\n");
	    cout << "\x1B[93m" << endl << receivedMessage << "\033[0m" << endl << endl;
            }
        }
        else
        {
                wait(NULL);
            main_flow(sharedMemory);
            string receivedMessage = readFromSharedMemory(sharedMemory);
          //  cout<<endl << "Received message from shared memory: " <<endl<< receivedMessage <<endl;
            printf("\x1B[93m\nReceived message from shared memory:\033[0m\n");
	    cout << "\x1B[93m" << endl << receivedMessage << "\033[0m" << endl << endl;
        }
    }
    else
    {
            wait(NULL);
        main_flow(sharedMemory);
        string receivedMessage = readFromSharedMemory(sharedMemory);
         //   cout<<endl << "Received message from shared memory: " <<endl<< receivedMessage <<endl;
         printf("\x1B[93m\nReceived message from shared memory:\033[0m\n");
	cout << "\x1B[93m" << endl << receivedMessage << "\033[0m" << endl << endl;
    }
    destroyMemory(sharedMemory);

        return 0;
}
