#pragma once

#include <iostream>
#include <map>

#include "olcPixelGameEngine.h"
#include "olcPGEX_Graphics2D.h"

/*+++++++++++++++
	Should really figure out how to throw exceptions instead of just outputing an error when things aren't loaded right here
+++++++++++++++*/


class Assets //Singleton     (Like managers with Hololens! or one gigantic global variable)
{
public:
	static Assets& get() //"static" makes it a method applied directly to class, don't need to create an instance to use
	{
		static Assets me;
		return me;
	}

	//Ensure use as singleton
	Assets(Assets const&) = delete; //Delete default copy constructor
	void operator=(Assets const&) = delete; //Make sure the one global instance can't be duplicated


	olc::Sprite* GetSprite(std::string name)
	{
		if (mapSprites[name] == nullptr)
			std::cerr << "\nError: Could not retrieve sprite with the name \"" << name << "\". Please ensure it exists on disk and is loaded from Assets class.\n";

		return mapSprites[name]; //If "name" is specified wrongly or doesn't exist, it will just return a blank sprite
	}

	olc::Decal* GetDecal(std::string name)
	{
		if (mapDecals[name] == nullptr)
			std::cerr << "\nError: Could not retrieve decal with the name \"" << name << "\". Please ensure it exists on disk and is loaded from Assets class.\n";

		return mapDecals[name].get();

	}


	void LoadSprites();
	void LoadTileSprites(olc::PixelGameEngine*, std::vector<std::string>);
	olc::Sprite* CreateNotFoundSprite(olc::PixelGameEngine* pge);

private:
	Assets();
	~Assets();

	std::map<std::string, olc::Sprite*> mapSprites;
	std::map<std::string, std::unique_ptr<olc::Decal>> mapDecals;
};

