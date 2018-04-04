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
#define TOUCH_LEFT 0
#define TOUCH_RIGHT 1
#define TOUCH_DOWN 2
#define TOUCH_UP 3
#define TOUCH_NOTHING 4

//number of objects 0:airplane / 1:house / 2:car / 3:sword
#define AIRPLANE 0
#define HOUSE 1
#define CAR 2
#define SWORD 3

// 물체의 초기 좌표
GLfloat fox_centerx = 0.0f, fox_centery = 0.0f;
GLfloat airplane_centerx = 0.0f, airplane_centery = 0.0f;
GLfloat house_centerx = 0.0f, house_centery = 0.0f;
GLfloat car_centerx = 0.0f, car_centery = 0.0f;
GLfloat sword_centerx = 0.0f, sword_centery = 0.0f;

bool fox_crash = 0;	// 0 for not crash, 1 for crashed; it is used in display()
unsigned int set_key = 0;	// 0 for left, 1 for right, 2 for down, 3 for up

// 물체의 deltax, deltay 값
GLfloat airplane_deltax = -10.0, airplane_deltay = -10.0;
GLfloat house_deltax = -10.0, house_deltay = 10.0;
GLfloat car_deltax = 16.0, car_deltay = 7.0;
GLfloat sword_deltax = 7.0, sword_deltay = 3.0;

// collider에 사용할 구조체
struct collider_rect {
	GLfloat x0;
	GLfloat x1;
	GLfloat y0;
	GLfloat y1;
};

struct collider_tri {
	GLfloat v0;
	GLfloat v1;
	GLfloat v2;
};

int win_width = 0, win_height = 0;
float centerx = 0.0f, centery = 0.0f, rotate_angle = 0.0f;
int time_interval = 100;
///////////////////////////////////////////////////////////////////////////////////////
///////////////////       functions for objects			///////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
GLfloat setting_deltax(unsigned int object);
GLfloat setting_deltay(unsigned int object);
#define TWICE
#ifdef TWICE
	#define MULTIPLE 2.0
#else
	#define MULTIPLE 1.0
#endif
// 충돌 체크 함수
bool check_crash(struct collider_rect object1, struct collider_rect object2){
	if (object1.x0<object2.x1 && object1.x1>object2.x0 && object1.y0>object2.y1 && object1.y1<object2.y0) {		
		return true;
	}
	else return false;
}

GLfloat arr_endpoint[4][4]= {	{-20.0,20.0,25.0,-25.0},	// [object number][x0,x1,y0,y1]
								{-12.0,12.0,14.0,-14.0},
								{-16.0,16.0,10.0,-12.0 },
								{-6.0,6.0,19.46,-8.0}
							};

/*
// 윈도우에 닿으면 여우의 방향 자동 변경
if (fox_centerx<-win_width / 2.0f + MULTIPLE * 28.0f)	// left (여우의 팔 가장 왼쪽이 -28.0f)
	set_key = 1; // 1 for right
else if (fox_centerx>win_width / 2.0f - MULTIPLE * 28.0f) // right (여우의 팔 가장 오른쪽이 +28.0f)
set_key = 0; // 0 for left
else if (fox_centery<-win_height / 2.0f + MULTIPLE * 32.0f) // down  (여우의 신발 가장 아래가 -32.0f)
	set_key = 2; // 2 for up
else if (fox_centery>win_height / 2.0f - MULTIPLE * 56.0f) // up (여우의 모자 leaf 가장 위가 +56.0f)
set_key = 3; // 3 for down
*/
//윈도우와 물체 간의 충돌 체크
unsigned int check_direction(GLfloat object_centerx, GLfloat object_centery, int object_number){  
	if(object_centerx<-win_width/2.0f - MULTIPLE * arr_endpoint[object_number][0]) // touch left
		return TOUCH_LEFT;
	else if (object_centerx>win_width / 2.0f - MULTIPLE * arr_endpoint[object_number][1]) // touch right
		return TOUCH_RIGHT;
	else if (object_centery>win_height / 2.0f - MULTIPLE * arr_endpoint[object_number][2]) // touch up
		return TOUCH_UP;
	else if (object_centery<-win_height / 2.0f - MULTIPLE * arr_endpoint[object_number][3]) // touch down
		return TOUCH_DOWN;
	else								// not touch any place
		return TOUCH_NOTHING;
	/*
	if(object_centerx<-win_width/2.0f)	// touch left
		return TOUCH_LEFT;
	else if(object_centerx>win_width/2.0f) // touch right
		return TOUCH_RIGHT;
	else if(object_centery<-win_height/2.0f) // touch down
		return TOUCH_DOWN;
	else if(object_centery>win_height/2.0f) // touch up
		return TOUCH_UP;
	else								// not touch any place
		return TOUCH_NOTHING;
	*/
}

