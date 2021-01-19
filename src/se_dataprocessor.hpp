#ifndef SE_DATASOURCEPROCESSOR_H
#define SE_DATASOURCEPROCESSOR_H
#include <vector>
#include <memory>
#include <cassert>
#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>

template <typename T>
class SEDataProcessor
{
public:
    virtual void onDataAvailable(std::shared_ptr<T>& obs) = 0;
    virtual ~SEDataProcessor() = default;
};

template <typename T>
class SEDataSource
{
    inline void spinLock(){
        while (mBusy.test_and_set(std::memory_order_acquire))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    inline void spinUnlock(){
        mBusy.clear(std::memory_order_release);
    }
public:
    inline void addProcessor(SEDataProcessor<T> *processor) {
        spinLock();
        for(auto it = mProcessors.begin(); it != mProcessors.end(); it++){
            if(*it == processor){
                assert(false);
                return;
            }
        }
        mProcessors.push_back(processor);
        spinUnlock();
    }
    inline void removeProcessor(SEDataProcessor<T> *processor) {
        spinLock();
        for(auto it = mProcessors.begin(); it != mProcessors.end(); it++){
            if(*it == processor){
                mProcessors.erase(it);
                return;
            }
        }
        assert(false);
    }
    inline void dispatch(std::shared_ptr<T>& data){
        if(mBusy.test_and_set(std::memory_order_acquire))return;
        for(auto processor : mProcessors){
            processor->onDataAvailable(data);
        }
        spinUnlock();
    }
    virtual ~SEDataSource() = default;
private:
    std::vector<SEDataProcessor<T> *> mProcessors;
    std::atomic_flag mBusy = ATOMIC_FLAG_INIT;
};

class SEDataModule
{
private:
    std::thread *mThread;
    bool mRunning;
protected:
    virtual void process() = 0;
public:
    SEDataModule() : mThread(nullptr), mRunning(false){}
    virtual ~SEDataModule(){
        stop();
    };
    virtual void init(){}
    virtual void start(){
        if(!mRunning && !mThread){
            mThread = new std::thread([this](){
                mRunning = true;
                while (mRunning)
                {
                    process();
                }
            });
        }
    }
    virtual void stop(){
        if(mRunning && mThread){
            mRunning = false;
            mThread->join();
            delete mThread;
            mThread = nullptr;
        }
    }
};

template <typename T>
class SEConsumerModule : public SEDataModule, public SEDataProcessor<T>
{
private:
    std::vector<std::shared_ptr<T>> mDataBuf;
    std::mutex mDataLock;
    std::condition_variable mCondition;
protected:
    virtual void process() override{
        std::shared_ptr<T> data;
        {
            std::unique_lock<std::mutex> lk(mDataLock);
            mCondition.wait_for(lk, std::chrono::seconds(3), [this](){return !mDataBuf.empty();});
            if(!mDataBuf.empty()){
                data = mDataBuf.front();
                mDataBuf.erase(mDataBuf.begin());
            }
        }
        if(data)process(data);
    }
public:
    SEConsumerModule() = default;
    virtual ~SEConsumerModule() = default;
    virtual void process(std::shared_ptr<T>& data) = 0;
    virtual void onDataAvailable(std::shared_ptr<T>& data) override{
        if(data){
            mDataLock.lock();
            mDataBuf.push_back(data);
            mDataLock.unlock();
            mCondition.notify_all();
        }
    }
};

#endif //SE_DATASOURCEPROCESSOR_H