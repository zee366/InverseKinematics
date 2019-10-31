#include "MatrixMath.h"

namespace mm {

	float determinant(glm::mat2& mat) {
		return glm::determinant(mat);
	}

	bool invertible(glm::mat2& mat) {
		return determinant(mat) != 0;
	}

	glm::mat2 inverse(glm::mat2& mat) {
		return glm::inverse(mat);
	}

	glm::mat2x3 transpose(glm::mat3x2& mat) {
		return glm::transpose(mat);
	}

	glm::vec3 findXYZ(float radius, float theta, float phi) {
		float x = radius * glm::sin(theta) * glm::cos(phi);
		float y = radius * glm::sin(theta) * glm::sin(phi);
		float z = radius * cos(theta);

		return glm::vec3(x, y, z);
	}

	glm::mat2x3 calcJPlus(glm::mat3x2& j) {
		glm::mat2x3 jT = transpose(j);
		glm::mat2 jjT = j * jT;
		if (invertible(jjT)) {
			glm::mat2 jI = inverse(jjT);
			return jT * jI;
		}
	}

	void printMatrix(glm::mat3x2& mat) {
		for (int i = 0; i < 3; ++i) {
			for (int j = 0; j < 2; ++j) {
				std::cout << mat[i][j] << " ";
			}
			std::cout << std::endl;
		}
	}

	void printMatrix(glm::mat2x3& mat) {
		for (int i = 0; i < 2; ++i) {
			for (int j = 0; j < 3; ++j) {
				std::cout << mat[i][j] << " ";
			}
			std::cout << std::endl;
		}
	}

	Jacobian3x2::Jacobian3x2() :
	mat() {}

	Jacobian3x2::Jacobian3x2(float a, float b, float c, float theta, float phi, float psi) {
		float cosTheta = glm::cos(theta);
		float cosThetaPhi = glm::cos(theta + phi);
		float cosThetaPhiPsi = glm::cos(theta + phi + psi);

		float sinTheta = glm::sin(theta);
		float sinThetaPhi = glm::sin(theta + phi);
		float sinThetaPhiPsi = glm::sin(theta + phi + psi);

		// x values
		mat[0][0] = -a * sinTheta - b * sinThetaPhi - c * sinThetaPhiPsi;
		mat[1][0] = mat[0][0] + a * sinTheta;
		mat[2][0] = mat[1][0] + b * sinThetaPhi;

		// y values
		mat[0][1] = a * cosTheta + b * cosThetaPhi + c * cosThetaPhiPsi;
		mat[1][1] = mat[0][1] - a * cosTheta;
		mat[2][1] = mat[1][1] - b * cosThetaPhi;
	}
}