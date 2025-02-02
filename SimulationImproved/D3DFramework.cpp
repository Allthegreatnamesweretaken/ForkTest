#include "D3DFramework.h"
#include <directxcolors.h>
#include <vector>
#include "Resource.h"
#include <string>
#include <sstream>
#include <algorithm>
#include <set>
#include "DDSTextureLoader.h"
#include <unordered_map>
#include <iostream>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "directxcollision.h"
//Can be final but want scene changes first.

/*Need to Do list:

---Priority---
- Need to work on redoing everything seperating it all into different files, this could help efficiency and make it easier to read, will also help with the fact that the code is getting very long.
- Need to refine the shaders (if i am keeping them) as the way they are loaded and managed currently is not efficient.
- File manager, for simplicity of being able to find parts of code easier, as in files for shaders etc etc.

-- Maps could be done by having a map loader that loads in the objects and textures, the map loader would be built from config files that would be loaded in at the start of the game, menu would pick between them.


- Remember to Update the README everytime i push a new change.
---Secondary---
- FPS limiter, need to add one as i can hear my fans crying every time i run this. (Maybe find a way to load faster(Or it could be just how DX11 loads?))
- Fix Collision as the player can still currently warp up through the floor.

- Need to think more about how the game will work, while the baseline is okay need more idea on it like multiple guns? More maps? Enemines? and general feeling for the game.
-- Am liking the idea of a base parkour game for the beginning, can add multiple guns with a switch statment to change the way they fire,(And maybe their model) Enemies would be a long way off if not impossible because of the way the game is designed. 

-- Have Implemented new movement as a way around, this movement is only controlled by the W key and the player moving the mouse the player loses controller of the W key while in air and has to reply on the launching to get to the end.
- Add in a way to let the player have in air controls to make teh movement while falling feel smoother, can also change gravity if needed as it right now is strange.
- change the wya the shaders are loaded should be able to be done in a loop.
- Change the way they are added to an object should be able to do that inside of where we create the object.
*/

std::unique_ptr<D3DFramework> D3DFramework::_instance = std::make_unique<D3DFramework>();
//Config can stay as a way of loading objects can be changed later on though if can think of better method. Also see if Objects can be laoded that aren't an obj / are more efficient.
bool D3DFramework::LoadConfig(const std::string& pathname) {

	//Left here for simplicity of loading in objects and textures, can be changed later on.
	std::ifstream fin(pathname);
	if (!fin) {
		return false;
	}

	std::string tag;
	while (fin >> tag) {
		if (tag == "name") {
			ObjModel tempObject;
			std::string objectName;
			fin >> objectName;
			objectNames.push_back(objectName);
			_modelLoader.LoadOBJ(objectName, tempObject);
			_models.push_back(tempObject);
		}
		else if (tag == "texture") {
			std::string texturePath;
			fin >> texturePath;
			textureNames.push_back(texturePath);
		}
		else if (tag == "position") {
			XMFLOAT3 vertex;
			fin >> vertex.x >> vertex.y >> vertex.z;
			objectPositions.push_back(vertex);
		}
		else if (tag == "player_position") {
			fin >> _firstObjectPosition.x >> _firstObjectPosition.y >> _firstObjectPosition.z;
		}
		else if (tag == "light_direction") {
			fin >> _initialSunLightDirection.x >> _initialSunLightDirection.y >> _initialSunLightDirection.z;
			_lightDirection = _initialSunLightDirection;
		}
		else {
			std::string line;
			std::getline(fin, line);
		}
	}
	return true;
}

//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
D3DFramework::~D3DFramework() {
	try {
		if (_pImmediateContext)
			_pImmediateContext->ClearState();
	}
	catch (...) {

	}
}


void D3DFramework::zoomIn() {
	if (_zoomFactor > 0.1f) {
		_zoomFactor -= 0.1f; // Decrease zoom factor, but not less than 0.1
		updateViewMatrix();
	}
}
size_t i = 0;
void D3DFramework::zoomOut() {
	if (_zoomFactor < 10.0f) {
		_zoomFactor += 0.1f; // Increase zoom factor, but not more than 10.0
		updateViewMatrix();
	}
}
