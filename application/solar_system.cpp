
///////////////////////////////// includes ///////////////////////////////////
#include <glbinding/gl/gl.h>
// load glbinding extensions
#include <glbinding/Binding.h>

//dont load gl bindings from glfw
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

// use floats and med precision operations
#define GLM_PRECISION_MEDIUMP_FLOAT
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <cmath>



#include "shader_loader.hpp"
#include "model_loader.hpp"
#include "texture_loader.hpp"
#include "utils.hpp"
#include "planet.hpp"

#include <cstdlib>
#include <iostream>


// use gl definitions from glbinding 
using namespace gl;

//Defina Astronomical Unit(AU) unit of length roughly equal to the distance from the earth to the sun 
const float AU = 30.0f;
const float AU_scale = 1.6;
const int number_of_stars = 10000;
float speed_time = 1.0f;

float Shading_Option = 1; // at the moment 1 for Blinn-Phong and 2 for Toon

//const int number_of_orbitFragment = 3.6*50000;
const int number_of_orbitFragment = 50000;
#ifndef M_PI
  #define M_PI 3.14f
#endif

/////////////////////////// variable definitions //////////////////////////////
// vertical field of view of camera
const float camera_fov = glm::radians(60.0f);
// initial window dimensions
//const unsigned window_width = 640;
//const unsigned window_height = 480;
const unsigned window_width = 1920;
const unsigned window_height = 1080;
// the rendering window
GLFWwindow* window;

// variables for fps computation
double last_second_time = 0;
unsigned frames_per_second = 0;

// the main shader program
GLuint planet_program = 0;
GLuint starCloud_program = 0;
GLuint orbit_program = 0;

// cpu representation of model
model planet_model{};
model star_model{};
model orbit_model{};

// holds gpu representation of model
struct model_object {
  GLuint vertex_AO = 0;
  GLuint vertex_BO = 0;
  GLuint element_BO = 0;
};
model_object planet_object;
model_object starfield_object;
model_object orbit_object;

// camera matrices
glm::mat4 camera_view = glm::translate(glm::mat4{}, glm::vec3{50.0f, 30.0f, 150.0f});
glm::mat4 camera_projection{1.0f};

// uniform locations
GLint location_normal_matrix = -1;
GLint location_model_matrix = -1;
GLint location_view_matrix = -1;
GLint location_projection_matrix = -1;
GLint location_planet_color = -1;
GLint location_sun_position = -1;
GLint location_Shading_Option = -1;

// starCloud location
GLint starCloud_view_matrix = -1;
GLint starCloud_projection_matrix = -1;

// orbit locations
GLint orbit_model_matrix = -1;
GLint orbit_view_matrix = -1;
GLint orbit_projection_matrix = -1;

// path to the resource folders
std::string resource_path{};

// holds the solar System
std::vector<Planet*> solarSystem;


/////////////////////////// forward declarations //////////////////////////////
void quit(int status);
void update_view(GLFWwindow* window, int width, int height);
void update_camera();
void update_uniform_locations();
void update_shader_programs();
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void cursor_callback(GLFWwindow * window, double x, double y);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void generate_solarSystem();
void  generate_starCloud();
void generate_orbits();

model_object initialize_geometry( model & mod );

void show_fps();
void render();
//void render_planet();
void render_Planet(Planet* const& planet, glm::mat4 & model_matrix, float time);
//void render_starfield();
void render_orbit(Planet* const& planet, float delta);

