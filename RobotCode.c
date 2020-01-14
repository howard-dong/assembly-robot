/*
Assembly Robot CODE
November 15, 2019
*/

// constants to test for

int const DRIVE_SPEED = -10;
int const EXTEND_LIMIT = -170;
int const EXTEND_SPEED = -5;
int const EXTEND_DELAY = 50;
int const ROTATE_LIMIT = 70;
int const ROTATE_SPEED_D = 2;
int const ROTATE_SPEED_U = -5;

int const THICKNESS = 2; // color strip thickness so robot can center on strip

// motor objects for easier manipulation
typedef struct
{
	tMotor port;
	int motorPower;
} MotorInfo;

MotorInfo rotator;
MotorInfo extender;
MotorInfo left;
MotorInfo right;

void setPower(MotorInfo&m, int newPower)
{
	m.motorPower = newPower;
	motor[m.port] = m.motorPower;
}

// arm code
void rotate(bool pos)	// true - horizontal, false - vertical
{
	if (pos)
	{
		setPower(rotator, ROTATE_SPEED_D);
		nMotorEncoder[rotator.port] = 0;
		while (nMotorEncoder[rotator.port] < ROTATE_LIMIT) {}
		setPower(rotator, 0);
	}
	else
	{
		setPower(rotator, ROTATE_SPEED_U);
		nMotorEncoder[rotator.port] = 0;
		while (nMotorEncoder[rotator.port] >= -ROTATE_LIMIT) {}
		setPower(rotator, 0);
	}
}

void extend(int direction) // 0 - stop, 1 - extend, -1 - retract, 2 - full extend, -2 - full retract
{
	if (direction == 1)
	{
		setPower(extender, EXTEND_SPEED);
	}
	else if (direction == -1)
	{
		setPower(extender, -EXTEND_SPEED);
	}
	else
	{
		setPower(extender, 0);
	}
}

// Add new layer
void addLayer()
{
	rotate(true);
	extend(1);
	nMotorEncoder[extender.port] = 0;
	while (nMotorEncoder[extender.port] > EXTEND_LIMIT && SensorValue[S1] == 0)
	{
		displayString(6, "%f", nMotorEncoder[extender.port]);
		displayString(12, "%d", SensorValue[S1]);
	}
	//if (SensorValue[S1] == 1)
	//{
	//	time1[T4] = 0;
	//	while (nMotorEncoder[extender.port] > EXTEND_LIMIT && time1[T4] < EXTEND_DELAY) {}
	//}
	extend(-1);
	while (nMotorEncoder[extender.port] < 0)
	{
		displayString(6, "%f", nMotorEncoder[extender.port]);
	}
	extend(0);
	rotate(false);
}

// picks up layer
void pickUp()
{
	rotate(true);
	extend(1);
	nMotorEncoder[extender.port] = 0;
	while (nMotorEncoder[extender.port] > EXTEND_LIMIT && SensorValue[S1] == 0)
	{
		displayString(7, "%f", nMotorEncoder[extender.port]);
	}
	if (SensorValue[S1] == 1)
	{
		time1[T4] = 0;
		while (nMotorEncoder[extender.port] > EXTEND_LIMIT && time1[T4] < EXTEND_DELAY) {}
	}
	extend(0);
	rotate(false);
	extend(-1);
	while (nMotorEncoder[extender.port] < 0) {}
	extend(0);
}

// drops off assembly
void dropOff()
{
	setPower(rotator, ROTATE_SPEED_D);
	nMotorEncoder[rotator.port] = 0;
	while (nMotorEncoder[rotator.port] < ROTATE_LIMIT/2) {}
	setPower(rotator, 0);

	extend(1);
	while (nMotorEncoder[extender.port] > EXTEND_LIMIT) {}
	extend(0);

	setPower(rotator, ROTATE_SPEED_D);
	while (nMotorEncoder[rotator.port] < ROTATE_LIMIT) {}
	setPower(rotator, 0);

	extend(-1);
	while (nMotorEncoder[extender.port] < 0) {}
	extend(0);

	rotate(false);
}

// STATION POSITION
const int ORDER_STATION = 0;
const int GREEN_STATION = 1;
const int RED_STATION = 2;
const int BROWN_STATION = 3;
const int BLUE_STATION = 4;
const int CURING_STATION = 5;
const int STORAGE_STATION = 6;

// time to cure per layer
const int LAYER_CURING_TIME = 10000;
// color of track
const int TRACK = 6;

const int toColor[7] = {4, 3, 5, 7, 2, 1, 4};

void drive(int direction)
{
	setPower(left, DRIVE_SPEED * direction);
	setPower(right, DRIVE_SPEED * direction);
}

int moveTo(int startPos, int endPos)
{
	if (startPos == endPos)
	{
		return endPos;
	}
	if (startPos < endPos)
	{
		drive(1);
	}
	else
	{
		drive(-1);
	}
	if (endPos == 0 || endPos == 6)
	{
		while (SensorValue[S2] != TRACK) {}
	}
	while (SensorValue[S2] != toColor[endPos]) {
		displayString(12, "going to color %d", toColor[endPos]);}
	nMotorEncoder[left.port] = 0;
	if (startPos > endPos || endPos == 0)
	{
		while (nMotorEncoder[left.port] < THICKNESS/2*180/(PI * 2.75)) {}
	}
	else
	{
		while (nMotorEncoder[left.port] > -THICKNESS/2*180/(PI * 2.75)) {}
	}

	drive(0);
	return endPos;
}

