#pragma once

#include <string_view>
#include <new>
#include <stdexcept>
#include <cstddef>

namespace result
{
	namespace impl
	{
		template <typename T, typename ...Args>
		constexpr inline static std::monostate construct_helper(void* placement, Args&&... args)
		{
			new (placement) T(std::forward<Args>(args)...);
			return {};
		}
	}

	template <typename T>
	class Result;


	// intermediate for an error
	template <>
	class Result<void>
	{
	public:
		constexpr Result(const std::string_view&& reason)
			: m_error_reason(std::move(reason))
		{}

	public:
		const std::string_view&& m_error_reason;
	};


	template<typename T>
	class Result
	{
	public:
		constexpr Result(Result<void>&& re)
			: m_error_reason(std::move(re.m_error_reason))
		{}

		template <typename ...Args>
		constexpr Result(Args&&... args)
				requires(std::is_constructible_v<T, Args...>)
			: dummy(impl::construct_helper<T>(&m_result, std::forward<Args>(args)...)),
				m_error_reason()
		{}

		template<typename ...Args>
		Result() requires(!std::is_constructible_v<T, Args...>) = delete;

		constexpr ~Result()
		{
			if (m_error_reason.empty())
				std::destroy_at(std::launder(reinterpret_cast<T*>(&m_result)));
		}

		constexpr T&& unwrap() &&
		{
			if (!m_error_reason.empty())
				throw std::runtime_error("Tried unwrapping an erroneous Result<T>!");

			return std::move(*std::launder(reinterpret_cast<T*>(&m_result)));
		}

		constexpr T& unwrap() &
		{
			if (!m_error_reason.empty())
				throw std::runtime_error("Tried unwrapping an erroneous Result<T>!");

			return *std::launder(reinterpret_cast<T*>(&m_result));
		}

		constexpr const T& unwrap() const&
		{
			if (!m_error_reason.empty())
				throw std::runtime_error("Tried unwrapping an erroneous Result<T>!");

			return *std::launder(reinterpret_cast<const T*>(&m_result));
		}


		constexpr const std::string_view& unwrapErr() &
		{
			if (m_error_reason.empty())
				throw std::runtime_error("Tried unwrapping the error from a valid Result<T>!");

			return m_error_reason;
		}

		constexpr const std::string_view& unwrapErr() const&
		{
			if (m_error_reason.empty())
				throw std::runtime_error("Tried unwrapping the error from a valid Result<T>!");

			return m_error_reason;
		}


		constexpr bool isOk() const { return m_error_reason.empty(); }
		constexpr bool isErr() const { return !m_error_reason.empty(); }

		constexpr operator bool() const { return m_error_reason.empty(); }

	private:
		union
		{
			alignas(T) std::byte m_result[sizeof(T)];
			std::monostate dummy;
		};
		const std::string_view m_error_reason;
	};

	template <typename T, typename ...Args>
	constexpr inline static Result<T> Ok(Args&&... args)
	{
		return Result<T>{ std::forward<Args>(args)... };
	}

	constexpr inline static Result<void> Err(const std::string_view& reason = "")
	{
		return Result<void>{ std::move(reason) };
	}
}
