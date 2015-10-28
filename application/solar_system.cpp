
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

#include <math.h> // log2()

#include <cstdlib>
#include <iostream>


// use gl definitions from glbinding 
using namespace gl;

//Defina Astronomical Unit(AU) unit of length roughly equal to the distance from the earth to the sun 
const float AU = 30.0f;

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

// cpu representation of model
model planet_model{};
model sun{};
// holds gpu representation of model
struct model_object {
  GLuint vertex_AO = 0;
  GLuint vertex_BO = 0;
  GLuint element_BO = 0;
};
model_object planet_object;

// camera matrices
glm::mat4 camera_view = glm::translate(glm::mat4{}, glm::vec3{50.0f, 0.0f, 150.0f});
glm::mat4 camera_projection{1.0f};

// uniform locations
GLint location_normal_matrix = -1;
GLint location_model_matrix = -1;
GLint location_view_matrix = -1;
GLint location_projection_matrix = -1;

// path to the resource folders
std::string resource_path{};

/////////////////////////// forward declarations //////////////////////////////
void quit(int status);
void update_view(GLFWwindow* window, int width, int height);
void update_camera();
void update_uniform_locations();
void update_shader_programs();
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void initialize_geometry();
void show_fps();
void render();

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
   changeDirectory = "/..";
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
  initialize_geometry();

  // enable depth testing
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  
  // rendering loop
  while(!glfwWindowShouldClose(window)) {
    // query input
    glfwPollEvents();
    // clear buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // draw geometry
    render();
    // swap draw buffer to front
    glfwSwapBuffers(window);
    // display fps
    show_fps();
  }

  quit(EXIT_SUCCESS);
}

