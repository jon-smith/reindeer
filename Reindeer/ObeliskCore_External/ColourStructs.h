#pragma once

#include <cmath>

namespace obelisk
{
	// RGBA class, all values expected to be between 0 and 1
	struct ColourRGBA
	{
		ColourRGBA() = default;

		ColourRGBA(float red, float green, float blue, float alpha = 1.f) : red(red), green(green), blue(blue), alpha(alpha)
		{
		}

		ColourRGBA(const ColourRGBA &o) : ColourRGBA(o.red, o.green, o.blue, o.alpha)
		{
		}

		ColourRGBA(const ColourRGBA &o, float alphaToReplace) : ColourRGBA(o.red, o.green, o.blue, alphaToReplace)
		{
		}

		float red = 0.0;
		float green = 0.0;
		float blue = 0.0;
		float alpha = 1.0;
	};

	// HSV class, all values expected to be between 0 and 1
	struct ColourHSV
	{
		ColourHSV() = default;

		ColourHSV(float hue, float saturation, float value, float alpha = 1.f)
			: hue(hue), saturation(saturation), value(value), alpha(alpha)
		{
		}

		float hue = 0.0;
		float saturation = 0.0;
		float value = 0.0;
		float alpha = 1.0;
	};

	// Slight refactoring of code from http://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
	inline ColourHSV convertToHSV(const ColourRGBA in)
	{
		auto min = in.red < in.green ? in.red : in.green;
		min = min < in.blue ? min : in.blue;

		auto max = in.red > in.green ? in.red : in.green;
		max = max > in.blue ? max : in.blue;

		ColourHSV out;
		out.alpha = in.alpha;
		out.value = max;

		const auto delta = max - min;
		if (delta < 0.00001f)
		{
			out.saturation = 0.f;
			out.hue = 0.f;
			return out;
		}
		if (max > 0.0f)
		{
			out.saturation = (delta / max);
		}
		else
		{
			// if max is 0, then r = g = b = 0
			// s = 0, v is undefined
			out.saturation = 0.0f;
			out.hue = NAN;  // its now undefined
			return out;
		}
		if (in.red >= max)
			out.hue = (in.green - in.blue) / delta;  // between yellow & magenta
		else if (in.green >= max)
			out.hue = 2.0f + (in.blue - in.red) / delta;  // between cyan & yellow
		else
			out.hue = 4.0f + (in.red - in.green) / delta;  // between magenta & cyan

		out.hue *= 60.0f;  // degrees

		if (out.hue < 0.0f)
			out.hue += 360.0f;

		return out;
	}

	inline ColourRGBA toRGB(const ColourHSV in)
	{
		ColourRGBA out;
		out.alpha = in.alpha;

		if (in.saturation <= 0.0f)
		{
			out.red = in.value;
			out.green = in.value;
			out.blue = in.value;
			return out;
		}

		auto hh = in.hue;
		if (hh >= 360.0f)
			hh = 0.0f;
		hh /= 60.0f;
		const auto i = static_cast<long>(hh);
		auto ff = hh - i;
		auto p = in.value * (1.0f - in.saturation);
		auto q = in.value * (1.0f - (in.saturation * ff));
		auto t = in.value * (1.0f - (in.saturation * (1.0f - ff)));

		switch (i)
		{
		case 0:
			out.red = in.value;
			out.green = t;
			out.blue = p;
			break;
		case 1:
			out.red = q;
			out.green = in.value;
			out.blue = p;
			break;
		case 2:
			out.red = p;
			out.green = in.value;
			out.blue = t;
			break;

		case 3:
			out.red = p;
			out.green = q;
			out.blue = in.value;
			break;
		case 4:
			out.red = t;
			out.green = p;
			out.blue = in.value;
			break;
		case 5:
		default:
			out.red = in.value;
			out.green = p;
			out.blue = q;
			break;
		}
		return out;
	}
}