std::string print(glm::mat4 mat) {
  std::string os ="\n{ \n";    // more generic version? - a mat type for every
  for (int i=0; i<=3; i++) {   // mat.lenght() --> number of columns
    for (int j=0; j<=3; j++) { // mat[0].lenght() --> number of components in column
      if ( j==0) os+="[ ";
      os+=std::to_string(mat[j][i]);
      if (j==3) os+=" ]\n"; else os+=" ;";
    }
  }
  return os+" }";
};
/////////////////////////////// main function /////////////////////////////////
int main(int argc, char* argv[]) {

  glfwSetErrorCallback(utils::glsl_error);

  if(!glfwInit()) {
    std::exit(EXIT_FAILURE);  
  }
    
//on MacOS, set OGL version explicitly
    std::string changeDirectory; // dirty fix for directory ../debug/debug/..
#ifdef __APPLE__
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//   changeDirectory = "/..";
#endif

  // create window, if unsuccessfull, quit
  window = glfwCreateWindow(window_width, window_height, "OpenGL Framework", NULL, NULL);
  if(!window) {
    glfwTerminate();
    std::exit(EXIT_FAILURE);
  }

  // use the windows context
  glfwMakeContextCurrent(window);
  // disable vsync
  glfwSwapInterval(0);
  // register key input function
  glfwSetKeyCallback(window, key_callback);
  //register curser position input function
  glfwSetCursorPosCallback(window,cursor_callback);
  glfwSetScrollCallback(window, scroll_callback);
  // allow free mouse movement
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  // register resizing function
  glfwSetFramebufferSizeCallback(window, update_view);

  // initialize glindings in this context
  glbinding::Binding::initialize();

  // activate error checking after each gl function call
  utils::watch_gl_errors();

  //first argument is resource path
  if (argc > 1) {
    resource_path = argv[1];
  }
  // no resource path specified, use default
  else {
    std::string exe_path{argv[0]};
    resource_path = exe_path.substr(0, exe_path.find_last_of("/\\"));
      resource_path += changeDirectory;
    resource_path += "/../../resources/";
  }

  // do before framebuffer_resize call as it requires the projection uniform location
  update_shader_programs();

  // initialize projection and view matrices
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  update_view(window, width, height);
  update_camera();

  // set up models
  generate_solarSystem();
  generate_starCloud();
  generate_orbits();

  // enable depth testing
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  
  // rendering loop
  while(!glfwWindowShouldClose(window)) {
    // query input
    glfwPollEvents();
    // clear buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    render();

    glfwSwapBuffers(window);
    // display fps
    show_fps();
  }

  quit(EXIT_SUCCESS);
}

struct Shader_Programm {
    std::string vertex_path;
    std::string frag_path;
    GLuint* programm;
};


void generate_solarSystem() {

    //                         name,  distance,  speed, size, type [sun,planet,moon]
  Planet* sun =     new Planet{"sun",      0.0f   ,0.0f,AU_scale *  9.90f};
  Planet* mercury = new Planet{"mercury",  0.4f*AU,2.5f,AU_scale *  0.3829f,"planet",{0.91f,0.78f,0.65f}};
  Planet* venus =   new Planet{"venus",    0.7f*AU,2.0f,AU_scale *  0.9499f,"planet",{0.88f,0.56f,0.18f}};
  Planet* earth =   new Planet{"earth",         AU,1.2f,AU_scale *  1.0f,"planet",{0.40f,0.78f,1.00f}};
  Planet* mars =    new Planet{"mars",     1.5f*AU,1.0f,AU_scale *  0.533f,"planet",{0.55f,0.46f,0.00f}};
  Planet* jupiter = new Planet{"jupiter",  3.2f*AU,0.8f,AU_scale *  6.0f,"planet",{0.55f,0.53f,0.31f}};
  Planet* saturn =  new Planet{"saturn",   5.5f*AU,0.7f,AU_scale *  5.0f,"planet",{0.67f,0.40f,0.00f}};
  Planet* uranus =  new Planet{"uranus",  19.2f*AU,0.5f,AU_scale *  3.929f,"planet",{0.90f,0.91f,0.80f}};
  Planet* neptun =  new Planet{"neptun",  18.0f*AU,0.4f,AU_scale *  3.883f,"planet",{0.43f,0.61f,0.95f}};
  // http://www.december.com/html/spec/colorper.html
  // generate moon and add them
  Planet* moon = new Planet{"moon",0.1f*AU,3.0f,0.273f,"moon",{0.92f,0.92f,0.92f}};
  earth->moon.push_back(moon);
  mars->moon.push_back(moon);

  // add planets to solar System
  solarSystem.push_back(sun);
  solarSystem.push_back(mercury);
  solarSystem.push_back(venus);
  solarSystem.push_back(earth);
  solarSystem.push_back(mars);
  solarSystem.push_back(jupiter);
  solarSystem.push_back(saturn);
  solarSystem.push_back(uranus);
  solarSystem.push_back(neptun);

  float accumulatedSize = 0.0f;
  for (auto p:solarSystem) {
    if (p->is_root()) {
      // it is sun
      accumulatedSize = p->size/2; // half size of the sun
    }else if (p->is_planet()) {
      p->distance+=accumulatedSize + p->size; // every planets distance is given by the surface distance
      //accumulatedSize += p->size;   // add the full size of the planet
      // we dont need the size of each planet. the distance is given by sun surface to planet surface.
      //std::cout << p->name << ": " << p->distance << " next will be " << accumulatedSize << std::endl;
    }
  }
  planet_model = model_loader::obj(resource_path + "models/Planet.obj", model::NORMAL);
  planet_object = initialize_geometry(planet_model);
}

