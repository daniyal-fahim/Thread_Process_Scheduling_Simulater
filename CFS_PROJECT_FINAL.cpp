#include <queue>

#include <iomanip>

#include <string>

#include <cstdlib>

#include <iostream>

#include <pthread.h> //THREADING AND MUTEX

#include <sys/types.h> //SHM

#include <sys/wait.h> //WAIT

#include <iostream> //STDINPUT OUTPUT

#include <fstream> //FILE

#include <sstream> // DATA SPLIT PROCESS FROM THE CSV FILE

#include <cstring> //STRING CONVERSION

#include <fcntl.h> //SHM

#include <sys/mman.h> //SHM

#include <sys/stat.h> //SHM

#include <unistd.h>

#define MAX_BUFFER_SIZE 10000

#include <ctime> //FOE CORRECT WORKING OF THE SRAND FUNCTION

using namespace std;

#define NUMTHREADS 5

// Global Variables
int NUMPRO = 10;
bool same = false;
int cnt = 0;

// Forward Decalrations of functions
string readFromSharedMemory(void *ptr);
void writeToSharedMemory(void *ptr, const char *message);
void *createMemory();
void destroyMemory(void *ptr);

// Process Class
class process

{
    string name;

    int arrivalTime;

    int burstTime;

    int dupburstTime;

    int vruntime;

    int nicevalue;

    static int numProcess;

public:
    bool processing;

    bool done;

    int starttime;

    int endtime;

    process()

    {
    }
    void setVrunTime(int k)
    {
        vruntime = k;
    }
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

        nicevalue = 0;

        numProcess++;
    }

    void setprocess(string n, int a, int b, int v, int ni)

    {

        name = n;

        arrivalTime = a;

        burstTime = b;

        done = false;

        processing = false;

        vruntime = v;

        starttime = 999;

        endtime = 999;

        dupburstTime = b;

        nicevalue = ni;
    }

    int getArrivalTime() const

    {

        return arrivalTime;
    }

    void setArrivalTime(int arr)

    {

        arrivalTime = arr;
    }

    int getVrunTime() const

    {

        return vruntime;
    }

    void updateVrunTime(int n)

    {

        vruntime = vruntime + n;
    }

    int getBurstTime() const

    {

        return burstTime;
    }

    int getdupBurstTime() const

    {

        return dupburstTime;
    }

    void setdupBurstTime(int n)

    {

        dupburstTime = n;
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

    // Calculation of VRUNTIME
    void calculateVrunTime(process *p)

    {

        // cout << "Hello this is vruntime" << endl;

        int size = numProcess;

        int nice = p->nicevalue; // storing nice value in int nice
        cout << "\nNICE : " << nice << endl;
        int cpuTime = p->dupburstTime; // storing CPU burst time into int cpuTime

        if (nice == 0)

        {

            p->vruntime = dupburstTime; // Here, if nice value is zero, then the vruntime remains unchanges, this concept is implemented
        }

        else if (nice < 0)

        {

            p->vruntime = cpuTime + (cpuTime / 2); // Here, if nice value is -ve, then the vruntime increases, this concept is implemented
        }

        else if (nice > 0)

        {

            p->vruntime = cpuTime / 2; // Here, if nice value is +ve, then the vruntime decreases, this concept is implemented
        }

        if (p->vruntime <= 0)
        {
            if (p->vruntime == 0)
            {
                p->vruntime = 1; // Here if pruntime becomes zero, then it is set to 1
            }
            else
            {
                p->vruntime = -1 * p->vruntime; // Here if pruntime becomes -ve, then it is set to +ve
            }
        }
    }

    void printSchedule() const

    {

        cout << setw(10) << left << name << "    | ";

        cout << setw(10) << left << vruntime << "    | ";

        cout << setw(13) << left << arrivalTime << " | ";

        cout << setw(11) << left << burstTime << " | ";

        cout << setw(10) << left << starttime << " | ";

        cout << setw(9) << left << endtime << " | " << endl;
    }

    int niceUpdate(int newCpuTime) // Nice Value is updated here
    {
        int oldCpuTime = dupburstTime;
        int nice = nicevalue;

        if (newCpuTime > oldCpuTime) // If CPU burst time becomes greater than previous burst time, then nice value is increased by a random value
        {
            nice = (nice + (rand() % 20)) % 20; // here we have maked sure that nice value remains in its range i.e. -19 to +20
        }
        else
        {
            nice = (nice - (rand() % 20));
            if (nice < -19)
            {
                nice = -19; // here we have maked sure that nice value remains in its range i.e. -19 to +20
            }
        }

        return nice;
    }

    void cpuUpdate(int burst, process *p) // Here, CPU burst time is updated
                                          // The process of updating nice value and calculating vruntime also starts in this function

    {

        p->nicevalue = niceUpdate(burst);

        p->setdupBurstTime(burst);

        calculateVrunTime(p);
    }

    int getResponseTime() const
    {
        return (starttime - arrivalTime);
    }

    //       int getTurnaroundTime() const
    //    {
    //        return (endtime - arrivalTime);
    //    }

    int getWaitingTime() const
    {
        return (getTurnaroundTime() - burstTime);
    }
};

