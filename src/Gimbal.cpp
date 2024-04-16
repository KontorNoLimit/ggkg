#include "Gimbal.h"

#include "config.h"
#include "time.h"

// (10/10/2023 kontornl) allocate memory for STATIC members
Servo Gimbal::servoPitch, Gimbal::servoYaw;
time_t Gimbal::tsPitchLastOpened, Gimbal::tsYawLastOpened;
bool Gimbal::silentTaskDonePitch, Gimbal::silentTaskDoneYaw;
int Gimbal::currPitchMs, Gimbal::currYawMs;

Gimbal::Gimbal()
{
}

Gimbal::~Gimbal()
{
}

void Gimbal::init()
{
    this->silentIntervMs = 6;

    this->servoPitch.attach(SERVO_PITCH);
    this->servoPitch.write(pitchAngle);
    this->currPitchMs = SERVO_ANGLE_TO_MS(pitchAngle);
    this->targetPitchMs = -1;
    this->silentTaskDonePitch = true;
    time(&this->tsPitchLastOpened);

    this->servoYaw.attach(SERVO_YAW);
    this->servoYaw.write(yawAngle);
    this->currYawMs = SERVO_ANGLE_TO_MS(yawAngle);
    this->targetYawMs = -1;
    this->silentTaskDoneYaw = true;
    time(&this->tsYawLastOpened);
}

int Gimbal::getPitchAngle()
{
    return pitchAngle;
}

int Gimbal::getYawAngle()
{
    return yawAngle;
}

void Gimbal::setPitchAngle(int angle)
{
    if(!this->servoPitch.attached()) {
        this->servoPitch.attach(SERVO_PITCH);
    }
    pitchAngle = angle;
    this->servoPitch.write(pitchAngle);
    this->currPitchMs = this->servoPitch.readMicroseconds();
    time(&this->tsPitchLastOpened);
}

void Gimbal::setYawAngle(int angle)
{
    if(!this->servoYaw.attached()) {
        this->servoYaw.attach(SERVO_YAW);
    }
    yawAngle = angle;
    this->servoYaw.write(yawAngle);
    this->currYawMs = this->servoYaw.readMicroseconds();
    time(&this->tsYawLastOpened);
}

void Gimbal::setPitchMs(int ms)
{
    if(!this->servoPitch.attached()) {
        this->servoPitch.attach(SERVO_PITCH);
    }
    this->servoPitch.writeMicroseconds(ms);
    this->currPitchMs = ms;
    time(&this->tsPitchLastOpened);
}

void Gimbal::setYawMs(int ms)
{
    if(!this->servoYaw.attached()) {
        this->servoYaw.attach(SERVO_YAW);
    }
    this->servoYaw.writeMicroseconds(ms);
    this->currYawMs = ms;
    time(&this->tsYawLastOpened);
}

void Gimbal::setTargetPitchAngle(int angle)
{
    this->silentTaskDonePitch = angle < 0;
    if(!this->silentTaskDonePitch) {
        pitchAngle = angle;
        this->targetPitchMs = SERVO_ANGLE_TO_MS(angle);
        // (10/10/2022 kontornl) wake up servo
        this->setPitchMs(this->currPitchMs);
    }
}

void Gimbal::setTargetYawAngle(int angle)
{
    this->silentTaskDoneYaw = angle < 0;
    if(!this->silentTaskDoneYaw) {
        yawAngle = angle;
        this->targetYawMs = SERVO_ANGLE_TO_MS(angle);
        // (10/10/2022 kontornl) wake up servo
        this->setYawMs(this->currYawMs);
    }
}

void Gimbal::setSilentIntervalMs(int interval)
{
    this->silentIntervMs = interval;
}

bool Gimbal::reqSleepPitch(bool force)
{
    time_t tsCurr; time(&tsCurr);
    if(this->servoPitch.attached()) {
        if(force || tsCurr - this->tsPitchLastOpened > SERVO_POWER_TIME_MAX) {
            this->currPitchMs = this->servoPitch.readMicroseconds();
            this->servoPitch.detach();
            return true;
        }
    }
    return false;
}

bool Gimbal::reqSleepYaw(bool force)
{
    time_t tsCurr; time(&tsCurr);
    if(this->servoYaw.attached()) {
        if(force || tsCurr - this->tsYawLastOpened > SERVO_POWER_TIME_MAX) {
            this->currYawMs = this->servoYaw.readMicroseconds();
            this->servoYaw.detach();
            return true;
        }
    }
    return false;
}

void Gimbal::handleSilentTask()
{
    if(this->targetPitchMs >= 0 || this->targetYawMs >= 0) {
        if(!this->silentTaskDonePitch) {
            time(&this->tsPitchLastOpened);
            if(this->targetPitchMs - this->currPitchMs > 2) this->servoPitch.writeMicroseconds(++this->currPitchMs);
            else if(this->currPitchMs - this->targetPitchMs > 2) this->servoPitch.writeMicroseconds(--this->currPitchMs);
            else {
                this->servoPitch.write(pitchAngle);
                this->targetPitchMs = -1;
                this->silentTaskDonePitch = true;
            }
        }
        if(!this->silentTaskDoneYaw) {
            time(&this->tsYawLastOpened);
            if(this->targetYawMs - this->currYawMs > 2) this->servoYaw.writeMicroseconds(++this->currYawMs);
            else if(this->currYawMs - this->targetYawMs > 2) this->servoYaw.writeMicroseconds(--this->currYawMs);
            else {
                this->servoYaw.write(yawAngle);
                this->targetYawMs = -1;
                this->silentTaskDoneYaw = true;
            }
        }
        delay(this->silentIntervMs);
        // uart0.printf("silent: y%4d-%c%4d p%4d-%c%4d\r\n", s_yaw.readMicroseconds(), done_yaw?'-':'>', dest_yaw, s_pitch.readMicroseconds(), done_pitch?'-':'>', dest_pitch);
        // uart0.printf("Pitch: %4dus -%c %4dus, yaw: %4dus -%c %4dus;\r\n", curr_pitch, done_pitch?'-':'>', a_pitch, curr_yaw, done_yaw?'-':'>', a_yaw);
        // uart0.printf("loop: silent, pitch a%d, yaw a%d\r\n", s_pitch.attached(), s_yaw.attached());
        // if(silentTaskDonePitch && silentTaskDoneYaw) {
        //     uart0.println("silent: done.");
        // }
    }
    /* (09/10/2023 kontornl) update silent dest
    if(!done_pitch || !done_yaw) { // update silent dest
        uart0.printf("loop: silent update, pitch a%d, yaw a%d\r\n", s_pitch.attached(), s_yaw.attached());
        dest_pitch = pitch * 100 / 9 + 500;
        dest_yaw = yaw * 100 / 9 + 500;
    }
    */
}
