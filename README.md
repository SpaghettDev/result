# result

Small, relatively fast, header-only result class.

## API Example

```cpp
#include <string>
#include <result/result.hpp>

result::Result<int> parseFromString(const std::string& str)
{
    try
    {
        return std::stoi(str);
        // or
        return result::Ok<int>(std::stoi(str))
    }
    catch (...)
    {
        return result::Err("String couldn't be converted to number");
    }
}

void someOtherFunction()
{
    auto result = parseFromString("tspmo");

    if (result.isOk()) // or !result.isErr()
        doSomethingElse(result.unwrap());
    else
        log(result.unwrapErr());
}
```

## Exceptions

The only time exceptions are thrown when using this is when unwrapping the value of an erroneous `Result<T>`, and when unrapping the error of a valid `Result<T>`. Pretty standard stuff.

## Why?

Because I find coding fun. It's also pretty lightweight (didn't feel like using [geode-sdk/result](https://github.com/geode-sdk/result)).