void  generate_starCloud() {
  std::vector<float> stars;

  for (int i=0;i<number_of_stars;i++) {
    float x = float(fmod(std::rand() , 100*AU)-50*AU);
    float y = float(fmod(std::rand() , 100*AU)-50*AU);
    float z = float(fmod(std::rand() , 100*AU)-50*AU);
    stars.push_back(x);
    stars.push_back(y);
    stars.push_back(z);
    // Color
    stars.push_back(1.0f);
    stars.push_back(1.0f);
    stars.push_back(1.0f);
  }
  star_model = {stars, model::POSITION | model::NORMAL};
  starfield_object = initialize_geometry(star_model);
}

void generate_orbits() {
  std::vector<float> orbVec;
  for (int i =0; i<number_of_orbitFragment;i++) {
    float x = float (glm::cos( ( 360 / float (number_of_orbitFragment) * i ) * M_PI));
    float y = 0.0f;
    float z = float (glm::sin( ( 360 / float (number_of_orbitFragment) * i ) * M_PI));
    orbVec.push_back(x);
    orbVec.push_back(y);
    orbVec.push_back(z);

    orbVec.push_back(0.2f);
    orbVec.push_back(0.2f);
    orbVec.push_back(0.2f);
  }
  orbit_model = {orbVec, model::POSITION|model::NORMAL};
  orbit_object= initialize_geometry(orbit_model);
}

///////////////////////// initialisation functions ////////////////////////////
// load models
model_object initialize_geometry( model & mod ) {

  model_object object_;
  // generate vertex array object
  glGenVertexArrays(1, &object_.vertex_AO);
  // bind the array for attaching buffers
  glBindVertexArray(object_.vertex_AO);

  // generate generic buffer
  glGenBuffers(1, &object_.vertex_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ARRAY_BUFFER, object_.vertex_BO);
  // configure currently bound array buffer
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mod.data.size(), mod.data.data(), GL_STATIC_DRAW);

  // activate first attribute on gpu
  glEnableVertexAttribArray(0);
  // first attribute is 3 floats with no offset & stride
  glVertexAttribPointer(0, model::POSITION.components, (gl::GLenum) model::POSITION.type, GL_FALSE, mod.vertex_bytes, mod.offsets[model::POSITION]);
  // activate second attribute on gpu
  glEnableVertexAttribArray(1);
  // second attribute is 3 floats with no offset & stride
  glVertexAttribPointer(1, model::NORMAL.components, (gl::GLenum) model::NORMAL.type, GL_FALSE, mod.vertex_bytes, mod.offsets[model::NORMAL]);

   // generate generic buffer
  glGenBuffers(1, &object_.element_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object_.element_BO);
  // configure currently bound array buffer
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, model::INDEX.size * mod.indices.size(), mod.indices.data(), GL_STATIC_DRAW);

  return object_;
}


