#pragma once

#include <exception>
#include <string>

namespace RFIT_NS::utils {
class RFITException : public std::exception
{
  public:
    explicit RFITException(std::string message)
      : _message(std::move(message))
    {}

    [[nodiscard]] const char* what() const noexcept override
    {
        return _message.c_str();
    }

  protected:
    std::string _message;
};
}
