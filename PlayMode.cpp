#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"
#include "TransparentColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <string>

#include <random>

GLuint hexapod_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > hexapod_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("out.pnct"));
	hexapod_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

GLuint transparent_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > transparent_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("transparent.pnct"));
	transparent_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > hexapod_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("out.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		// std::cout << mesh_name << std::endl;
		try {
			Mesh const &mesh = hexapod_meshes->lookup(mesh_name);
			scene.drawables.emplace_back(transform);
			Scene::Drawable &drawable = scene.drawables.back();

			drawable.pipeline = lit_color_texture_program_pipeline;

			drawable.pipeline.vao = hexapod_meshes_for_lit_color_texture_program;
			drawable.pipeline.type = mesh.type;
			drawable.pipeline.start = mesh.start;
			drawable.pipeline.count = mesh.count;
		} catch (const std::runtime_error& error) 
		{ // runtime error catching from: https://stackoverflow.com/questions/7491877/c-catch-runtime-error
			Mesh const &mesh = transparent_meshes->lookup(mesh_name);
			scene.drawables.emplace_back(transform);
			Scene::Drawable &drawable = scene.drawables.back();

			drawable.pipeline = transparent_color_texture_program_pipeline;
			drawable.pipeline.vao = transparent_meshes_for_lit_color_texture_program;
			drawable.pipeline.type = mesh.type;
			drawable.pipeline.start = mesh.start;
			drawable.pipeline.count = mesh.count;

		}

		

	});
});




glm::vec3 select_destination(float *time, glm::vec3 box) 
{
	// from https://cplusplus.com/reference/random/mersenne_twister_engine/min/
	 unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	 std::mt19937 generator (seed);  // mt19937 is a standard mersenne_twister_engine
	 *time = generator()%4 + 1;
	 return glm::vec3{generator()%15*((-2*generator()%2)+1), 
	 				generator()%15*((-2*generator()%2)+1), 10.0f - generator()%10};


}

