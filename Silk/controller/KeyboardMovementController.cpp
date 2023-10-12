#include "KeyboardMovementController.h"

namespace sk
{

	void KeyboardMovementController::moveInPlaneXZ(GLFWwindow* window, float dt, skGameObject& gameObject)
	{
		glm::vec3 rotate{ 0 };
		// rotation about the y axis simulates looking "left" or "right"
		if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS) rotate.y += 1.f;
		if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) rotate.y -= 1.f;
		// rotation about the x axis simulates looking "up" or "down"
		if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS) rotate.x += 1.f;
		if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS) rotate.x -= 1.f;

		// glm::normalize breaks when rotate is (0, 0, 0), so to avoid that, we check this condition
		// one way to do it is to use dot product.
		if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
			gameObject.transform.rotation += lookSpeed * dt * glm::normalize(rotate);

		// limit pitch values between about +/- 85ish degrees
		gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f); // prevents object from going upside down
		gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>()); // prevents overflow caused by multiple revolutions in the same direction

		float yaw = gameObject.transform.rotation.y; // facing orientation in XZ plane (angle in radians).
		const glm::vec3 forwardDir{ sin(yaw), 0.f, cos(yaw) };
		const glm::vec3 rightDir{ forwardDir.z, 0.f, -forwardDir.x };
		const glm::vec3 upDir{ 0.f, -1.f, 0.f };

		glm::vec3 moveDir{ 0.f };
		if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
		if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
		if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
		if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;
		if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) moveDir += upDir;
		if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) moveDir -= upDir;

		if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
			gameObject.transform.translation += lookSpeed * dt * glm::normalize(moveDir);
	}

} // namespace sk