GLfloat setting_deltax(unsigned int object){
	if (object == 0)
		return airplane_deltax;
	else if (object == 1)
		return house_deltax;
	else if (object == 2)
		return car_deltax;
	else if (object == 3)
		return sword_deltax;
}
GLfloat setting_deltay(unsigned int object) {
	if (object == 0)
		return airplane_deltay;
	else if (object == 1)
		return house_deltay;
	else if (object == 2)
		return car_deltay;
	else if (object == 3)
		return sword_deltay;
}

// 물체의 이동 방향 수정(윈도우와 충돌시)
void modify_direction(GLfloat *deltax, GLfloat *deltay, unsigned int object){  // object 0:airplane / 1:house / 2:car / 3:sword
	unsigned int touch_place;
	switch(object){
	case AIRPLANE:
		touch_place = check_direction(airplane_centerx, airplane_centery, AIRPLANE);
		break;
	case HOUSE:
		touch_place = check_direction(house_centerx, house_centery, HOUSE);
		break;
	case CAR:
		touch_place = check_direction(car_centerx, car_centery, CAR);
		break;
	case SWORD:
		touch_place = check_direction(sword_centerx, sword_centery, SWORD);
		break;
	}
	if(touch_place==TOUCH_NOTHING) return;

	// 델타값 변화를 통한 방향 변화
	if(*deltax>0.0f && *deltay>0.0f){
		switch(touch_place){          // ↗
		case TOUCH_RIGHT:
			*deltax = -setting_deltax(object);
			break;
		case TOUCH_UP:
			*deltay = -setting_deltay(object);
			break;
		}
	}
	else if (*deltax>0.0f && *deltay<0.0f) { //↘
		switch (touch_place) {
		case TOUCH_RIGHT:
			*deltax = -setting_deltax(object);
			break;
		case TOUCH_DOWN:
			*deltay = -setting_deltay(object);
			break;
		}
	}
	else if (*deltax<0.0f && *deltay>0.0f) { // ↖
		switch (touch_place) {
		case TOUCH_LEFT:
			*deltax = -setting_deltax(object);
			break;
		case TOUCH_UP:
			*deltay = -setting_deltay(object);
			break;
		}
	}
	else if(*deltax<0.0f && *deltay<0.0f) {  // ↙
		switch (touch_place) {
		case TOUCH_LEFT:
			*deltax = -setting_deltax(object);
			break;
		case TOUCH_DOWN:
			*deltay = -setting_deltay(object);
			break;
		}
	}
	else{}
}

// 다른 함수에 의해 정해진 변화량만큼 물체 이동
void move_object(GLfloat deltax, GLfloat deltay, unsigned int object){
	if(object==AIRPLANE){
		airplane_centerx += deltax;
		airplane_centery += deltay;
	}
	else if(object==HOUSE){
		house_centerx += deltax;
		house_centery += deltay;
	}
	else if(object==CAR){
		car_centerx += deltax;
		car_centery += deltay;
	}
	else if(object==SWORD){
		sword_centerx += deltax;
		sword_centery += deltay;
	}

}
///////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////// 충돌체 - airplane     ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#define AIRPLANE_COLLIDER_UP 0
#define AIRPLANE_COLLIDER_DOWN 1
#define AIRPLANE_UP_X0 -20.0
#define AIRPLANE_UP_X1 20.0
#define AIRPLANE_UP_Y0 25.0
#define AIRPLANE_UP_Y1 3.0
#define AIRPLANE_DOWN_X0 -12.0
#define AIRPLANE_DOWN_X1 12.0
#define AIRPLANE_DOWN_Y0 3.0
#define AIRPLANE_DOWN_Y1 -25.0

