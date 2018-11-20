#include <stdio.h>
#include <GL/glew.h>
#include <hodefines.h>
#include <homath.h>

#define UNICODE
#include <windows.h>

#define HO_ARENA_IMPLEMENT
#include <memory_arena.h>

const char quad_vshader[] = R"(
	#version 420 core
	layout(location = 0) in vec3 vertex;
	layout(location = 1) in vec2 tcoords;
	layout(location = 2) in vec4 vert_color;

	out vec2 texcoords;

	uniform mat4 projection = mat4(1.0);
	uniform mat4 position;

	void main(){
		gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
		texcoords = tcoords;
	}
	)";

#if 0
const char quad_fshader[] = R"(
	#version 330 core
	in vec2 texcoords;
	out vec4 color;

	uniform sampler2D text;

	void main(){
		color = texture(text, texcoords);
	}
	)";
#else
/*
const char quad_fshader[] = R"(
	#version 420 core
	in vec2 texcoords;
	out vec4 color;

	uniform sampler2D text;
	
	struct Complex{
		float re;
		float im;
	};

	Complex add(Complex l, Complex r){
		Complex result;
		result.re = l.re + r.re;
		result.im = l.im + r.im;
		return result;
	}

	Complex mul(Complex l, Complex r){
		Complex result;
		result.re = l.re * r.re - l.im * r.im;
		result.im = l.re * r.im + l.im * r.re;
		return result;
	}

	uniform float scale;
	uniform int its;
	uniform vec2 offset;

	void main(){
		float half_scale = scale / 2.0;

		float x_coord = (texcoords.x * scale - half_scale) + offset.x;
		float y_coord = (texcoords.y * scale - half_scale) + offset.y;

		float r = 0;

		Complex z;
		Complex coord;
		coord.re = x_coord;
		coord.im = y_coord;

		for(int i = 0; i < its; ++i){
			z = add(mul(z, z), coord);
			float val = sqrt(z.re * z.re + z.im * z.im);
			if (val > 2.0f) {
				float its_f = float(its);
				r = i / its_f;
				break;
			}
		}
		color = vec4(r, r, r, 1.0);
	}
	)";
	*/
const char quad_fshader[] = R"(
	#version 420 core
	in vec2 texcoords;
	out vec4 color;

	uniform sampler2D text;
	
	struct Complex{
		double re;
		double im;
	};

	Complex add(Complex l, Complex r){
		Complex result;
		result.re = l.re + r.re;
		result.im = l.im + r.im;
		return result;
	}

	Complex mul(Complex l, Complex r){
		Complex result;
		result.re = l.re * r.re - l.im * r.im;
		result.im = l.re * r.im + l.im * r.re;
		return result;
	}

	uniform double scale;
	uniform int its;
	uniform dvec2 offset;

	void main(){
		double half_scale = scale / 2.0;

		double x_coord = (texcoords.x * scale - half_scale) + offset.x;
		double y_coord = (texcoords.y * scale - half_scale) + offset.y;

		double r = 0;

		Complex z;
		Complex coord;
		coord.re = x_coord;
		coord.im = y_coord;

		for(int i = 0; i < its; ++i){
			z = add(mul(z, z), coord);
			double val = sqrt(z.re * z.re + z.im * z.im);
			if (val > 2.0f) {
				double its_f = double(its);
				r = i / its_f;
				break;
			}
		}
		color = vec4(r, r, r, 1.0);
	}
	)";
#endif

u32 shader_load(const char* vert_shader, const char* frag_shader, GLint vert_length, GLint frag_length) {

	GLuint vs_id = glCreateShader(GL_VERTEX_SHADER);
	GLuint fs_id = glCreateShader(GL_FRAGMENT_SHADER);

	GLint compile_status;

	const GLchar* p_v[1] = { vert_shader };
	glShaderSource(vs_id, 1, p_v, &vert_length);

	const GLchar* p_f[1] = { frag_shader };
	glShaderSource(fs_id, 1, p_f, &frag_length);

	glCompileShader(vs_id);
	glGetShaderiv(vs_id, GL_COMPILE_STATUS, &compile_status);
	if (!compile_status) {
		char error_buffer[512] = { 0 };
		glGetShaderInfoLog(vs_id, sizeof(error_buffer), NULL, error_buffer);
		fprintf(stdout, "%s", error_buffer);
		return -1;
	}

	glCompileShader(fs_id);
	glGetShaderiv(fs_id, GL_COMPILE_STATUS, &compile_status);
	if (!compile_status) {
		char error_buffer[512] = { 0 };
		glGetShaderInfoLog(fs_id, sizeof(error_buffer) - 1, NULL, error_buffer);
		fprintf(stdout, "%s", error_buffer);
		return -1;
	}

	GLuint shader_id = glCreateProgram();
	glAttachShader(shader_id, vs_id);
	glAttachShader(shader_id, fs_id);
	glDeleteShader(vs_id);
	glDeleteShader(fs_id);
	glLinkProgram(shader_id);

	glGetProgramiv(shader_id, GL_LINK_STATUS, &compile_status);
	if (compile_status == 0) {
		GLchar error_buffer[512] = { 0 };
		glGetProgramInfoLog(shader_id, sizeof(error_buffer) - 1, NULL, error_buffer);
		fprintf(stdout, "%s", error_buffer);
		return -1;
	}

	glValidateProgram(shader_id);
	return shader_id;
}