///////////////////////////// render functions ////////////////////////////////
// render model
//Actual size of the planets referenced to the earth.
void render() {
  glUseProgram(starCloud_program);
  glBindVertexArray(starfield_object.vertex_AO);
  utils::validate_program(starCloud_program);
  glDrawArrays(GL_POINTS, 0, number_of_stars);


  glUseProgram(planet_program);
  Planet* sun =     new Planet{"sun",      0.0f   ,0.0f,9.90f};
  glm::mat4 translation_sun{};
  render_Planet(sun,translation_sun,0);
  for (auto const &p : solarSystem) {
    float time = float(glfwGetTime());
    Planet* current = p;
    glm::mat4 translation;
    glm::vec3 position;
    if (p->name!="sun") render_Planet(p, translation, time);
  }


  //render_planet();

  glUseProgram(orbit_program);
  for (auto const&p:solarSystem) {
      float deltaDistance = p->distance - (p->size/2);
      // p isnt needed at the moment.
      render_orbit(p, deltaDistance);
  }

}

void render_Planet(Planet* const& planet, glm::mat4 & model_matrix, float time) {
  glUseProgram(planet_program);
  model_matrix = glm::rotate(model_matrix,time * planet->speed * speed_time, glm::vec3{ 0.0f, 1.0f, 0.0f }); // axis of rotation
  model_matrix = glm::translate(model_matrix, glm::vec3{ 0.0f, 0.0f, planet->distance }); // radius of the rotation axis defined in AU
  model_matrix = glm::scale(model_matrix, glm::vec3{planet->size});
  //if (planet->name == "sun") std::cout <<  planet->name << " " << print(model_matrix) << std::endl;
  glUniformMatrix4fv(location_model_matrix, 1, GL_FALSE, glm::value_ptr(model_matrix));
  // extra matrix for normal transformation to keep them orthogonal to surface
  glm::mat4 normal_matrix = glm::inverseTranspose(glm::inverse(camera_view * model_matrix));
  glUniformMatrix4fv(location_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrix));

  // upload color

  glUniform3f(location_planet_color, planet->color.r, planet->color.g, planet->color.b);
  glUniform1f(location_Shading_Option,Shading_Option);

  glBindVertexArray(planet_object.vertex_AO);
  utils::validate_program(planet_program);
  // draw bound vertex array as triangles using bound shader
  glDrawElements(GL_TRIANGLES, GLsizei(planet_model.indices.size()), (gl::GLenum) model::INDEX.type, NULL);

  if (!planet->moon.empty()) {
    for (auto const &moon : planet->moon) {
      render_Planet(moon, model_matrix, time);
    }
  }

}

void render_orbit(Planet* const& planet, float delta) {
  glm::mat4 model_matrix = glm::scale(glm::mat4{},glm::vec3{delta});
  glUniformMatrix4fv(orbit_model_matrix, 1, GL_FALSE, glm::value_ptr(model_matrix));
  glBindVertexArray(orbit_object.vertex_AO);
  utils::validate_program(orbit_program);
  glDrawArrays(GL_LINES,0,number_of_orbitFragment);
}

