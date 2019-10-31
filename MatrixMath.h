#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <iostream>

namespace mm {
	const float PI = glm::pi<float>();
	const float TWO_PI = glm::two_pi<float>();
	const float HALF_PI = glm::half_pi<float>();
	const float QUARTER_PI = glm::quarter_pi<float>();

	struct Jacobian3x2 {
		glm::mat3x2 mat;

		Jacobian3x2();
		Jacobian3x2(float a, float b, float c, float theta, float phi, float psi);
	};

	float determinant(glm::mat2& mat);
	bool invertible(glm::mat2& mat);
	glm::mat2 inverse(glm::mat2& mat);
	glm::mat2x3 transpose(glm::mat3x2& mat);
	void printMatrix(glm::mat3x2& mat);
	void printMatrix(glm::mat2x3& mat);

	/*
	Returns the xyz coordinates of a point, given it's radius from the origin and angles theta and phi.
	Angles should be in radians.

	theta: the zenith angle, where theta=0 is directly above (the +Z axis), and theta=PI is directly below (the -Z axis),
	and theta=PI/2 is what you would consider an 'elevation' of 0 degrees, e.g., 0 <= theta <= PI.

	phi: the azimuth angle, where phi=0 is to the 'right' (the +X axis), and as you turn 'anticlockwise', phi=PI/2 (the +Y axis),
	phi=PI (the -X axis), phi=3*PI/2 (the -Y axis), and phi=2*PI - equivalent to the phi=0 (back to the +X axis). e.g., 0 <= phi < 2*PI
	*/
	glm::vec3 findXYZ(float radius, float theta, float phi);

	/*
	Returns the 2x3 (column major) pseudo-inverse of the Jacobian 3x2 matrix. The method checks if J * JTranspose is non-singular (determinant != 0)
	*/
	glm::mat2x3 calcJPlus(glm::mat3x2& J);
}