#define OPENGL_MAJOR_VERSION 3
#define OPENGL_MINOR_VERSION 1

#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_FLAGS_ARB                   0x2094

#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))

struct Vertex3D {
	hm::vec3 position;
	hm::vec2 texture_coord;
	hm::vec4 color;
};

struct Quad_2D {
	hm::vec3 position;
	GLuint vao, vbo, ebo;
	u32 num_indices;
};

struct Window_State {
	int width, height;
	HWND handle;
	WINDOWPLACEMENT g_wpPrev;
	HDC device_context;
	HGLRC rendering_context;
};

Window_State wstate;

void quad_create(Quad_2D* quad, hm::vec3 pos, r32 width, r32 height) {
	glGenVertexArrays(1, &quad->vao);
	glBindVertexArray(quad->vao);
	glGenBuffers(1, &quad->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, quad->vbo);
	glGenBuffers(1, &quad->ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad->ebo);

	quad->position = pos;
	quad->num_indices = 6;

	u16 indices[6] = { 0,   1,  2,  2,  1,  3 };

	Vertex3D vertices[4] = {
		{ pos,									{ 0.0f, 0.0f }, hm::vec4(0.0f, 0.0f, 0.0f, 0.0f) },
		{ pos + hm::vec3(width, 0.0f, 0.0f),	{ 1.0f, 0.0f }, hm::vec4(0.0f, 0.0f, 0.0f, 0.0f) },
		{ pos + hm::vec3(0.0f, height, 0.0f),	{ 0.0f, 1.0f }, hm::vec4(0.0f, 0.0f, 0.0f, 0.0f) },
		{ pos + hm::vec3(width, height, 0.0f),	{ 1.0f, 1.0f }, hm::vec4(0.0f, 0.0f, 0.0f, 0.0f) }
	};

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)&((Vertex3D*)0)->texture_coord);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)&((Vertex3D*)0)->color);

	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
}

u32 its = 256;
r64 scale = 2.0f;
r64 xoff = 0.0;
r64 yoff = 0.0;

void quad_render(u32 shader_id, Quad_2D* quad, u32 texture) {
	glUseProgram(shader_id);
	hm::mat4 projection = hm::mat4::ortho(0, wstate.width, 0, wstate.height);
	glUniformMatrix4fv(glGetUniformLocation(shader_id, "projection"), 1, GL_TRUE, projection.data);
	glUniform1d(glGetUniformLocation(shader_id, "scale"), scale);
	glUniform1i(glGetUniformLocation(shader_id, "its"), its);
	glUniform2d(glGetUniformLocation(shader_id, "offset"), xoff, yoff);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glBindVertexArray(quad->vao);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glDrawElements(GL_TRIANGLES, quad->num_indices, GL_UNSIGNED_SHORT, 0);

	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
}

int init_opengl(Window_State* win_state) {
	int PixelFormat;
	win_state->device_context = GetDC(win_state->handle);

	PIXELFORMATDESCRIPTOR pfd;
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.iLayerType = PFD_MAIN_PLANE;
	pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
	pfd.cDepthBits = 32;
	pfd.cColorBits = 24;
	pfd.iPixelType = PFD_TYPE_RGBA;

	PixelFormat = ChoosePixelFormat(win_state->device_context, &pfd);
	if (!SetPixelFormat(win_state->device_context, PixelFormat, &pfd)) {
		fprintf(stdout, "Error setting pixel format.");
		return -1;
	}

	HGLRC temp_context = wglCreateContext(win_state->device_context);
	BOOL error = wglMakeCurrent(win_state->device_context, temp_context);

	int attribs[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, OPENGL_MAJOR_VERSION,
		WGL_CONTEXT_MINOR_VERSION_ARB, OPENGL_MINOR_VERSION,
		WGL_CONTEXT_FLAGS_ARB, 0,
		0
	};
	HGLRC(WINAPI* wglCreateContextAttribsARB)(HDC hDC, HGLRC hShareContext, int *attribList);
	wglCreateContextAttribsARB = (HGLRC(WINAPI*)(HDC, HGLRC, int *))wglGetProcAddress("wglCreateContextAttribsARB");
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(temp_context);
	win_state->rendering_context = wglCreateContextAttribsARB(win_state->device_context, 0, attribs);
	wglMakeCurrent(win_state->device_context, win_state->rendering_context);
	return 0;
}