///////////////////////////// update functions ////////////////////////////////
// update viewport and field of view
void update_view(GLFWwindow* window, int width, int height) {
  // resize framebuffer
  glViewport(0, 0, width, height);

  float aspect = float(width) / float(height);
  float fov_y = camera_fov;
  // if width is smaller, extend vertical fov 
  if(width < height) {
    fov_y = 2.0f * glm::atan(glm::tan(camera_fov * 0.5f) * (1.0f / aspect));
  }
  // projection is hor+  - Creates a matrix for a symetric perspective-view frustum.
    /*    detail::tmat4x4<T> glm::perspective	(	T const & 	fovy,
                                                    T const & 	aspect,
                                                    T const & 	near,
                                                    T const & 	far ) 
     changed far point from 10.0f to 100.0f  - maybe it should a bit smaller... 
     fixes the "bug" you can see in the W-key.mov */
  camera_projection = glm::perspective(fov_y, aspect, 0.1f*AU*2, 100*AU);
  // upload matrix to gpu
  glUseProgram(planet_program);
  glUniformMatrix4fv(location_projection_matrix, 1, GL_FALSE, glm::value_ptr(camera_projection));

  glUseProgram(starCloud_program);
  glUniformMatrix4fv(starCloud_projection_matrix, 1, GL_FALSE, glm::value_ptr(camera_projection));

  glUseProgram(orbit_program);
  glUniformMatrix4fv(orbit_projection_matrix, 1, GL_FALSE, glm::value_ptr(camera_projection));
}

// update camera transformation
void update_camera() {
  // vertices are transformed in camera space, so camera transform must be inverted
  /*
    glm::vec3 eye = camera_position;
    glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    camera_view = glm::inverse(glm::lookAt(eye,center,up));
  */
  glm::mat4 inv_camera_view = glm::inverse(camera_view);
  // upload matrix to gpu
  glUseProgram(planet_program);
  glm::vec4 sun = {0.0f, 0.0f, 0.0f, 1.0f};
  sun = inv_camera_view * sun;

  glUniform3f(location_sun_position, sun.x,sun.y,sun.z);
  glUniformMatrix4fv(location_view_matrix, 1, GL_FALSE, glm::value_ptr(inv_camera_view));

  glUseProgram(starCloud_program);
  glUniformMatrix4fv(starCloud_view_matrix, 1,GL_FALSE, glm::value_ptr(inv_camera_view));

  glUseProgram(orbit_program);
  glUniformMatrix4fv(orbit_view_matrix, 1, GL_FALSE,glm::value_ptr(inv_camera_view));
}

// load shaders and update uniform locations
void update_shader_programs() {
  /* defined at the top!!
    GLuint simple_program = 0;
    GLuint starCloud_program = 0;
    */
  std::vector<Shader_Programm> shaders;
  shaders.push_back({"shaders/simple.vert", "shaders/simple.frag", &planet_program});
  shaders.push_back({"shaders/starCloud.vert","shaders/starCloud.frag", &starCloud_program});
  shaders.push_back({"shaders/orbit.vert","shaders/orbit.frag", &orbit_program});

  try {
    for (auto item : shaders) {
      // throws exception when compiling was unsuccessful
      GLuint new_program = shader_loader::program(resource_path + item.vertex_path,
                                                  resource_path + item.frag_path);
      // free old shader
      glDeleteProgram(*item.programm);
      // save new shader
      *item.programm = new_program;
      // bind shader
      glUseProgram(*item.programm);
      // after shader is recompiled uniform locations may change

    }
    update_uniform_locations();
    // upload view uniforms to new shader
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    update_view(window, width, height);
    update_camera();
  }
  catch(std::exception&) {
    // don't crash, allow another try
  }
}

// update shader uniform locations
void update_uniform_locations() {
  location_normal_matrix = glGetUniformLocation(planet_program, "NormalMatrix");
  location_model_matrix = glGetUniformLocation(planet_program, "ModelMatrix");
  location_view_matrix = glGetUniformLocation(planet_program, "ViewMatrix");
  location_projection_matrix = glGetUniformLocation(planet_program, "ProjectionMatrix");
  location_planet_color = glGetUniformLocation(planet_program, "ColorVec");
  location_Shading_Option = glGetUniformLocation(planet_program,"ShadingOption");

  starCloud_projection_matrix = glGetUniformLocation(starCloud_program, "ProjectionMatrix");
  starCloud_view_matrix = glGetUniformLocation(starCloud_program, "ViewMatrix");

  orbit_model_matrix = glGetUniformLocation(orbit_program, "ModelMatrix");
  orbit_view_matrix = glGetUniformLocation(orbit_program, "ViewMatrix");
  orbit_projection_matrix = glGetUniformLocation(orbit_program, "ProjectionMatrix");
}

