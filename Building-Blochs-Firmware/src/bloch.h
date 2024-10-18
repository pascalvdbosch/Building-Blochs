#ifndef __BLOCH_H
#define __BLOCH_H

#include <Arduino.h>
#include <ArduinoEigenDense.h>
#include <AccelStepper.h>
#include <Module_Stepmotor.h>
#include <queue>

using namespace Eigen;

#define AXIS_X                {1, 0, 0}
#define AXIS_Y                {0, 1, 0}
#define AXIS_Z                {0, 0, 1}
// #define VECTOR_H              {0.707106, 0, 0.707106} // sqrt(2)/2

typedef std::queue<Vector3d> MovementQueue_t;

class BlochSphere
{
	public:
		BlochSphere();
		~BlochSphere();

		bool begin();
		void loop();

		// FIXME: rotate(axis, degrees) pls
		bool rotate(const Vector3d axis, const int steps);

		typedef enum
		{
			PERMUTE_X = 1,
			PERMUTE_Y,
			PERMUTE_Z,
			PERMUTE_H,
			PERMUTE_S,
			PERMUTE_NS,
			PERMUTE_T,
			PERMUTE_NT,
		} permute_t;
		bool permute(const permute_t permute);

	private:
		typedef enum{
			_NONE,				// Needed for prv_state detection
			RESET,				// _state init value
			IDLE,
			MOVE_START,
			MOVE_BUSY,
			MOVE_END,
		} state_t;
		const char* StateNames[MOVE_END + 1] = {"<none>", "RESET", "IDLE", "START", "BUSY", "END" };
		state_t _state = RESET;
		MovementQueue_t _queue;

		Module_Stepmotor _driver;
		AccelStepper _motor1;
		AccelStepper _motor2;
		AccelStepper _motor3;
		Matrix3d _motormatrix;
};


#endif // __BLOCH_H