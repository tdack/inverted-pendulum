/**
 * @file pid-new.cpp
 * @brief Threaded PID controller
 *
 * Threaded PID controller based on https://github.com/br3ttb/Arduino-PID-Library
 *
 * @author Troy Dack
 * @date Copyright (C) 2015
 *
 * @license
 * \verbinclude "Troy Dack GPL-2.0.txt"
 *
 **/

#include <pendulum.h>
#include <pid-new.h>
#include <cstdbool>
#include <iostream>
#include <atomic>
#include <string>

	pid_new::pid_new(double* Input, double* Output, double* SetPoint, double _kp, double _ki, double _kd, int dir)
: myInput(Input), myOutput(Output), mySetPoint(SetPoint), inAuto(false) {
	bExit.store(false);
	SampleTime = 0.100;
	pid_new::SetOutputLimits(0,100);
	pid_new::SetControllerDirection(dir);
	pid_new::SetTunings(_kp, _ki, _kd);
	lastTime = std::chrono::high_resolution_clock::now();
}

void pid_new::Compute() {
	   if(!inAuto) return;
	   std::chrono::duration<float, std::deci> timeChange;
	   std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
	   timeChange = (now - lastTime);
	   if(timeChange.count() >= SampleTime)
	   {
		   std::cout << timeChange.count();
		  /*Compute all the working error variables*/
		  double input = *myInput;
		  double error = *mySetPoint - input;
		  std::cout << "\t Input: " << std::to_string(input) << "\tError: " << std::to_string(error) << "\t";
		  ITerm += (ki * error);
		  if (ITerm > outMax) ITerm = outMax;
		  else if(ITerm < outMin) ITerm = outMin;
		  double dInput = (input - lastInput);

		  /*Compute PID Output*/
		  double output = kp * error + ITerm - kd * dInput;

		  if (output > outMax) output = outMax;
		  else if(output < outMin) output = outMin;
		  *myOutput = output;

		  /*Remember some variables for next time*/
		  lastInput = input;
		  lastTime = now;
		  std::cout << "Output: " << std::to_string(*myOutput) << std::endl;
	   }
	   return;
}

void pid_new::onStartHandler(){
	while (!bExit.load()) {
		this->Compute();
		yield();
	}
}

void pid_new::stop(){
	bExit.store(true);
}


/* SetTunings(...)*************************************************************
 * This function allows the controller's dynamic performance to be adjusted.
 * it's called automatically from the constructor, but tunings can also
 * be adjusted on the fly during normal operation
 ******************************************************************************/
void pid_new::SetTunings(double Kp, double Ki, double Kd)
{
   if (Kp<0 || Ki<0 || Kd<0) return;

   dispKp = Kp; dispKi = Ki; dispKd = Kd;

   double SampleTimeInSec = ((double)SampleTime)/1000;
   kp = Kp;
   ki = Ki * SampleTimeInSec;
   kd = Kd / SampleTimeInSec;

  if(controllerDirection == 1)
   {
      kp = (0 - kp);
      ki = (0 - ki);
      kd = (0 - kd);
   }
}

/* SetSampleTime(...) *********************************************************
 * sets the period, in Milliseconds, at which the calculation is performed
 ******************************************************************************/
void pid_new::SetSampleTime(int NewSampleTime)
{
   if (NewSampleTime > 0)
   {
      double ratio  = (double)NewSampleTime/1000
                      / (double)SampleTime;
      ki *= ratio;
      kd /= ratio;
      SampleTime = (double)NewSampleTime/1000.0;
      std::cout << std::to_string(SampleTime) << std::endl;
   }
}

/* SetOutputLimits(...)****************************************************
 *     This function will be used far more often than SetInputLimits.  while
 *  the input to the controller will generally be in the 0-1023 range (which is
 *  the default already,)  the output will be a little different.  maybe they'll
 *  be doing a time window and will need 0-8000 or something.  or maybe they'll
 *  want to clamp it from 0-125.  who knows.  at any rate, that can all be done
 *  here.
 **************************************************************************/
void pid_new::SetOutputLimits(double Min, double Max)
{
   if(Min >= Max) return;
   outMin = Min;
   outMax = Max;

   if(inAuto)
   {
	   if(*myOutput > outMax) *myOutput = outMax;
	   else if(*myOutput < outMin) *myOutput = outMin;

	   if(ITerm > outMax) ITerm= outMax;
	   else if(ITerm < outMin) ITerm= outMin;
   }
}

/* SetMode(...)****************************************************************
 * Allows the controller Mode to be set to manual (0) or Automatic (non-zero)
 * when the transition from manual to auto occurs, the controller is
 * automatically initialized
 ******************************************************************************/
void pid_new::SetMode(int Mode)
{
    bool newAuto = (Mode == 1);
    if(newAuto == !inAuto)
    {  /*we just went from manual to auto*/
        pid_new::Initialize();
    }
    inAuto = newAuto;
}

/* Initialize()****************************************************************
 *	does all the things that need to happen to ensure a bumpless transfer
 *  from manual to automatic mode.
 ******************************************************************************/
void pid_new::Initialize()
{
   ITerm = *myOutput;
   lastInput = *myInput;
   if(ITerm > outMax) ITerm = outMax;
   else if(ITerm < outMin) ITerm = outMin;
}

/* SetControllerDirection(...)*************************************************
 * The PID will either be connected to a DIRECT acting process (+Output leads
 * to +Input) or a REVERSE acting process(+Output leads to -Input.)  we need to
 * know which one, because otherwise we may increase the output when we should
 * be decreasing.  This is called from the constructor.
 ******************************************************************************/
void pid_new::SetControllerDirection(int Direction)
{
   if(inAuto && Direction != controllerDirection)
   {
	  kp = (0 - kp);
      ki = (0 - ki);
      kd = (0 - kd);
   }
   controllerDirection = Direction;
}

/* Status Funcions*************************************************************
 * Just because you set the Kp=-1 doesn't mean it actually happened.  these
 * functions query the internal state of the PID.  they're here for display
 * purposes.  this are the functions the PID Front-end uses for example
 ******************************************************************************/
double pid_new::GetKp(){ return  dispKp; }
double pid_new::GetKi(){ return  dispKi;}
double pid_new::GetKd(){ return  dispKd;}
int pid_new::GetMode(){ return  inAuto ? 1 : 0;}
int pid_new::GetDirection(){ return controllerDirection;}
