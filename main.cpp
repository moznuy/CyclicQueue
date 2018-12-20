#include <iostream>

#include "Queue.h"

using namespace std;

mutex io;

bool ed = false;
const int sleep_m = 1;
const int sleep_p = 5;
const int sleep_c = 1;

const int ps = 20;
const int cs = 20;

void producer(int i, Queue &q) {
    int k = 0;
    while (!ed) {
        {
            lock_guard<mutex> l(io);
            cout << ">> " << i << " try to push " << k << endl;
        }

        q.push(k);

        {
            lock_guard<mutex> l(io);
            cout << "\t>> " << i << " pushed " << k << endl;
        }
        k++;

        this_thread::sleep_for(chrono::milliseconds(10 * sleep_m * sleep_p));
    }
}

void consumer(int i, Queue &q) {
    int last_k = 0;
    while (!ed) {
        this_thread::sleep_for(chrono::milliseconds(20 * sleep_m * sleep_c));
        {
            lock_guard<mutex> l(io);
            cout << "<< " << i << " try to pull " << endl;
        }

//        if (last_k == 3)
//            last_k = 9;
        Tmp k(q.pull());

        {
            lock_guard<mutex> l(io);
            cout << "\t<< " << i << " pulled " << k << endl;
        }
        last_k = k.get();
    }
}

int main() {
    Queue q(5);

    vector<thread> cons;
    vector<thread> prods;

    for (int i=0; i<cs; i++) {
        cons.emplace_back(consumer, i, ref(q));
    }
    for (int i=0; i<ps; i++) {
        prods.emplace_back(producer, i, ref(q));
    }

    for (auto &t: cons) {
        t.join();
    }
    for (auto &t: prods) {
        t.join();
    }

    return 0;
}