GLfloat airplane_collider_up[4][2] = { { -20.0, 25.0 },{ -20.0, 3.0 },{ 20.0, 3.0 },{ 20.0, 25.0 } };
GLfloat airplane_collider_down[4][2] = { { -12.0, 3.0 },{ -12.0, -25.0 },{ 12.0, -25.0 },{ 12.0, 3.0 } };
struct collider_rect struct_airplane_up = { airplane_centerx + AIRPLANE_UP_X0,airplane_centerx + AIRPLANE_UP_X1,
airplane_centery + AIRPLANE_UP_Y0,airplane_centery + AIRPLANE_UP_Y1 };
struct collider_rect struct_airplane_down = { airplane_centerx + AIRPLANE_DOWN_X0,airplane_centerx + AIRPLANE_DOWN_X1,
airplane_centery + AIRPLANE_DOWN_Y0,airplane_centery + AIRPLANE_DOWN_Y1 };

GLfloat airplane_collider_color[2][3] = {
	{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f },
	{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f }
};

GLuint VBO_airplane_collider, VAO_airplane_collider;
void prepare_airplane_collider() {
	GLsizeiptr buffer_size = sizeof(airplane_collider_up) + sizeof(airplane_collider_down);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_airplane_collider);	//generate buffer object names

	glBindBuffer(GL_ARRAY_BUFFER, VBO_airplane_collider);	//bind a named buffer object
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory(create a new data store for a buffer object)

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(airplane_collider_up), airplane_collider_up);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(airplane_collider_up), sizeof(airplane_collider_down), airplane_collider_down);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_airplane_collider);
	glBindVertexArray(VAO_airplane_collider);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_airplane_collider);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_airplane_collider() {
	glBindVertexArray(VAO_airplane_collider);

	glUniform3fv(loc_primitive_color, 1, airplane_collider_color[AIRPLANE_COLLIDER_UP]);
	glDrawArrays(GL_LINE_LOOP, 0, 4);	// VBO에서 glBufferSubData로 정한 순서대로 나옴 

	glUniform3fv(loc_primitive_color, 1, airplane_collider_color[AIRPLANE_COLLIDER_DOWN]);
	glDrawArrays(GL_LINE_LOOP, 4, 4);	// VBO에서 glBufferSubData로 정한 순서대로 나옴 

	glBindVertexArray(0);
}

///////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////// 충돌체 - house     ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#define HOUSE_COLLIDER 0
#define HOUSE_X0 -12.0
#define HOUSE_X1 12.0
#define HOUSE_Y0 14.0
#define HOUSE_Y1 -14.0

GLfloat house_collider[4][2] = { { -12.0, -14.0 },{ 12.0, -14.0 },{ 12.0, 14.0 },{ -12.0, 14.0 } };
struct collider_rect struct_house = { house_centerx + HOUSE_X0,house_centerx + HOUSE_X1,
	house_centery + HOUSE_Y0,house_centery + HOUSE_Y1 };

GLfloat house_collider_color[1][3] = {
	{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f }
};

GLuint VBO_house_collider, VAO_house_collider;
void prepare_house_collider() {
	GLsizeiptr buffer_size = sizeof(house_collider);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_house_collider);	//generate buffer object names

	glBindBuffer(GL_ARRAY_BUFFER, VBO_house_collider);	//bind a named buffer object
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory(create a new data store for a buffer object)

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(house_collider), house_collider);	//updates a subset of a buffer object's data store
	
	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_house_collider);
	glBindVertexArray(VAO_house_collider);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_house_collider);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_house_collider() {
	glBindVertexArray(VAO_house_collider);

	glUniform3fv(loc_primitive_color, 1, house_collider_color[HOUSE_COLLIDER]);
	glDrawArrays(GL_LINE_LOOP, 0, 4);	// VBO에서 glBufferSubData로 정한 순서대로 나옴 

	glBindVertexArray(0);
}
///////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////// 충돌체 - car     ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#define CAR_COLLIDER 0
#define CAR_X0 -16.0
#define CAR_X1 16.0
#define CAR_Y0 10.0
#define CAR_Y1 -12.0

GLfloat car_collider[4][2] = { { -16.0, -12.0 },{ -16.0, 10.0 },{ 16.0, 10.0 },{ 16.0, -12.0 } };
struct collider_rect struct_car = { car_centerx + CAR_X0,car_centerx + CAR_X1,
	car_centery + CAR_Y0,car_centery + CAR_Y1 };