PlayMode::PlayMode() : scene(*hexapod_scene) {

	std::vector<std::string> transform_names = {"Sphere", "Sphere.001", "Sphere.002", "Cube"};
	std::vector<std::string> person_names = {"Head", "Body", 
												"RShoulder", "RForearm", "RHand", 
												"LShoulder", "LForearm", "LHand",
												"LThigh", "LCalf", "LFoot", 
												"RThigh", "RCalf", "RFoot", "Sphere.004"};
	std::vector<Scene::Transform **> person1_pointers = {&person1.head, &person1.body, 
												&person1.rshoulder, &person1.rforearm, &person1.rhand,
												&person1.lshoulder, &person1.lforearm, &person1.lhand,
												&person1.lthigh, &person1.lcalf, &person1.lfoot,
												&person1.rthigh, &person1.rcalf, &person1.rfoot,
												&person1.collider};
	std::vector<Scene::Transform **> person2_pointers = {&person2.head, &person2.body, 
												&person2.rshoulder, &person2.rforearm, &person2.rhand,
												&person2.lshoulder, &person2.lforearm, &person2.lhand,
												&person2.lthigh, &person2.lcalf, &person2.lfoot,
												&person2.rthigh, &person2.rcalf, &person2.rfoot,
												&person2.collider};
	std::vector<Scene::Transform **> transform_pointers = {&fish, &fin1, &fin2, &end_box};

	//get pointers to leg for convenience:
	for (auto &transform : scene.transforms) {

		// from: https://www.geeksforgeeks.org/how-to-find-index-of-a-given-element-in-a-vector-in-cpp/
		auto iterator = std::find(transform_names.begin(), transform_names.end(), transform.name);
		if (iterator != transform_names.end())
		{
			size_t index = iterator - transform_names.begin();
			*transform_pointers[index] = &transform;
		} else {
			auto iterator_person = std::find(person_names.begin(), person_names.end(), transform.name);
			if (iterator_person != person_names.end())
			{
				size_t index = iterator_person - person_names.begin();
				*person1_pointers[index] = &transform;
			} else {

				// remove the <name of object>.001 from person 2 objects
				std::string recovered_name = transform.name.substr(0, transform.name.length()-4);
				auto iterator_person2 = std::find(person_names.begin(), person_names.end(), recovered_name);
				if (iterator_person2 != person_names.end())
				{
					size_t index = iterator_person2 - person_names.begin();
					*person2_pointers[index] = &transform;

				}
		}
	}
	}
	
	fin1_base_rotation = fin1->rotation;
	fin2_base_rotation = fin2->rotation;
	fish_base_rotation = fish->rotation;
	
	// set the player position, destination, and collider
	person1.head->position -= glm::vec3(0.0f, 0.0f, 15.0f);
	person1.original_pos =  person1.head->position;
	person1.from =  person1.head->position;
	person1.dest = select_destination(&person1.final_time, end_box->position);
	person1.collider->position = person1.head->position + person1.body->position;
	person1.lshoulder_base_rot = person1.lshoulder->rotation;
	person1.rshoulder_base_rot = person1.rshoulder->rotation;
	person1.rthigh_base_rot = person1.rthigh->rotation;
	person1.lthigh_base_rot = person1.lthigh->rotation;

	// do the same thing for person 2
	person2.head->position -= glm::vec3(0.0f, 0.0f, 10.0f);
	person2.original_pos =  person2.head->position;
	person2.from =  person2.head->position;
	person2.dest = select_destination(&person2.final_time, end_box->position);
	person2.collider->position = person2.head->position + person2.body->position;
	person2.lshoulder_base_rot = person2.lshoulder->rotation;
	person2.rshoulder_base_rot = person2.rshoulder->rotation;
	person2.rthigh_base_rot = person2.rthigh->rotation;
	person2.lthigh_base_rot = person2.lthigh->rotation;

	people.emplace_back(person1);
	people.emplace_back(person2);

	

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

	dist_to_camera = camera->transform->position - fish->position;
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {


	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_z) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.downs += 1;
			down.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_LSHIFT || evt.key.keysym.sym == SDLK_RSHIFT) {
			shift.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_LSHIFT || evt.key.keysym.sym == SDLK_RSHIFT) {
			shift.pressed = false;
			return true;	
		} 
		} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
			if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
				SDL_SetRelativeMouseMode(SDL_TRUE);
				return true;
			}
		}  else if (evt.type == SDL_MOUSEMOTION) {
		if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				-evt.motion.yrel / float(window_size.y)
			);
			glm::quat future_rot = glm::normalize(
				camera->transform->rotation
				* glm::angleAxis(-motion.x * (camera->fovy), 
					glm::vec3(0.0f, 1.0f, 0.0f))
			);

			if (!(glm::eulerAngles(future_rot).z > M_PI*0.75 ||
				glm::eulerAngles(future_rot).z < M_PI*0.25))
				camera->transform->rotation = future_rot;

			return true;
		}
	} else if ((isDead || isWin) && evt.type == SDL_MOUSEBUTTONUP) {
			fish->position = original_pos;
			person1.head->position = glm::vec3{5.0f, 5.0f, 5.0f} + fish->position;
			//reset button press counters:
			left.downs = 0;
			right.downs = 0;
			up.downs = 0;
			down.downs = 0;
			space.downs = 0;
			shift.downs = 0;
			isDead = false;
			isWin = false;
		} 


	return false;
}

