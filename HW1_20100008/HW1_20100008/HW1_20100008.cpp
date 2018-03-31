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

#define CAKE_FIRE 0
#define CAKE_CANDLE 1
#define CAKE_BODY 2
#define CAKE_BOTTOM 3
#define CAKE_DECORATE 4

GLfloat cake_fire[4][2] = { { -0.5, 14.0 }, { -0.5, 13.0 }, { 0.5, 13.0 }, { 0.5, 14.0 } };
GLfloat cake_candle[4][2] = { { -1.0, 8.0 } ,{ -1.0, 13.0 },{ 1.0, 13.0 },{ 1.0, 8.0 } };
GLfloat cake_body[4][2] = { { 8.0, 5.0 }, { -8.0, 5.0} ,{ -8.0, 8.0 }, { 8.0, 8.0 } };
GLfloat cake_bottom[4][2] = { { -10.0, 1.0 },{ -10.0, 5.0 },{ 10.0, 5.0 }, {10.0, 1.0} };
GLfloat cake_decorate[4][2] = { { -10.0, 0.0 },{ -10.0, 1.0 },{ 10.0, 1.0 },{ 10.0, 0.0 } };

GLfloat cake_color[5][3] = {
	{ 255 / 255.0f, 0 / 255.0f, 0 / 255.0f },
	{ 255 / 255.0f, 204 / 255.0f, 0 / 255.0f },
	{ 255 / 255.0f, 102 / 255.0f, 255 / 255.0f },
	{ 255 / 255.0f, 102 / 255.0f, 255 / 255.0f },
	{ 102 / 255.0f, 51 / 255.0f, 0 / 255.0f }
};
GLuint VBO_cake, VAO_cake;

void prepare_cake() {
	int size = sizeof(cake_fire);
	GLsizeiptr buffer_size = sizeof(cake_fire) * 5;

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_cake);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_cake);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, size, cake_fire);
	glBufferSubData(GL_ARRAY_BUFFER, size, size, cake_candle);
	glBufferSubData(GL_ARRAY_BUFFER, size * 2, size, cake_body);
	glBufferSubData(GL_ARRAY_BUFFER, size * 3, size, cake_bottom);
	glBufferSubData(GL_ARRAY_BUFFER, size * 4, size, cake_decorate);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_cake);
	glBindVertexArray(VAO_cake);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_cake);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_cake() {
	glBindVertexArray(VAO_cake);

	glUniform3fv(loc_primitive_color, 1, cake_color[CAKE_FIRE]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glUniform3fv(loc_primitive_color, 1, cake_color[CAKE_CANDLE]);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 8);

	glUniform3fv(loc_primitive_color, 1, cake_color[CAKE_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 12);

	glUniform3fv(loc_primitive_color, 1, cake_color[CAKE_BOTTOM]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 16);

	glUniform3fv(loc_primitive_color, 1, cake_color[CAKE_DECORATE]);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 20);

	glBindVertexArray(0);
}

#define SWORD_BODY 0
#define SWORD_BODY2 1
#define SWORD_HEAD 2
#define SWORD_HEAD2 3
#define SWORD_IN 4
#define SWORD_DOWN 5
#define SWORD_BODY_IN 6

GLfloat sword_body[4][2] = { { -6.0, 0.0 },{ -6.0, -4.0 },{ 6.0, -4.0 },{ 6.0, 0.0 } };
GLfloat sword_body2[4][2] = { { -2.0, -4.0 },{ -2.0, -6.0 } ,{ 2.0, -6.0 },{ 2.0, -4.0 } };
GLfloat sword_head[4][2] = { { -2.0, 0.0 },{ -2.0, 16.0 } ,{ 2.0, 16.0 },{ 2.0, 0.0 } };
GLfloat sword_head2[3][2] = { { -2.0, 16.0 },{ 0.0, 19.46 } ,{ 2.0, 16.0 } };
GLfloat sword_in[4][2] = { { -0.3, 0.7 },{ -0.3, 15.3 } ,{ 0.3, 15.3 },{ 0.3, 0.7 } };
GLfloat sword_down[4][2] = { { -2.0, -6.0 } ,{ 2.0, -6.0 },{ 4.0, -8.0 },{ -4.0, -8.0 } };
GLfloat sword_body_in[4][2] = { { 0.0, -1.0 } ,{ 1.0, -2.732 },{ 0.0, -4.464 },{ -1.0, -2.732 } };

GLfloat sword_color[7][3] = {
	{ 139 / 255.0f, 69 / 255.0f, 19 / 255.0f },
{ 139 / 255.0f, 69 / 255.0f, 19 / 255.0f },
{ 155 / 255.0f, 155 / 255.0f, 155 / 255.0f },
{ 155 / 255.0f, 155 / 255.0f, 155 / 255.0f },
{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f },
{ 139 / 255.0f, 69 / 255.0f, 19 / 255.0f },
{ 255 / 255.0f, 0 / 255.0f, 0 / 255.0f }
};

