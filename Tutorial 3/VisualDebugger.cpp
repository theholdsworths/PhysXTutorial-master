#ifndef aHighResTimerFILE
#define aHighResTimerFILE
#include "VisualDebugger.h"
#include <vector>
#include "Extras\Camera.h"
#include "Extras\Renderer.h"
#include "Extras\HUD.h"
#include <iostream>
#include <chrono>

namespace VisualDebugger
{
	using namespace physx;

	enum RenderMode
	{
		DEBUG,
		NORMAL,
		BOTH
	};

	enum HUDState
	{
		EMPTY = 0,
		HELP_MENU = 1,
		PAUSE = 2
	};

	class SZ_HighResTimer
	{
	private:
		typedef std::chrono::high_resolution_clock Clock;
		std::chrono::steady_clock::time_point startChrono;

	public:
		SZ_HighResTimer();
		void resetChronoTimer();
		float getChronoTime();
	};
#endif
	SZ_HighResTimer::SZ_HighResTimer()
	{
	}
	void SZ_HighResTimer::resetChronoTimer()
	{
		startChrono = Clock::now();
	}
	float SZ_HighResTimer::getChronoTime()
	{
		std::chrono::steady_clock::time_point now = Clock::now();
		auto timeDiff = std::chrono::duration_cast<std::chrono::nanoseconds>(now - startChrono).count();
		return (float)timeDiff;
	}

	//function declarations
	void KeyHold();
	void KeySpecial(int key, int x, int y);
	void KeyRelease(unsigned char key, int x, int y);
	void KeyPress(unsigned char key, int x, int y);

	void motionCallback(int x, int y);
	void mouseCallback(int button, int state, int x, int y);
	void exitCallback(void);
	void FPS();

	void RenderScene();
	void ToggleRenderMode();
	void HUDInit();

	///simulation objects
	Camera* camera;
	PhysicsEngine::MyScene* scene;
	PxReal delta_time = 1.f / 60.f;
	PxReal gForceStrength = 20;
	RenderMode render_mode = NORMAL;
	const int MAX_KEYS = 256;
	bool key_state[MAX_KEYS];
	bool hud_show = true;
	HUD hud;
	
	float fps = 0;
	int time, timebase = 0, frame = 0;
	
	void FPS()
	{

		// Calculate the frames per second.
		frame++;

		time = glutGet(GLUT_ELAPSED_TIME);

		if (time - timebase > 1000)
		{
			fps = frame * 1000.0 / (time - timebase);
			timebase = time;
			frame = 0;
		}
		
		cerr << "FPS: " << fps << endl;
	}
	

	//Init the debugger
	void Init(const char *window_name, int width, int height)
	{
		///Init PhysX
		PhysicsEngine::PxInit();
		scene = new PhysicsEngine::MyScene();
		scene->Init();

		///Init renderer
		Renderer::BackgroundColor(PxVec3(150.f / 255.f, 150.f / 255.f, 150.f / 255.f));
		Renderer::SetRenderDetail(40);
		Renderer::InitWindow(window_name, width, height);
		Renderer::Init();

		camera = new Camera(PxVec3(-30.0f, 30.0f, 0.0f), PxVec3(0.8f, -0.6f, 0.0f), 5.f);

		//initialise HUD
		HUDInit();

		///Assign callbacks
		//render
		glutDisplayFunc(RenderScene);

		//keyboard
		glutKeyboardFunc(KeyPress);
		glutSpecialFunc(KeySpecial);
		glutKeyboardUpFunc(KeyRelease);

		//mouse
		glutMouseFunc(mouseCallback);
		glutMotionFunc(motionCallback);

		//exit
		atexit(exitCallback);

		//init motion callback
		motionCallback(0, 0);
	}

	void HUDInit()
	{
		//initialise HUD
		//add an empty screen
		hud.AddLine(EMPTY, "");
		//add a help screen
		hud.AddLine(HELP_MENU, "                                                       Medieval Rugby");
		hud.AddLine(HELP_MENU, " Simulation");
		hud.AddLine(HELP_MENU, "    F9 - Select next actor");
		hud.AddLine(HELP_MENU, "    F10 - Pause");
		hud.AddLine(HELP_MENU, "    Home Key - Reset");
		hud.AddLine(HELP_MENU, "");
		hud.AddLine(HELP_MENU, " Display");
		hud.AddLine(HELP_MENU, "    F5 - Help on/off");
		hud.AddLine(HELP_MENU, "    F6 - Shadows on/off");
		hud.AddLine(HELP_MENU, "    F7 - Render mode view");
		hud.AddLine(HELP_MENU, "    F8 - Reset camera view");
		hud.AddLine(HELP_MENU, "");
		hud.AddLine(HELP_MENU, "");
		//add a pause screen
		hud.AddLine(PAUSE, "");
		hud.AddLine(PAUSE, "");
		hud.AddLine(PAUSE, "");
		hud.AddLine(PAUSE, "   Simulation paused. Press F10 to continue.");
		//set font size for all screens
		hud.FontSize(0.018f);
		//set font color for all screens
		hud.Color(PxVec3(0.f, 0.f, 0.f));
	}

	//Start the main loop
	void Start()
	{
		glutMainLoop();
	}

