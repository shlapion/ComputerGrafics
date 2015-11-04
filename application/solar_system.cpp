
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
const int number_of_stars = 10000;

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
GLuint simple_program = 0;
GLuint starCloud_program = 0;

// cpu representation of model
model planet_model{};
model star_model{};

// holds gpu representation of model
struct model_object {
  GLuint vertex_AO = 0;
  GLuint vertex_BO = 0;
  GLuint element_BO = 0;
};
model_object planet_object;
model_object starfield_object;

// camera matrices
glm::mat4 camera_view = glm::translate(glm::mat4{}, glm::vec3{50.0f, 0.0f, 150.0f});
glm::mat4 camera_projection{1.0f};

// uniform locations
GLint location_normal_matrix = -1;
GLint location_model_matrix = -1;
GLint location_view_matrix = -1;
GLint location_projection_matrix = -1;

// starClaud location
GLint starCloud_view_matrix = -1;
GLint starCloud_projection_matrix = -1;

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
void generate_solar_system();
std::vector<float>  generate_starfield(int num_stars);
void initialize_planet_geometry();
void initialize_starfield_geometry();
void show_fps();
void render();
void render_planet();
void render_starfield();

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
  initialize_planet_geometry();
  initialize_starfield_geometry();

  // enable depth testing
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  
  // rendering loop
  while(!glfwWindowShouldClose(window)) {
    // query input
    glfwPollEvents();
    // clear buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//	std::vector<float> starPosition;
//	std::generate(starPosition.begin(), starPosition.end(), std::rand());

    generate_solar_system();

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

void generate_solar_system() {
  Planet* sun = new Planet{"sun",0.0f,0.0f,10.90f,nullptr};
  Planet* mercury = new Planet{"mercury",0.4f*AU,2.0f,0.3829f,sun};
  Planet* venus = new Planet{"venus",0.7f*AU,2.0f,0.9499f,sun};
  Planet* earth = new Planet{"earth",AU,1.0f,1.0f,sun};
  Planet* mars = new Planet{"mars",1.5f*AU,1.0f,0.533f,sun};
  Planet* jupiter = new Planet{"jupiter",3.2f*AU,0.8f,6.0f,sun};
  Planet* saturn = new Planet{"saturn",5.5f*AU,0.7f,5.0f,sun};
  Planet* uranus = new Planet{"uranus",19.2f*AU,0.5f,3.929f,sun};
  Planet* neptun = new Planet{"neptun",18.0f*AU,0.4f,3.883f,sun};

  Planet* moon = new Planet{"moon",0.1f*AU,3.0f,0.273f,earth};

  solarSystem.push_back(sun);
  solarSystem.push_back(mercury);
  solarSystem.push_back(venus);
  solarSystem.push_back(earth);
  solarSystem.push_back(mars);
  solarSystem.push_back(jupiter);
  solarSystem.push_back(saturn);
  solarSystem.push_back(uranus);
  solarSystem.push_back(neptun);

  solarSystem.push_back(moon);
}

std::vector<float>  generate_starfield(int num_stars) {
  std::vector<float> stars;
  for (int i=0;i<num_stars;i++) {
    float x = std::rand();
    float y = std::rand();
    float z = std::rand();
    stars.push_back(x);
    stars.push_back(y);
    stars.push_back(z);
  }
  return stars;
}

///////////////////////// initialisation functions ////////////////////////////
// load models
void initialize_planet_geometry() {
  planet_model = model_loader::obj(resource_path + "models/Planet.obj", model::NORMAL);
 

  // generate vertex array object
  glGenVertexArrays(1, &planet_object.vertex_AO);
  // bind the array for attaching buffers
  glBindVertexArray(planet_object.vertex_AO);

  // generate generic buffer
  glGenBuffers(1, &planet_object.vertex_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ARRAY_BUFFER, planet_object.vertex_BO);
  // configure currently bound array buffer
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * planet_model.data.size(), planet_model.data.data(), GL_STATIC_DRAW);

  // activate first attribute on gpu
  glEnableVertexAttribArray(0);
  // first attribute is 3 floats with no offset & stride
  glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::POSITION]);
  // activate second attribute on gpu
  glEnableVertexAttribArray(1);
  // second attribute is 3 floats with no offset & stride
  glVertexAttribPointer(1, model::NORMAL.components, model::NORMAL.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::NORMAL]);

   // generate generic buffer
  glGenBuffers(1, &planet_object.element_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planet_object.element_BO);
  // configure currently bound array buffer
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, model::INDEX.size * planet_model.indices.size(), planet_model.indices.data(), GL_STATIC_DRAW);
}


void initialize_starfield_geometry() {
  std::vector<float> starfield_data = generate_starfield(number_of_stars);

  star_model = {starfield_data, model::POSITION | model::NORMAL};

  // generate vertex array object
  glGenVertexArrays(1, &starfield_object.vertex_AO);
  // bind the array for attaching buffers
  glBindVertexArray(starfield_object.vertex_AO);

  // generate generic buffer
  glGenBuffers(1, &starfield_object.vertex_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ARRAY_BUFFER, starfield_object.vertex_BO);
  // configure currently bound array buffer
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * star_model.data.size(), star_model.data.data(), GL_STATIC_DRAW);

  // activate first attribute on gpu
  glEnableVertexAttribArray(0);
  // first attribute is 3 floats with no offset & stride
  glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, star_model.vertex_bytes, star_model.offsets[model::POSITION]);

  glEnableVertexAttribArray(1);
  // second attribute is 3 floats with no offset & stride
  glVertexAttribPointer(1, model::NORMAL.components, model::NORMAL.type, GL_FALSE, star_model.vertex_bytes, star_model.offsets.at(model::NORMAL));

  // generate generic buffer
  glGenBuffers(1, &planet_object.element_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planet_object.element_BO);
  // configure currently bound array buffer
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, model::INDEX.size * star_model.indices.size(), star_model.indices.data(), GL_STATIC_DRAW);
}


