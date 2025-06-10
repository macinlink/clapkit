/**
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * CKValueContainingControl
 * ----------------------------------------------------------------------
 * Describes a control where it has a "value" - like a textbox or a
 * checkbox, radiobox, etc.
 */

#pragma once
#include <string>
#include <variant>

enum class CKValueType {
	None,
	Boolean,
	Text,
	Misc
};

class CKValueContainingControl {

	public:
		void SetValue(bool value) {
			_value.bValue = value;
			_type = CKValueType::Boolean;
		}

		void SetValue(void* value) {
			_value.mValue = value;
			_type = CKValueType::Misc;
		}

		bool GetBoolean() const {
			return _type == CKValueType::Boolean ? _value.bValue : false;
		}

		void* GetMisc() const {
			return _type == CKValueType::Misc ? _value.mValue : nullptr;
		}

	protected:
		CKValueType _type = CKValueType::None;
		union {
				bool bValue;
				void* mValue;
		} _value;
};