///////////////////////////// misc functions ////////////////////////////////
// handle key input
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, 1);
  }
  else if(key == GLFW_KEY_R && action == GLFW_PRESS) {
    update_shader_programs();
  }
  else if(key == GLFW_KEY_W && (action == GLFW_PRESS | action == GLFW_REPEAT)) {
    camera_view = glm::translate(camera_view, glm::vec3{0.0f, 0.0f, -0.1f*AU});
    update_camera();
  }
  else if(key == GLFW_KEY_S && (action == GLFW_PRESS | action == GLFW_REPEAT)) {
    camera_view = glm::translate(camera_view, glm::vec3{0.0f, 0.0f, 0.1f*AU});
    update_camera();
  }
  else if(key == GLFW_KEY_UP && (action == GLFW_PRESS | action == GLFW_REPEAT)) {
      camera_view = glm::translate(camera_view, glm::vec3{0.0f, 0.1f*AU, 0.0f});
      update_camera();
  }
  else if(key == GLFW_KEY_DOWN && (action == GLFW_PRESS | action == GLFW_REPEAT)) {
      camera_view = glm::translate(camera_view, glm::vec3{0.0f, -0.1f*AU, 0.0f});
      update_camera();
  }
  else if(key == GLFW_KEY_RIGHT && (action == GLFW_PRESS | action == GLFW_REPEAT)) {
      camera_view = glm::translate(camera_view, glm::vec3{0.1f*AU, 0.0f, 0.0f});
      update_camera();
  }
  else if(key == GLFW_KEY_LEFT && (action == GLFW_PRESS | action == GLFW_REPEAT)) {
      camera_view = glm::translate(camera_view, glm::vec3{-0.1f*AU, 0.0f, 0.0f});
      update_camera();
  }
  else if(key == GLFW_KEY_A && (action == GLFW_PRESS | action == GLFW_REPEAT)) {
    camera_view = glm::rotate(camera_view, 0.1f, glm::vec3{0.0f, -0.1f*AU, 0.0f});
    update_camera();
  }
  else if(key == GLFW_KEY_D && (action == GLFW_PRESS | action == GLFW_REPEAT)) {
    camera_view = glm::rotate(camera_view, -0.1f, glm::vec3{0.0f, -0.1f*AU, 0.0f});
    update_camera();
  }
  else if(key == GLFW_KEY_Q && (action == GLFW_PRESS | action == GLFW_REPEAT)) {
    //std::cout << print(camera_view);
    camera_view = glm::rotate(camera_view, 0.1f, glm::vec3{-0.1f*AU, 0.0f, 0.0f});
    update_camera();
    //std::cout << print(camera_view);
  }
  else if(key == GLFW_KEY_E && (action == GLFW_PRESS | action == GLFW_REPEAT)) {
    camera_view = glm::rotate(camera_view, -0.1f, glm::vec3{-0.1f*AU, 0.0f, 0.0f});
    update_camera();
  }
  else if((key == GLFW_KEY_Y | key==GLFW_KEY_Z) && (action == GLFW_PRESS | action == GLFW_REPEAT)) {
    //std::cout << print(camera_view);
    camera_view = glm::rotate(camera_view, 0.1f, glm::vec3{0.0f, 0.0f, -0.1f*AU});
    update_camera();
    //std::cout << print(camera_view);
  }
  else if(key == GLFW_KEY_X && (action == GLFW_PRESS | action == GLFW_REPEAT)) {
    camera_view = glm::rotate(camera_view, -0.1f, glm::vec3{0.0f, 0.0f, -0.1f*AU});
    update_camera();
  }
  else if(key==GLFW_KEY_9 && action == GLFW_PRESS) {
    glClearColor(1.0f,1.0f,1.0f,1.0f);
  }
  else if(key==GLFW_KEY_8 && action == GLFW_PRESS) {
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    // at the moment i don't know exactly how to change color of stars
  }
  else if (key==GLFW_KEY_P && action == GLFW_PRESS) {
    std::cout << "Print Camera_View Matrix "<< print(camera_view) << std::endl;
  }
  else if (key==GLFW_KEY_B && action == GLFW_PRESS) {
    camera_view = glm::translate(glm::mat4{}, glm::vec3{50.0f, 30.0f, 150.0f});
    update_camera();
  }
  else if (key==GLFW_KEY_T && action == GLFW_PRESS) {
    glm::vec3 pos = glm::vec3(0.0f, solarSystem.back()->distance*2 , 0.0f);
    glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);

    camera_view = glm::lookAt(pos, target, up);
    camera_view = glm::inverse(camera_view);
    update_camera();
  }
  else if (key==GLFW_KEY_L && action == GLFW_PRESS) {
    speed_time += 0.1f;
  }
  else if (key==GLFW_KEY_K && action == GLFW_PRESS) {
    speed_time -= 0.1f;
  }
  else if (key==GLFW_KEY_SPACE && action == GLFW_PRESS) {
    if (speed_time == 0.0f) {
      speed_time = 1.0f;
    } else {
      speed_time = 0.0f;
    }
  }
  else if (key==GLFW_KEY_1 && action == GLFW_PRESS) {
    Shading_Option = 1; // Blinn Phong Shading in simple.frag
  }
  else if (key==GLFW_KEY_2 && action == GLFW_PRESS) {
    Shading_Option = 2; // Toon Shading in simple.frag
  }
}