typedef struct{
	int value[5];
} OrderInfo;
// order array struct

int getOrder(OrderInfo&o) // returns number of layers
{
	// resets order array
	for (int x = 0; x < 5; x++)
	{
		o.value[x] = 0;
	}
	int count = 0;
	eraseDisplay();
	displayString(2, "Pick layers from top to bottom.");
	displayString(3, "Press Enter when ready");

	// gets new order array
	while ((!getButtonPress(buttonEnter) && count < 5) || count < 1)
	{
		if (getButtonPress(buttonLeft))
		{
			while(getButtonPress(buttonLeft)) {}
			o.value[count] = RED_STATION;
			count ++;
			displayString(count + 4, "You got red on layer %d", count);
		}
		else if (getButtonPress(buttonRight))
		{
			while(getButtonPress(buttonRight)) {}
			o.value[count] = BLUE_STATION;
			count ++;
			displayString(count + 4, "You got blue on layer %d", count);
		}
		else if (getButtonPress(buttonUp))
		{
			while(getButtonPress(buttonUp)) {}
			o.value[count] = GREEN_STATION;
			count ++;
			displayString(count + 4, "You got green on layer %d", count);
		}
		else if (getButtonPress(buttonDown))
		{
			while(getButtonPress(buttonDown)) {}
			o.value[count] = BROWN_STATION;
			count ++;
			displayString(count + 4, "You got brown on layer %d", count);
		}

	}
	while (getButtonPress(buttonEnter)) {}
	return count;
}

// IDLE TIME LIMIT IN MILLISECONDS
const int IDLE_LIMIT = 20000;
const int BEEP_LIMIT = IDLE_LIMIT - 10000;

bool checkIdle(bool isIdle)
{
	if (isIdle)
	{
		displayString(2, "idle timer: %2f secs", time1[T1]/1000);
		if (time1[T1] > BEEP_LIMIT)
		{
			playTone(500, 1);
		}
		if (time1[T1] > IDLE_LIMIT)
		{
			return true;
		}
	}
	else
	{
		time1[T1] = 0;
	}
	return false;
}

// performs code that picks up assembly from curing to order station
bool cure(int currentPos) // returns true if not picked up
{
	moveTo(currentPos, CURING_STATION);
	pickUp();
	moveTo(CURING_STATION, ORDER_STATION);
	// wait for part to be picked up
	time1[T1] = 0;
	while (!getButtonPress(buttonEnter) && time1[T1] > IDLE_LIMIT) {}
	if (time1[T1] > IDLE_LIMIT)
	{
		return true;
	}
	wait1Msec(1000);
	return false;
}

bool doneCuring(int curingTime)
{
	displayString(10, "curing time: %d" , time1[T2] /1000);
	if (time1[T2] > curingTime)
	{
		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////
task main()
{
	rotator.port = motorA;
	extender.port = motorD;
	left.port = motorB;
	right.port = motorC;

	rotator.motorPower = 0;
	extender.motorPower = 0;
	left.motorPower = 0;
	right.motorPower = 0;

	SensorType[S1] = sensorEV3_Touch;
	SensorType[S2] = sensorEV3_Color;
	wait1Msec(50);
	SensorMode[S2] = modeEV3Color_Color;
	wait1Msec(50);

	OrderInfo theOrder;

	time1[T1] = 0;
	bool curing = false;
	bool idle = false;
	bool assembling = false;
	int station = 0;
	int curingTime = 0;
	int numLayers = 0;

	displayString(2, "Press Enter to Continue");


	while (!idle || curing)
	{
		idle = checkIdle(true);
		if (getButtonPress(buttonEnter))
		{
			idle = checkIdle(false);
			while(getButtonPress(buttonEnter)) {}
			numLayers = getOrder(theOrder);
			curingTime = numLayers * LAYER_CURING_TIME;
			idle = checkIdle(false);
			assembling = true;
		}

		// check if curing -- if curing, then perform curing algorithm
		if (curing && doneCuring(curingTime))
		{
			if (cure(ORDER_STATION))
			{
				idle = checkIdle(true);
			}
			else
			{
				idle = checkIdle(false);
			}
			curing = false;
		}

		if (assembling)
		{
			for (int layer = numLayers; layer > 0; layer--)
			{
				int x = layer - 1;
				displayString(13, "to station: %d", theOrder.value[x]);
				station = moveTo(station, theOrder.value[x]);
				addLayer();
				//while building, constantly check if curing is done
				if (curing && doneCuring(curingTime))
				{
					moveTo(station, STORAGE_STATION);
					dropOff();
					if (cure(STORAGE_STATION))
					{
						return;
					}
					moveTo(ORDER_STATION, STORAGE_STATION);
					pickUp();
					station = STORAGE_STATION;
					curing = false;
				}
			}
			moveTo(station, CURING_STATION);
			dropOff();
			curing = true;
			time1[T2] = 0;
			moveTo(CURING_STATION, ORDER_STATION);
			assembling = false;
			idle = checkIdle(false);
		}
		// CURING TIME
		if (curing)
		{
			idle = checkIdle(false);
			displayString(10, "curing time: %d" , time1[T2] /1000);
		}
	}
	playTone(246, 1000);
	wait1Msec(3000);
}