///////////////////////// initialisation functions ////////////////////////////
// load models
void initialize_geometry() {
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

///////////////////////////// render functions ////////////////////////////////
// render model
//Actual size of the planets referenced to the earth.
void render() {
//Sun
glm::mat4 sunSize = glm::scale(glm::mat4{}, glm::vec3{10.90f});  // scaled down
  glm::mat4 model_matrixSun = glm::rotate(sunSize, float(glfwGetTime()), glm::vec3{0.0f, 1.0f, 0.0f}); // axis of rotation
  model_matrixSun = glm::translate(model_matrixSun, glm::vec3{0.0f, 0.0f, log2( 0.0f )}); // radius of the rotation axis defined in AU
  glUniformMatrix4fv(location_model_matrix, 1, GL_FALSE, glm::value_ptr(model_matrixSun));
  // extra matrix for normal transformation to keep them orthogonal to surface
  glm::mat4 normal_matrixSun = glm::inverseTranspose(camera_view * model_matrixSun);
  glUniformMatrix4fv(location_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrixSun));

  glBindVertexArray(planet_object.vertex_AO);
  utils::validate_program(simple_program);
  // draw bound vertex array as triangles using bound shader
  glDrawElements(GL_TRIANGLES, GLsizei(planet_model.indices.size()), model::INDEX.type, NULL);


  //Mercury
  glm::mat4 MercurySize = glm::scale(glm::mat4{}, glm::vec3{ 0.3829f });
  glm::mat4 model_matrixMercury = glm::rotate(MercurySize, float(glfwGetTime()), glm::vec3{ 0.0f, 1.0f, 0.0f }); // axis of rotation
  model_matrixMercury = glm::translate(model_matrixMercury, glm::vec3{ 0.0f, 0.0f, log2( 0.4f*AU )}); // radius of the rotation axis defined in AU
  glUniformMatrix4fv(location_model_matrix, 1, GL_FALSE, glm::value_ptr(model_matrixMercury));
  // extra matrix for normal transformation to keep them orthogonal to surface
  glm::mat4 normal_matrixMercury = glm::inverseTranspose(camera_view * model_matrixMercury);
  glUniformMatrix4fv(location_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrixMercury));

  glBindVertexArray(planet_object.vertex_AO);
  utils::validate_program(simple_program);
  // draw bound vertex array as triangles using bound shader
  glDrawElements(GL_TRIANGLES, GLsizei(planet_model.indices.size()), model::INDEX.type, NULL);

  //Venus
  glm::mat4 VenusSize = glm::scale(glm::mat4{}, glm::vec3{ 0.9499f });
  glm::mat4 model_matrixVenus = glm::rotate(VenusSize, float(glfwGetTime()), glm::vec3{ 0.0f, 1.0f, 0.0f }); // axis of rotation
  model_matrixVenus = glm::translate(model_matrixVenus, glm::vec3{ 0.0f, 0.0f, log2( 0.7f * AU )}); // radius of the rotation axis defined in AU
  glUniformMatrix4fv(location_model_matrix, 1, GL_FALSE, glm::value_ptr(model_matrixVenus));
  // extra matrix for normal transformation to keep them orthogonal to surface
  glm::mat4 normal_matrixVenus = glm::inverseTranspose(camera_view * model_matrixVenus);
  glUniformMatrix4fv(location_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrixVenus));

  glBindVertexArray(planet_object.vertex_AO);
  utils::validate_program(simple_program);
  // draw bound vertex array as triangles using bound shader
  glDrawElements(GL_TRIANGLES, GLsizei(planet_model.indices.size()), model::INDEX.type, NULL);


  //Earth
  glm::mat4 EarthSize = glm::scale(glm::mat4{}, glm::vec3{ 1.0f });
  glm::mat4 model_matrixEarth = glm::rotate(EarthSize, float(glfwGetTime()), glm::vec3{ 0.0f, 1.0f, 0.0f }); // axis of rotation
  model_matrixEarth = glm::translate(model_matrixEarth, glm::vec3{ 0.0f, 0.0f, log2( AU )}); // radius of the rotation axis defined in AU
  glUniformMatrix4fv(location_model_matrix, 1, GL_FALSE, glm::value_ptr(model_matrixEarth));
  // extra matrix for normal transformation to keep them orthogonal to surface
  glm::mat4 normal_matrixEarth = glm::inverseTranspose(camera_view * model_matrixEarth);
  glUniformMatrix4fv(location_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrixEarth));

  glBindVertexArray(planet_object.vertex_AO);
  utils::validate_program(simple_program);
  // draw bound vertex array as triangles using bound shader
  glDrawElements(GL_TRIANGLES, GLsizei(planet_model.indices.size()), model::INDEX.type, NULL);


  //Moon
  glm::mat4 MoonSize = glm::scale(model_matrixEarth, glm::vec3{ 0.273f });
  glm::mat4 model_matrixMoon = glm::rotate(MoonSize, float(glfwGetTime()), glm::vec3{ 0.0f, 1.0f, 0.0f }); // axis of rotation
  model_matrixMoon = glm::translate(model_matrixMoon, glm::vec3{ 0.0f, 0.0f, log2( 0.2f * AU )}); // radius of the rotation axis defined in AU
  glUniformMatrix4fv(location_model_matrix, 1, GL_FALSE, glm::value_ptr(model_matrixMoon));
  // extra matrix for normal transformation to keep them orthogonal to surface
  glm::mat4 normal_matrixMoon = glm::inverseTranspose(camera_view * model_matrixMoon);
  glUniformMatrix4fv(location_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrixMoon));

  glBindVertexArray(planet_object.vertex_AO);
  utils::validate_program(simple_program);
  // draw bound vertex array as triangles using bound shader
  glDrawElements(GL_TRIANGLES, GLsizei(planet_model.indices.size()), model::INDEX.type, NULL);

  //Mars
  glm::mat4 MarsSize = glm::scale(glm::mat4{}, glm::vec3{ 0.533f });
  glm::mat4 model_matrixMars = glm::rotate(MarsSize, float(glfwGetTime()), glm::vec3{ 0.0f, 1.0f, 0.0f }); // axis of rotation
  model_matrixMars = glm::translate(model_matrixMars, glm::vec3{ 0.0f, 0.0f, log2( 1.5f * AU )}); // radius of the rotation axis defined in AU
  glUniformMatrix4fv(location_model_matrix, 1, GL_FALSE, glm::value_ptr(model_matrixMars));
  // extra matrix for normal transformation to keep them orthogonal to surface
  glm::mat4 normal_matrixMars = glm::inverseTranspose(camera_view * model_matrixMars);
  glUniformMatrix4fv(location_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrixMars));

  glBindVertexArray(planet_object.vertex_AO);
  utils::validate_program(simple_program);
  // draw bound vertex array as triangles using bound shader
  glDrawElements(GL_TRIANGLES, GLsizei(planet_model.indices.size()), model::INDEX.type, NULL);

  //Jupiter
  glm::mat4 JupiterSize = glm::scale(glm::mat4{}, glm::vec3{ 11.209f });
  glm::mat4 model_matrixJupiter = glm::rotate(JupiterSize, float(glfwGetTime()), glm::vec3{ 0.0f, 1.0f, 0.0f }); // axis of rotation
  model_matrixJupiter = glm::translate(model_matrixJupiter, glm::vec3{ 0.0f, 0.0f, log2( 5.2f * AU )}); // radius of the rotation axis defined in AU
  glUniformMatrix4fv(location_model_matrix, 1, GL_FALSE, glm::value_ptr(model_matrixJupiter));
  // extra matrix for normal transformation to keep them orthogonal to surface
  glm::mat4 normal_matrixJupiter = glm::inverseTranspose(camera_view * model_matrixJupiter);
  glUniformMatrix4fv(location_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrixJupiter));

  glBindVertexArray(planet_object.vertex_AO);
  utils::validate_program(simple_program);
  // draw bound vertex array as triangles using bound shader
  glDrawElements(GL_TRIANGLES, GLsizei(planet_model.indices.size()), model::INDEX.type, NULL);

  //Saturn
  glm::mat4 SaturnSize = glm::scale(glm::mat4{}, glm::vec3{ 9.4492f });
  glm::mat4 model_matrixSaturn = glm::rotate(SaturnSize, float(glfwGetTime()), glm::vec3{ 0.0f, 1.0f, 0.0f }); // axis of rotation
  model_matrixSaturn = glm::translate(model_matrixSaturn, glm::vec3{ 0.0f, 0.0f, log2( 9.5f * AU )}); // radius of the rotation axis defined in AU
  glUniformMatrix4fv(location_model_matrix, 1, GL_FALSE, glm::value_ptr(model_matrixSaturn));
  // extra matrix for normal transformation to keep them orthogonal to surface
  glm::mat4 normal_matrixSaturn = glm::inverseTranspose(camera_view * model_matrixSaturn);
  glUniformMatrix4fv(location_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrixSaturn));

  glBindVertexArray(planet_object.vertex_AO);
  utils::validate_program(simple_program);
  // draw bound vertex array as triangles using bound shader
  glDrawElements(GL_TRIANGLES, GLsizei(planet_model.indices.size()), model::INDEX.type, NULL);


  //Uranus
  glm::mat4 UranusSize = glm::scale(glm::mat4{}, glm::vec3{ 3.929f });
  glm::mat4 model_matrixUranus = glm::rotate(UranusSize, float(glfwGetTime()), glm::vec3{ 0.0f, 1.0f, 0.0f }); // axis of rotation
  model_matrixUranus = glm::translate(model_matrixUranus, glm::vec3{ 0.0f, 0.0f, log2( 19.2f * AU )}); // radius of the rotation axis defined in AU
  glUniformMatrix4fv(location_model_matrix, 1, GL_FALSE, glm::value_ptr(model_matrixUranus));
  // extra matrix for normal transformation to keep them orthogonal to surface
  glm::mat4 normal_matrixUranus = glm::inverseTranspose(camera_view * model_matrixUranus);
  glUniformMatrix4fv(location_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrixUranus));

  glBindVertexArray(planet_object.vertex_AO);
  utils::validate_program(simple_program);
  // draw bound vertex array as triangles using bound shader
  glDrawElements(GL_TRIANGLES, GLsizei(planet_model.indices.size()), model::INDEX.type, NULL);

  //Neptune
  glm::mat4 NeptuneSize = glm::scale(glm::mat4{}, glm::vec3{ 3.883f });
  glm::mat4 model_matrixNeptune = glm::rotate(NeptuneSize, float(glfwGetTime()), glm::vec3{ 0.0f, 1.0f, 0.0f }); // axis of rotation
  model_matrixNeptune = glm::translate(model_matrixNeptune, glm::vec3{ 0.0f, 0.0f, log2( 18.0f * AU )}); // radius of the rotation axis defined in AU
  glUniformMatrix4fv(location_model_matrix, 1, GL_FALSE, glm::value_ptr(model_matrixNeptune));
  // extra matrix for normal transformation to keep them orthogonal to surface
  glm::mat4 normal_matrixNeptune = glm::inverseTranspose(camera_view * model_matrixNeptune);
  glUniformMatrix4fv(location_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrixNeptune));

  glBindVertexArray(planet_object.vertex_AO);
  utils::validate_program(simple_program);
  // draw bound vertex array as triangles using bound shader
  glDrawElements(GL_TRIANGLES, GLsizei(planet_model.indices.size()), model::INDEX.type, NULL);


 
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
  glUniformMatrix4fv(location_projection_matrix, 1, GL_FALSE, glm::value_ptr(camera_projection));
}

