#include "MotionGenerator.h"

MotionGenerator::MotionGenerator(float aMaxVel, float aMaxAcc, float aInitPos)
		: maxVel(aMaxVel), maxAcc(aMaxAcc), initPos(aInitPos)
{
	init();
}


void MotionGenerator::init() {	
	// Time variables
    oldTime = 0;
    lastTime = oldTime;
    deltaTime = 0;	
	
	// State variables
	reset();	
	
	// Misc
	signM = 1;		// 1 = positive change, -1 = negative change
	shape = true;   // true = trapezoidal, false = triangular
	isFinished = false;
}

float MotionGenerator::update(float posRef) {	
		
	if (oldPosRef != posRef)  // reference changed
	{
		isFinished = false;
		// Shift state variables
		oldPosRef = posRef;
		oldPos = pos;
		oldVel = vel;
		oldTime = lastTime;
		
		// Calculate braking time and distance (in case is neeeded)
		tBrk = abs(oldVel) / maxAcc;
		dBrk = tBrk * abs(oldVel) / 2;
		
		// Caculate Sign of motion
		signM = sign(posRef - (oldPos + sign(oldVel)*dBrk));
		
		if (signM != sign(oldVel))  // means brake is needed
		{
			tAcc = (maxVel / maxAcc);
			dAcc = tAcc * (maxVel / 2);
		}
		else
		{
			tBrk = 0;
			dBrk = 0;
			tAcc = (maxVel - abs(oldVel)) / maxAcc;
			dAcc = tAcc * (maxVel + abs(oldVel)) / 2;
		}
		
		// Calculate total distance to go after braking
		dTot = abs(posRef - oldPos + signM*dBrk);
		
		tDec = maxVel / maxAcc;
		dDec = tDec * (maxVel) / 2;
		dVel = dTot - (dAcc + dDec);
		tVel = dVel / maxVel;
		
		if (tVel > 0)    // trapezoidal shape
			shape = true;
		else             // triangular shape
		{
			shape = false;
			// Recalculate distances and periods
			if (signM != sign(oldVel))  // means brake is needed
			{
				velSt = sqrt(maxAcc*(dTot));
				tAcc = (velSt / maxAcc);
				dAcc = tAcc * (velSt / 2);
			}
			else
			{
				tBrk = 0;
				dBrk = 0;
				dTot = abs(posRef - oldPos);      // recalculate total distance
				velSt = sqrt(0.5*oldVel*oldVel + maxAcc*dTot);
				tAcc = (velSt - abs(oldVel)) / maxAcc;
				dAcc = tAcc * (velSt + abs(oldVel)) / 2;
			}
			tDec = velSt / maxAcc;
			dDec = tDec * (velSt) / 2;
		}
		
	}
	
	unsigned long time = 0;		// current time
	// Calculate time since last set-point change
	deltaTime = (time - oldTime);
	// Calculate new setpoint
	calculateTrapezoidalProfile(posRef);
	// Update last time
	lastTime = time;
	
	//calculateTrapezoidalProfile(setpoint);
	return pos;
}


void MotionGenerator::calculateTrapezoidalProfile(float posRef) {

	float t = static_cast<float> (deltaTime) / 1000;	// conversion from milliseconds to seconds				
	
	if (shape)   // trapezoidal shape
	{
		if (t <= (tBrk+tAcc))
		{
			pos = oldPos + oldVel*t + signM * 0.5*maxAcc*t*t;
			vel = oldVel + signM * maxAcc*t;
			acc = signM * maxAcc;
		}
		else if (t > (tBrk+tAcc) && t < (tBrk+tAcc+tVel))
		{
			pos = oldPos + signM * (-dBrk + dAcc + maxVel*(t-tBrk-tAcc));
			vel = signM * maxVel;
			acc = 0;
		}
		else if (t >= (tBrk+tAcc+tVel) && t < (tBrk+tAcc+tVel+tDec))
		{
			pos = oldPos + signM * (-dBrk + dAcc + dVel + maxVel*(t-tBrk-tAcc-tVel) - 0.5*maxAcc*(t-tBrk-tAcc-tVel)*(t-tBrk-tAcc-tVel));
			vel = signM * (maxVel - maxAcc*(t-tBrk-tAcc-tVel));
			acc = - signM * maxAcc;
		}
		else
		{
			pos = posRef;
			vel = 0;
			acc = 0;
			isFinished = true;
		}
	}
	else            // triangular shape
	{
		if (t <= (tBrk+tAcc))
		{
			pos = oldPos + oldVel*t + signM * 0.5*maxAcc*t*t;
			vel = oldVel + signM * maxAcc*t;
			acc = signM * maxAcc;
		}
		else if (t > (tBrk+tAcc) && t < (tBrk+tAcc+tDec))
		{
			pos = oldPos + signM * (-dBrk + dAcc + velSt*(t-tBrk-tAcc) - 0.5*maxAcc*(t-tBrk-tAcc)*(t-tBrk-tAcc));
			vel = signM * (velSt - maxAcc*(t-tBrk-tAcc));
			acc = - signM * maxAcc;
		}
		else
		{
			pos = posRef;
			vel = 0;
			acc = 0;
			isFinished = true;
		}
		
	}

} 

// Getters and setters
bool MotionGenerator::getFinished() {
	return isFinished;
}

float MotionGenerator::getVelocity() {
	return vel;
}

float MotionGenerator::getAcceleration() {
	return acc;
}

void MotionGenerator::setMaxVelocity(float aMaxVel) {
	maxVel = aMaxVel;
}

void MotionGenerator::setMaxAcceleration(float aMaxAcc) {
	maxAcc = aMaxAcc;
}

void MotionGenerator::setInitPosition(float aInitPos) {
	initPos 	= aInitPos;
	pos 		= aInitPos;
	oldPos 		= aInitPos;
}

short int MotionGenerator::sign(float aVal) {
	if (aVal < 0)
		return -1;
	else if (aVal > 0)
		return 1;
	else
		return 0;
}

void MotionGenerator::reset() {
	// Reset all state variables	

	pos 		= initPos;
	oldPos 		= initPos;
    oldPosRef 	= 0;
	vel 		= 0;
	acc 		= 0;
	oldVel 		= 0;
	
	dBrk 		= 0;
	dAcc 		= 0;
	dVel 		= 0;
	dDec 		= 0;
	dTot 		= 0;
	
	tBrk 		= 0;
	tAcc 		= 0;
	tVel 		= 0;
	tDec 		= 0;
	
	velSt 		= 0;
}

