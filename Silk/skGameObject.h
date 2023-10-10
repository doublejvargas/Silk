#pragma once

//libs
#include <glm/gtc/matrix_transform.hpp>

#include "model/skModel.h"

// std
#include <memory>

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
		glm::mat4 mat4() {
			const float c3 = glm::cos(rotation.z);
			const float s3 = glm::sin(rotation.z);
			const float c2 = glm::cos(rotation.x);
			const float s2 = glm::sin(rotation.x);
			const float c1 = glm::cos(rotation.y);
			const float s1 = glm::sin(rotation.y);
			return glm::mat4{
				{
					scale.x * (c1 * c3 + s1 * s2 * s3),
					scale.x * (c2 * s3),
					scale.x * (c1 * s2 * s3 - c3 * s1),
					0.0f,
				},
				{
					scale.y * (c3 * s1 * s2 - c1 * s3),
					scale.y * (c2 * c3),
					scale.y * (c1 * c3 * s2 + s1 * s3),
					0.0f,
				},
				{
					scale.z * (c2 * s1),
					scale.z * (-s2),
					scale.z * (c1 * c2),
					0.0f,
				},
				{translation.x, translation.y, translation.z, 1.0f} };
		}
	};

	class skGameObject
	{
	public:
		using id_t = unsigned int;

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
