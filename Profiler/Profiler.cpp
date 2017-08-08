#include "Profiler.h"

#include "../nclgl/Window.h"
#include "../nclgl/Renderer.h"

#define FRAME_MIN 1
#define TEXT_SIZE 15.0f

Profiler::Profiler(Renderer* ren, Window* win, int numTimers)
{
	window			= win;
	renderer		= ren;
	fpsCounter		= FramerateCounter(window->GetTimer()->GetMS());

	//Upper bound of how many timers can be added
	this->numTimers = numTimers;
}

void Profiler::Update(float deltatime)
{
	updateTimer.StartTimer();

	/*
	  Hard-coded to the "p" button...I don't know what the button in the 
	  top left of the keyboard, below ESC, is called...(The one that brings up 
	  the console to activate godmode on Fallout 3...).
	*/
	//if (window->GetKeyboard()->KeyTriggered(KEYBOARD_P)) renderingEnabled = !renderingEnabled;

	UpdateProfiling();
	//if (renderingEnabled)	RenderToScreen();
	//else					updateTimer.StopTimer();

	RenderToScreen();
	updateTimer.StopTimer();

	//Timer is stopped in the render function if that is enabled.
	//So it can time itself, with a minimal loss of accuracy.
}

void Profiler::UpdateProfiling() 
{
	++fpsCounter.frames;

	fpsCounter.CalculateFPS(window->GetTimer()->GetMS());
}

void Profiler::RenderToScreen()
{
	//FPS COUNTER
	fpsCounter.CalculateFPS(window->GetTimer()->GetMS());
	renderer->textbuffer.push_back(Text(
		("FPS: " + std::to_string(fpsCounter.fps)),
		Vector3(0, 0, 0), TEXT_SIZE));

	//TIMERS
	float offset = 100.0f;
	for each(std::pair<string, SubsystemTimer*> timer in timers) {
		renderer->textbuffer.push_back(Text(
			(timer.first + ":" + std::to_string(timer.second->timePassed)),
			Vector3(0, offset, 0), TEXT_SIZE));
		offset += 20.0f;
	}

	/*
	  Very slight loss in accuracy of the Profiler's own timer.
	  Couldnt think of another way to display a timer without
	  actually stopping the timer...
	*/
	updateTimer.StopTimer();
	renderer->textbuffer.push_back(Text(
		("Profiler:" + std::to_string(updateTimer.timePassed)),
		Vector3(0, offset, 0), TEXT_SIZE));
}