GLfloat car_collider_color[1][3] = {
	{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f }
};

GLuint VBO_car_collider, VAO_car_collider;
void prepare_car_collider() {
	GLsizeiptr buffer_size = sizeof(car_collider);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_car_collider);	//generate buffer object names

	glBindBuffer(GL_ARRAY_BUFFER, VBO_car_collider);	//bind a named buffer object
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory(create a new data store for a buffer object)

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(car_collider), car_collider);	//updates a subset of a buffer object's data store

																					// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_car_collider);
	glBindVertexArray(VAO_car_collider);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_car_collider);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_car_collider() {
	glBindVertexArray(VAO_car_collider);

	glUniform3fv(loc_primitive_color, 1, car_collider_color[CAR_COLLIDER]);
	glDrawArrays(GL_LINE_LOOP, 0, 4);	// VBO에서 glBufferSubData로 정한 순서대로 나옴 

	glBindVertexArray(0);
}

///////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////// 충돌체 - sword     ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#define SWORD_COLLIDER_UP 0
#define SWORD_COLLIDER_DOWN 1
#define SWORD_UP_X0 -2.0
#define SWORD_UP_X1 2.0
#define SWORD_UP_Y0 19.46
#define SWORD_UP_Y1 0.0
#define SWORD_DOWN_X0 -6.0
#define SWORD_DOWN_X1 6.0
#define SWORD_DOWN_Y0 0.0
#define SWORD_DOWN_Y1 -8.0


GLfloat sword_collider_up[4][2] = { { -2.0, 19.46 },{ -2.0, 0.0 },{ 2.0, 0.0 },{ 2.0, 19.46 } };
GLfloat sword_collider_down[4][2] = { { -6.0, 0.0 },{ -6.0, -8.0 },{ 6.0, -8.0 },{ 6.0, 0.0 } };

GLfloat sword_collider_color[2][3] = {
	{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f },
	{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f }
};
struct collider_rect struct_sword_up = { sword_centerx + SWORD_UP_X0,sword_centerx + SWORD_UP_X1,
	sword_centery + SWORD_UP_Y0,sword_centery + SWORD_UP_Y1 };
struct collider_rect struct_sword_down = { sword_centerx + SWORD_DOWN_X0,sword_centerx + SWORD_DOWN_X1,
	sword_centery + SWORD_DOWN_Y0,sword_centery + SWORD_DOWN_Y1 };

GLuint VBO_sword_collider, VAO_sword_collider;
void prepare_sword_collider() {
	GLsizeiptr buffer_size = sizeof(sword_collider_up) + sizeof(sword_collider_down);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_sword_collider);	//generate buffer object names

	glBindBuffer(GL_ARRAY_BUFFER, VBO_sword_collider);	//bind a named buffer object
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory(create a new data store for a buffer object)

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(sword_collider_up), sword_collider_up);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_collider_up), sizeof(sword_collider_down), sword_collider_down);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_sword_collider);
	glBindVertexArray(VAO_sword_collider);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_sword_collider);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_sword_collider() {
	glBindVertexArray(VAO_sword_collider);

	glUniform3fv(loc_primitive_color, 1, sword_collider_color[SWORD_COLLIDER_UP]);
	glDrawArrays(GL_LINE_LOOP, 0, 4);	// VBO에서 glBufferSubData로 정한 순서대로 나옴 

	glUniform3fv(loc_primitive_color, 1, sword_collider_color[SWORD_COLLIDER_DOWN]);
	glDrawArrays(GL_LINE_LOOP, 4, 4);	// VBO에서 glBufferSubData로 정한 순서대로 나옴 

	glBindVertexArray(0);
}
///////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////// 충돌체 - fox    ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#define FOX_COLLIDER_UP 0
#define FOX_COLLIDER_MID 1
#define FOX_COLLIDER_DOWN 2
#define FOX_UP_X0 -16.0
#define FOX_UP_X1 16.0
#define FOX_UP_Y0 50.0
#define FOX_UP_Y1 24.0
#define FOX_MID_X0 -28.0
#define FOX_MID_X1 28.0
#define FOX_MID_Y0 24.0
#define FOX_MID_Y1 -8.0
#define FOX_DOWN_X0 -24.0
#define FOX_DOWN_X1 24.0
#define FOX_DOWN_Y0 -8.0
#define FOX_DOWN_Y1 -32.0