process arr[1000]; // there are 1000 processes we are using right now

int process::numProcess = 0;

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

//********************* multi threading task parallelism *******************

// Function executed by each thread to calculate sum of turnaround time
pthread_mutex_t turn1  ;
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
        pthread_mutex_lock(&turn1);  //ADDING MUTEX LOCK JUST TO ENSURE THAT NO VALUE IS LEFT BEHIND DURING THE PROCESS
        sum += arr[i].getTurnaroundTime(); // Corrected access to process object
        pthread_mutex_unlock(&turn1);
    }
    int *result = new int(sum); // Dynamically allocate memory for result
    pthread_exit((void *)result);
}

// Function to calculate average turnaround time using multithreading
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

    // Join threads and accumulate results

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
    return sum1 / NUMPRO;
}
// %%%%%%%%%%%%%%%%%%%%5 RESPONSE TIME %%%%%%%%%%%%%%%%

// Function executed by each thread to calculate sum of response time
pthread_mutex_t res1  ;
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
        pthread_mutex_lock(&res1);  //ADDING MUTEX LOCK JUST TO ENSURE THAT NO VALUE IS LEFT BEHIND DURING THE PROCESS
        sum += arr[i].getResponseTime(); // Corrected access to process object
        pthread_mutex_unlock(&res1);
    }
    int *result = new int(sum); // Dynamically allocate memory for result
    pthread_exit((void *)result);
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
    return sum1 / NUMPRO;
}
// %%%%%%%%%%%%%%%%%%%%5 WAITING TIME %%%%%%%%%%%%%%%%

// Function executed by each thread to calculate sum of wait time
pthread_mutex_t wait1  ;
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
        pthread_mutex_lock(&wait1);  //ADDING MUTEX LOCK JUST TO ENSURE THAT NO VALUE IS LEFT BEHIND DURING THE PROCESS
        sum += arr[i].getWaitingTime(); // Corrected access to process object
         pthread_mutex_unlock(&wait1);
    }
   
    int *result = new int(sum); // Dynamically allocate memory for result
    pthread_exit((void *)result);
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
            int value;
                sum1 += *result;
            delete result;
        }
        else
        {
            // Handle error or exception
            cout << "Error: Thread " << i << " failed to return a result." << endl;
        }
    }
    return sum1 / NUMPRO;
}

void printScheduleend(process processes[], int numProcesses, void *ptr)
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

    string msg = readFromSharedMemory(ptr); // The variable msg reads content already present in shared memory

    if (!same)
    {
        msg += "\n RANDOM DATA IS USED \n";
    }
    else
    {
        msg += "\n THE DATA FROM CSV FILE IS USED \n";
    }
     msg += "\n  NUMBER OF PROCESS USED FOR CFS ::  ";
     msg+=to_string(NUMPRO);
      msg += "\n";
    cout << "Average Turnaround Time = " << fixed << setprecision(1) << calculateturnaroundtime() << endl;
    msg += "Average Turnaround Time :  ";
    int yy = calculateturnaroundtime();
    msg += to_string(yy); // concatenation into msg(previus content of shm)
    msg += "\n";
    cout << "Average Waiting Time = " << fixed << setprecision(1) << calculatewaitingtime() << endl;
    msg += "Average Waiting Time :  ";
    int yy2 = calculatewaitingtime();
    msg += to_string(yy2);
    msg += "\n";
    cout << "Average Response Time = " << fixed << setprecision(1) << calculateResponsetime() << endl;
    msg += "Average Response Time :  ";
    int yy3 = calculateResponsetime();
    msg += to_string(yy3);
    msg += "\n";
    msg += "\n";
    const char *msgg3 = msg.c_str();

    cnt++;

    writeToSharedMemory(ptr, msgg3); // write again the updated content into shared memory
}