GLuint VBO_sword, VAO_sword;
void prepare_sword() {
	GLsizeiptr buffer_size = sizeof(sword_body) + sizeof(sword_body2) + sizeof(sword_head) + sizeof(sword_head2) + sizeof(sword_in) + sizeof(sword_down) + sizeof(sword_body_in);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_sword);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_sword);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(sword_body), sword_body);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_body), sizeof(sword_body2), sword_body2);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_body) + sizeof(sword_body2), sizeof(sword_head), sword_head);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_body) + sizeof(sword_body2) + sizeof(sword_head), sizeof(sword_head2), sword_head2);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_body) + sizeof(sword_body2) + sizeof(sword_head) + sizeof(sword_head2), sizeof(sword_in), sword_in);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_body) + sizeof(sword_body2) + sizeof(sword_head) + sizeof(sword_head2) + sizeof(sword_in), sizeof(sword_down), sword_down);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_body) + sizeof(sword_body2) + sizeof(sword_head) + sizeof(sword_head2) + sizeof(sword_in) + sizeof(sword_down), sizeof(sword_body_in), sword_body_in);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_sword);
	glBindVertexArray(VAO_sword);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_sword);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_sword() {
	glBindVertexArray(VAO_sword);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_BODY2]);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_HEAD]);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_HEAD2]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 3);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_IN]);
	glDrawArrays(GL_TRIANGLE_FAN, 15, 4);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_DOWN]);
	glDrawArrays(GL_TRIANGLE_FAN, 19, 4);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_BODY_IN]);
	glDrawArrays(GL_TRIANGLE_FAN, 23, 4);

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

GLfloat fox_fixed_head_ear_left[3][2] = { {-4.0,11.0},{-2.0,9.0},{-3.0,8.0} };
GLfloat fox_fixed_head_ear_right[3][2] = { {4.0,11.0},{2.0,9.0},{3.0,8.0} };
GLfloat fox_fixed_head_top[4][2] = { { -4.0,6.0 },{-2.0,9.0},{2.0,9.0},{4.0,6.0} };
GLfloat fox_fixed_head_bottom[3][2] = { {-4.0,6.0},{0.0,2.0},{4.0,6.0} };
GLfloat fox_fixed_body_left[4][2] = { {-3.0,-2.0},{-3.0,5.0},{0.0,2.0},{0.0,-2.0} };
GLfloat fox_fixed_body_right[4][2] = { {3.0,-2.0 },{3.0,5.0},{0.0,2.0},{0.0,-2.0} };


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
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//			FOX_ARM_1								                       //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define FOX_ARM_1_LEFT 0
#define FOX_ARM_1_RIGHT 1

GLfloat fox_arm_1_left[4][2] = { {3.0,5.0},{3.0,3.0},{7.0,1.0},{7.0,3.0} };
GLfloat fox_arm_1_right[4][2] = { {-3.0,5.0},{-3.0,3.0},{-7.0,1.0},{-7.0,3.0} };

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
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//			FOX_ARM_2							                       //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define FOX_ARM_2_TOP_LEFT 0
#define FOX_ARM_2_BOTTOM_LEFT 1
#define FOX_ARM_2_TOP_RIGHT 2
#define FOX_ARM_2_BOTTOM_RIGHT 3

GLfloat fox_arm_2_top_left[4][2] = { { -3.0,3.0 },{ -3.0,5.0 },{ -6.0,3.0 },{ -6.0,1.0 } };
GLfloat fox_arm_2_bottom_left[4][2] = { { -4.0,-1.0 },{ -4.0,1.0 },{ -6.0,3.0 },{ -6.0,1.0 } };
GLfloat fox_arm_2_top_right[4][2] = { { 3.0,3.0 },{ 3.0,5.0 },{ 6.0,3.0 },{ 6.0,1.0 } };
GLfloat fox_arm_2_bottom_right[4][2] = { { 4.0,-1.0 },{ 4.0,1.0 },{ 6.0,3.0 },{ 6.0,1.0 } };

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
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//			FOX_LEG_SHOES							                       //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define FOX_LEG_LEFT 0
#define FOX_LEG_RIGHT 1
#define FOX_SHOES_LEFT 2
#define FOX_SHOES_RIGHT 3

GLfloat fox_leg_left[4][2] = { { -5.0,-6.0 },{ -2.0,-6.0 },{ 0.0,-2.0 },{ -3.0,-2.0 } };
GLfloat fox_leg_right[4][2] = { { 5.0,-6.0 },{ 2.0,-6.0 },{ 0.0,-2.0 },{ 3.0,-2.0 } };
GLfloat fox_shoes_left[4][2] = { {-2.0,-6.0},{-2.0,-8.0},{-6.0,-8.0},{-6.0,-6.0} };
GLfloat fox_shoes_right[4][2] = { {2.0,-6.0},{2.0,-8.0},{6.0,-8.0},{6.0,-6.0} };

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

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0f, 2.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_hat();

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-300.0f, -5.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(5.0f, 5.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_cake();

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(300.0f, 10.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3.5f, 3.5f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_sword();

	//////////////////DRAW_FOX BELOW//////////////////////////
	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(500.0f, 10.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(7.0f, 7.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_fox_fixed();

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(500.0f, 10.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(7.0f, 7.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	//draw_fox_arm_1();

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(500.0f, 10.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(7.0f, 7.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_fox_arm_2();

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(500.0f, 10.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(7.0f, 7.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_fox_leg_shoes();


	glFlush();
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
	prepare_hat();
	prepare_cake();
	prepare_sword();
	prepare_fox_fixed();
	prepare_fox_arm_1();
	prepare_fox_arm_2();
	prepare_fox_leg_shoes();
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


