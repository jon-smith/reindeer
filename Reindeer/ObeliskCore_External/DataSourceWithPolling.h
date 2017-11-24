#pragma once

#include <vector>

#include "DataSourceBase.h"
#include "MutexedObject.h"
#include "DataPollerT.h"

namespace obelisk
{
  // Data source that updates data at regular intervals
  // It takes a policy class that contains the public function
  // void updateData(std::vector<std::tuple<VariableTypes...>> &);
  template <typename DataGetPolicy, size_t PollingTimeMilliseconds, typename...VariableTypes>
  class DataSourceWithPolling : public IDataSource<VariableTypes...>    
  {
    using PollingBaseT = DataPoller<std::vector<std::tuple<VariableTypes...>>>;

  public:

    using DataPointT = std::tuple<VariableTypes...>;

    template <typename ...Types>
    DataSourceWithPolling(Types&&...types) :
      policy(std::forward<Types>(types)...),
      dataPoller(nullptr,
        std::bind(&DataSourceWithPolling::inLoopFunc, this, std::placeholders::_1),
        std::chrono::milliseconds(PollingTimeMilliseconds))
    {
      dataPoller.startPolling();
    }

    ~DataSourceWithPolling()
    {

    }

    void clear() override final
    {
      dataPoller.modifyData([this](std::vector<DataPointT> &data)
      {
        data.clear();
      });
    }

  private:

    void lockedAccessData(std::function<void(const std::vector<DataPointT>&)> fn) const override
    {
      dataPoller.accessData(fn);
    }

    void inLoopFunc(std::vector<DataPointT> &data)
    {
      policy.updateData(data); 
      IDataSource<VariableTypes...>::onDataUpdate.asyncInvokeHandlers();
    }
    
    DataGetPolicy policy;
    DataPoller<std::vector<std::tuple<VariableTypes...>>> dataPoller;
  };
}