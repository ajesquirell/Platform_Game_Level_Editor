#include "Assets.h"

using namespace std;

Assets::Assets()
{
}

Assets::~Assets()
{
}

void Assets::LoadSprites() //Single one time load of all sprite resources
{
	auto load = [&](string name, string filename)
	{
		mapSprites[name] = new olc::Sprite(filename);
	};

	//Floor
	//load("Floor", "../Sprites/Floor.png");

}

void Assets::LoadTileSprites(olc::PixelGameEngine* pge, std::vector<std::string> vecTileNames) // One time load from sprite names provided in Tiles.key
{
	auto load = [&](string name, string filename)
	{
		mapSprites[name] = new olc::Sprite(filename);
	};

	for (int i = 0; i < vecTileNames.size(); i++)
	{
		load(vecTileNames[i], "../Sprites/" + vecTileNames[i] + "/" + vecTileNames[i] + ".png"); // Creates a sprite in assets with the same name as the main png. 
																							// E.x. If you put "Brick" and "Floor" in Tiles.key, this creates 
																							// a Sprite called "Brick" and "Floor" from "Brick.png" and "Floor.png" in their respective folders
																							// We can expand this later to have multiple png's as the main state, loading the animation, and including different states like breaking

		mapDecals[vecTileNames[i]] = std::make_unique<olc::Decal>(mapSprites[vecTileNames[i]]); // Added this here to also have all our sprites as decals
	}

	// Create Not Found Sprite in case something goes wrong with reading the file and Tiles.key
	CreateNotFoundSprite(pge);

	// Other static sprite load for the error screen (NOT A TILE)
	load("Error", "../Sprites/Error.png");

	//Ex
	//load("Floor", "../Sprites/Floor.png");

}

void Assets::CreateNotFoundSprite(olc::PixelGameEngine* pge)
{
	auto load = [&](string name, string filename)
	{
		mapSprites[name] = new olc::Sprite(filename);
	};

	olc::Sprite* sp = new olc::Sprite(22, 22);
	olc::Sprite* sprText = new olc::Sprite(48, 8);
	

	pge->SetDrawTarget(sprText);
	pge->Clear(olc::BLACK);
	pge->DrawString(olc::vi2d(0, 0), "Sprite");

	pge->SetDrawTarget(sp);
	olc::GFX2D::Transform2D t;
	t.Scale(0.49f, 1.0f);
	t.Translate(0.0f, 7.0f);
	olc::GFX2D::DrawSprite(sprText, t);
	pge->DrawLine(olc::vi2d(0, 0), olc::vi2d(22, 22), olc::DARK_RED);
	pge->DrawLine(olc::vi2d(0, 22), olc::vi2d(22, 0), olc::DARK_RED);

	pge->SetDrawTarget(nullptr);
	mapSprites["Not_Found"] = sp;

	mapDecals["Not_Found"] = std::make_unique<olc::Decal>(sp); // Add it to decals too
}

