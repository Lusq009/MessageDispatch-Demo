#include <iostream>
#include <string>
#include <chrono>
#include <random>
#include "se_dataprocessor.hpp"

struct DataA
{
    int data;
    virtual ~DataA(){
        std::cout << "DataA destroied!" << std::endl;
    }
};

struct DataB
{
    float data;
    virtual ~DataB(){
        std::cout << "DataB destroied!" << std::endl;
    }
};

class DataProducer : public SEDataModule, public SEDataSource<DataA>
{
private:
    /* data */
protected:
    virtual void process() override {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::shared_ptr<DataA> data(new DataA);
        data->data = rand();
        dispatch(data);
    }
public:
    DataProducer () = default;
    virtual ~DataProducer () = default;
};

class DataConsumerA : public SEConsumerModule<DataA>
{
private:
    /* data */
protected:
    void process(std::shared_ptr<DataA> &data) override {
        std::cout << "Got data from DataConsumerA: " << data->data << std::endl;
    }
public:
    DataConsumerA(/* args */) = default;
    virtual ~DataConsumerA() = default;
};
class DataConsumerB : public SEConsumerModule<DataA>, public SEDataSource<DataB>
{
private:
    /* data */
protected:
    void process(std::shared_ptr<DataA> &data) override {
        std::cout << "Got data from DataConsumerB: " << data->data << std::endl;
        std::shared_ptr<DataB> new_data(new DataB);
        new_data->data = (float)data->data;
        new_data->data *= 0.53f;
        dispatch(new_data);
    }

public:
    DataConsumerB(/* args */) = default;
    virtual ~DataConsumerB() = default;
};

class DataConsumerC : public SEConsumerModule<DataA>, public SEDataProcessor<DataB>
{
private:
    /* data */
protected:
    void process(std::shared_ptr<DataA> &data){
        std::cout << "In thread : " << std::this_thread::get_id() << " Got data A from DataConsumerC: " << data->data << std::endl;
    }
    void onDataAvailable(std::shared_ptr<DataB> &data) override {
        std::cout << "In thread : " << std::this_thread::get_id() << " Got data B from DataConsumerC: " << data->data << std::endl;
    }
public:
    DataConsumerC(/* args */) = default;
    virtual ~DataConsumerC() = default;
};

int main(int argc, char *argv[])
{
    DataProducer producer;
    DataConsumerA consumerA;
    DataConsumerB consumerB;
    DataConsumerC consumerC;
    producer.addProcessor(&consumerA);
    producer.addProcessor(&consumerB);
    producer.addProcessor(&consumerC);
    consumerB.addProcessor(&consumerC);

    producer.start();
    consumerA.start();
    consumerB.start();
    consumerC.start();

    std::this_thread::sleep_for(std::chrono::seconds(10));

    producer.stop();
    std::cout << "producer stoped!" << std::endl;
    consumerA.stop();
    std::cout << "consumerA stoped!" << std::endl;
    consumerB.stop();
    std::cout << "consumerB stoped!" << std::endl;
    consumerC.stop();
    std::cout << "consumerC stoped!" << std::endl;

    return 0;
}
