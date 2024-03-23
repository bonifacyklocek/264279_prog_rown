#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <vector>
#include <string>
#include <ctime>
#include <cstdlib>

using namespace std;

const int num_of_phil = 5;
const int max_hunger = 10;
const int min_hunger = 0;

int add_randomizer = 0;

int RANDOM_INT(int limit){
    srand(time(nullptr)+add_randomizer);
    return rand() % limit;
}

enum PhilosopherState {THINKING, EATING, DEAD};

class Philosopher{
public:
    Philosopher(int nid){
        id = nid;
        state = THINKING;
        hunger = min_hunger;
    }
    int getId() {
        return id;
    }
    PhilosopherState getState(){
        return state;
    }
    int getHunger() {
        return hunger;
    }
    void think(){
        state = THINKING;
    }

    void eat(){
        state = EATING;
    }
    void increaseHunger(){
        if (hunger < max_hunger){
            hunger++;
        }else{
            state = DEAD;
        }
    }
    void decreaseHunger(){
        if (hunger > min_hunger){
            hunger--;
        }
    }
    bool isDead(){
        return state == DEAD;
    }
private:
    int id;
    PhilosopherState state;
    int hunger;
};

void philosopherThread(Philosopher *philosopher, mutex *forks);
void displayStatus(vector<Philosopher*>&philosophers);
string stateToString(PhilosopherState state);

int main() {
    vector<Philosopher*>philosophers;
    mutex forks[num_of_phil];
    for(int i = 0; i < num_of_phil; i++){
        philosophers.push_back(new Philosopher(i));
    }
    vector<thread>philosopherThreads;
    for(int i = 0; i < num_of_phil; i++){
        philosopherThreads.push_back(thread(philosopherThread, philosophers[i], forks));
    }

    while(any_of(philosophers.begin(), philosophers.end(), [](Philosopher *p){return !p->isDead(); })){
        displayStatus(philosophers);
        this_thread::sleep_for(chrono::seconds(2));
        add_randomizer %= 500;
    }
    for (auto& t : philosopherThreads){
        t.join();
    }
    for(auto p : philosophers){
        delete p;
    }

    return 0;
}

void philosopherThread(Philosopher * philosopher, mutex* forks){
    while(!philosopher->isDead()){
        philosopher->think();
        this_thread::sleep_for(chrono::seconds(RANDOM_INT(3) + 1));
        philosopher->increaseHunger();
        if(philosopher->isDead()){
            break;
        }

        unique_lock<mutex>left_fork_lock(forks[philosopher->getId()]);
        unique_lock<mutex>right_fork_lock(forks[(philosopher->getId() + 1) % num_of_phil]);

        philosopher->eat();
        this_thread::sleep_for(chrono::seconds(RANDOM_INT(3) + 1));
        philosopher->decreaseHunger();
        add_randomizer += philosopher->getId();
    }
}

void displayStatus(vector<Philosopher*>&philosophers){
    system("CLS");
    cout << ""<<endl;
    printf("\n%14s %8s %8s\n", "PhilosopherId", "State", "Hunger");

    for(auto p : philosophers){
        string state = stateToString(p->getState());
        printf("%14d %10s %8d\n", p->getId(), state.c_str(), p->getHunger());
    }
}

string stateToString(PhilosopherState state){
    switch(state){
        case THINKING:
            return "Thinking";
        case EATING:
            return "Eating";
        case DEAD:
            return "Dead";
        default:
            return "State unknown";
    }
}