GLfloat fox_collider_up[4][2] = { { -16.0, 50.0 },{ -16.0, 24.0 },{ 16.0, 24.0 },{ 16.0, 50.0 } };
GLfloat fox_collider_mid[4][2] = { { -28.0, 24.0 },{ -28.0, -8.0 },{ 28.0, -8.0 },{ 28.0, 24.0 } };
GLfloat fox_collider_down[4][2] = { { -24.0, -8.0 },{ -24.0, -32.0 },{ 24.0, -32.0 },{ 24.0, -8.0 } };


GLfloat fox_collider_color[3][3] = {
	{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f },
	{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f },
	{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f }
};
struct collider_rect struct_fox_up = { fox_centerx + FOX_UP_X0,fox_centerx + FOX_UP_X1,
	fox_centery + FOX_UP_Y0,fox_centery + FOX_UP_Y1 };
struct collider_rect struct_fox_mid = { fox_centerx + FOX_MID_X0,fox_centerx + FOX_MID_X1,
	fox_centery + FOX_MID_Y0,fox_centery + FOX_MID_Y1 };
struct collider_rect struct_fox_down = { fox_centerx + FOX_DOWN_X0,fox_centerx + FOX_DOWN_X1,
	fox_centery + FOX_DOWN_Y0,fox_centery + FOX_DOWN_Y1 };


GLuint VBO_fox_collider, VAO_fox_collider;
void prepare_fox_collider() {
	GLsizeiptr buffer_size = sizeof(fox_collider_up) + sizeof(fox_collider_mid) + sizeof(fox_collider_down);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_fox_collider);	//generate buffer object names

	glBindBuffer(GL_ARRAY_BUFFER, VBO_fox_collider);	//bind a named buffer object
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory(create a new data store for a buffer object)

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(fox_collider_up), fox_collider_up);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fox_collider_up), sizeof(fox_collider_mid), fox_collider_mid);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fox_collider_up)+sizeof(fox_collider_mid), sizeof(fox_collider_down), fox_collider_down);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_fox_collider);
	glBindVertexArray(VAO_fox_collider);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_fox_collider);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_fox_collider() {
	glBindVertexArray(VAO_fox_collider);

	glUniform3fv(loc_primitive_color, 1, fox_collider_color[FOX_COLLIDER_UP]);
	glDrawArrays(GL_LINE_LOOP, 0, 4);	// VBO에서 glBufferSubData로 정한 순서대로 나옴 

	glUniform3fv(loc_primitive_color, 1, fox_collider_color[FOX_COLLIDER_MID]);
	glDrawArrays(GL_LINE_LOOP, 4, 4);	// VBO에서 glBufferSubData로 정한 순서대로 나옴 

	glUniform3fv(loc_primitive_color, 1, fox_collider_color[FOX_COLLIDER_DOWN]);
	glDrawArrays(GL_LINE_LOOP, 8, 4);	// VBO에서 glBufferSubData로 정한 순서대로 나옴 

	glBindVertexArray(0);
}
////////////// 충돌체 제작 완료 //////////////////////

