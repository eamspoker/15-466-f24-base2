#include "Mode.hpp"

#include "Scene.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>


struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, space, shift;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	//fish
	Scene::Transform *fish = nullptr;
	glm::vec3 original_pos;
	glm::vec3 dist_to_camera;
	glm::mat4x3 fish_box;
	glm::quat fish_base_rotation;

	// fins to wobble
	Scene::Transform *fin1 = nullptr;
	glm::quat fin1_base_rotation;

	Scene::Transform *fin2 = nullptr;
	glm::quat fin2_base_rotation;

	struct Person {
		Scene::Transform *head = nullptr;
		Scene::Transform *body = nullptr;

		// right and left arms
		glm::quat rshoulder_base_rot;
		Scene::Transform *rforearm = nullptr;
		Scene::Transform *rshoulder = nullptr;
		Scene::Transform *rhand = nullptr;

		glm::quat lshoulder_base_rot;
		Scene::Transform *lforearm = nullptr;
		Scene::Transform *lshoulder = nullptr;
		Scene::Transform *lhand = nullptr;


		// right and left legs
		glm::quat rthigh_base_rot;
		Scene::Transform *rthigh = nullptr;
		Scene::Transform *rcalf = nullptr;
		Scene::Transform *rfoot = nullptr;

		glm::quat lthigh_base_rot;
		Scene::Transform *lthigh = nullptr;
		Scene::Transform *lcalf = nullptr;
		Scene::Transform *lfoot = nullptr;

		// spherical collider
		Scene::Transform *collider = nullptr;

		// for moving around
		glm::vec3 dest = glm::vec3(0.0f);
		glm::vec3 from = glm::vec3(0.0f);
		float curr_time = 0.0f;
		float final_time = 0.0f;

		glm::vec3 original_pos;
	} person1, person2, person3;

	std::vector<Person> people;

	Scene::Transform *end_box = nullptr;

	
	
	// radius of spherical fish collider
	float fish_radius = 2.0f;
	float person_radius = 4.0f;
	float wobble = 0.0f;
	bool isDead = false;
	bool isWin = false;
	
	//camera:
	Scene::Camera *camera = nullptr;

};
