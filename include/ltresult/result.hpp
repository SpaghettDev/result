#pragma once

#include <type_traits>
#include <string_view>
#include <new>
#include <stdexcept>
#include <cstddef>

namespace result
{
	namespace impl
	{
		struct dummy_t {};

		template <typename T, typename ...Args>
		constexpr inline static dummy_t construct_helper(void* placement, Args&&... args)
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
		using base_t = std::remove_cvref_t<T>;

		constexpr Result(Result<void>&& re)
			: m_error_reason(std::move(re.m_error_reason))
		{}

		constexpr Result(base_t&& res)
			: dummy(impl::construct_helper<base_t>(&m_result, std::move(res))),
				m_error_reason()
		{}
		constexpr Result(const base_t& res)
			: dummy(impl::construct_helper<base_t>(&m_result, std::move(res))),
				m_error_reason()
		{}

		template <typename ...Args>
		constexpr Result(Args&&... args)
				requires(std::is_constructible_v<T, Args...> || std::is_aggregate_v<T>)
			: dummy(impl::construct_helper<base_t>(&m_result, std::forward<Args>(args)...)),
				m_error_reason()
		{}

		constexpr Result<T>& operator=(Result<T>&& other)
		{
			auto&& otherResult = std::move(other.m_result);
			
			this->~Result<T>();
			new (this) Result<T>();

			std::memcpy(
				m_result,
				std::launder(reinterpret_cast<base_t*>(&otherResult)),
				sizeof(base_t)
			);

			return *this;
		}

		constexpr Result<T>& operator=(Result<void>&& other)
		{
			this->~Result<T>();

			return *new (this) Result<T>(std::move(other));
		}

		template<typename ...Args>
		Result() requires(!std::is_constructible_v<T, Args...>) = delete;

		constexpr ~Result()
		{
			if (m_error_reason.empty())
				std::destroy_at(std::launder(reinterpret_cast<base_t*>(&m_result)));
		}

		constexpr T&& unwrap() &&
		{
			if (!m_error_reason.empty())
				throw std::runtime_error("Tried unwrapping an erroneous Result<T>!");

			return std::move(*std::launder(reinterpret_cast<base_t*>(&m_result)));
		}

		constexpr T& unwrap() &
		{
			if (!m_error_reason.empty())
				throw std::runtime_error("Tried unwrapping an erroneous Result<T>!");

			return *std::launder(reinterpret_cast<base_t*>(&m_result));
		}

		constexpr const T& unwrap() const&
		{
			if (!m_error_reason.empty())
				throw std::runtime_error("Tried unwrapping an erroneous Result<T>!");

			return *std::launder(reinterpret_cast<const base_t*>(&m_result));
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
		constexpr Result()
			: m_error_reason()
		{}

	private:
		union
		{
			alignas(T) std::byte m_result[sizeof(T)];
			impl::dummy_t dummy;
		};
		const std::string_view m_error_reason;
	};

	template <typename T, typename ...Args>
	constexpr inline static Result<T> Ok(Args&&... args)
	{
		return Result<T>{ std::forward<Args>(args)... };
	}

	template <typename T>
	constexpr inline static Result<T> Ok(T&& res)
	{
		return Result<T>{ std::move(res) };
	}

	constexpr inline static Result<void> Err(const std::string_view& reason = "")
	{
		return Result<void>{ std::move(reason) };
	}
}
