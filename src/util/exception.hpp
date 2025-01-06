#pragma once

class APIException : public std::exception
{
public:
  explicit APIException( const std::string& msg ) : message(msg) {}

  _NODISCARD inline const char* what() const override
  {
    return message.data();
  }

private:
  std::string message;
};

class InjectorException : public std::exception
{
public:
  explicit InjectorException( const std::string& msg ) : message( msg ) {}

  _NODISCARD inline const char* what() const override
  {
    return message.data();
  }

private:
  std::string message;
};