LRESULT CALLBACK WndProc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_KILLFOCUS: {
	// clear keyboard state
	}break;
	case WM_CREATE: break;
	case WM_CLOSE: PostQuitMessage(0); break;
	case WM_SIZE: {
		RECT r;
		GetClientRect(window, &r);
		wstate.width = r.right - r.left;
		wstate.height = r.bottom - r.top;
		glViewport(0, 0, wstate.width, wstate.height);
	} break;
	case WM_DROPFILES: {
	}break;
	default:
		return DefWindowProc(window, msg, wparam, lparam);
	}
	return 0;
}

struct Complex {
	r64 re;
	r64 im;

	Complex() : re(0.0f), im(0.0f) {}
	Complex(r64 real, r64 imag) : re(real), im(imag){}

	inline Complex operator*(Complex r) {
		// (a + bi) * (c + di)
		// ac + adi + cbi - bd
		// ac - bd + i(ad + cb)
		Complex result;
		result.re = this->re * r.re - this->im * r.im;
		result.im = this->re * r.im + this->im * r.re;
		return result;
	}

	inline Complex operator+(Complex r) {
		// (a + bi) + (c + di)
		// (a + c) + (b + d)i
		Complex result;
		result.re = this->re + r.re;
		result.im = this->im + r.im;
		return result;
	}
};

struct Timer {
	LARGE_INTEGER frequency;

	Timer() {
		QueryPerformanceFrequency(&frequency);
	}

	double get_time() {
		LARGE_INTEGER pcount = {};
		QueryPerformanceCounter(&pcount);
		return double(pcount.QuadPart) / double(frequency.QuadPart) * 1000.0;
	}

};

