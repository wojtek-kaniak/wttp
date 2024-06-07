#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdatomic.h>
#include <assert.h>

#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include <raylib.h>
#include "http_thread.h"
#include "nfd.h"

#include "logging.h"

extern char** environ;

static int mgmt_main(char* arg0);
static int server_main(long port, const char* webroot);

/// Open a directory picker
/// Returns a path or nullptr
static char* open_dir_picker();

int main(int argc, char** argv)
{
	logging_initialize();

	assert(argc >= 1);

	const char* webroot = getenv("WTTP_GUI_WEBROOT");
	const char* port = getenv("WTTP_GUI_PORT");

	if (webroot && port)
		server_main(strtol(port, nullptr, 10), webroot);
	else
	 	mgmt_main(argv[0]);
}

int mgmt_main(char* arg0)
{
	InitWindow(300, 200, "WTTP");

	// Disabled when a dialog is open
	// bool window_active = true;

	bool input_hover = false;
	bool input_active = false;

	Rectangle input_rect = {
		.x = 170,
		.y = 15,
		.width = 110,
		.height = 40,
	};

	#define INPUT_MAX_CHARS 5
	char input_chars[INPUT_MAX_CHARS + 1] = "8080\0";
	size_t input_chars_len = 4;

	bool webroot_hover = false;

	Rectangle webroot_rect = {
		.x = 170,
		.y = 65,
		.width = 110,
		.height = 40,
	};

	const char* webroot = nullptr;

	bool start_hover = false;

	Rectangle start_rect = {
		.x = 170,
		.y = 140,
		.width = 110,
		.height = 40,
	};

	// TODO: custom render loop
	SetTargetFPS(10);
	
	while (!WindowShouldClose())
	{
		input_hover = CheckCollisionPointRec(GetMousePosition(), input_rect);
		webroot_hover = CheckCollisionPointRec(GetMousePosition(), webroot_rect);
		start_hover = CheckCollisionPointRec(GetMousePosition(), start_rect);

		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
		{
			input_active = input_hover;

			if (webroot_hover)
				webroot = open_dir_picker();

			if (start_hover && webroot)
				goto run_server;
		}

		if (input_active)
			SetMouseCursor(MOUSE_CURSOR_IBEAM);
		else
		 	SetMouseCursor(MOUSE_CURSOR_DEFAULT);

		if (input_active)
		{
			int key;
			while ((key = GetKeyPressed()) > 0)
			{
				if (key >= '0' && key <= '9' && input_chars_len != INPUT_MAX_CHARS)
				{
					input_chars[input_chars_len] = key;
					input_chars_len++;
				}
				else if (key == KEY_BACKSPACE && input_chars_len != 0)
				{
					input_chars_len--;
					input_chars[input_chars_len] = '\0';
				}
			}
		}

		BeginDrawing();
			ClearBackground(WHITE);
			DrawText("port:", 15, 20, 30, BLACK);

			DrawRectangleRec(input_rect, RAYWHITE);

			if (input_active || input_hover)
				DrawRectangleLinesEx(input_rect, 2.0f, BLACK);
			else
				DrawRectangleLinesEx(input_rect, 1.5f, BLACK);
			
			DrawText(input_chars, input_rect.x + 10, input_rect.y + 5, 30, BLACK);
			
			DrawText("web root:", 15, 70, 30, BLACK);

			DrawRectangleRec(webroot_rect, RAYWHITE);

			if (webroot_hover)
				DrawRectangleLinesEx(webroot_rect, 2.0f, BLACK);
			else
				DrawRectangleLinesEx(webroot_rect, 1.5f, BLACK);

			DrawText("select", webroot_rect.x + 10, webroot_rect.y + 5, 30, BLACK);

			DrawRectangleRec(start_rect, RAYWHITE);

			if (start_hover)
				DrawRectangleLinesEx(start_rect, 2.0f, BLACK);
			else
				DrawRectangleLinesEx(start_rect, 1.5f, BLACK);

			DrawText("start", start_rect.x + 15, start_rect.y + 5, 30, BLACK);
		EndDrawing();
	}

	CloseWindow();
	return 0;

	run_server:
	CloseWindow();

	setenv("WTTP_GUI_WEBROOT", webroot, true);
	setenv("WTTP_GUI_PORT", input_chars, true);
	char* args[] = { arg0, nullptr };

	// TODO: instead posix_spawn self, kill the child on stop
	pid_t child = vfork();

	if (child == 0)
	{
		execve("/proc/self/exe", args, environ);

		perror("exec failed");
		assert(false);
	}
	else if (child == -1)
	{
		perror("vfork failed");
		assert(false);
	}

	Rectangle stop_rect = {
		.x = 20,
		.y = 20,
		.width = 110,
		.height = 60,
	};

	bool stop_hover = true;

	InitWindow(150, 100, "WTTP");
	
	// TODO: custom render loop
	SetTargetFPS(10);

	while (!WindowShouldClose())
	{
		stop_hover = CheckCollisionPointRec(GetMousePosition(), stop_rect);

		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && stop_hover)
			break;

		BeginDrawing();
			ClearBackground(WHITE);

			DrawRectangleRec(stop_rect, RAYWHITE);

			if (stop_hover)
				DrawRectangleLinesEx(stop_rect, 2.0f, BLACK);
			else
				DrawRectangleLinesEx(stop_rect, 1.5f, BLACK);

			DrawText("stop", stop_rect.x + 20, stop_rect.y + 15, 30, BLACK);
		EndDrawing();
	}

	kill(child, SIGINT);

	return 0;
}

static int server_main(long port, const char* webroot)
{
	if (port > UINT16_MAX)
	{
		fprintf(stderr, "invalid port\n");
		return 2;
	}

	fprintf(stderr, "starting a server, port `%ld`, webroot `%s`\n", port, webroot);

	HttpConfig http_config = {
		.webroot = webroot,
		.port = (uint16_t)port,
		.continue_flag = ATOMIC_FLAG_INIT,
	};
	atomic_flag_test_and_set(&http_config.continue_flag);

	webroot_chroot(http_config.webroot);

	http_thread(&http_config);

	return 0;
}

static char* open_dir_picker()
{
	nfdchar_t* out_path = nullptr;
	nfdresult_t res = NFD_PickFolder(nullptr, &out_path);

	if (res == NFD_ERROR)
		fprintf(stderr, "failed to open a folder picker: `%s`\n", NFD_GetError());
	
	return out_path;
}