	//Render the scene and perform a single simulation step
	void RenderScene()
	{
		if (scene->egg != nullptr)
			camera->setEye(((PxRigidDynamic*)scene->egg->Get())->getGlobalPose().p);

		int score = scene->GetScore();

		
		hud.AmendLine(HELP_MENU, "FPS: " + to_string(fps));
		
		hud.AmendLine(HELP_MENU, "SCORE: " + to_string(score));
	

	
		
		/*
		hud.AddLine(HELP_MENU, "SCORE: " + to_string(score));
		hud.AddLine(HELP_MENU, "FPS: " + to_string(fps));
		hud.AmendLine(HELP_MENU, "FPS: " + to_string(fps));*/

		//handle pressed keys
		KeyHold();

		//start rendering
		Renderer::Start(camera->getEye(), camera->getDir());

		if ((render_mode == DEBUG) || (render_mode == BOTH))
		{
			Renderer::Render(scene->Get()->getRenderBuffer());
		}

		if ((render_mode == NORMAL) || (render_mode == BOTH))
		{
			std::vector<PxActor*> actors = scene->GetAllActors();
			if (actors.size())
				Renderer::Render(&actors[0], (PxU32)actors.size());
		}

		//adjust the HUD state
		if (hud_show)
		{
			if (scene->Pause())
				hud.ActiveScreen(PAUSE);
			else
				hud.ActiveScreen(HELP_MENU);
		}
		else
			hud.ActiveScreen(EMPTY);

		//render HUD
		hud.Render();

		//finish rendering
		Renderer::Finish();

		FPS();

		//perform a single simulation step
		scene->Update(delta_time);


	}

	//user defined keyboard handlers
	void UserKeyPress(int key)
	{
		switch (toupper(key))
		{
			//implement your own
		case 'R':
			scene->KeyPressR();
			break;
		case 'B':
			scene->KeyPressB();
			break;
		case 'P':
			scene->KeyPressP();
			break;
		default:
			break;
		}
	}
	
	void UserKeyRelease(int key)
	{
		switch (toupper(key))
		{
		case 'B':
			scene->KeyReleaseB();
			break;
		case 'Y':
			//scene->KeyReleaseY();
			break;
		default:
			break;
		}
	}

	void UserKeyHold(int key)
	{
	}

	//handle camera control keys
	void CameraInput(int key)
	{
		switch (toupper(key))
		{
		case 'W':
			camera->MoveForward(delta_time);
			break;
		case 'S':
			camera->MoveBackward(delta_time);
			break;
		case 'A':
			camera->MoveLeft(delta_time);
			break;
		case 'D':
			camera->MoveRight(delta_time);
			break;
		case 'Q':
			camera->MoveUp(delta_time);
			break;
		case 'Z':
			camera->MoveDown(delta_time);
			break;
		default:
			break;
		}
	}

	//handle force control keys
	void ForceInput(int key)
	{
	}

	///handle special keys
	void KeySpecial(int key, int x, int y)
	{
		//simulation control
		switch (key)
		{
			//display control
		case GLUT_KEY_F5:
			//hud on/off
			hud_show = !hud_show;
			break;
		case GLUT_KEY_F6:
			//shadows on/off
			Renderer::ShowShadows(!Renderer::ShowShadows());
			break;
		case GLUT_KEY_F7:
			//toggle render mode
			ToggleRenderMode();
			break;
		case GLUT_KEY_F8:
			//reset camera view
			camera->Reset();
			break;
			//simulation control
		case GLUT_KEY_F9:
			//select next actor
			scene->SelectNextActor();
			break;
		case GLUT_KEY_F10:
			//toggle scene pause
			scene->Pause(!scene->Pause());
			break;
		case GLUT_KEY_HOME:
			//resect scene
			scene->Reset();
			break;
		default:
			break;
		}
	}

	//handle single key presses
	void KeyPress(unsigned char key, int x, int y)
	{
		//do it only once
		if (key_state[key] == true)
			return;

		key_state[key] = true;

		//exit
		if (key == 27)
			exit(0);

		UserKeyPress(key);
	}

	//handle key release
	void KeyRelease(unsigned char key, int x, int y)
	{
		key_state[key] = false;
		UserKeyRelease(key);
	}

	//handle holded keys
	void KeyHold()
	{
		for (int i = 0; i < MAX_KEYS; i++)
		{
			if (key_state[i]) // if key down
			{
				CameraInput(i);
				ForceInput(i);
				UserKeyHold(i);
			}
		}
	}

	///mouse handling
	int mMouseX = 0;
	int mMouseY = 0;

	void motionCallback(int x, int y)
	{
		int dx = mMouseX - x;
		int dy = mMouseY - y;

		camera->Motion(dx, dy, delta_time);

		mMouseX = x;
		mMouseY = y;
	}

	void mouseCallback(int button, int state, int x, int y)
	{
		mMouseX = x;
		mMouseY = y;
	}

	void ToggleRenderMode()
	{
		if (render_mode == NORMAL)
			render_mode = DEBUG;
		else if (render_mode == DEBUG)
			render_mode = BOTH;
		else if (render_mode == BOTH)
			render_mode = NORMAL;
	}

	///exit callback
	void exitCallback(void)
	{
		delete camera;
		delete scene;
		PhysicsEngine::PxRelease();
	}
}