//// 충돌 관련 함수
// collider_rect 구조체 업데이트 함수
void update_collider(void) {
	struct_airplane_up.x0 = MULTIPLE*(airplane_centerx + AIRPLANE_UP_X0);
	struct_airplane_up.x1 = MULTIPLE*(airplane_centerx + AIRPLANE_UP_X1);
	struct_airplane_up.y0 = MULTIPLE*(airplane_centery + AIRPLANE_UP_Y0);
	struct_airplane_up.y1 = MULTIPLE*(airplane_centery + AIRPLANE_UP_Y1);

	struct_airplane_down.x0 = MULTIPLE*(airplane_centerx + AIRPLANE_DOWN_X0);
	struct_airplane_down.x1 = MULTIPLE*(airplane_centerx + AIRPLANE_DOWN_X1);
	struct_airplane_down.y0 = MULTIPLE*(airplane_centery + AIRPLANE_DOWN_Y0);
	struct_airplane_down.y1 = MULTIPLE*(airplane_centery + AIRPLANE_DOWN_Y1);

	struct_house.x0 = MULTIPLE*(house_centerx + HOUSE_X0);
	struct_house.x1 = MULTIPLE * (house_centerx + HOUSE_X1);
	struct_house.y0 = MULTIPLE * (house_centery + HOUSE_Y0);
	struct_house.y1 = MULTIPLE * (house_centery + HOUSE_Y1);

	struct_car.x0 = MULTIPLE * (car_centerx + CAR_X0);
	struct_car.x1 = MULTIPLE * (car_centerx + CAR_X1);
	struct_car.y0 = MULTIPLE * (car_centery + CAR_Y0);
	struct_car.y1 = MULTIPLE * (car_centery + CAR_Y1);

	struct_sword_up.x0 = MULTIPLE * (sword_centerx + SWORD_UP_X0);
	struct_sword_up.x1 = MULTIPLE * (sword_centerx + SWORD_UP_X1);
	struct_sword_up.y0 = MULTIPLE * (sword_centery + SWORD_UP_Y0);
	struct_sword_up.y1 = MULTIPLE * (sword_centery + SWORD_UP_Y1);

	struct_sword_down.x0 = MULTIPLE * (sword_centerx + SWORD_DOWN_X0);
	struct_sword_down.x1 = MULTIPLE * (sword_centerx + SWORD_DOWN_X1);
	struct_sword_down.y0 = MULTIPLE * (sword_centery + SWORD_DOWN_Y0);
	struct_sword_down.y1 = MULTIPLE * (sword_centery + SWORD_DOWN_Y1);

	
	struct_fox_up.x0 = MULTIPLE*(fox_centerx + FOX_UP_X0);
	struct_fox_up.x1 = MULTIPLE*(fox_centerx + FOX_UP_X1);
	struct_fox_up.y0 = MULTIPLE*(fox_centery + FOX_UP_Y0);
	struct_fox_up.y1 = MULTIPLE*(fox_centery + FOX_UP_Y1);

	struct_fox_mid.x0 = MULTIPLE*(fox_centerx + FOX_MID_X0);
	struct_fox_mid.x1 = MULTIPLE*(fox_centerx + FOX_MID_X1);
	struct_fox_mid.y0 = MULTIPLE*(fox_centery + FOX_MID_Y0);
	struct_fox_mid.y1 = MULTIPLE*(fox_centery + FOX_MID_Y1);

	struct_fox_down.x0 = MULTIPLE*(fox_centerx + FOX_DOWN_X0);
	struct_fox_down.x1 = MULTIPLE*(fox_centerx + FOX_DOWN_X1);
	struct_fox_down.y0 = MULTIPLE*(fox_centery + FOX_DOWN_Y0);
	struct_fox_down.y1 = MULTIPLE*(fox_centery + FOX_DOWN_Y1);
	
}




///////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
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
/////////////// airplane : object number 0
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
/////////////// house : ojbect number 1
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
///////////////  car : object number 2
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

/////////////////////////////////////////////////////////////////////////
//////////// sword : object number 3
/////////////////////////////////////////////////////////////////////////
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

/// finish : originated from 3.1.2DObjects_GLSL
/////////////////////////////////////////////////
/////////////////////////////////////////////////

