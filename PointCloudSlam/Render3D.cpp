#include <time.h>

#include <iostream>
#include <cmath>

#include "Render3D.h"

float trans_z = -1.0f;
float trans_x = 0.0f;
float trans_y = 0.0f;
float angle_y = 180.0f;
float angle_x = 0.0f;



struct line_vertex
{
	unsigned char r, g, b, a;
	float x, y, z;
};

line_vertex g_lineVertices[] =
{
	{ 255, 0, 0, 255, 0.0f, 0.0f, 0.0f },
	{ 255, 0, 0, 255, 0.2f, 0.0f, 0.0f },
	{ 0, 255, 0, 255, 0.0f, 0.0f, 0.0f },
	{ 0, 255, 0, 255, 0.0f, 0.2f, 0.0f },
	{ 0, 0, 255, 255, 0.0f, 0.0f, 0.0f },
	{ 0, 0, 255, 255, 0.0f, 0.0f, 0.2f }
};

line_vertex g_lineVertices_realsense[] =
{
	{ 255, 0, 0, 255, 0.0f, 0.0f, 0.0f },
	{ 255, 0, 0, 255, 0.2f, 0.0f, 0.0f },
	{ 255, 0, 0, 255, 0.0f, 0.0f, 0.0f },
	{ 255, 0, 0, 255, 0.0f, 0.2f, 0.0f },
	{ 255, 0, 0, 255, 0.0f, 0.0f, 0.0f },
	{ 255, 0, 0, 255, 0.0f, 0.0f, 0.2f }
};