// update camera transformation
void update_camera() {
  // vertices are transformed in camera space, so camera transform must be inverted
  glm::mat4 inv_camera_view = glm::inverse(camera_view);
  // upload matrix to gpu
  glUniformMatrix4fv(location_view_matrix, 1, GL_FALSE, glm::value_ptr(inv_camera_view));
}

// load shaders and update uniform locations
void update_shader_programs() {
  try {
    // throws exception when compiling was unsuccessfull
    GLuint new_program = shader_loader::program(resource_path + "shaders/simple.vert",
                                                resource_path + "shaders/simple.frag");
    // free old shader
    glDeleteProgram(simple_program);
    // save new shader
    simple_program = new_program;
    // bind shader
    glUseProgram(simple_program);
    // after shader is recompiled uniform locations may change
    update_uniform_locations();

    // upload view uniforms to new shader
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    update_view(window, width, height);
    update_camera();
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
  else if(key == GLFW_KEY_W && action == GLFW_PRESS) {
    camera_view = glm::translate(camera_view, glm::vec3{0.0f, 0.0f, -0.1f*AU});
    update_camera();
  }
  else if(key == GLFW_KEY_S && action == GLFW_PRESS) {
    camera_view = glm::translate(camera_view, glm::vec3{0.0f, 0.0f, 0.1f*AU});
    update_camera();
  }
  else if(key == GLFW_KEY_UP && action == GLFW_PRESS) {
      camera_view = glm::translate(camera_view, glm::vec3{0.0f, 0.1f*AU, 0.0f});
      update_camera();
  }
  else if(key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
      camera_view = glm::translate(camera_view, glm::vec3{0.0f, -0.1f*AU, 0.0f});
      update_camera();
  }
  else if(key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
      camera_view = glm::translate(camera_view, glm::vec3{0.1f*AU, 0.0f, 0.0f});
      update_camera();
  }
  else if(key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
      camera_view = glm::translate(camera_view, glm::vec3{-0.1f*AU, 0.0f, 0.0f});
      update_camera();
  }
    
  else if(key == GLFW_KEY_W && action == GLFW_REPEAT) {
      camera_view = glm::translate(camera_view, glm::vec3{0.0f, 0.0f, -0.1f*AU});
      update_camera();
  }
  else if(key == GLFW_KEY_S && action == GLFW_REPEAT) {
      camera_view = glm::translate(camera_view, glm::vec3{0.0f, 0.0f, 0.1f*AU});
      update_camera();
  }
  else if(key == GLFW_KEY_UP && action == GLFW_REPEAT) {
      camera_view = glm::translate(camera_view, glm::vec3{0.0f, 0.1f*AU, 0.0f});
      update_camera();
  }
  else if(key == GLFW_KEY_DOWN && action == GLFW_REPEAT) {
      camera_view = glm::translate(camera_view, glm::vec3{0.0f, -0.1f*AU, 0.0f});
      update_camera();
  }
  else if(key == GLFW_KEY_RIGHT && action == GLFW_REPEAT) {
      camera_view = glm::translate(camera_view, glm::vec3{0.1f*AU, 0.0f, 0.0f});
      update_camera();
  }
  else if(key == GLFW_KEY_LEFT && action == GLFW_REPEAT) {
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