void PlayMode::update(float elapsed) {

	// idea from https://stackoverflow.com/questions/3232318/sphere-sphere-collision-detection-reaction
	auto sphereCollision = [&](glm::vec3 center1, float radius1, glm::vec3 center2, float radius2)
	{
		float dist_squared = pow(glm::length(center1-center2),2.0f);
		float radii_squared = pow(radius1+radius2,2.0f);

		return dist_squared < radii_squared;
	};

	auto movePerson = [&](Person *person)
	{
		person->curr_time += elapsed;
		if (person->curr_time >= person->final_time)
		{
			person->from = person->head->position;
			person->dest = select_destination(&(person->final_time), end_box->position);
			person->curr_time = 0.0f;
		} else {
			// lerp between start and end position using current time
			float t = (person->final_time-person->curr_time)/person->final_time;

			person->head->position = 
				t*(person->from) + (1-t)*(person->dest);

			person1.collider->position = person1.head->position + person1.body->position;

		}

	};

	//slowly rotates through [0,1):
		wobble += elapsed / 10.0f;
		wobble -= std::floor(wobble);

	
	if (!isDead && !isWin)
	{
		
		//move characters and camera:
		{

			//combine inputs into a move:
			constexpr float PlayerSpeed = 10.0f;
			glm::vec3 move = glm::vec3(0.0f);
			if (left.pressed && !right.pressed) move.x =-1.0f;
			if (!left.pressed && right.pressed) move.x = 1.0f;
			if (down.pressed && !up.pressed) move.y =  1.0f;
			if (!down.pressed && up.pressed) move.y = -1.0f;
			if (space.pressed && !shift.pressed) move.z = -1.0f;
			if (space.pressed && shift.pressed) move.z = 1.0f;

			//make it so that moving diagonally doesn't go faster:
			if (move != glm::vec3(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;
		
			// adapted from the hexapod code
			fin1->rotation = fin1_base_rotation * glm::angleAxis(
				glm::radians(2.0f * std::sin(wobble * 2.0f * float(M_PI))),
				glm::vec3(1.0f, 0.0f, 0.0f)
				);

			fin2->rotation = fin2_base_rotation * glm::angleAxis(
				glm::radians(2.0f * std::sin(wobble * 2.0f * float(M_PI))),
				glm::vec3(1.0f, 0.0f, 0.0f)
				);

			for (auto person : people)
			{
				person.rshoulder->rotation = person.rshoulder_base_rot * glm::angleAxis(
				glm::radians(10.0f * std::sin(wobble * 3.0f * float(M_PI)+float(M_PI))),
				glm::vec3(0.0f, 1.0f, 0.0f)
				);
				person.lshoulder->rotation = person.lshoulder_base_rot * glm::angleAxis(
				glm::radians(10.0f * std::sin(wobble * 3.0f * float(M_PI)+float(M_PI))),
				glm::vec3(0.0f, 1.0f, 0.0f)
				);
				person.rthigh->rotation = person.rthigh_base_rot * glm::angleAxis(
				glm::radians(10.0f * std::sin(wobble * 3.0f * float(M_PI)+float(M_PI))),
				glm::vec3(0.0f, 1.0f, 0.0f)
				);
				person.lthigh->rotation = person.lthigh_base_rot * glm::angleAxis(
				glm::radians(10.0f * std::sin(wobble * 3.0f * float(M_PI)+float(M_PI))),
				glm::vec3(0.0f, 1.0f, 0.0f)
				);
			}
			

			auto prev_fish = fish->position;
			fish->position += glm::vec3{ move.y, move.x, move.z};

			fish->position += glm::vec3{ 0.0f, 0.0f, elapsed*0.2};

			


			// adjust position
			{
				if (fish->position.z < -13.0f)
				{
					fish->position.z = prev_fish.z;
				}

				if (fish->position.z > 15.0f)
				{
					fish->position.z = prev_fish.z;
				}

				if (abs(fish->position.x) >= 15.0f
					|| abs(fish->position.x + dist_to_camera.x) >= 15.0f)
				{
					fish->position.x = prev_fish.x;

				}

				if (abs(fish->position.y) >= 25.0f
					|| abs(fish->position.y + dist_to_camera.y) >= 25.0f)
				{
					fish->position.y = prev_fish.y;

				}

			}

			camera->transform->position = dist_to_camera + fish->position;



			fish->rotation = fish_base_rotation * glm::angleAxis(
				glm::radians(10.0f * std::sin(30.0f*wobble*float(M_PI))),
				glm::vec3(0.0f, 0.0f, 1.0f)
				);

			for (auto person : people)
			{
				movePerson(&person);
			}
			

			if (sphereCollision(fish->position, fish_radius, person1.head->position+person1.body->position, person_radius))
			{
				isDead = true;
			} else if(abs(fish->position.x - end_box->position.x) <= 1.0f &&
					abs(fish->position.y - end_box->position.y) <= 1.0f &&
					abs(fish->position.z - end_box->position.z) <= 1.0f)
			{
				isWin = true;
				
			}

	
		}
	 } else if (isWin)
		{

			fish->rotation = fish_base_rotation * glm::angleAxis(
				glm::radians(10.0f * std::sin(50.0f*wobble*float(M_PI))),
				glm::vec3(1.0f, 1.0f, 1.0f)
				);


		} else if (isDead)
		{
			fish->rotation = fish_base_rotation * glm::angleAxis(
				glm::radians(720.0f*std::sin(wobble)),
				glm::vec3(1.0f, 0.0f, 0.0f)
				);
		}


	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
	space.downs = 0;
	shift.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	

	glm::vec3 energy;
	if (isDead)
	{
		energy = glm::vec3(300.0f, 0.75f, 0.75f);
	} else if (isWin)
	{
		energy = glm::vec3(3.5f, 10.0f, 0.5f);
	}
	else
	{
		energy = glm::vec3(0.5f, 0.5f, 0.5f);
	}

	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(energy));

	glUseProgram(transparent_color_texture_program->program);
	glUniform1i(transparent_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(transparent_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(transparent_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(energy));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	// premultiplied alpha: https://stackoverflow.com/questions/28079159/opengl-glsl-texture-transparency
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	GL_ERRORS(); //print any errors produced by this setup code

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		constexpr float H = 0.09f;
		lines.draw_text("Mouse motion rotates camera; WASD moves; shift + space up, space down, z ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("Mouse motion rotates camera; WASD moves; shift + space up, space down, z ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
}
