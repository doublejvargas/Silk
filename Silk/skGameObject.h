#pragma once

#include "model/skModel.h"

// std
#include <memory>

namespace sk
{
	struct Transform2dComponent
	{
		glm::vec2 translation{}; // (position offset)
		glm::vec2 scale{ 1.f, 1.f };
		float rotation; 

		// scaling  | scale.x	  0   |
		//		    |    0	   scale.y|
		//
		//
		// rotating | cos		-sin |
		//			| sin		 cos |

		glm::mat2 mat2() { 
			const float s = glm::sin(rotation);
			const float c = glm::cos(rotation);
			glm::mat2 rotMatrix{ {c, s}, {-s, c} };

			glm::mat2 scaleMat{ {scale.x, .0f}, {.0f, scale.y} };
			return rotMatrix * scaleMat; 
		};
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
		Transform2dComponent transform2D{};

	private:
		// constructor is private to ensure every game object has a unique id
		skGameObject(id_t objId) : m_id{ objId } {}
		id_t m_id;
	};
} // namespace sk
