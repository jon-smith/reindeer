#pragma once

#include <atomic>

#include "StdThreadSupportWrappers.h"
#include "SimpleAsyncTask.h"

namespace obelisk
{
  template <typename DataT>
  class DataPoller
  {
  public:

    template <typename FnInit, typename FnLoop, typename ...Args>
    DataPoller(FnInit&& initFunc, FnLoop&& inLoopFunc,
      std::chrono::milliseconds rate_ms, Args&&...dataConstructArgs) :
      data(std::forward<Args>(dataConstructArgs)...),
      initFunc(initFunc),
      inLoopFunc(inLoopFunc),
      pollingRate(std::chrono::milliseconds(rate_ms))
    {
      if (!this->inLoopFunc)
        throw std::runtime_error("Null inLoopFunc supplied to DataPoller");

      pollingThread = SimpleAsyncTask([this]()
      {
        while (!killThread)
        {
          UniqueLock<Mutex> lk(m);
          // Only call inLoopFunc if we manually notified the condition variable or polling is active
          if (singleUpdateNotified || (pollingCV.waitFor(lk, pollingRate.load()), doPolling))
          {
            if (killThread)
              return;

            singleUpdateNotified = false;
            lk.unlock();

            if (killThread)
              return;

            if (!hasInitialised)
            {
              if (this->initFunc)
                data.lockedModify(this->initFunc);
              hasInitialised = true;
            }

            if (killThread)
              return;

            data.lockedModify(this->inLoopFunc);
          }
        }
      });
    }    

    void startPolling()
    {
      LockGuard lk(m);
      doPolling = true;
      pollingCV.notifyOne();
    }

    void stopPolling(bool allowFinalUpdate = true)
    {
      LockGuard lk(m);
      doPolling = false;
      if (allowFinalUpdate)
      {
        singleUpdateNotified = true;
        pollingCV.notifyOne();
      }      
    }

    void setPollingRate(std::chrono::milliseconds rate)
    {
      pollingRate.store(rate);
    }

    void requestImmediateUpdate()
    {
      LockGuard lk(m);
      singleUpdateNotified = true;
      pollingCV.notifyOne();
    }    

    DataT getData() const
    {
      return data.get();
    }

    void accessData(const std::function<void(const DataT&)> &accessFunc) const
    {
      data.lockedAccess(accessFunc);
    }

    void modifyData(const std::function<void(DataT&)> &modFunc)
    {
      data.lockedModify(modFunc);
    }

    ~DataPoller()
    {
      cleanUpPollingThread();
    }

  protected:
    
    void cleanUpPollingThread()
    {
      stopPolling(false);
      killThread = true;

      lockAndCall(m, [this]() {
        singleUpdateNotified = true;
        pollingCV.notifyOne();
      });

      pollingThread.wait();
    }
    
    MutexedObject<DataT> data;

  private:

    const std::function<void(DataT &)> initFunc;
    const std::function<void(DataT &)> inLoopFunc;

    Mutex m;
    bool hasInitialised = false;
    bool killThread = false;
    bool singleUpdateNotified = false;
    bool doPolling = false;
    std::atomic<std::chrono::milliseconds> pollingRate;
    ConditionVariable pollingCV;

    // pollingThread must be last in the class definition, as it's initialized last
    SimpleAsyncTask pollingThread;
  };  
}
