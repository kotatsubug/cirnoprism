#pragma once

#include "math/math_etc.hh"

class InputControl
{
private:
	float _amt;
public:
	InputControl();
	void AddAmt(float val);
	float GetAmt();
};

inline InputControl::InputControl() :
	_amt(0.0f) {}

inline void InputControl::AddAmt(float val)
{
	_amt += val;
}

inline float InputControl::GetAmt()
{
	return qt::Clamp(_amt, -1.0f, 1.0f);
}