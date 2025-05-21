#ifndef DARABONBA_EXCEPTIONS_HPP
#define DARABONBA_EXCEPTIONS_HPP

#include <string>
#include <exception>
#include <memory>

// TeaException Base Class
class TeaException : public std::exception {
public:
    explicit TeaException(const std::string &message);
    virtual const char *what() const noexcept override;
    
protected:
    std::string message_;
};

// DaraException Class
class DaraException : public TeaException {
public:
    DaraException(const std::string &code, const std::string &message, const std::string &description = "", const std::string &accessDeniedDetail = "");
    
    const char *what() const noexcept override;
    
private:
    std::string code_;
    std::string description_;
    std::string accessDeniedDetail_;
};

// ResponseException Class
class ResponseException : public DaraException {
public:
    ResponseException(const std::string &code, const std::string &message, int statusCode = 0, int retryAfter = 0, const std::string &description = "", const std::string &accessDeniedDetail = "");

private:
    int statusCode_;
    int retryAfter_;
};

// ValidateException Class
class ValidateException : public std::exception {};

// RequiredArgumentException Class
class RequiredArgumentException : public TeaException {
public:
    explicit RequiredArgumentException(const std::string &arg);

    const char *what() const noexcept override;
    
private:
    std::string arg_;
};

// RetryError Class
class RetryError : public std::exception {
public:
    explicit RetryError(const std::string &message);

    const char *what() const noexcept override;
    
private:
    std::string message_;
};

// UnretryableException Class
class UnretryableException : public TeaException {
public:
    UnretryableException(const std::string &message, const std::shared_ptr<TeaException> &internalEx);

    const char *what() const noexcept override;

private:
    std::shared_ptr<TeaException> inner_exception_;
};

#endif // DARABONBA_EXCEPTIONS_HPP