void modify_image(u8* image, u32 width, u32 height, r32 scale, r64 xoff, r64 yoff) {
	for (u32 y = 0; y < height; ++y) {
		for (u32 x = 0; x < width; ++x) {

			r64 half_scale = scale / 2.0;

			u32 index = (y * width * 4) + x * 4;
			r64 x_coord = ((r64)x / width)  * scale - half_scale;
			r64 y_coord = ((r64)y / height) * scale - half_scale;

			x_coord += xoff;
			y_coord += yoff;

			u8 r = 0, g = 0, b = 0;

			Complex z;
			Complex coord(x_coord, y_coord);

			for (u32 it = 0; it < its; ++it) {
				z = z * z + coord;
				r64 v = z.re * z.re + z.im * z.im;
				if (sqrt(v) > 2.0) {
					r = g = b = it;
					break;
				}
			}
			*(u32*)(image + index) = 0xff000000 | (r << 16) | (g << 8) | b;
			//(*(u32(*)[800][800])image)[y][x] = 0xff000000 | (r << 16) | (g << 8) | b;
		}
	}
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line, int cmd_show)
{
	Timer timer;

	WNDCLASSEX window_class;
	window_class.cbSize = sizeof(WNDCLASSEX);
	window_class.style = 0;
	window_class.lpfnWndProc = WndProc;
	window_class.cbClsExtra = 0;
	window_class.cbWndExtra = 0;
	window_class.hInstance = instance;
	window_class.hIcon = LoadIcon(0, IDI_SHIELD);//MAKEINTRESOURCE(NULL));
	window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
	window_class.hbrBackground = 0;
	window_class.lpszMenuName = NULL;
	window_class.lpszClassName = TEXT("HoEXditor_Class");
	window_class.hIconSm = NULL;

	if (!RegisterClassEx(&window_class)) fprintf(stdout, "Error creating window class.\n", 0);

	// alloc console
#if 1
	AllocConsole();
	FILE* pCout;
	freopen_s(&pCout, "CONOUT$", "w", stdout);
#endif

	u32 window_style_exflags = WS_EX_ACCEPTFILES | WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	u32 window_style_flags = WS_OVERLAPPEDWINDOW|WS_VISIBLE;
	// Note: Client area must be correct, so windows needs to get the WindowRect
	// area depending on the style of the window
	RECT window_rect = { 0 };
	window_rect.right = 800;
	window_rect.bottom = 800;
	AdjustWindowRectEx(&window_rect, window_style_flags, FALSE, window_style_exflags);

	wstate.g_wpPrev.length = sizeof(WINDOWPLACEMENT);
	wstate.width = window_rect.right - window_rect.left;
	wstate.height = window_rect.bottom - window_rect.top;

	wstate.handle = CreateWindowEx(
		window_style_exflags,
		window_class.lpszClassName,
		TEXT("HoImageEdit"),
		window_style_flags,
		CW_USEDEFAULT, CW_USEDEFAULT,
		wstate.width, wstate.height, NULL, NULL, instance, NULL
	);
	if (!wstate.handle) fprintf(stdout, "Error criating window context.\n", 0);

	if (init_opengl(&wstate) == -1) {
		fprintf(stdout, "Error initializing opengl!\n");
		return -1;
	}
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		fprintf(stdout, "Error, could not initialize Glew\n");
		return -1;
	}
	void(*wglSwapIntervalEXT)(int) = (void(*)(int))wglGetProcAddress("wglSwapIntervalEXT");
	wglSwapIntervalEXT(1);

	bool running = true;
	MSG msg;

	// Track mouse events
	TRACKMOUSEEVENT mouse_event = { 0 };
	mouse_event.cbSize = sizeof(mouse_event);
	mouse_event.dwFlags = TME_LEAVE;
	mouse_event.dwHoverTime = HOVER_DEFAULT;
	mouse_event.hwndTrack = wstate.handle;

	u32 shader_id = shader_load(quad_vshader, quad_fshader, sizeof(quad_vshader) - 1, sizeof(quad_fshader) - 1);

	s32 width = 800;
	s32 height = 800;
	u8* image = (u8*)calloc(1, sizeof(u32) * width * height);

	double start = timer.get_time();
	modify_image(image, width, height, scale, xoff, yoff);
	double end = timer.get_time();
	printf("elapsed: %f ms.\n", end - start);

	u32 image_id = -1;
	glGenTextures(1, &image_id);
	glBindTexture(GL_TEXTURE_2D, image_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	Quad_2D quad;
	quad_create(&quad, hm::vec3(0.0f, 0.0f, 0.0f), width, height);
	
#if 0
	int n = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &n);
	for (int i = 0; i < n; ++i) {
		const GLubyte* val = glGetStringi(GL_EXTENSIONS, i);
		printf("%s\n", val);
	}
#endif

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

	while (running) {
		TrackMouseEvent(&mouse_event);
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0) {
			if (msg.message == WM_QUIT) {
				running = false;
				continue;
			}
			switch (msg.message) {
			case WM_KEYDOWN: {
				int key = msg.wParam;
				int mod = msg.lParam;
				bool modify = false;

				if (key == 'R') {
					scale -= scale * 0.1f;
					modify = true;
				}
				if (key == 'F') {
					scale += scale * 0.1f;
					modify = true;
				}
				if (key == VK_RIGHT) {
					xoff += 0.05f * scale;
					modify = true;
				}
				if (key == VK_LEFT) {
					xoff -= 0.05f* scale;
					modify = true;
				}
				if (key == VK_UP) {
					yoff += 0.05f* scale;
					modify = true;
				}
				if (key == VK_DOWN) {
					yoff -= 0.05f* scale;
					modify = true;
				}

				if (key == 'E') {
					its += 10;
					modify = true;
				}
				if (key == 'Q') {
					its -= 10;
					modify = true;
				}

				if (modify) {
					//double start = timer.get_time();
					//modify_image(image, width, height, scale, xoff, yoff);
					//double end = timer.get_time();
					//printf("elapsed: %f ms.\n", end - start);
					//modify_image(image, width, height, scale, xoff, yoff);
					glTextureSubImage2D(image_id, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, image);
				}
			} break;
			case WM_KEYUP: {
				int key = msg.wParam;
			} break;
			case WM_MOUSEMOVE: {
				int x = GET_X_LPARAM(msg.lParam);
				int y = GET_Y_LPARAM(msg.lParam);
			} break;
			case WM_LBUTTONDOWN: {
				int x = GET_X_LPARAM(msg.lParam);
				int y = GET_Y_LPARAM(msg.lParam);
			} break;
			case WM_CHAR: {
				int key = msg.wParam;
			} break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
		quad_render(shader_id, &quad, image_id);

		SwapBuffers(wstate.device_context);
	}

	return 0;
}