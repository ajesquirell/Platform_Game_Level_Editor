#pragma once
#include "olcPixelGameEngine.h"


class Button
{
public:
	olc::vf2d pos;
	olc::Pixel buttonColor1 = olc::DARK_GREY, borderColor1 = olc::VERY_DARK_GREY, textColor1 = olc::WHITE, buttonColor2 = olc::DARK_GREY, borderColor2 = olc::VERY_DARK_GREY, textColor2 = olc::WHITE;
	olc::Pixel buttonColorToShow = buttonColor1, borderColorToShow = borderColor1, textColorToShow = textColor1;
	std::string sButtonText;
	float fTextScale = 1.0f;

	//Callback function for button 
	//Can assing in program with a lambda ex. "buttonInstance->func = [](void* e) -> void {static_cast<Editor*>(e)->nBrushSize++; };"
	void (*func)(void*);

public:

	Button(olc::vf2d p, std::string str = {}, float textScale = 1.0f, void (*f)(void*) = nullptr) //Better performance than sButtonText = "";
	{
		pos = p;
		sButtonText = str;
		fTextScale = textScale;
		func = f;
	}

	virtual bool IsPointInside(olc::vf2d point) = 0;
	virtual void OnMouseOver() {}
	virtual void OnMouseExit() {}
	virtual void DrawSelf(olc::PixelGameEngine* pge) = 0;

	virtual void OnClick(void* instance) // Pass in and static_cast the instance of class you want to modify (so in Editor class/OnUserUpdate, we use this)
	{
		if (func != nullptr)
			func(instance);
	}
};


class RectButton : public Button
{
private:
	olc::vf2d size;

public:
	RectButton(olc::vf2d p, olc::vf2d s, std::string str = {}, float fTextScale = 1.0f, void (*f)(void*) = nullptr) : Button(p, str, fTextScale, f)
	{
		size = s;
	}

	virtual bool IsPointInside(olc::vf2d point) override
	{
		if (point.x >= pos.x && point.x < pos.x + size.x
			&& point.y >= pos.y && point.y < pos.y + size.y)
		{
			return true;
		}
		else
			return false;
	}

	virtual void OnMouseOver() override
	{
		buttonColorToShow = buttonColor2;
		borderColorToShow = borderColor2;
		textColorToShow = textColor2;
	}

	virtual void OnMouseExit() override
	{
		buttonColorToShow = buttonColor1;
		borderColorToShow = borderColor1;
		textColorToShow = textColor1;
	}

	virtual void DrawSelf(olc::PixelGameEngine* pge)
	{
		pge->FillRectDecal(pos, size, borderColorToShow);
		pge->FillRectDecal(pos + olc::vf2d(1, 1), size - olc::vf2d(2, 2), buttonColorToShow);

		pge->DrawStringDecal(pos + (size / 2.0f) - olc::vf2d((sButtonText.length() * 8 * fTextScale) / 2.0f, 4 * fTextScale), sButtonText, textColorToShow, { fTextScale, fTextScale });
	}
};


class CircleButton : public Button
{
private:
	int radius;
public:
	CircleButton(olc::vf2d p, int rad, std::string str = {}, float fTextScale = 1.0f, void (*f)(void*) = nullptr) : Button(p, str, fTextScale, f)
	{
		radius = rad;
	}

	virtual bool IsPointInside(olc::vf2d point) override
	{
		int a = std::abs(point.x - pos.x);
		int b = std::abs(point.y - pos.y);
		if (std::sqrt(a * a + b * b) < radius)
		{
			return true;
		}
		else
			return false;
	}

	virtual void OnMouseOver() override
	{
		buttonColorToShow = buttonColor2;
		borderColorToShow = borderColor2;
		textColorToShow = textColor2;
	}

	virtual void OnMouseExit() override
	{
		buttonColorToShow = buttonColor1;
		borderColorToShow = borderColor1;
		textColorToShow = textColor1;
	}

	virtual void DrawSelf(olc::PixelGameEngine* pge)
	{
		pge->FillCircle(pos, radius, borderColorToShow);
		pge->FillCircle(pos, radius - 1, buttonColorToShow);

		if (sButtonText == "+" || sButtonText == "-")
			pge->DrawStringDecal(pos - olc::vf2d((sButtonText.length() * 5.5 * fTextScale) / 2.0f, 3.3 * fTextScale), sButtonText, textColorToShow, { fTextScale, fTextScale }); //Annoying me with the + and - buttons so just doing it directly for now
		else
		pge->DrawStringDecal(pos - olc::vf2d((sButtonText.length() * 8 * fTextScale) / 2.0f, 4 * fTextScale), sButtonText, textColorToShow, { fTextScale, fTextScale });
		//pge->DrawString(pos - olc::vf2d((sButtonText.length() * 8 * fTextScale) / 2.0f, 4 * fTextScale), sButtonText, textColorToShow, 1);

	}
};

//TEST
class Butt
{
public:
	int data;
	void (*func)(void*);

	void OnClick(void* e)
	{
		func(e);
	}
};