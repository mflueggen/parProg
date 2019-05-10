#include <iostream>
#include <pthread.h>

class MyThread {
public:
    MyThread(void *args){
        pthread_create(&_encapsulated_thread, nullptr, this->helloWorld, args);
    }

    ~MyThread(){
        pthread_join(_encapsulated_thread, nullptr);
    }

    static void *helloWorld(void *args) {
        std::cout << "Hello from Thread\n" << std::endl;
        return nullptr;
    }

private:
    pthread_t _encapsulated_thread;
};


int main() {
    {
        MyThread t(nullptr);

        //here we have an implicit join
    }
    std::cout << "Hello From Main Thread\n" << std::endl;
    return 0;
}