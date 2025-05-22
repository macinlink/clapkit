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

		// write access
		CKProperty& operator=(const T& v) {
			if (value_ != v) {
				value_ = v;
				if (onChange) {
					onChange();
				}
			}
			return *this;
		}

		std::function<void()> onChange;

	private:
		T value_;
};