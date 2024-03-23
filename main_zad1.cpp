#include <iostream>
#include <vector>
#include <thread>
#include <cstdlib>
#include <mutex>
#include <time.h>

using namespace std;

const int num_of_readers = 5;
const int num_of_writers = 2;
const int num_of_datas = 5;

int add_randomizer = 0;

int RANDOM_INT(int limit){
    srand(time(nullptr)+add_randomizer);
    return rand() % limit;
}

class Data{
public:
    Data(int nid, int nvalue){
        id = nid;
        value = nvalue;
    };

    int Read(){
        times_read_since_edit += 1;
        return value;
    }

    void StartOverwriting(){
        lock_guard<mutex>lock(DataLock);
        isEdited = true;
        readable = false;
    }

    void FinishOverwriting(int new_value){
        value = new_value;
        times_read_since_edit = 0;
        isEdited = false;
        readable = true;
        DataLock.unlock();
    }

    bool isBeingEdited(){
        return isEdited;
    }
    bool isReadable(){
        return readable;
    }
    bool isEditable(){
        if(isBeingEdited() || times_read_since_edit < 3){
            return false;
        }else{
            return true;
        }
    }
    int getTimesRead(){
        return times_read_since_edit;
    }

    int getValue(){
        return value;
    }
    int getId(){
        return id;
    }
private:
    int id;
    int value;
    int times_read_since_edit = 5;
    bool readable = false;
    bool isEdited = false;
    mutex DataLock;
};

class Reader{
public:
    Reader(int nid){
        id = nid;
    }
    bool Read(Data * data){
        if ( data->isReadable() ){
            int value_read = data->Read();
            this_thread::sleep_for(chrono::seconds(1));
            //cout << "Reader " << id << ": wlasnie przeczytalem dane '" <<value_read<<"'.\n";
            return true;
        }
        return false;
    }
    int getId(){
        return id;
    }
private:
    int id;
};

class Writer{
public:
    Writer(int nid){
        id = nid;
    }
    bool Write(Data *data){
        if (data->isEditable()) {
            data->StartOverwriting();
            this_thread::sleep_for(chrono::seconds(5));

            int new_value = RANDOM_INT(100);
            data->FinishOverwriting(new_value);
            //cout << "Writer " << id << ": wpisalem do danych (" << data.getId() << ") wartosc: " << new_value << ". \n";
        }
    }
    int getId(){
        return id;
    }
private:
    int id;
};

void ReaderCycle(Reader * reader, vector<Data*> datas);
void WriterCycle(Writer * writer, vector<Data*> datas);

int main() {
    vector<Data*>Datas;
    vector<Reader*>Readers;
    vector<Writer*>Writers;

    vector<thread>ReadersThreads;
    vector<thread>WritersThreads;

    for(int i = 0; i < num_of_datas; i++){
        Datas.push_back(new Data(i, RANDOM_INT(100)));
    }

    for(int i = 0; i < num_of_readers; i++){
        Readers.push_back(new Reader(i));
        ReadersThreads.push_back(thread(ReaderCycle, Readers[i], Datas));
    }
    for(int i = 0; i < num_of_writers; i++){
        Writers.push_back(new Writer(i));
        WritersThreads.push_back(thread(WriterCycle, Writers[i], Datas));
    }

    while(true){
        this_thread::sleep_for(chrono::milliseconds(500));
        //system("CLS");
        printf("\n\n%4s %12s %12s %12s %12s\n", "BookId", "content", "times read", "is edited", "is readable");
        for(int i = 0; i < num_of_datas; i++){
            printf("%4d %12d %12d %12d %12d\n", Datas[i]->getId(), Datas[i]->getValue(), Datas[i]->getTimesRead(), Datas[i]->isBeingEdited(), Datas[i]->isReadable());
        }
        add_randomizer %= 500;
    }
    for (auto& rt : ReadersThreads) rt.join();
    for (auto& wt : WritersThreads) wt.join();
    for (auto r : Readers) delete r;
    for (auto w : Writers) delete w;
    return 0;
}

void ReaderCycle(Reader* reader, vector<Data*>datas){
    int index = 0;
    while(true){
        index = RANDOM_INT(num_of_datas);

        if(!datas[index]->isReadable()) continue;

        reader->Read(datas[index]);

        this_thread::sleep_for(chrono::milliseconds (1000));
        add_randomizer += index;
    }
}

void WriterCycle(Writer* writer, vector<Data*>datas){
    int index = 0;
    while(true){
        index = RANDOM_INT(num_of_datas);

        if(!datas[index]->isEditable()) continue;

        writer->Write(datas[index]);

        index++;
        this_thread::sleep_for(chrono::milliseconds (1000));

        add_randomizer += index;
    }
}