void cursor_callback(GLFWwindow * window, double x, double y) {
  int sensitivity = 10;
  camera_view = glm::translate(camera_view, glm::vec3{-x/sensitivity,y/sensitivity,0.0f});
  glfwSetCursorPos(window,0,0);
  update_camera();
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  //int sensitivity = 10;
  camera_view = glm::translate(camera_view, glm::vec3{0.0f, 0.0f, 0.1f*AU*yoffset});
  //camera_view = glm::rotate(camera_view, -0.1f, glm::vec3{0.0f, 0.0f, 0.1f*xoffset});
  // rotation, sometimes black screen
  update_camera();
}

// calculate fps and show in window title
void show_fps() {
  ++frames_per_second;
  double current_time = glfwGetTime();
  if(current_time - last_second_time >= 1.0) {
    std::string title{"OpenGL Framework - "};
    title += std::to_string(frames_per_second) + " fps";

    glfwSetWindowTitle(window, title.c_str());
    frames_per_second = 0;
    last_second_time = current_time;
  }
}

void quit(int status) {
  // free opengl resources
  glDeleteProgram(planet_program);
  glDeleteBuffers(1, &planet_object.vertex_BO);
  glDeleteVertexArrays(1, &planet_object.element_BO);
  glDeleteVertexArrays(1, &planet_object.vertex_AO);

  glDeleteProgram(starCloud_program);
  glDeleteBuffers(1, &starfield_object.vertex_BO);
  glDeleteVertexArrays(1, &starfield_object.element_BO);
  glDeleteVertexArrays(1, &starfield_object.vertex_AO);

  glDeleteProgram(orbit_program);
  glDeleteBuffers(1, &orbit_object.vertex_BO);
  glDeleteVertexArrays(1, &orbit_object.element_BO);
  glDeleteVertexArrays(1, &orbit_object.vertex_AO);

  // free glfw resources
  glfwDestroyWindow(window);
  glfwTerminate();

  std::exit(status);
}