class node // class for node of binary tree

{

public:
    node *left;

    node *right;

    process *p;

    node()

    {

        left = NULL;

        right = NULL;

        p = NULL;
    }

    node(process *d)

    {

        left = NULL;

        right = NULL;

        p = d;
    }
};

// GLOBAL VARIABLES

node *head;
int currtime;
int l;
// const int NUMTHREADS = 10;
// const int NUMPRO = 80;

// FORWARD DECLARATIONS OF PROCESSES AND CLASSES

bool BINARY_TREE(node *&head, process *p);

bool updatenode(node *&head, process *p);

void inorder(node *head);

node *deleteNode(node *head, int k, int arrival);

node *smallest(node *h);

node *CFQ_WORKING(node *head);

void *treeinit(void *arg);

void csvfile();

void split(const string &s, char delimiter, string tokens[]);

int main_flow(void *ptr) // This function controls Main flow of our application

{
    currtime = 0;
    int choice;
    srand(static_cast<unsigned int>(time(nullptr)));

    cout << endl
         << "PLEASE ENTER ON HOW MANY PROCESSES DO YOU WANT TO WORK\n";
    int temp;
    cin >> temp;
    NUMPRO = temp;
    cout << endl
         << "PLEASE CHOOSE WHICH INPUT METHOD YOU WANT \n1) CSV FILE READING \n2) RANDOM INPUT \n3) USER INPUT\n";
    cin >> choice;

    while (1)
    { 
         if (choice==456)
         {
            cout << endl
         << "PLEASE CHOOSE CORRECT INPUT METHOD YOU WANT \n1) CSV FILE READING \n2) RANDOM INPUT \n3) USER INPUT\n";
            cin >> choice;
         }
        if (choice == 2)
        {
            for (int k = 0; k < NUMPRO; k++)

            {
                arr[k].setprocess(" P " + to_string(k), 0, rand() % 100, (rand() % 20) + 1, 0);
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

            for (int k = 0; k < NUMPRO; k++)

            {
                cout << endl
                     << " PLEASE ENTER DETAILS OF PROCESS NUM : " << k;
                cout << endl
                     << "ENTER THE PROCESS NAME : ";
                cin >> name;
                cout << endl
                     << "ENTER THE BURST TIME : ";
                cin >> b;
                cout << endl
                     << "ENTER THE VIRTUAL RUN TIME FOR THE FIRST TIME : ";
                cin >> v;

                arr[k].setprocess(name, 0, b, v, 0);
            }

            break;
        }
        choice=456;
    }

    cout << "Process Name | VRUN TIME Time | Arrival Time | Burst Time | Start Time | End Time |" << endl;
    cout << "---------------------------------------------------------------" << endl;

    for (int i = 0; i < NUMPRO; i++)

    {

        arr[i].printSchedule();
    }

    head = new node(&arr[0]);

    pthread_t tid[NUMTHREADS];

    int thread_ids[NUMTHREADS];

    for (int i = 0; i < NUMTHREADS; i++)

    {

        thread_ids[i] = i;

        pthread_create(&tid[i], NULL, treeinit, (void *)&thread_ids[i]);
    }

    for (int i = 0; i < NUMTHREADS; i++)

    {

        pthread_join(tid[i], NULL);
    }

    cout << endl

         << endl;

    cout << "Process Name | VRUN TIME Time | Arrival Time | Burst Time | Start Time | End Time |" << endl;

    cout << "---------------------------------------------------------------" << endl;

    inorder(head);

    currtime = 0;

    while (head != NULL)

    {

        head = CFQ_WORKING(head);
        inorder(head);
    }

    cout << "Process Name | VRUN TIME Time | Arrival Time | Burst Time | Start Time | End Time |" << endl;

    cout << "---------------------------------------------------------------" << endl;
    for (int i = 0; i < NUMPRO; i++)

    {

        arr[i].printSchedule();
    }

    printScheduleend(arr, NUMPRO, ptr);
    return 0;
}

bool BINARY_TREE(node *&head, process *p);

void csvfile() // the data in csv file has been generated syntheticly using randbetween function
{
    same = true;
    ifstream file("data.csv");
    if (!file.is_open())
    {
        cerr << "Error opening file!" << endl;
    }

    string cells[NUMPRO][2];
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
        if (numRows == NUMPRO)
        {
            break;
        }
    }

    file.close();

    for (int k = 0; k < NUMPRO; k++)

    {
        arr[k].setprocess(" P " + to_string(k), 0, atoi(cells[k][0].c_str()), atoi(cells[k][1].c_str()), 0);
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

bool updatenode(node *&head, process *p) // Function to handle same vrntime problem
{
    int lp = p->getVrunTime();

    lp += rand() % 100;

    p->setVrunTime(lp);

    return BINARY_TREE(head, p);
}
bool BINARY_TREE(node *&head, process *p)

{

    node *tmp = head;

    node *n = new node(p);

    int val = p->getVrunTime();

    if (tmp == NULL)

    {

        return true;
    }

    while (tmp != NULL)

    {

        int he = tmp->p->getVrunTime();

        if (he == val)

        {
            cout << endl
                 << endl
                 << "REPEATED VRUNTIME PROBLEM HANDLED SUCCESSFULLY" << endl;
            return false;
        }

        else if (tmp->p->getVrunTime() > n->p->getVrunTime())

        {

            if (tmp->left != NULL)

            {

                tmp = tmp->left;
            }

            else

            {

                tmp->left = n;

                return true;
            }
        }

        else if (tmp->p->getVrunTime() < n->p->getVrunTime())

        {

            if (tmp->right != NULL)

            {

                tmp = tmp->right;
            }

            else

            {

                tmp->right = n;

                return true;
            }
        }
    }
       return false;
}

void inorder(node *head)

{

    if (head == NULL)

    {

        return;
    }

    inorder(head->left);

    head->p->printSchedule();

    inorder(head->right);
}

node *deleteNode(node *head, int k, int arrival)

{

    if (head == NULL)

        return head;

    if (head->p->getVrunTime() > k)

    {

        head->left = deleteNode(head->left, k, arrival);

        return head;
    }

    else if (head->p->getVrunTime() < k)

    {

        head->right = deleteNode(head->right, k, arrival);

        return head;
    }
    if (head->p->getArrivalTime() == arrival)
    {
        node *temp = NULL;

        if (head->left == NULL)

        {

            temp = head->right;

            delete head;

            return temp;
        }

        else if (head->right == NULL)

        {

            temp = head->left;

            delete head;

            return temp;
        }

        else

        {

            node *parent = head;

            node *child = head->right;

            while (child->left != NULL)

            {

                parent = child;

                child = child->left;
            }

            if (parent != head)

                parent->left = child->right;

            else

                parent->right = child->right;

            head->p = child->p;

            delete child;
        }
        return head;
    }
    return head;
}

node *smallest(node *h)

{

    node *head = h;

    while (h->left != NULL)

    {

        h = h->left;
    }

    return h;
}

node *CFQ_WORKING(node *head)

{

    node *proc = smallest(head);

    node *newp = new node(proc->p);

    if (proc->p->getdupBurstTime() >= 0 && !proc->p->done)

    {
        if (!proc->p->processing)
        {
            proc->p->starttime = currtime;
            proc->p->processing = true;
        }

        cout << "PROCESS IS RUNNING" << proc->p->getname() << endl;

        head = deleteNode(head, proc->p->getVrunTime(), proc->p->getArrivalTime());

        int newBurst = newp->p->getdupBurstTime() - newp->p->getVrunTime();

        newp->p->setdupBurstTime(newBurst);

        currtime = currtime + newp->p->getVrunTime();

        newp->p->cpuUpdate(newBurst, newp->p);

        if (newBurst <= 0)

        {

            newp->p->done = true;
        }

        if (!BINARY_TREE(head, (newp->p)))
        {
            // updatenode(head, (newp->p));
            if (!updatenode(head, (newp->p)))
            {
                while (!updatenode(head, (newp->p)))
                    ;
            }
        }
        if (head == NULL) // it is only for the last element of array
        {
            currtime += newp->p->getdupBurstTime();

            proc->p->endtime = currtime;

            cout << endl

                 << endl

                 << endl

                 << "PROCESS HAS BEEN ENDED NAME:" << proc->p->getname() << " ON TIME : " << currtime << endl

                 << endl

                 << endl;

            head = deleteNode(head, proc->p->getVrunTime(), proc->p->getArrivalTime());
        }
    }

    else

    {
        proc->p->endtime = currtime;
        cout << endl

             << endl

             << endl

             << "PROCESS HAS BEEN ENDED NAME:" << proc->p->getname() << " ON TIME : " << currtime << endl

             << endl

             << endl;

        head = deleteNode(head, proc->p->getVrunTime(), proc->p->getArrivalTime());
    }

    return head;
}

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // solving critical section problem by mutex

void *treeinit(void *arg)

{

    int x = NUMPRO;

    int tid = *((int *)arg);

    int start = tid * (x / NUMTHREADS);

    int end = start + (x / NUMTHREADS);

    if (tid == NUMTHREADS - 1)

    {

        end = x;
    }

    for (int i = start; i < end; i++)

    {

        pthread_mutex_lock(&mutex);
        if (start == 0)
        {
            start = 1;
        }
        if (!BINARY_TREE(head, &arr[i]))
        {

            if (!updatenode(head, &arr[i]))
            {

                updatenode(head, &arr[i]);
            }
        }
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(0);
}

/// IPC WORKING AND MAIN MAIN()
void *createMemory()
{
    shm_unlink("/my_shared_memory3");
    int fd = shm_open("/my_shared_memory3", O_CREAT | O_RDWR, 0666);
    if (fd == -1)
    {
        perror("shm_open");
        exit(1);
    }

    if (ftruncate(fd, MAX_BUFFER_SIZE) == -1)
    {
        perror("ftruncate");
        exit(1);
    }

    void *ptr = mmap(0, MAX_BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }

    close(fd);

    return ptr;
}

void destroyMemory(void *ptr)
{
    if (munmap(ptr, MAX_BUFFER_SIZE) == -1)
    {
        perror("munmap");
        exit(1);
    }
}

void writeToSharedMemory(void *ptr, const char *message)
{
    if (strlen(message) >= MAX_BUFFER_SIZE)
    {
        cerr << "Message too large for shared memory" << endl;
        return;
    }

    strcpy((char *)ptr, message);
}

string readFromSharedMemory(void *ptr)
{
    return string((char *)ptr);
}

int main()
{
    void *sharedMemory = createMemory();
    pthread_t pid, pid2, pid3;
    pid = fork();
    if (pid == 0)
    {
        pid2 = fork();
        if (pid2 == 0)
        {
            pid3 = fork();
            if (pid3 == 0)
            {
                main_flow(sharedMemory);
            }
            else
            {
                wait(NULL);
                main_flow(sharedMemory);
                string receivedMessage = readFromSharedMemory(sharedMemory);
                printf("\x1B[94m\nReceived message from shared memory:\033[0m\n");
                cout << "\x1B[94m" << endl
                     << receivedMessage << "\033[0m" << endl
                     << endl;
            }
        }
        else
        {
            wait(NULL);
            main_flow(sharedMemory);
            string receivedMessage = readFromSharedMemory(sharedMemory);
            printf("\x1B[93m\nReceived message from shared memory:\033[0m\n");
            cout << "\x1B[93m" << endl
                 << receivedMessage << "\033[0m" << endl
                 << endl;
        }
    }
    else
    {
        wait(NULL);
        main_flow(sharedMemory);
        string receivedMessage = readFromSharedMemory(sharedMemory);
        printf("\x1B[93m\nReceived message from shared memory:\033[0m\n");
        cout << "\x1B[93m" << endl
             << receivedMessage << "\033[0m" << endl
             << endl;
    }
    destroyMemory(sharedMemory);

    return 0;
}