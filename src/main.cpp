#include <SDL.h>
#include <SDL_image.h>

#include <emscripten.h>
#include <emscripten/html5.h>

#include <cmath>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "Engine/Camera.h"
#include "Engine/Window.h"

#include "Engine/Render/Mesh.h"
#include "Engine/Render/MeshInstance.h"

#include "Utils/MeshLoader.h"

static CCamera s_Camera;
static double s_LastTime = 0.0;
static int s_Width, s_Height;
static const char* s_Canvas = "#canvas";

void OnUpdate()
{
	const double time = emscripten_performance_now() * 0.001;
	const double delta = time - s_LastTime;

	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_MOUSEMOTION:
			if (CWindow::Instance().HasMouse())
			{
				s_Camera.localRotate(CQuaternion(
					Eigen::AngleAxisf(-event.motion.yrel * M_PI * (1.0f / 1024.0f), Eigen::Vector3f::UnitX()) *
					Eigen::AngleAxisf(-event.motion.xrel * M_PI * (1.0f / 1024.0f), Eigen::Vector3f::UnitY())));
			}
			break;
		}
	}

	static float cam_speed = 5;
	const Uint8 buttons = SDL_GetMouseState(NULL, NULL);
	if (buttons & SDL_BUTTON_LMASK)
	{
		CWindow::Instance().GrabMouse();
	}

	const Uint8* keys = SDL_GetKeyboardState(NULL);
	if (keys[SDL_SCANCODE_W])
	{
		s_Camera.localTranslate(-CVector3f::UnitZ() * delta * cam_speed);
	}
	if (keys[SDL_SCANCODE_A])
	{
		s_Camera.localTranslate(-CVector3f::UnitX() * delta * cam_speed);
	}
	if (keys[SDL_SCANCODE_S])
	{
		s_Camera.localTranslate(CVector3f::UnitZ() * delta * cam_speed);
	}
	if (keys[SDL_SCANCODE_D])
	{
		s_Camera.localTranslate(CVector3f::UnitX() * delta * cam_speed);
	}
	if (keys[SDL_SCANCODE_LCTRL])
	{
		s_Camera.localTranslate(-CVector3f::UnitY() * delta * cam_speed);
	}
	if (keys[SDL_SCANCODE_SPACE])
	{
		s_Camera.localTranslate(CVector3f::UnitY() * delta * cam_speed);
	}

	if (CWindow::Instance().IsInitialized())
	{
		s_Camera.activateGL();

		static float timer = 0.f;
		timer += delta;

		static bool loaded = false;
		static std::shared_ptr<NRender::SMesh> mesh = std::make_shared<NRender::SMesh>();
		static NRender::CMeshInstance mesh_instance(mesh);

		if (!loaded)
		{
			NUtils::LoadMesh("assets/models/muro.obj", "", *mesh);
			mesh_instance.Reload();
			mesh_instance.Scale(0.1f);
			loaded = true;
		}

		CMatrix3f m(Eigen::AngleAxisf(0.125 * M_PI * delta, CVector3f::UnitY()));

		mesh_instance.Rotate(m);
		mesh_instance.Draw();

		CWindow::Instance().Present();
	}

	s_LastTime = time;
}

static EM_BOOL OnResize(int, const EmscriptenUiEvent* event, void*)
{
	s_Width = event->windowInnerWidth, s_Height = event->windowInnerHeight;
	emscripten_set_canvas_element_size(s_Canvas, s_Width, s_Height);
	s_Camera.setViewport(s_Width, s_Height);

	return EM_TRUE;
}

static EM_BOOL OnLockChange(int, const EmscriptenPointerlockChangeEvent* event, void*)
{
	if (!event->isActive)
	{
		CWindow::Instance().ReleaseMouse();
	}

	return EM_TRUE;
}

int main()
{
	emscripten_set_main_loop(&OnUpdate, 0, 0);

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
	{
		emscripten_cancel_main_loop();
		printf("Error initializing SDL %s\n", SDL_GetError());
		return -1;
	}

	emscripten_get_canvas_element_size(s_Canvas, &s_Width, &s_Height);

	if (!CWindow::Instance().Initialize(s_Width, s_Height))
	{
		emscripten_cancel_main_loop();
		return -1;
	}

	s_Camera.setPosition(CVector3f(35.0f, 35.0f, 35.0f));
	s_Camera.setTarget(CVector3f(0.0f, 12.0f, 0.0f));
	s_Camera.setViewport(s_Width, s_Height);

	emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 0, OnResize);
	emscripten_set_pointerlockchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, 0, 0, OnLockChange);
}