///////////////////////////// render functions ////////////////////////////////
// render model
//Actual size of the planets referenced to the earth.
void render() {
  render_planet();
  render_starfield();
}

void render_planet() {
  glUseProgram(simple_program);
  update_shader_programs();


  // draw geometry
  for (auto p: solarSystem) { // maybe better in renderer fro planets?
    glm::vec3 position;
    glm::mat4 transformation;
    glm::mat4 normal_transformation;

    float time = glfwGetTime();
    Planet* current = p;
    while (current != nullptr) { // maybe better in renderer. Interesting moonaction....
      position.x += glm::cos(time * current->speed()) * current->distance();
      position.y += glm::sin(time * current->speed()) * current->distance();
      current = current->child();
    }
    transformation = glm::translate(transformation,position);
    transformation = glm::scale(transformation,glm::vec3 {current->size()});

//    glm::mat4 model_matrix = glm::rotate(transformation, float(glfwGetTime()*current->speed()), glm::vec3{ 0.0f, 1.0f, 0.0f }); // axis of rotation

    glUniformMatrix4fv(location_model_matrix, 1, GL_FALSE, glm::value_ptr(transformation));
    // extra matrix for normal transformation to keep them orthogonal to surface
    normal_transformation = glm::inverseTranspose(glm::inverse(camera_view) * transformation);
    glUniformMatrix4fv(location_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_transformation));

    glBindVertexArray(planet_object.vertex_AO);
    utils::validate_program(simple_program);
    // draw bound vertex array as triangles using bound shader
    glDrawElements(GL_TRIANGLES, GLsizei(planet_model.indices.size()), model::INDEX.type, NULL);

  }
}

void render_starfield() {
  glUseProgram(starCloud_program);
  update_shader_programs();



  glBindVertexArray(starfield_object.vertex_AO);
  utils::validate_program(starCloud_program);
  glDrawArrays(GL_POINT, 0, number_of_stars);
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
  camera_projection = glm::perspective(fov_y, aspect, 0.1f, 1000.0f);
  // upload matrix to gpu
  glUseProgram(simple_program);
  glUniformMatrix4fv(location_projection_matrix, 1, GL_FALSE, glm::value_ptr(camera_projection));

  glUseProgram(starCloud_program);
  glUniformMatrix4fv(starCloud_projection_matrix, 1, GL_FALSE, glm::value_ptr(camera_projection));
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
  glUseProgram(simple_program);
  glUniformMatrix4fv(location_view_matrix, 1, GL_FALSE, glm::value_ptr(inv_camera_view));

  glUseProgram(starCloud_program);
  glUniformMatrix4fv(starCloud_view_matrix, 1,GL_FALSE, glm::value_ptr(inv_camera_view));
}

// load shaders and update uniform locations
void update_shader_programs() {
  /* defined at the top!!
    GLuint simple_program = 0;
    GLuint starCloud_program = 0;
    */
    std::vector<Shader_Programm> shaders;
  shaders.push_back({"shaders/simple.vert", "shaders/simple.frag", &simple_program});
  shaders.push_back({"shaders/starCloud.vert","shaders/starCloud.frag", &starCloud_program});

  try {
    for (auto item : shaders) {
      // throws exception when compiling was unsuccessfull
      GLuint new_program = shader_loader::program(resource_path + item.vertex_path,
                                                  resource_path + item.frag_path);
      // free old shader
      glDeleteProgram(*item.programm);
      // save new shader
      *item.programm = new_program;
      // bind shader
      glUseProgram(*item.programm);
      // after shader is recompiled uniform locations may change
      //update_uniform_locations();

      // upload view uniforms to new shader
      int width, height;
      glfwGetFramebufferSize(window, &width, &height);
      update_view(window, width, height);
      update_camera();
    }
  }
  catch(std::exception&) {
    // dont crash, allow another try
  }
}

// update shader uniform locations
void update_uniform_locations() {
  location_normal_matrix = glGetUniformLocation(simple_program, "NormalMatrix");
  location_model_matrix = glGetUniformLocation(simple_program, "ModelMatrix");
  location_view_matrix = glGetUniformLocation(simple_program, "ViewMatrix");
  location_projection_matrix = glGetUniformLocation(simple_program, "ProjectionMatrix");

  starCloud_projection_matrix = glGetUniformLocation(starCloud_program, "ProjectionMatrix");
  starCloud_view_matrix = glGetUniformLocation(starCloud_program, "ViewMatrix");
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
  glDeleteProgram(simple_program);
  glDeleteBuffers(1, &planet_object.vertex_BO);
  glDeleteVertexArrays(1, &planet_object.element_BO);
  glDeleteVertexArrays(1, &planet_object.vertex_AO);

  // free glfw resources
  glfwDestroyWindow(window);
  glfwTerminate();

  std::exit(status);
}