#pragma comment(lib, "nclgl.lib")

#include "../../nclgl/Window.h"
#include "Renderer.h"

int main() {
	Window w("Volcano Island!", 1280, 760, false);
	if (!w.HasInitialised()) {
		return -1;
	}

	Renderer renderer(w);
	if (!renderer.HasInitialised()) {
		return -1;
	}

	w.LockMouseToWindow(true);
	w.ShowOSPointer(false);

	while (w.UpdateWindow() && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE)) {
		float time = w.GetTimer()->GetTimedMS();
		renderer.UpdateScene(time);
		renderer.RenderScene();
		renderer.CalculateFPS(time);
	}

	return 0;
}