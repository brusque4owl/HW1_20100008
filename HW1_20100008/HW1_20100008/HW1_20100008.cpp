#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "Shaders/LoadShaders.h"
GLuint h_ShaderProgram; // handle to shader program
GLint loc_ModelViewProjectionMatrix, loc_primitive_color; // indices of uniform variables

// include glm/*.hpp only if necessary
//#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, ortho, etc.
glm::mat4 ModelViewProjectionMatrix;
glm::mat4 ViewMatrix, ProjectionMatrix, ViewProjectionMatrix;

#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f
#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))

#define LOC_VERTEX 0
///////////////////////////////////////////////////////////////////////////////////////
/////////////////////     variables for objects          //////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
GLfloat fox_place = 0.0f;

///////////////////////////////////////////////////////////////////////////////////////
int win_width = 0, win_height = 0;
float centerx = 0.0f, centery = 0.0f, rotate_angle = 0.0f;

GLfloat axes[4][2];
GLfloat axes_color[3] = { 0.0f, 0.0f, 0.0f };
GLuint VBO_axes, VAO_axes;

void prepare_axes(void) { // Draw axes in their MC.
	axes[0][0] = -win_width / 2.5f; axes[0][1] = 0.0f;
	axes[1][0] = win_width / 2.5f; axes[1][1] = 0.0f;
	axes[2][0] = 0.0f; axes[2][1] = -win_height / 2.5f;
	axes[3][0] = 0.0f; axes[3][1] = win_height / 2.5f;

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_axes);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_axes);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axes), axes, GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_axes);
	glBindVertexArray(VAO_axes);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_axes);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void update_axes(void) {
	axes[0][0] = -win_width / 2.25f; axes[1][0] = win_width / 2.25f;
	axes[2][1] = -win_height / 2.25f;
	axes[3][1] = win_height / 2.25f;

	glBindBuffer(GL_ARRAY_BUFFER, VBO_axes);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axes), axes, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void draw_axes(void) {
	glUniform3fv(loc_primitive_color, 1, axes_color);
	glBindVertexArray(VAO_axes);
	glDrawArrays(GL_LINES, 0, 4);
	glBindVertexArray(0);
}

GLfloat line[2][2];
GLfloat line_color[3] = { 1.0f, 0.0f, 0.0f };
GLuint VBO_line, VAO_line;