#define HAT_LEAF 0
#define HAT_BODY 1
#define HAT_STRIP 2
#define HAT_BOTTOM 3
GLfloat a = 40.0f;
GLfloat hat_leaf[4][2] = { { 1.5, a + 10.0 },{ 1.5, a + 14.0 },{ 4.5, a + 16.0 },{ 4.5, a + 12.0 } };
GLfloat hat_body[4][2] = { { -7.5, a + 10.0 },{ -9.5, a + 1.0 },{ 9.5, a + 1.0 },{ 7.5, a + 10.0 } };
GLfloat hat_strip[4][2] = { { -9.5, a + 2.0 },{ -10.0, a + 0.0 },{ 10.0, a + 0.0 },{ 9.5, a + 2.0 } };
GLfloat hat_bottom[4][2] = { { 12.5, a + 0.0 },{ -12.5, a + 0.0 },{ -12.5, a + -4.0 },{ 12.5, a + -4.0 } };

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
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);

	glUniform3fv(loc_primitive_color, 1, hat_color[HAT_STRIP]);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);

	glUniform3fv(loc_primitive_color, 1, hat_color[HAT_BOTTOM]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);

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
GLfloat fox_arm_1_left[4][2] = { { -12.0,20.0 },{ -12.0,12.0 },{ -28.0,4.0 },{ -28.0,12.0 } };
GLfloat fox_arm_1_right[4][2] = { { 12.0,20.0 },{ 12.0,12.0 },{ 28.0,4.0 },{ 28.0,12.0 } }; 

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

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(airplane_centerx, airplane_centery, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(MULTIPLE, MULTIPLE, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_airplane();

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(house_centerx, house_centery, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(MULTIPLE, MULTIPLE, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_house();

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(car_centerx, car_centery, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(MULTIPLE, MULTIPLE, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_car();

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(sword_centerx, sword_centery, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(MULTIPLE, MULTIPLE, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_sword();

	//////////////////DRAW_FOX BELOW//////////////////////////
	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(fox_centerx, fox_centery, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(MULTIPLE, MULTIPLE, 1.0f));
	ModelMatrix = glm::rotate(ModelMatrix, rotate_angle, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_fox_fixed();

	if(fox_crash){
	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(fox_centerx, fox_centery, 0.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(MULTIPLE, MULTIPLE, 1.0f));
		ModelMatrix = glm::rotate(ModelMatrix, rotate_angle, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_fox_arm_1();
	}
	else{
		ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(fox_centerx, fox_centery, 0.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(MULTIPLE, MULTIPLE, 1.0f));
		ModelMatrix = glm::rotate(ModelMatrix, rotate_angle, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_fox_arm_2();
	}

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(fox_centerx, fox_centery, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(MULTIPLE, MULTIPLE, 1.0f));
	ModelMatrix = glm::rotate(ModelMatrix, rotate_angle, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_fox_leg_shoes();

	if(!fox_crash){
		ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(fox_centerx, fox_centery, 0.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(MULTIPLE, MULTIPLE, 1.0f));
		ModelMatrix = glm::rotate(ModelMatrix, rotate_angle, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_fox_faces_basic();
	}
	else{
		ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(fox_centerx, fox_centery, 0.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(MULTIPLE, MULTIPLE, 1.0f));
		ModelMatrix = glm::rotate(ModelMatrix, rotate_angle, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_fox_faces_crash();
	}
/////////////////////////finish fox//////////////////////////////////////////////////////////////////////
	
	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(fox_centerx, fox_centery, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(MULTIPLE, MULTIPLE, 1.0f));
	ModelMatrix = glm::rotate(ModelMatrix, rotate_angle, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_hat();

/////////////////////// start collider ////////////////////////////////
	// collider draw part
	/*
	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(airplane_centerx, airplane_centery, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(MULTIPLE, MULTIPLE, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_airplane_collider();

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(house_centerx, house_centery, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(MULTIPLE, MULTIPLE, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_house_collider();

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(car_centerx, car_centery, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(MULTIPLE, MULTIPLE, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_car_collider();

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(sword_centerx, sword_centery, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(MULTIPLE, MULTIPLE, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_sword_collider();
	
	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(fox_centerx, fox_centery, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(MULTIPLE, MULTIPLE, 1.0f));
	ModelMatrix = glm::rotate(ModelMatrix, rotate_angle, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_fox_collider();
	*/
	glFlush();
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {


	case 27: // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups.
		break;
	case 117: // lower case 'u' -> increase time interval
		time_interval++;
		printf("time_interval : %d\n", time_interval);
		break;
	case 100: // lower case 'd' -> decrease time interval(time_interval이 1아래로 내려가지 않도록 조정)
		printf("time_interval : %d\n", time_interval);
		time_interval--;
		if(time_interval==0) time_interval = 1;
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

void timer(int value) {
#define SENSITIVITY 5.0
#define PERIOD 20
	update_collider();
	static unsigned int total_time = 0;	// 프로그램 시작 후부터 timer 실행시마다 1씩 카운트 됨.
	if(total_time%PERIOD==0|| total_time % PERIOD == 1 || total_time % PERIOD == 2 || total_time % PERIOD == 3){
			rotate_angle = rotate_angle + 90.0f*TO_RADIAN;
	}
	total_time++;

	// 물체 이동
	move_object(airplane_deltax, airplane_deltay, AIRPLANE);
	move_object(house_deltax, house_deltay, HOUSE);
	move_object(car_deltax, car_deltay, CAR);
	move_object(sword_deltax, sword_deltay, SWORD);

	// 윈도우와 물체의 충돌 체크
	modify_direction(&airplane_deltax, &airplane_deltay, AIRPLANE);
	modify_direction(&house_deltax, &house_deltay, HOUSE);
	modify_direction(&car_deltax, &car_deltay, CAR);
	modify_direction(&sword_deltax, &sword_deltay, SWORD);

	fox_crash = check_crash(struct_airplane_up, struct_fox_up) || check_crash(struct_airplane_up, struct_fox_mid) || check_crash(struct_airplane_up, struct_fox_down) ||
				check_crash(struct_airplane_down, struct_fox_up) || check_crash(struct_airplane_down, struct_fox_mid) || check_crash(struct_airplane_down, struct_fox_down) ||
				check_crash(struct_house, struct_fox_up) ||	check_crash(struct_house, struct_fox_mid) || check_crash(struct_house, struct_fox_down) ||
				check_crash(struct_car, struct_fox_up) || check_crash(struct_car, struct_fox_mid) || check_crash(struct_car, struct_fox_down) ||
				check_crash(struct_sword_up, struct_fox_up) || 	check_crash(struct_sword_up, struct_fox_mid) ||	check_crash(struct_sword_up, struct_fox_down) ||
				check_crash(struct_sword_down, struct_fox_up) || check_crash(struct_sword_down, struct_fox_mid) ||	check_crash(struct_sword_down, struct_fox_down);

	// 키보드 방향키 입력 받아서 방향 변경
	switch(set_key){ 
	case 0:	// left
		fox_centerx -= SENSITIVITY;
		break;
	case 1:	// right
		fox_centerx += SENSITIVITY;
		break;
	case 2:	// up
		fox_centery += SENSITIVITY;
		break;
	case 3:	// down
		fox_centery -= SENSITIVITY;
		break;
	}
	// 윈도우에 닿으면 여우의 방향 자동 변경
	if(fox_centerx<-win_width/2.0f+MULTIPLE * 28.0f)	// left (여우의 팔 가장 왼쪽이 -28.0f)
		set_key=1; // 1 for right
	else if(fox_centerx>win_width/2.0f- MULTIPLE *28.0f) // right (여우의 팔 가장 오른쪽이 +28.0f)
		set_key=0; // 0 for left
	else if(fox_centery<-win_height/2.0f+ MULTIPLE *32.0f) // down  (여우의 신발 가장 아래가 -32.0f)
		set_key=2; // 2 for up
	else if(fox_centery>win_height/2.0f- MULTIPLE *56.0f) // up (여우의 모자 leaf 가장 위가 +56.0f)
		set_key=3; // 3 for down

	glutPostRedisplay();
	glutTimerFunc(time_interval, timer, value);
}

void special(int key, int x, int y) {
//#define SENSITIVITY 5.0
	switch (key) {
	case GLUT_KEY_LEFT:
		set_key = 0;
		//rotate_angle += 90.0f*TO_RADIAN;
		glutPostRedisplay();
		break;
	case GLUT_KEY_RIGHT:
		set_key = 1;
		//rotate_angle -= 90.0f*TO_RADIAN;
		glutPostRedisplay();
		break;
	case GLUT_KEY_UP:
		set_key = 2;
		glutPostRedisplay();
		break;
	case GLUT_KEY_DOWN:
		set_key = 3;
		glutPostRedisplay();
		break;
	}
}

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);
	glutCloseFunc(cleanup);
	glutTimerFunc(30, timer, 30);
	glutSpecialFunc(special);
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

	glClearColor(80 / 255.0f, 230 / 255.0f, 250 / 255.0f, 1.0f);
	ViewMatrix = glm::mat4(1.0f);
}

void prepare_scene(void) {
	prepare_axes();
	prepare_airplane();
	prepare_house();
	prepare_car();
	prepare_sword();
	prepare_hat();
	prepare_fox_fixed();
	prepare_fox_arm_1();
	prepare_fox_arm_2();
	prepare_fox_leg_shoes();
	prepare_fox_faces_basic();
	prepare_fox_faces_crash();
	prepare_airplane_collider();
	prepare_house_collider();
	prepare_car_collider();
	prepare_sword_collider();
	prepare_fox_collider();
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
		"    - Keys used: 'ESC' \n    - Keys used: 'u' for increasing timer interval\n    - Keys used: 'd' for decreasing timer interval\n"
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


