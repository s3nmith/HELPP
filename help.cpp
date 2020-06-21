#include <unistd.h>
#include <sys/socket.h>
#include <iostream>
#include <system_error>
#include <functional>

namespace sys()
{
    namespace
    {
        /// syscall actually has a return value
        template<typename U, typename T, typename... Args>
        struct syscall_wrapper
        {
            std::function<T(Args...)> _syscall;
            syscall_wrapper(T syscall(Args...)) : _syscall(syscall) {}

            U operator() (Args... args)
            {
                T const ret(_syscall(args...));
                int const error_code(errno);
                if (ret == T(-1))
                {
                    throw std::system_error(error_code, std::system_category());
                }
                return U(ret);
            }
        };
        /// syscall only has return value for error code
        template<typename T, typename... Args>
        struct syscall_wrapper<void, T, Args...>
        {
            std::function<T(Args...)> _syscall;
            syscall_wrapper(T syscall(Args...)) : _syscall(syscall) {}

            void operator() (Args... args)
            {
                T const ret(syscall(args...));
                int const error_code(errno);
                if (ret == T(-1))
                {
                    throw std::system_error(error_code, std::system_category());
                }
            }
        };
        /// helper to avoid having to list T and Args...
        template<typename U, typename T, typename... Args>
        syscall_wrapper<U, T, Args...> make_wrapper(T syscall(Args...))
        {
            return syscall_wrapper<U, T, Args...>(syscall);
        }
    }

    /// return value has -1 but is of same type otherwise
    int socket(int domain, int type, int protocol)
    {
        return make_wrapper<int>(socket) (domain, type, protocol);
    }
    /// return value is for error flagging only
    void unlink(const char* pathname)
    {
        return make_wrapper<void>(unlink) (pathname);
    }
    /// return value would be of different type if not encoding errors in it
    size_t read(int filedes, void* buf, size_t nbyte)
    {
        return make_wrapper<size_t>(read) (filedes, buf, nbyte);
    }
}

int main(int, char**)
{
    try
    {
        sys::unlink("/hopefully_nonexisting_file");
    }
    catch (std::runtime_error const& ex)
    {
        std::cerr << "E: " << ex.what() << std::endl;
    }
    return 0;
}