void prepare_line(void) { 	// y = x - win_height/4
	line[0][0] = (1.0f / 4.0f - 1.0f / 2.5f)*win_height;
	line[0][1] = (1.0f / 4.0f - 1.0f / 2.5f)*win_height - win_height / 4.0f;
	line[1][0] = win_width / 2.5f;
	line[1][1] = win_width / 2.5f - win_height / 4.0f;

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_line);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_line);
	glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_line);
	glBindVertexArray(VAO_line);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_line);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void update_line(void) { 	// y = x - win_height/4
	line[0][0] = (1.0f / 4.0f - 1.0f / 2.5f)*win_height;
	line[0][1] = (1.0f / 4.0f - 1.0f / 2.5f)*win_height - win_height / 4.0f;
	line[1][0] = win_width / 2.5f;
	line[1][1] = win_width / 2.5f - win_height / 4.0f;

	glBindBuffer(GL_ARRAY_BUFFER, VBO_line);
	glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void draw_line(void) { // Draw line in its MC.
	// y = x - win_height/4
	glUniform3fv(loc_primitive_color, 1, line_color);
	glBindVertexArray(VAO_line);
	glDrawArrays(GL_LINES, 0, 2);
	glBindVertexArray(0);
}

/// start : originated from 3.1.2DObjects_GLSL
///////////////////////////////////////////////////////////////////////////////////
/////////////// airplane
///////////////////////////////////////////////////////////////////////////////////

#define AIRPLANE_BIG_WING 0
#define AIRPLANE_SMALL_WING 1
#define AIRPLANE_BODY 2
#define AIRPLANE_BACK 3
#define AIRPLANE_SIDEWINDER1 4
#define AIRPLANE_SIDEWINDER2 5
#define AIRPLANE_CENTER 6
GLfloat big_wing[6][2] = { { 0.0, 0.0 },{ -20.0, 15.0 },{ -20.0, 20.0 },{ 0.0, 23.0 },{ 20.0, 20.0 },{ 20.0, 15.0 } };
GLfloat small_wing[6][2] = { { 0.0, -18.0 },{ -11.0, -12.0 },{ -12.0, -7.0 },{ 0.0, -10.0 },{ 12.0, -7.0 },{ 11.0, -12.0 } };
GLfloat body[5][2] = { { 0.0, -25.0 },{ -6.0, 0.0 },{ -6.0, 22.0 },{ 6.0, 22.0 },{ 6.0, 0.0 } };
GLfloat back[5][2] = { { 0.0, 25.0 },{ -7.0, 24.0 },{ -7.0, 21.0 },{ 7.0, 21.0 },{ 7.0, 24.0 } };
GLfloat sidewinder1[5][2] = { { -20.0, 10.0 },{ -18.0, 3.0 },{ -16.0, 10.0 },{ -18.0, 20.0 },{ -20.0, 20.0 } };
GLfloat sidewinder2[5][2] = { { 20.0, 10.0 },{ 18.0, 3.0 },{ 16.0, 10.0 },{ 18.0, 20.0 },{ 20.0, 20.0 } };
GLfloat center[1][2] = { { 0.0, 0.0 } };
GLfloat airplane_color[7][3] = {
	{ 150 / 255.0f, 129 / 255.0f, 183 / 255.0f },  // big_wing
{ 245 / 255.0f, 211 / 255.0f,   0 / 255.0f },  // small_wing
{ 111 / 255.0f,  85 / 255.0f, 157 / 255.0f },  // body
{ 150 / 255.0f, 129 / 255.0f, 183 / 255.0f },  // back
{ 245 / 255.0f, 211 / 255.0f,   0 / 255.0f },  // sidewinder1
{ 245 / 255.0f, 211 / 255.0f,   0 / 255.0f },  // sidewinder2
{ 255 / 255.0f,   0 / 255.0f,   0 / 255.0f }   // center
};

GLuint VBO_airplane, VAO_airplane;

int airplane_clock = 0;
float airplane_s_factor = 1.0f;

void prepare_airplane() {
	GLsizeiptr buffer_size = sizeof(big_wing) + sizeof(small_wing) + sizeof(body) + sizeof(back)
		+ sizeof(sidewinder1) + sizeof(sidewinder2) + sizeof(center);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_airplane);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_airplane);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(big_wing), big_wing);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(big_wing), sizeof(small_wing), small_wing);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(big_wing) + sizeof(small_wing), sizeof(body), body);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(big_wing) + sizeof(small_wing) + sizeof(body), sizeof(back), back);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(big_wing) + sizeof(small_wing) + sizeof(body) + sizeof(back),
		sizeof(sidewinder1), sidewinder1);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(big_wing) + sizeof(small_wing) + sizeof(body) + sizeof(back)
		+ sizeof(sidewinder1), sizeof(sidewinder2), sidewinder2);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(big_wing) + sizeof(small_wing) + sizeof(body) + sizeof(back)
		+ sizeof(sidewinder1) + sizeof(sidewinder2), sizeof(center), center);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_airplane);
	glBindVertexArray(VAO_airplane);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_airplane);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_airplane() { // Draw airplane in its MC.
	glBindVertexArray(VAO_airplane);

	glUniform3fv(loc_primitive_color, 1, airplane_color[AIRPLANE_BIG_WING]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 6);

	glUniform3fv(loc_primitive_color, 1, airplane_color[AIRPLANE_SMALL_WING]);
	glDrawArrays(GL_TRIANGLE_FAN, 6, 6);

	glUniform3fv(loc_primitive_color, 1, airplane_color[AIRPLANE_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 5);

	glUniform3fv(loc_primitive_color, 1, airplane_color[AIRPLANE_BACK]);
	glDrawArrays(GL_TRIANGLE_FAN, 17, 5);

	glUniform3fv(loc_primitive_color, 1, airplane_color[AIRPLANE_SIDEWINDER1]);
	glDrawArrays(GL_TRIANGLE_FAN, 22, 5);

	glUniform3fv(loc_primitive_color, 1, airplane_color[AIRPLANE_SIDEWINDER2]);
	glDrawArrays(GL_TRIANGLE_FAN, 27, 5);

	glUniform3fv(loc_primitive_color, 1, airplane_color[AIRPLANE_CENTER]);
	glPointSize(5.0);
	glDrawArrays(GL_POINTS, 32, 1);
	glPointSize(1.0);
	glBindVertexArray(0);
}

///////////////////////////////////////////////////////////////////////////////////
/////////////// house
///////////////////////////////////////////////////////////////////////////////////
#define HOUSE_ROOF 0
#define HOUSE_BODY 1
#define HOUSE_CHIMNEY 2
#define HOUSE_DOOR 3
#define HOUSE_WINDOW 4

GLfloat roof[3][2] = { { -12.0, 0.0 },{ 0.0, 12.0 },{ 12.0, 0.0 } };
GLfloat house_body[4][2] = { { -12.0, -14.0 },{ -12.0, 0.0 },{ 12.0, 0.0 },{ 12.0, -14.0 } };
GLfloat chimney[4][2] = { { 6.0, 6.0 },{ 6.0, 14.0 },{ 10.0, 14.0 },{ 10.0, 2.0 } };
GLfloat door[4][2] = { { -8.0, -14.0 },{ -8.0, -8.0 },{ -4.0, -8.0 },{ -4.0, -14.0 } };
GLfloat window[4][2] = { { 4.0, -6.0 },{ 4.0, -2.0 },{ 8.0, -2.0 },{ 8.0, -6.0 } };

GLfloat house_color[5][3] = {
	{ 200 / 255.0f, 39 / 255.0f, 42 / 255.0f },
{ 235 / 255.0f, 225 / 255.0f, 196 / 255.0f },
{ 255 / 255.0f, 0 / 255.0f, 0 / 255.0f },
{ 233 / 255.0f, 113 / 255.0f, 23 / 255.0f },
{ 44 / 255.0f, 180 / 255.0f, 49 / 255.0f }
};

GLuint VBO_house, VAO_house;
void prepare_house() {
	GLsizeiptr buffer_size = sizeof(roof) + sizeof(house_body) + sizeof(chimney) + sizeof(door)
		+ sizeof(window);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_house);	//generate buffer object names

	glBindBuffer(GL_ARRAY_BUFFER, VBO_house);	//bind a named buffer object
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory(create a new data store for a buffer object)

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(roof), roof);	//updates a subset of a buffer object's data store
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(roof), sizeof(house_body), house_body);	// 2nd parameter, offset, is increasing by adding object
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(roof) + sizeof(house_body), sizeof(chimney), chimney);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(roof) + sizeof(house_body) + sizeof(chimney), sizeof(door), door);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(roof) + sizeof(house_body) + sizeof(chimney) + sizeof(door),
		sizeof(window), window);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_house);
	glBindVertexArray(VAO_house);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_house);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_house() {
	glBindVertexArray(VAO_house);

	glUniform3fv(loc_primitive_color, 1, house_color[HOUSE_ROOF]);	//loc_primitive_color는 몰라도됨. specifies the number of elements that are to be modified
	glDrawArrays(GL_TRIANGLE_FAN, 0, 3);	// VBO에서 glBufferSubData로 정한 순서대로 나옴 

	glUniform3fv(loc_primitive_color, 1, house_color[HOUSE_BODY]);	// 하나씩 그리므로 2nd para가 1이 됨.
	glDrawArrays(GL_TRIANGLE_FAN, 3, 4);

	glUniform3fv(loc_primitive_color, 1, house_color[HOUSE_CHIMNEY]);	// 3rd para는 uniform var인 loc_prim_color바꾸는 값
	glDrawArrays(GL_TRIANGLE_FAN, 7, 4);

	glUniform3fv(loc_primitive_color, 1, house_color[HOUSE_DOOR]);
	glDrawArrays(GL_TRIANGLE_FAN, 11, 4);

	glUniform3fv(loc_primitive_color, 1, house_color[HOUSE_WINDOW]);
	glDrawArrays(GL_TRIANGLE_FAN, 15, 4);

	glBindVertexArray(0);
}

///////////////////////////////////////////////////////////////////////////////////
///////////////  car 
///////////////////////////////////////////////////////////////////////////////////
#define CAR_BODY 0
#define CAR_FRAME 1
#define CAR_WINDOW 2
#define CAR_LEFT_LIGHT 3
#define CAR_RIGHT_LIGHT 4
#define CAR_LEFT_WHEEL 5
#define CAR_RIGHT_WHEEL 6

