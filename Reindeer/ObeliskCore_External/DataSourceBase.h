#pragma once

#include <tuple>
#include <vector>
#include <functional>

#include "EventObserver.h"
#include "AsyncEvent.h"

namespace obelisk
{
  // Generic data source interface
  // Override lockedAccessData and clear in derived classes to use
  // Derived classes should call onDataUpdate.asyncInvokeHandlers() when data is updated
  // Watchers can subscribe to onDataUpdate to be notified when data is updated
  template <typename...VariableTypes>
  class IDataSource : SubjectWithEvents
  {
  public:

    AsyncEvent<> onDataUpdate;

    // Use a tuple to contain the variables for each datapoint
    using DataPointT = std::tuple<VariableTypes...>;

    IDataSource() : onDataUpdate(*this, L"Data source data update") {}

    virtual ~IDataSource() = default;

    // Get a vector of all of the data
    std::vector<DataPointT> getAllData() const
    {
      std::vector<DataPointT> data;

      lockedAccessData([&data](const std::vector<DataPointT> &d)
      {
        data = d;
      });

      return data;
    }

    // Use the variable index to obtain a vector of that variable
    template <size_t TupleIdx, typename T = std::decay_t<decltype(std::get<TupleIdx>(DataPointT()))>>
    std::vector<T> getSelectedVariable() const
    {
      std::vector<T> selectedVariableData;

      lockedAccessData([&selectedVariableData](const std::vector<DataPointT> &data)
      {
        selectedVariableData.reserve(data.size());
        for (auto const &d : data)
          selectedVariableData.push_back(std::get<TupleIdx>(d));
      });

      return selectedVariableData;
    }

    virtual void clear() = 0;

  private:
    virtual void lockedAccessData(std::function<void(const std::vector<DataPointT>&)> fn) const = 0;
  };
}