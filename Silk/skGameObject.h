#pragma once

//libs
#include <glm/gtc/matrix_transform.hpp>

#include "model/skModel.h"

// std
#include <memory>
#include <unordered_map>

namespace sk
{
	struct TransformComponent
	{
		glm::vec3 translation{}; // (position offset)
		glm::vec3 scale{ 1.f, 1.f, 1.f};
		glm::vec3 rotation{};

		// Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
		// Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
		// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
		// This is more efficient than our previous way of computing the rotation.
		glm::mat4 mat4();

		// transform for normal vectors. requires a different procedure (inverse-transpose of model transformation matrix --> [(M)^-1]^T )
		glm::mat3 normalMatrix();
	};

	class skGameObject
	{
	public:
		using id_t = unsigned int;
		using Map = std::unordered_map<id_t, skGameObject>;

		static skGameObject createGameObject()
		{
			static id_t currentId = 0;
			return skGameObject{ currentId++ }; // assigns a unique id each on call.
		}

		// delete copy constructors
		skGameObject(const skGameObject &) = delete;
		skGameObject &operator=(const skGameObject &) = delete;
		// default move operators (somehow used when std::move() is used?)
		skGameObject(skGameObject&&) = default;
		skGameObject& operator=(skGameObject&&) = default;

		inline id_t getId() const { return m_id; }

		std::shared_ptr<skModel> model{};
		glm::vec3 color{};
		TransformComponent transform{};

	private:
		// constructor is private to ensure every game object has a unique id
		skGameObject(id_t objId) : m_id{ objId } {}
		id_t m_id;
	};
} // namespace sk