GLfloat car_body[4][2] = { { -16.0, -8.0 },{ -16.0, 0.0 },{ 16.0, 0.0 },{ 16.0, -8.0 } };
GLfloat car_frame[4][2] = { { -10.0, 0.0 },{ -10.0, 10.0 },{ 10.0, 10.0 },{ 10.0, 0.0 } };
GLfloat car_window[4][2] = { { -8.0, 0.0 },{ -8.0, 8.0 },{ 8.0, 8.0 },{ 8.0, 0.0 } };
GLfloat car_left_light[4][2] = { { -9.0, -6.0 },{ -10.0, -5.0 },{ -9.0, -4.0 },{ -8.0, -5.0 } };
GLfloat car_right_light[4][2] = { { 9.0, -6.0 },{ 8.0, -5.0 },{ 9.0, -4.0 },{ 10.0, -5.0 } };
GLfloat car_left_wheel[4][2] = { { -10.0, -12.0 },{ -10.0, -8.0 },{ -6.0, -8.0 },{ -6.0, -12.0 } };
GLfloat car_right_wheel[4][2] = { { 6.0, -12.0 },{ 6.0, -8.0 },{ 10.0, -8.0 },{ 10.0, -12.0 } };

GLfloat car_color[7][3] = {
	{ 0 / 255.0f, 149 / 255.0f, 159 / 255.0f },
{ 0 / 255.0f, 149 / 255.0f, 159 / 255.0f },
{ 216 / 255.0f, 208 / 255.0f, 174 / 255.0f },
{ 249 / 255.0f, 244 / 255.0f, 0 / 255.0f },
{ 249 / 255.0f, 244 / 255.0f, 0 / 255.0f },
{ 21 / 255.0f, 30 / 255.0f, 26 / 255.0f },
{ 21 / 255.0f, 30 / 255.0f, 26 / 255.0f }
};

GLuint VBO_car, VAO_car;
void prepare_car() {
	GLsizeiptr buffer_size = sizeof(car_body) + sizeof(car_frame) + sizeof(car_window) + sizeof(car_left_light)
		+ sizeof(car_right_light) + sizeof(car_left_wheel) + sizeof(car_right_wheel);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_car);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_car);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(car_body), car_body);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car_body), sizeof(car_frame), car_frame);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car_body) + sizeof(car_frame), sizeof(car_window), car_window);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car_body) + sizeof(car_frame) + sizeof(car_window), sizeof(car_left_light), car_left_light);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car_body) + sizeof(car_frame) + sizeof(car_window) + sizeof(car_left_light),
		sizeof(car_right_light), car_right_light);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car_body) + sizeof(car_frame) + sizeof(car_window) + sizeof(car_left_light)
		+ sizeof(car_right_light), sizeof(car_left_wheel), car_left_wheel);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car_body) + sizeof(car_frame) + sizeof(car_window) + sizeof(car_left_light)
		+ sizeof(car_right_light) + sizeof(car_left_wheel), sizeof(car_right_wheel), car_right_wheel);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_car);
	glBindVertexArray(VAO_car);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_car);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_car() {
	glBindVertexArray(VAO_car);

	glUniform3fv(loc_primitive_color, 1, car_color[CAR_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glUniform3fv(loc_primitive_color, 1, car_color[CAR_FRAME]);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);

	glUniform3fv(loc_primitive_color, 1, car_color[CAR_WINDOW]);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);

	glUniform3fv(loc_primitive_color, 1, car_color[CAR_LEFT_LIGHT]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);

	glUniform3fv(loc_primitive_color, 1, car_color[CAR_RIGHT_LIGHT]);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);

	glUniform3fv(loc_primitive_color, 1, car_color[CAR_LEFT_WHEEL]);
	glDrawArrays(GL_TRIANGLE_FAN, 20, 4);

	glUniform3fv(loc_primitive_color, 1, car_color[CAR_RIGHT_WHEEL]);
	glDrawArrays(GL_TRIANGLE_FAN, 24, 4);

	glBindVertexArray(0);
}

/// finish : originated from 3.1.2DObjects_GLSL
/////////////////////////////////////////////////
/////////////////////////////////////////////////

#define HAT_LEAF 0
#define HAT_BODY 1
#define HAT_STRIP 2
#define HAT_BOTTOM 3

GLfloat hat_leaf[4][2] = { { 3.0, 20.0 },{ 3.0, 28.0 },{ 9.0, 32.0 },{ 9.0, 24.0 } };
GLfloat hat_body[4][2] = { {-19.5, 2.0}, {19.5, 2.0}, {15.0, 20.0}, {-15.0, 20.0} };
GLfloat hat_strip[4][2] = { { -20.0, 0.0 },{ 20.0, 0.0 },{ 19.5, 2.0 },{ -19.5, 2.0 } };
GLfloat hat_bottom[4][2] = { { 25.0, 0.0 },{ -25.0, 0.0 },{ -25.0, -4.0 },{ 25.0, -4.0 } };

GLfloat hat_color[4][3] = {
	{ 167 / 255.0f, 255 / 255.0f, 55 / 255.0f },
	{ 255 / 255.0f, 144 / 255.0f, 32 / 255.0f },
	{ 255 / 255.0f, 40 / 255.0f, 33 / 255.0f },
	{ 255 / 255.0f, 144 / 255.0f, 32 / 255.0f }
};
GLuint VBO_hat, VAO_hat;

