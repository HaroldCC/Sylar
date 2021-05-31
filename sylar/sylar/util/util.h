#ifndef __UTIL_H__
#define __UTIL_H__

#include <cstdint>

#if defined(linux) || defined(__linux) || defined(__linux__)
#include <unistd.h>
#include <sys/types.h>
#elif defined(WIN32) || defined(_WIN32)
#include <windows.h>
#endif

/*
 *@brief 获取线程ID
 * @return 整形值线程id
 */
inline std::uint32_t getThreadId()
{
#if defined(linux) || defined(__linux) || defined(__linux__)
    return static_cast<uint32_t>(gettid());
#elif defined(WIN32) || defined(_WIN32)
    return static_cast<uint32_t>(GetCurrentThreadId());
#endif
}

#include <typeinfo>
#include <exception>
#include <type_traits>
#include <sstream>
#include <limits>

namespace Util
{
    class bad_lexical_cast : public std::bad_cast
    {
    public:
        bad_lexical_cast()
            : source(&typeid(void)), target(&typeid(void)) {}

        bad_lexical_cast(const std::type_info &source_type, const std::type_info &target_type)
            : source(&source_type), target(&target_type) {}

        const std::type_info &source_type() const { return *source; }

        const std::type_info &target_type() const { return *target; }

        virtual const char *what() const throw()
        {
            return "bad lexical cast: "
                   "source type value could not be interpreted as target";
        }

        virtual ~bad_lexical_cast() throw() {}

    private:
        const std::type_info *source;

        const std::type_info *target;
    };

    template <typename Target, typename Source>
    class lexical_stream
    {
    private:
        typedef char char_type;

        std::basic_stringstream<char_type> stream;

    public:
        lexical_stream()
        {
            stream.unsetf(std::ios::skipws);
            if (std::numeric_limits<Target>::is_specialized)
                stream.precision(std::numeric_limits<Target>::digits10 + 1);
            else if (std::numeric_limits<Source>::is_specialized)
                stream.precision(std::numeric_limits<Source>::digits10 + 1);
        }

        ~lexical_stream() {}

        //把Source类型输入到流中
        bool operator<<(const Source &input) { return !(stream << input).fail(); }

        //把流转换为Target类型输出
        template <typename InputStreamable>
        bool operator>>(InputStreamable &output)
        {
            return !std::is_pointer<InputStreamable>::value &&
                   stream >> output &&
                   stream.get() ==
                       std::char_traits<char_type>::eof();
        }

        //string特化
        bool operator>>(std::string &output)
        {
            output = stream.str();
            return true;
        }
    };

    template <class T>
    struct array_to_pointer_decay
    {
        typedef T type;
    };

    template <class T, std::size_t N>
    struct array_to_pointer_decay<T[N]>
    {
        typedef const T *type;
    };

    template <typename Target, typename Source>
    Target lexical_cast(const Source &arg)
    {
        typedef typename array_to_pointer_decay<Source>::type NewSource;
        lexical_stream<Target, NewSource> interpreter;
        Target result;

        if (!(interpreter << arg && interpreter >> result))
            throw(bad_lexical_cast(typeid(NewSource), typeid(Target)));

        return result;
    }
}

#endif // __UTIL_H__