GLFWwindow* initWindow(const int width, const int height, int pos_x, int pos_y, const char * title)
{
	GLFWwindow* window;
	const GLubyte* renderer;
	const GLubyte* version;
	glfwInit();

	window = glfwCreateWindow(width, height, title, NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSetWindowPos(window, pos_x, pos_y);
	glewExperimental = GL_TRUE;
	glewInit();
	renderer = glGetString(GL_RENDERER);
	version = glGetString(GL_VERSION);
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	return window;
}

void idle(GLFWwindow* window){
	glfwMakeContextCurrent(window);
	if (!glfwWindowShouldClose(window))
	{
		glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void show_points(GLFWwindow* window, GlobalMap s, Camera camera, Camera realsense){
	glfwMakeContextCurrent(window);
	if (!glfwWindowShouldClose(window))
	{
		GLint windowWidth, windowHeight;
		glfwGetWindowSize(window, &windowWidth, &windowHeight);
		glViewport(0, 0, windowWidth, windowHeight);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_PROJECTION_MATRIX);
		glLoadIdentity();
		gluPerspective(60, (GLdouble)windowWidth / (GLdouble)windowHeight, 0.1, 100);

		glMatrixMode(GL_MODELVIEW_MATRIX);

		float theta_x_camera = atan2(camera.get_rotation()[7], camera.get_rotation()[8]);
		float theta_y_camera = atan2(-camera.get_rotation()[6], sqrt(pow(camera.get_rotation()[7], 2) + pow(camera.get_rotation()[8], 2)));
		float theta_z_camera = atan2(camera.get_rotation()[3], camera.get_rotation()[0]);

		float angle_x_camera = (theta_x_camera*180.0f) / (float)PI;
		float angle_y_camera = (theta_y_camera*180.0f) / (float)PI;
		float angle_z_camera = (theta_z_camera*180.0f) / (float)PI;

		float pos_x_camera = camera.get_position()[0];
		float pos_y_camera = camera.get_position()[1];
		float pos_z_camera = camera.get_position()[2];

		glTranslatef(pos_x_camera, pos_y_camera, pos_z_camera);
		glRotatef(angle_x_camera, 1.0f, 0.0f, 0.0f);
		glRotatef(angle_y_camera, 0.0f, 1.0f, 0.0f);
		glRotatef(angle_z_camera, 0.0f, 0.0f, 1.0f);

		glInterleavedArrays(GL_C4UB_V3F, 0, g_lineVertices);
		glDrawArrays(GL_LINES, 0, 6);

		for (int i = 0; i < (int)s.point_clouds.size(); i++){
			PointCloud current_point_cloud = s.point_clouds[i];
			Render rs = current_point_cloud.get_rendering_structures();
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_COLOR_ARRAY);
			glVertexPointer(3, GL_FLOAT, 0, rs.vertices);
			glColorPointer(3, GL_FLOAT, 0, rs.colors);
			glDrawArrays(GL_POINTS, 0, (GLsizei)s.point_clouds[i].points.size());
			glDisableClientState(GL_COLOR_ARRAY);
			glDisableClientState(GL_VERTEX_ARRAY);
		}

		float theta_x_realsense = atan2(realsense.get_rotation()[7], realsense.get_rotation()[8]);
		float theta_y_realsense = atan2(-realsense.get_rotation()[6], sqrt(pow(realsense.get_rotation()[7], 2) + pow(realsense.get_rotation()[8], 2)));
		float theta_z_realsense = atan2(realsense.get_rotation()[3], realsense.get_rotation()[0]);

		float angle_x_realsense = (theta_x_realsense*180.0f) / (float)PI;
		float angle_y_realsense = (theta_y_realsense*180.0f) / (float)PI;
		float angle_z_realsense = (theta_z_realsense*180.0f) / (float)PI;

		float pos_x_realsense = realsense.get_position()[0];
		float pos_y_realsense = realsense.get_position()[1];
		float pos_z_realsense = realsense.get_position()[2];

		glTranslatef(pos_x_realsense, pos_y_realsense, pos_z_realsense);
		glRotatef(angle_x_realsense, 1.0f, 0.0f, 0.0f);
		glRotatef(angle_y_realsense, 0.0f, 1.0f, 0.0f);
		glRotatef(angle_z_realsense, 0.0f, 0.0f, 1.0f);

		glInterleavedArrays(GL_C4UB_V3F, 0, g_lineVertices_realsense);
		glDrawArrays(GL_LINES, 0, 6);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void show_map(GLFWwindow* window, int n, GLfloat *vertices, GLfloat *colors){
	glfwMakeContextCurrent(window);
	if (!glfwWindowShouldClose(window))
	{
		GLint windowWidth, windowHeight;
		glfwGetWindowSize(window, &windowWidth, &windowHeight);
		glViewport(0, 0, windowWidth, windowHeight);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_PROJECTION_MATRIX);
		glLoadIdentity();
		gluPerspective(60, (GLdouble)windowWidth / (GLdouble)windowHeight, 0.1, 100.0);

		glMatrixMode(GL_MODELVIEW_MATRIX);

		glTranslatef(0.0f, 0.0f, 0.1f);
		glRotatef(0.0f, 1.0f, 0.0f, 0.0f);
		glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
		glRotatef(0.0f, 0.0f, 0.0f, 1.0f);

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, vertices);
		glColorPointer(3, GL_FLOAT, 0, colors);
		glDrawArrays(GL_POINTS, 0, n);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void show_location(GLFWwindow* window, int n, GLfloat *vertices, GLfloat *colors, Camera camera, Camera realsense, vector<Camera> real_sense_pos){
	glfwMakeContextCurrent(window);
	if (!glfwWindowShouldClose(window))
	{
		// Scale to window size
		GLint windowWidth, windowHeight;
		glfwGetWindowSize(window, &windowWidth, &windowHeight);
		glViewport(0, 0, windowWidth, windowHeight);

		// Draw stuff
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_PROJECTION_MATRIX);
		glLoadIdentity();
		gluPerspective(60, (GLdouble)windowWidth / (GLdouble)windowHeight, 0.1, 100);

		glMatrixMode(GL_MODELVIEW_MATRIX);

		float theta_x_camera = atan2(camera.get_rotation()[6], camera.get_rotation()[0]);
		float theta_z_camera = atan2(camera.get_rotation()[5], camera.get_rotation()[4]);
		float theta_y_camera = asin(camera.get_rotation()[3]);

		float angle_x_camera = (theta_x_camera*180.0f) / (float)PI;
		float angle_y_camera = (theta_y_camera*180.0f) / (float)PI;
		float angle_z_camera = (theta_z_camera*180.0f) / (float)PI;

		float pos_x_camera = camera.get_position()[0];
		float pos_y_camera = camera.get_position()[1];
		float pos_z_camera = camera.get_position()[2];

		glTranslatef(pos_x_camera, pos_y_camera, pos_z_camera);
		glRotatef(angle_x_camera, 1.0f, 0.0f, 0.0f);
		glRotatef(angle_y_camera, 0.0f, 1.0f, 0.0f);
		glRotatef(angle_z_camera, 0.0f, 0.0f, 1.0f);

		float theta_x_realsense = atan2(realsense.get_rotation()[6], realsense.get_rotation()[0]);
		float theta_z_realsense = atan2(realsense.get_rotation()[5], realsense.get_rotation()[4]);
		float theta_y_realsense = asin(realsense.get_rotation()[3]);

		float angle_x_realsense = (theta_x_realsense*180.0f) / (float)PI;
		float angle_y_realsense = (theta_y_realsense*180.0f) / (float)PI;
		float angle_z_realsense = (theta_z_realsense*180.0f) / (float)PI;

		float pos_x_realsense = realsense.get_position()[0];
		float pos_y_realsense = realsense.get_position()[1];
		float pos_z_realsense = realsense.get_position()[2];

		glTranslatef(pos_x_realsense, pos_y_realsense, pos_z_realsense);
		glRotatef(angle_x_realsense, 1.0f, 0.0f, 0.0f);
		glRotatef(angle_y_realsense, 0.0f, 1.0f, 0.0f);
		glRotatef(angle_z_realsense, 0.0f, 0.0f, 1.0f);

		glInterleavedArrays(GL_C4UB_V3F, 0, g_lineVertices);
		glDrawArrays(GL_LINES, 0, 6);

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, vertices);
		glColorPointer(3, GL_FLOAT, 0, colors);
		glDrawArrays(GL_POINTS, 0, n);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);


		for (int i = 0; i < real_sense_pos.size(); i++){
			glLineWidth(0.01f);
			glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
			glBegin(GL_POINTS);
			glVertex3f(real_sense_pos[i].get_position()[0], real_sense_pos[i].get_position()[1], real_sense_pos[i].get_position()[2]);
			glEnd();
		}


		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

int terminate_point_renderer(GLFWwindow* window)
{
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}