void prepare_hat() {
	GLsizeiptr buffer_size = sizeof(hat_leaf) + sizeof(hat_body) + sizeof(hat_strip) + sizeof(hat_bottom);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_hat);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_hat);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(hat_leaf), hat_leaf);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(hat_leaf), sizeof(hat_body), hat_body);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(hat_leaf) + sizeof(hat_body), sizeof(hat_strip), hat_strip);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(hat_leaf) + sizeof(hat_body) + sizeof(hat_strip), sizeof(hat_bottom), hat_bottom);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_hat);
	glBindVertexArray(VAO_hat);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_hat);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_hat() {
	glBindVertexArray(VAO_hat);

	glUniform3fv(loc_primitive_color, 1, hat_color[HAT_LEAF]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glUniform3fv(loc_primitive_color, 1, hat_color[HAT_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 8);

	glUniform3fv(loc_primitive_color, 1, hat_color[HAT_STRIP]);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 12);

	glUniform3fv(loc_primitive_color, 1, hat_color[HAT_BOTTOM]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 16);

	glBindVertexArray(0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//			FOX_FIXED								                       //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define FOX_FIXED_HEAD_EAR_LEFT 0
#define FOX_FIXED_HEAD_EAR_RIGHT 1
#define FOX_FIXED_HEAD_TOP 2
#define FOX_FIXED_HEAD_BOTTOM 3
#define FOX_FIXED_BODY_LEFT 4
#define FOX_FIXED_BODY_RIGHT 5
/* 원본
GLfloat fox_fixed_head_ear_left[3][2] = { {-4.0,11.0},{-2.0,9.0},{-3.0,8.0} };
GLfloat fox_fixed_head_ear_right[3][2] = { {4.0,11.0},{2.0,9.0},{3.0,8.0} };
GLfloat fox_fixed_head_top[4][2] = { { -4.0,6.0 },{-2.0,9.0},{2.0,9.0},{4.0,6.0} };
GLfloat fox_fixed_head_bottom[3][2] = { {-4.0,6.0},{0.0,2.0},{4.0,6.0} };
GLfloat fox_fixed_body_left[4][2] = { {-3.0,-2.0},{-3.0,5.0},{0.0,2.0},{0.0,-2.0} };
GLfloat fox_fixed_body_right[4][2] = { {3.0,-2.0 },{3.0,5.0},{0.0,2.0},{0.0,-2.0} };
*/
// 아래는 수정본
GLfloat fox_fixed_head_ear_left[3][2] = { { -16.0,44.0 },{ -8.0,36.0 },{ -12.0,32.0 } };
GLfloat fox_fixed_head_ear_right[3][2] = { { 16.0,44.0 },{ 8.0,36.0 },{ 12.0,32.0 } };
GLfloat fox_fixed_head_top[4][2] = { { -16.0,24.0 },{ -8.0,36.0 },{ 8.0,36.0 },{ 16.0,24.0 } };
GLfloat fox_fixed_head_bottom[3][2] = { { -16.0,24.0 },{ 0.0,8.0 },{ 16.0,24.0 } };
GLfloat fox_fixed_body_left[4][2] = { { -12.0,-8.0 },{ -12.0,20.0 },{ 0.0,8.0 },{ 0.0,-8.0 } };
GLfloat fox_fixed_body_right[4][2] = { { 12.0,-8.0 },{ 12.0,20.0 },{ 0.0,8.0 },{ 0.0,-8.0 } };


GLfloat fox_fixed_color[6][3] = {
	{ 253 / 255.0f, 238 / 255.0f, 245 / 255.0f },	// left ear
	{ 253 / 255.0f, 238 / 255.0f, 245 / 255.0f },	// right ear
	{ 194 / 255.0f, 9 / 255.0f, 6 / 255.0f },		// top
	{ 215 / 255.0f, 181 / 255.0f, 117 / 255.0f },	// bottom
	{ 213 / 255.0f, 209 / 255.0f, 122 / 255.0f },	// left body
	{ 213 / 255.0f, 209 / 255.0f, 122 / 255.0f },	// right body
};

GLuint VBO_fox_fixed, VAO_fox_fixed;
void prepare_fox_fixed() {
	GLsizeiptr buffer_size = sizeof(fox_fixed_head_ear_left) + sizeof(fox_fixed_head_ear_right) + sizeof(fox_fixed_head_top) + sizeof(fox_fixed_head_bottom)
		+ sizeof(fox_fixed_body_left) + sizeof(fox_fixed_body_right);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_fox_fixed);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_fox_fixed);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(fox_fixed_head_ear_left), fox_fixed_head_ear_left);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fox_fixed_head_ear_left), sizeof(fox_fixed_head_ear_right), fox_fixed_head_ear_right);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fox_fixed_head_ear_left) + sizeof(fox_fixed_head_ear_right), sizeof(fox_fixed_head_top), fox_fixed_head_top);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fox_fixed_head_ear_left) + sizeof(fox_fixed_head_ear_right) + sizeof(fox_fixed_head_top), sizeof(fox_fixed_head_bottom), fox_fixed_head_bottom);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fox_fixed_head_ear_left) + sizeof(fox_fixed_head_ear_right) + sizeof(fox_fixed_head_top) + sizeof(fox_fixed_head_bottom), 
										sizeof(fox_fixed_body_left), fox_fixed_body_left);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fox_fixed_head_ear_left) + sizeof(fox_fixed_head_ear_right) + sizeof(fox_fixed_head_top) + sizeof(fox_fixed_head_bottom)
		+ sizeof(fox_fixed_body_left), sizeof(fox_fixed_body_right), fox_fixed_body_right);


	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_fox_fixed);
	glBindVertexArray(VAO_fox_fixed);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_fox_fixed);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_fox_fixed() {
	glBindVertexArray(VAO_fox_fixed);

	glUniform3fv(loc_primitive_color, 1, fox_fixed_color[FOX_FIXED_HEAD_EAR_LEFT]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 3);

	glUniform3fv(loc_primitive_color, 1, fox_fixed_color[FOX_FIXED_HEAD_EAR_RIGHT]);
	glDrawArrays(GL_TRIANGLE_FAN, 3, 3);

	glUniform3fv(loc_primitive_color, 1, fox_fixed_color[FOX_FIXED_HEAD_TOP]);
	glDrawArrays(GL_TRIANGLE_FAN, 6, 4);

	glUniform3fv(loc_primitive_color, 1, fox_fixed_color[FOX_FIXED_HEAD_BOTTOM]);
	glDrawArrays(GL_TRIANGLE_FAN, 10, 3);

	glUniform3fv(loc_primitive_color, 1, fox_fixed_color[FOX_FIXED_BODY_LEFT]);
	glDrawArrays(GL_TRIANGLE_FAN, 13, 4);

	glUniform3fv(loc_primitive_color, 1, fox_fixed_color[FOX_FIXED_BODY_RIGHT]);
	glDrawArrays(GL_TRIANGLE_FAN, 17, 4);

	
	glBindVertexArray(0);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//			FOX_ARM_1								                       //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define FOX_ARM_1_LEFT 0
#define FOX_ARM_1_RIGHT 1
/* 원본 
GLfloat fox_arm_1_left[4][2] = { {3.0,5.0},{3.0,3.0},{7.0,1.0},{7.0,3.0} };
GLfloat fox_arm_1_right[4][2] = { {-3.0,5.0},{-3.0,3.0},{-7.0,1.0},{-7.0,3.0} };
*/
//아래는 수정본
GLfloat fox_arm_1_left[4][2] = { { 12.0,20.0 },{ 12.0,12.0 },{ 28.0,4.0 },{ 28.0,12.0 } };
GLfloat fox_arm_1_right[4][2] = { { -12.0,20.0 },{ -12.0,12.0 },{ -28.0,4.0 },{ -28.0,12.0 } };

GLfloat fox_arm_1_color[2][3] = {
	{ 184 / 255.0f, 12 / 255.0f, 0 / 255.0f },	// left arm top
	{ 184 / 255.0f, 12 / 255.0f, 0 / 255.0f }	// right arm top
};

GLuint VBO_fox_arm_1, VAO_fox_arm_1;
void prepare_fox_arm_1() {
	GLsizeiptr buffer_size = sizeof(fox_arm_1_left) + sizeof(fox_arm_1_right);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_fox_arm_1);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_fox_arm_1);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(fox_arm_1_left), fox_arm_1_left);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fox_arm_1_left), sizeof(fox_arm_1_right), fox_arm_1_right);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_fox_arm_1);
	glBindVertexArray(VAO_fox_arm_1);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_fox_arm_1);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_fox_arm_1() {
	glBindVertexArray(VAO_fox_arm_1);

	glUniform3fv(loc_primitive_color, 1, fox_arm_1_color[FOX_ARM_1_LEFT]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glUniform3fv(loc_primitive_color, 1, fox_arm_1_color[FOX_ARM_1_RIGHT]);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);

	glBindVertexArray(0);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//			FOX_ARM_2							                       //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define FOX_ARM_2_TOP_LEFT 0
#define FOX_ARM_2_BOTTOM_LEFT 1
#define FOX_ARM_2_TOP_RIGHT 2
#define FOX_ARM_2_BOTTOM_RIGHT 3
/* 원본
GLfloat fox_arm_2_top_left[4][2] = { { -3.0,3.0 },{ -3.0,5.0 },{ -6.0,3.0 },{ -6.0,1.0 } };
GLfloat fox_arm_2_bottom_left[4][2] = { { -4.0,-1.0 },{ -4.0,1.0 },{ -6.0,3.0 },{ -6.0,1.0 } };
GLfloat fox_arm_2_top_right[4][2] = { { 3.0,3.0 },{ 3.0,5.0 },{ 6.0,3.0 },{ 6.0,1.0 } };
GLfloat fox_arm_2_bottom_right[4][2] = { { 4.0,-1.0 },{ 4.0,1.0 },{ 6.0,3.0 },{ 6.0,1.0 } };
*/
//아래는 수정본
GLfloat fox_arm_2_top_left[4][2] = { { -12.0,12.0 },{ -12.0,20.0 },{ -24.0,12.0 },{ -24.0,4.0 } };
GLfloat fox_arm_2_bottom_left[4][2] = { { -16.0,-4.0 },{ -16.0,4.0 },{ -24.0,12.0 },{ -24.0,4.0 } };
GLfloat fox_arm_2_top_right[4][2] = { { 12.0,12.0 },{ 12.0,20.0 },{ 24.0,12.0 },{ 24.0,4.0 } };
GLfloat fox_arm_2_bottom_right[4][2] = { { 16.0,-4.0 },{ 16.0,4.0 },{ 24.0,12.0 },{ 24.0,4.0 } };

GLfloat fox_arm_2_color[4][3] = {
	{ 184 / 255.0f, 12 / 255.0f, 0 / 255.0f },	// left arm top_left
	{ 184 / 255.0f, 12 / 255.0f, 0 / 255.0f },	// right arm bottom_left
	{ 184 / 255.0f, 12 / 255.0f, 0 / 255.0f },	// left arm top_right
	{ 184 / 255.0f, 12 / 255.0f, 0 / 255.0f }	// right arm bottom_right
};

GLuint VBO_fox_arm_2, VAO_fox_arm_2;
void prepare_fox_arm_2() {
	GLsizeiptr buffer_size = sizeof(fox_arm_2_top_left) + sizeof(fox_arm_2_bottom_left) + sizeof(fox_arm_2_top_right) + sizeof(fox_arm_2_bottom_right);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_fox_arm_2);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_fox_arm_2);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(fox_arm_2_top_left), fox_arm_2_top_left);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fox_arm_2_top_left), sizeof(fox_arm_2_bottom_left), fox_arm_2_bottom_left);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fox_arm_2_top_left) + sizeof(fox_arm_2_bottom_left), sizeof(fox_arm_2_top_right), fox_arm_2_top_right);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fox_arm_2_top_left) + sizeof(fox_arm_2_bottom_left) + sizeof(fox_arm_2_top_right), sizeof(fox_arm_2_bottom_right), fox_arm_2_bottom_right);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_fox_arm_2);
	glBindVertexArray(VAO_fox_arm_2);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_fox_arm_2);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_fox_arm_2() {
	glBindVertexArray(VAO_fox_arm_2);

	glUniform3fv(loc_primitive_color, 1, fox_arm_2_color[FOX_ARM_2_TOP_LEFT]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glUniform3fv(loc_primitive_color, 1, fox_arm_2_color[FOX_ARM_2_BOTTOM_LEFT]);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);

	glUniform3fv(loc_primitive_color, 1, fox_arm_2_color[FOX_ARM_2_TOP_RIGHT]);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);

	glUniform3fv(loc_primitive_color, 1, fox_arm_2_color[FOX_ARM_2_BOTTOM_RIGHT]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);

	glBindVertexArray(0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//			FOX_LEG_SHOES							                       //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define FOX_LEG_LEFT 0
#define FOX_LEG_RIGHT 1
#define FOX_SHOES_LEFT 2
#define FOX_SHOES_RIGHT 3
/* 원본
GLfloat fox_leg_left[4][2] = { { -5.0,-6.0 },{ -2.0,-6.0 },{ 0.0,-2.0 },{ -3.0,-2.0 } };
GLfloat fox_leg_right[4][2] = { { 5.0,-6.0 },{ 2.0,-6.0 },{ 0.0,-2.0 },{ 3.0,-2.0 } };
GLfloat fox_shoes_left[4][2] = { {-2.0,-6.0},{-2.0,-8.0},{-6.0,-8.0},{-6.0,-6.0} };
GLfloat fox_shoes_right[4][2] = { {2.0,-6.0},{2.0,-8.0},{6.0,-8.0},{6.0,-6.0} };
*/
//아래는 수정본
GLfloat fox_leg_left[4][2] = { { -20.0,-24.0 },{ -8.0,-24.0 },{ 0.0,-8.0 },{ -12.0,-8.0 } };
GLfloat fox_leg_right[4][2] = { { 20.0,-24.0 },{ 8.0,-24.0 },{ 0.0,-8.0 },{ 12.0,-8.0 } };
GLfloat fox_shoes_left[4][2] = { { -8.0,-24.0 },{ -8.0,-32.0 },{ -24.0,-32.0 },{ -24.0,-24.0 } };
GLfloat fox_shoes_right[4][2] = { { 8.0,-24.0 },{ 8.0,-32.0 },{ 24.0,-32.0 },{ 24.0,-24.0 } };

GLfloat fox_leg_shoes_color[4][3] = {
	{ 110 / 255.0f, 57 / 255.0f, 73 / 255.0f },	// leg_left
	{ 110 / 255.0f, 57 / 255.0f, 73 / 255.0f },	// leg_right
	{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f },	// shoes_left
	{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f }	// shoes_right
};

GLuint VBO_fox_leg_shoes, VAO_fox_leg_shoes;
void prepare_fox_leg_shoes() {
	GLsizeiptr buffer_size = sizeof(fox_leg_left) + sizeof(fox_leg_right) + sizeof(fox_shoes_left) + sizeof(fox_shoes_right);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_fox_leg_shoes);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_fox_leg_shoes);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(fox_leg_left), fox_leg_left);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fox_leg_left), sizeof(fox_leg_right), fox_leg_right);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fox_leg_left) + sizeof(fox_leg_right), sizeof(fox_shoes_left), fox_shoes_left);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fox_leg_left) + sizeof(fox_leg_right) + sizeof(fox_shoes_left), sizeof(fox_shoes_right), fox_shoes_right);
	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_fox_leg_shoes);
	glBindVertexArray(VAO_fox_leg_shoes);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_fox_leg_shoes);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_fox_leg_shoes() {
	glBindVertexArray(VAO_fox_leg_shoes);

	glUniform3fv(loc_primitive_color, 1, fox_leg_shoes_color[FOX_LEG_LEFT]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glUniform3fv(loc_primitive_color, 1, fox_leg_shoes_color[FOX_LEG_RIGHT]);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);

	glUniform3fv(loc_primitive_color, 1, fox_leg_shoes_color[FOX_SHOES_LEFT]);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);

	glUniform3fv(loc_primitive_color, 1, fox_leg_shoes_color[FOX_SHOES_RIGHT]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);

	glBindVertexArray(0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//			FOX_FACES_BASIC							                       //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define FOX_BIG_EYES_LEFT 0
#define FOX_BIG_EYES_RIGHT 1
#define FOX_SMALL_EYES_LEFT 2
#define FOX_SAMLL_EYES_RIGHT 3
#define FOX_NOSE 4
#define FOX_MOUTH_LINE_1 5
#define FOX_MOUTH_LINE_2 6
#define FOX_MOUTH_LINE_3 7
#define FOX_MOUTH_LINE_4 8

GLfloat fox_big_eyes_left[4][2] = { { -2.0,26.0 },{ -2.0,32.0 },{ -8.0,32.0 },{ -8.0,26.0 } };
GLfloat fox_big_eyes_right[4][2] = { { 2.0,26.0 },{ 2.0,32.0 },{ 8.0,32.0 },{ 8.0,26.0 } };
GLfloat fox_small_eyes_left[4][2] = { { -2.0,26.0 },{ -2.0,30.0 },{ -6.0,30.0 },{ -6.0,26.0 } };
GLfloat fox_small_eyes_right[4][2] = { { 2.0,26.0 },{ 2.0,30.0 },{ 6.0,30.0 },{ 6.0,26.0 } };
GLfloat fox_nose[6][2] = { {-2.0,16.0},{-4.0,18.0},{-2.0,20.0},{2.0,20.0},{4.0,18.0},{2.0,16.0} };
GLfloat fox_mouth_line_1[2][2] = { {0.0,16.0},{0.0,12.0} };		// vertical
GLfloat fox_mouth_line_2[2][2] = { {-2.0,12.0},{2.0,12.0} };	// horizontal
GLfloat fox_mouth_line_3[2][2] = { {-2.0,12.0},{-4.0,14.0} };	// left
GLfloat fox_mouth_line_4[2][2] = { {2.0,12.0},{4.0,14.0} };		// right

GLfloat fox_faces_basic_color[9][3] = {
	{ 70 / 255.0f, 90 / 255.0f, 53 / 255.0f },	// big_eyes_left
	{ 70 / 255.0f, 90 / 255.0f, 53 / 255.0f },	// big_eyes_right
	{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f },		// small_eyes_left
	{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f },		// small_eyes_right
	{ 39 / 255.0f, 13 / 255.0f, 12 / 255.0f },	// nose
	{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f },		// mouth_line_1
	{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f },		// mouth_line_2
	{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f },		// mouth_line_3
	{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f }		// mouth_line_4
};

GLuint VBO_fox_faces_basic, VAO_fox_faces_basic;
void prepare_fox_faces_basic() {
	GLsizeiptr buffer_size = sizeof(fox_big_eyes_left) + sizeof(fox_big_eyes_right) + sizeof(fox_small_eyes_left) + sizeof(fox_small_eyes_right)
		+ sizeof(fox_nose) + sizeof(fox_mouth_line_1) + sizeof(fox_mouth_line_2) + sizeof(fox_mouth_line_3) + sizeof(fox_mouth_line_4);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_fox_faces_basic);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_fox_faces_basic);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(fox_big_eyes_left), fox_big_eyes_left);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fox_big_eyes_left), sizeof(fox_big_eyes_right), fox_big_eyes_right);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fox_big_eyes_left) + sizeof(fox_big_eyes_right), sizeof(fox_small_eyes_left), fox_small_eyes_left);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fox_big_eyes_left) + sizeof(fox_big_eyes_right) + sizeof(fox_small_eyes_left), sizeof(fox_small_eyes_right),
		fox_small_eyes_right);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fox_big_eyes_left) + sizeof(fox_big_eyes_right) + sizeof(fox_small_eyes_left) + sizeof(fox_small_eyes_right),
		sizeof(fox_nose), fox_nose);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fox_big_eyes_left) + sizeof(fox_big_eyes_right) + sizeof(fox_small_eyes_left) + sizeof(fox_small_eyes_right) +
		sizeof(fox_nose), sizeof(fox_mouth_line_1), fox_mouth_line_1);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fox_big_eyes_left) + sizeof(fox_big_eyes_right) + sizeof(fox_small_eyes_left) + sizeof(fox_small_eyes_right) +
		sizeof(fox_nose) + sizeof(fox_mouth_line_1), sizeof(fox_mouth_line_2), fox_mouth_line_2);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fox_big_eyes_left) + sizeof(fox_big_eyes_right) + sizeof(fox_small_eyes_left) + sizeof(fox_small_eyes_right) +
		sizeof(fox_nose) + sizeof(fox_mouth_line_1) + sizeof(fox_mouth_line_2), sizeof(fox_mouth_line_3), fox_mouth_line_3);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fox_big_eyes_left) + sizeof(fox_big_eyes_right) + sizeof(fox_small_eyes_left) + sizeof(fox_small_eyes_right) +
		sizeof(fox_nose) + sizeof(fox_mouth_line_1) + sizeof(fox_mouth_line_2) + sizeof(fox_mouth_line_3), sizeof(fox_mouth_line_4), fox_mouth_line_4);
	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_fox_faces_basic);
	glBindVertexArray(VAO_fox_faces_basic);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_fox_faces_basic);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_fox_faces_basic() {
	glBindVertexArray(VAO_fox_faces_basic);

	glUniform3fv(loc_primitive_color, 1, fox_faces_basic_color[FOX_BIG_EYES_LEFT]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glUniform3fv(loc_primitive_color, 1, fox_faces_basic_color[FOX_BIG_EYES_RIGHT]);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);

	glUniform3fv(loc_primitive_color, 1, fox_faces_basic_color[FOX_SMALL_EYES_LEFT]);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);

	glUniform3fv(loc_primitive_color, 1, fox_faces_basic_color[FOX_SAMLL_EYES_RIGHT]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);

	glUniform3fv(loc_primitive_color, 1, fox_faces_basic_color[FOX_NOSE]);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 6);

	glUniform3fv(loc_primitive_color, 1, fox_faces_basic_color[FOX_MOUTH_LINE_1]);
	glDrawArrays(GL_LINES, 22, 2);

	glUniform3fv(loc_primitive_color, 1, fox_faces_basic_color[FOX_MOUTH_LINE_2]);
	glDrawArrays(GL_LINES, 24, 2);

	glUniform3fv(loc_primitive_color, 1, fox_faces_basic_color[FOX_MOUTH_LINE_3]);
	glDrawArrays(GL_LINES, 26, 2);

	glUniform3fv(loc_primitive_color, 1, fox_faces_basic_color[FOX_MOUTH_LINE_4]);
	glDrawArrays(GL_LINES, 28, 2);

	glBindVertexArray(0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//			FOX_FACES_CRASH							                       //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define FOX_CRASH_EYES_LEFT 0
#define FOX_CRASH_EYES_RIGHT 1
#define FOX_CRASH_NOSE 2
#define FOX_CRASH_MOUTH_LINE_1 3
#define FOX_CRASH_MOUTH_LINE_2 4
#define FOX_CRASH_MOUTH_LINE_3 5
#define FOX_CRASH_MOUTH_LINE_4 6

/*
/////////////////////////////////////////////
GLfloat axes[4][2];
GLfloat axes_color[3] = { 0.0f, 0.0f, 0.0f };
GLuint VBO_axes, VAO_axes;

void prepare_axes(void) { // Draw axes in their MC.
	axes[0][0] = -win_width / 2.5f; axes[0][1] = 0.0f;
	axes[1][0] = win_width / 2.5f; axes[1][1] = 0.0f;
	axes[2][0] = 0.0f; axes[2][1] = -win_height / 2.5f;
	axes[3][0] = 0.0f; axes[3][1] = win_height / 2.5f;
//////////////////////////////////////////////////
*/
GLfloat fox_crash_eyes_left[4][2] = { { -2.0,26.0 },{ -8.0,32.0 },{ -2.0,32.0 },{ -8.0,26.0 } };
GLfloat fox_crash_eyes_right[4][2] = { { 2.0,26.0 },{ 8.0,32.0 },{ 2.0,32.0 },{ 8.0,26.0 } };
GLfloat fox_crash_nose[6][2] = { { -2.0,16.0 },{ -4.0,18.0 },{ -2.0,20.0 },{ 2.0,20.0 },{ 4.0,18.0 },{ 2.0,16.0 } };
GLfloat fox_crash_mouth_line_1[2][2] = { { 0.0,16.0 },{ 0.0,12.0 } };		// vertical
GLfloat fox_crash_mouth_line_2[2][2] = { { -2.0,12.0 },{ 2.0,12.0 } };		// horizontal
GLfloat fox_crash_mouth_line_3[2][2] = { { -2.0,12.0 },{ -4.0,10.0 } };		// left
GLfloat fox_crash_mouth_line_4[2][2] = { { 2.0,12.0 },{ 4.0,10.0 } };		// right

GLfloat fox_faces_crash_color[7][3] = {
	{ 70 / 255.0f, 90 / 255.0f, 53 / 255.0f },	// big_eyes_left
	{ 70 / 255.0f, 90 / 255.0f, 53 / 255.0f },	// big_eyes_right
	{ 39 / 255.0f, 13 / 255.0f, 12 / 255.0f },	// nose
	{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f },		// mouth_line_1
	{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f },		// mouth_line_2
	{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f },		// mouth_line_3
	{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f }		// mouth_line_4
};

GLuint VBO_fox_faces_crash, VAO_fox_faces_crash;
void prepare_fox_faces_crash() {
	GLsizeiptr buffer_size = sizeof(fox_crash_eyes_left) + sizeof(fox_crash_eyes_right)	+ sizeof(fox_crash_nose) + sizeof(fox_crash_mouth_line_1)
		+ sizeof(fox_crash_mouth_line_2) + sizeof(fox_crash_mouth_line_3) + sizeof(fox_crash_mouth_line_4);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_fox_faces_crash);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_fox_faces_crash);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(fox_crash_eyes_left), fox_crash_eyes_left);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fox_crash_eyes_left), sizeof(fox_crash_eyes_right), fox_crash_eyes_right);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fox_crash_eyes_left) + sizeof(fox_crash_eyes_right), sizeof(fox_crash_nose), fox_crash_nose);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fox_crash_eyes_left) + sizeof(fox_crash_eyes_right) + sizeof(fox_crash_nose), sizeof(fox_crash_mouth_line_1), 
		fox_crash_mouth_line_1);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fox_crash_eyes_left) + sizeof(fox_crash_eyes_right) + sizeof(fox_crash_nose) + sizeof(fox_crash_mouth_line_1), 
		sizeof(fox_crash_mouth_line_2), fox_crash_mouth_line_2);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fox_crash_eyes_left) + sizeof(fox_crash_eyes_right) + sizeof(fox_crash_nose) + sizeof(fox_crash_mouth_line_1) +
		sizeof(fox_crash_mouth_line_2) , sizeof(fox_crash_mouth_line_3), fox_crash_mouth_line_3);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fox_crash_eyes_left) + sizeof(fox_crash_eyes_right) + sizeof(fox_crash_nose) + sizeof(fox_crash_mouth_line_1) + 
		sizeof(fox_crash_mouth_line_2) + sizeof(fox_crash_mouth_line_3) , sizeof(fox_crash_mouth_line_4), fox_crash_mouth_line_4);
	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_fox_faces_crash);
	glBindVertexArray(VAO_fox_faces_crash);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_fox_faces_crash);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_fox_faces_crash() {
	glBindVertexArray(VAO_fox_faces_crash);

	glUniform3fv(loc_primitive_color, 1, fox_faces_crash_color[FOX_CRASH_EYES_LEFT]);
	glDrawArrays(GL_LINES, 0, 4);

	glUniform3fv(loc_primitive_color, 1, fox_faces_crash_color[FOX_CRASH_EYES_RIGHT]);
	glDrawArrays(GL_LINES, 4, 4);

	glUniform3fv(loc_primitive_color, 1, fox_faces_crash_color[FOX_CRASH_NOSE]);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 6);

	glUniform3fv(loc_primitive_color, 1, fox_faces_crash_color[FOX_CRASH_MOUTH_LINE_1]);
	glDrawArrays(GL_LINES, 14, 2);

	glUniform3fv(loc_primitive_color, 1, fox_faces_crash_color[FOX_CRASH_MOUTH_LINE_2]);
	glDrawArrays(GL_LINES, 16, 2);

	glUniform3fv(loc_primitive_color, 1, fox_faces_crash_color[FOX_CRASH_MOUTH_LINE_3]);
	glDrawArrays(GL_LINES, 18, 2);

	glUniform3fv(loc_primitive_color, 1, fox_faces_crash_color[FOX_CRASH_MOUTH_LINE_4]);
	glDrawArrays(GL_LINES, 20, 2);

	glBindVertexArray(0);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void display(void) {
	int i;
	float x, r, s, delx, delr, dels;
	glm::mat4 ModelMatrix;

	glClear(GL_COLOR_BUFFER_BIT);

	ModelMatrix = glm::mat4(1.0f);
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_axes();

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-500.0f, 0.0f, 0.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_airplane();

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-100.0f, 0.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0f, 2.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_house();

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(100.0f, 0.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0f, 2.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_car();

	//////////////////DRAW_FOX BELOW//////////////////////////
	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(fox_place, 10.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0f, 2.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_fox_fixed();

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(fox_place, 10.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0f, 2.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	//draw_fox_arm_1();

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(fox_place, 10.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0f, 2.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_fox_arm_2();

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(fox_place, 10.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0f, 2.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_fox_leg_shoes();

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(fox_place, 10.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0f, 2.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	//draw_fox_faces_basic();

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(fox_place, 10.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0f, 2.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_fox_faces_crash();
/////////////////////////finish fox//////////////////////////////////////////////////////////////////////

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 85.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_hat();

	glFlush();
}

void timer(int value){
	glutPostRedisplay();
	glutTimerFunc(30, timer, value);
}
void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 27: // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups.
		break;
	}
}

void reshape(int width, int height) {
	win_width = width, win_height = height;

	glViewport(0, 0, win_width, win_height);
	ProjectionMatrix = glm::ortho(-win_width / 2.0, win_width / 2.0,
		-win_height / 2.0, win_height / 2.0, -1000.0, 1000.0);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	update_axes();
	update_line();

	glutPostRedisplay();
}

void cleanup(void) {
	glDeleteVertexArrays(1, &VAO_axes);
	glDeleteBuffers(1, &VBO_axes);

	glDeleteVertexArrays(1, &VAO_line);
	glDeleteBuffers(1, &VBO_line);
}

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);
	glutCloseFunc(cleanup);
	glutTimerFunc(30, timer, 30);
}

void prepare_shader_program(void) {
	ShaderInfo shader_info[3] = {
		{ GL_VERTEX_SHADER, "Shaders/simple.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/simple.frag" },
		{ GL_NONE, NULL }
	};

	h_ShaderProgram = LoadShaders(shader_info);
	glUseProgram(h_ShaderProgram);

	loc_ModelViewProjectionMatrix = glGetUniformLocation(h_ShaderProgram, "u_ModelViewProjectionMatrix");
	loc_primitive_color = glGetUniformLocation(h_ShaderProgram, "u_primitive_color");
}

void initialize_OpenGL(void) {
	glEnable(GL_MULTISAMPLE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glClearColor(255 / 255.0f, 255 / 255.0f, 177 / 255.0f, 1.0f);
	ViewMatrix = glm::mat4(1.0f);
}

void prepare_scene(void) {
	prepare_axes();
	prepare_airplane();
	prepare_house();
	prepare_car();
	prepare_hat();
	prepare_fox_fixed();
	prepare_fox_arm_1();
	prepare_fox_arm_2();
	prepare_fox_leg_shoes();
	prepare_fox_faces_basic();
	prepare_fox_faces_crash();

}

void initialize_renderer(void) {
	register_callbacks();
	prepare_shader_program();
	initialize_OpenGL();
	prepare_scene();
}

void initialize_glew(void) {
	GLenum error;

	glewExperimental = GL_TRUE;

	error = glewInit();
	if (error != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(error));
		exit(-1);
	}
	fprintf(stdout, "*********************************************************\n");
	fprintf(stdout, " - GLEW version supported: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, " - OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(stdout, " - OpenGL version supported: %s\n", glGetString(GL_VERSION));
	fprintf(stdout, "*********************************************************\n\n");
}

void greetings(char *program_name, char messages[][256], int n_message_lines) {
	fprintf(stdout, "**************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);
	fprintf(stdout, "    This program was coded for CSE4170 students\n");
	fprintf(stdout, "      of Dept. of Comp. Sci. & Eng., Sogang University.\n\n");

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n**************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 1
void main(int argc, char *argv[]) {
	char program_name[64] = "Sogang CSE4170 2DObjects_GLSL_3.1";
	char messages[N_MESSAGE_LINES][256] = {
		"    - Keys used: 'ESC' "
	};

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_MULTISAMPLE);
	glutInitWindowSize(1200, 300);
	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
}


