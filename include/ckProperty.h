/**
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * CKProperty
 * ----------------------------------------------------------------------
 * A simple reactivity wrapper for properties.
 *
 */

#pragma once
#include <functional>

template <typename, typename = void>
struct has_subscribe : std::false_type {};

template <typename U>
struct has_subscribe<U, std::void_t<decltype(std::declval<U>().Subscribe(std::declval<std::function<void()>>()))>> : std::true_type {};

template <typename T>
class CKProperty {
	public:
		CKProperty() = default;
		CKProperty(T v)
			: value_(v) {}

		// read access
		operator const T&() const { return value_; }
		const T& get() const { return value_; }
		T& get() { return value_; }

		// For regular types
		template <typename U = T>
		auto operator->() -> typename std::enable_if<!std::is_pointer<U>::value, U*>::type {
			return &value_;
		}

		template <typename U = T>
		auto operator->() const -> typename std::enable_if<!std::is_pointer<U>::value, const U*>::type {
			return &value_;
		}

		// For pointer types — unwrap the pointer
		template <typename U = T>
		auto operator->() -> typename std::enable_if<std::is_pointer<U>::value, typename std::remove_pointer<U>::type*>::type {
			return value_;
		}

		template <typename U = T>
		auto operator->() const -> typename std::enable_if<std::is_pointer<U>::value, typename std::remove_pointer<U>::type*>::type {
			return value_;
		}

		void bindSubscribable() {
			if constexpr (has_subscribe<T>::value) {
				value_.Subscribe([this] { if(onChange) onChange(); });
			}
		}

		// write access
		CKProperty& operator=(const T& v) {
			if (value_ != v) {
				value_ = v;
				bindSubscribable();
				if (onChange) {
					onChange();
				}
			}
			return *this;
		}

		CKProperty& operator+=(const T& v) {
			value_ += v;
			if (onChange)
				onChange();
			return *this;
		}

		CKProperty& operator-=(const T& v) {
			value_ -= v;
			if (onChange)
				onChange();
			return *this;
		}

		CKProperty& operator*=(const T& v) {
			value_ *= v;
			if (onChange)
				onChange();
			return *this;
		}

		CKProperty& operator/=(const T& v) {
			value_ /= v;
			if (onChange)
				onChange();
			return *this;
		}

		CKProperty& operator%=(const T& v) {
			value_ %= v;
			if (onChange)
				onChange();
			return *this;
		}

		CKProperty& operator++() {
			++value_;
			if (onChange)
				onChange();
			return *this;
		}

		CKProperty operator++(int) {
			CKProperty tmp = *this;
			++(*this);
			return tmp;
		}

		CKProperty& operator--() {
			--value_;
			if (onChange)
				onChange();
			return *this;
		}

		CKProperty operator--(int) {
			CKProperty tmp = *this;
			--(*this);
			return tmp;
		}

		std::function<void()> onChange;

	private:
		T value_;
};