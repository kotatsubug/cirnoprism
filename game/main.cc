#include "libs.hh"
#include "game.hh"




/// TODO LIST
/// 
/// Sunlight shadow maps, abstract it
/// ECS system
///		-> now you can abstract lights and stuff. Make all sorts of components! Lights, NPCs, ... go wild!
/// Search and scatter or whatever it's called for Physics
/// [ ] Deferred Shading
/// 
/// [ ] Colored lights
/// Blinn-phong and regular phong lighting? Which looks better?
/// Collision boxes
/// Raycast bounce gun thing
/// 
int main()
{
	/// TODO: This should be stored in an .ini file
	const int glVersionMajor = 4;
	const int glVersionMinor = 5;
	const int WINDOW_WIDTH = 1920; // MUST be initialized nonzero.
	const int WINDOW_HEIGHT = 1080;
	int framebufferWidth = WINDOW_WIDTH;
	int framebufferHeight = WINDOW_HEIGHT;

	Game game("cirnoprism", WINDOW_WIDTH, WINDOW_HEIGHT, glVersionMajor, glVersionMinor);

	while (!game.GetWindowShouldClose())
	{
		game.Update();
		game.Render();
	}

	return 0;
}