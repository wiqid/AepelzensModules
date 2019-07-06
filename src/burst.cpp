#include "repelzen.hpp"
#include <math.h>

#define MAX_REPS 8
#define MAX_TIME 1.0f

struct Burst : Module
{
  enum ParamIds
    {
      BUTTON_PARAM,
      TIME_PARAM,
      REP_PARAM,
      ACCEL_PARAM,
      JITTER_PARAM,
      CV_MODE_PARAM,
      GATE_MODE_PARAM,
      REP_ATT_PARAM,
      TIME_ATT_PARAM,
      NUM_PARAMS
    };
  enum InputIds
    {
      GATE_INPUT,
      CLOCK_INPUT,
      REP_INPUT,
      TIME_INPUT,
      NUM_INPUTS
    };
  enum OutputIds
    {
      GATE_OUTPUT,
      EOC_OUTPUT,
      CV_OUTPUT,
      NUM_OUTPUTS
    };

  enum LightIds
    {
      NUM_LIGHTS
    };

  enum CvModes
    {
      CV_UP,
      CV_DOWN,
      CV_MODE3,
      CV_MODE4,
      CV_MODE_RANDOMP,
      CV_MODE_RANDOMN,
      CV_MODE_RANDOM_WALK,
      CV_MODE_RANDOM,
    };

  const float euler = exp(1);
  float timeParam = 0;
  float clockedTimeParam = 0;
  float pulseParam = 4;
  float timer = 0;
  float seconds = 0;
  int pulseCount = 0;
  int pulses = 4;
  float randomcv = 0;
  float cvOut = 0;

  dsp::SchmittTrigger m_buttonTrigger;
  dsp::SchmittTrigger gateTrigger;;
  dsp::SchmittTrigger clockTrigger;

  dsp::PulseGenerator outPulse;
  dsp::PulseGenerator eocPulse;

  //toggle for every received clock tick
  float clockTimer = 0;
  float lastClockTime = 0;
  float gateOutLength = 0.01;

  void process(const ProcessArgs &args) override;

  Burst() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configParam(Burst::BUTTON_PARAM, 0.0, 1.0, 0.0, "manual burst");
	configParam(Burst::REP_PARAM, 0, 8, 4, "repetitions");
	configParam(Burst::TIME_PARAM, 0.02, 1, 0.508, "time");
	configParam(Burst::ACCEL_PARAM, 1.0, 2.0, 1.0, "acceleration");
	configParam(Burst::JITTER_PARAM, 0.0, 1.0, 0.0, "jitter");
	configParam(Burst::CV_MODE_PARAM, 0, 7, 0, "cv mode");
	configParam(Burst::REP_ATT_PARAM, -1.0, 1.0, 0.0, "repetition modulation");
	configParam(Burst::TIME_ATT_PARAM, -1.0, 1.0, 0.0, "time modulation");
	configParam(Burst::GATE_MODE_PARAM, 0.0, 1.0, 0.0, "gate/trigger switch");
  }
};

void Burst::process(const ProcessArgs &args)
{
  float delta = args.sampleTime;
  float schmittValue = gateTrigger.process(inputs[GATE_INPUT].getVoltage());

  //seconds = params[TIME_PARAM].getValue() + (inputs[TIME_INPUT].getVoltage() / 20.0);
  float accel = params[ACCEL_PARAM].getValue();
  float jitter = params[JITTER_PARAM].getValue();
  float randomDelta = 0;

  timeParam = clamp(params[TIME_PARAM].getValue() + (params[TIME_ATT_PARAM].getValue() * inputs[TIME_INPUT].getVoltage() / 10.0 * MAX_TIME), 0.0f, MAX_TIME);
  pulseParam = clamp((int)params[REP_PARAM].getValue() + (inputs[REP_INPUT].getVoltage() * params[REP_ATT_PARAM].getValue() /10.0 * MAX_REPS), 0, MAX_REPS);

  //exponential scaling for timeparam
  timeParam = (exp(timeParam) - 1)/(euler - 1);

  if (inputs[CLOCK_INPUT].isConnected()) {
    clockTimer += delta;
    if( clockTrigger.process(inputs[CLOCK_INPUT].getVoltage()) ) {
      timeParam = params[TIME_PARAM].getValue();
      int mult = (int)(timeParam*8 - 4);
      //smooth clock (over 8 pulses) to reduce sensitivity
      // float clockDelta = clockTimer - lastClockTime;
      // clockTimer -= clockTimer - (clockDelta/8);
      lastClockTime = clockTimer;

      timeParam = clockTimer * pow(2, mult);
      clockedTimeParam = timeParam;
      clockTimer = 0;
    }
    timeParam = clockedTimeParam;
  }

  if( timer > seconds && pulseCount < pulses ) {
    pulseCount ++;
    timer = 0.0;

    seconds = timeParam;

    if(accel > 0) {
      seconds = timeParam/pow(accel,pulseCount);
    }

    if(jitter > 0) {
      randomDelta = random::uniform() * jitter * seconds;
      if (random::uniform() > 0.5) {
	seconds = seconds + randomDelta;
      }
      else {
	seconds = seconds - randomDelta;
      }
    }

    if (pulseCount == pulses) {
      eocPulse.trigger(0.01);
    }
    gateOutLength = (params[GATE_MODE_PARAM].getValue()) ? 0.01 : seconds/2;
    outPulse.trigger(gateOutLength);
    randomcv = random::uniform();

    //cv
    float cvDelta = 5.0/pulses;
    int mode = roundf(params[CV_MODE_PARAM].getValue());
    switch (mode) {
    case CV_UP:
      cvOut = pulseCount * cvDelta;
      break;
    case CV_DOWN:
      cvOut = pulseCount * cvDelta * (-1);
      break;
    case CV_MODE3:
      cvOut = trunc((pulseCount + 1)/2) * cvDelta;
      if(pulseCount % 2 == 1) {
	cvOut *= -1;
      }
      break;
    case CV_MODE4:
      cvOut = pulseCount * cvDelta;
      if(pulseCount %2 == 1) {
	cvOut *= -1;
      }
      break;
    case CV_MODE_RANDOMP:
      cvOut = randomcv *5.0;
      break;
    case CV_MODE_RANDOMN:
      cvOut = randomcv * (-5.0);
      break;
    case CV_MODE_RANDOM_WALK:
	cvOut = (randomcv > 0.5f) ? cvOut + cvDelta : cvOut - cvDelta;
	break;
    case CV_MODE_RANDOM:
      cvOut = randomcv *10 - 5;
      break;
    }
  }

  if (schmittValue || m_buttonTrigger.process(params[BUTTON_PARAM].getValue())) {
    pulseCount = 0;
    timer = 0.0;
    outPulse.trigger(gateOutLength);
    seconds = timeParam;
    pulses = pulseParam;
    cvOut = 0;
  }

  timer += delta;
  outputs[GATE_OUTPUT].setVoltage(outPulse.process(delta) ? 10.0 : 0.0);
  outputs[EOC_OUTPUT].setVoltage(eocPulse.process(delta) ? 10.0 : 0.0);
  outputs[CV_OUTPUT].setVoltage((timer < seconds) ? cvOut : 0.0);
}


struct BurstWidget : ModuleWidget {
  BurstWidget(Burst *module) {
    setModule(module);
    box.size = Vec(8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/reface/reburst_bg.svg")));

    addParam(createParam<ReButtonL>(Vec(39, 40), module, Burst::BUTTON_PARAM));
    addInput(createInput<ReIOPort>(Vec(48, 110), module, Burst::GATE_INPUT));;

    addParam(createParam<ReKnobMGrey>(Vec(8, 106), module, Burst::ACCEL_PARAM));
    addParam(createParam<ReKnobMGrey>(Vec(83, 106), module, Burst::JITTER_PARAM));
    addParam(createParam<ReKnobMYellow>(Vec(8, 161), module, Burst::TIME_PARAM));
    addParam(createParam<ReSnapKnobBlue>(Vec(83, 161), module, Burst::REP_PARAM));

    addInput(createInput<ReIOPort>(Vec(48, 164), module, Burst::CLOCK_INPUT));;

    addParam(createParam<ReKnobSYellow>(Vec(13, 213), module, Burst::TIME_ATT_PARAM));
    addInput(createInput<ReIOPort>(Vec(10.5, 256), module, Burst::TIME_INPUT));;
    addParam(createParam<ReKnobSBlue>(Vec(88, 213), module, Burst::REP_ATT_PARAM));
    addInput(createInput<ReIOPort>(Vec(85.5, 256), module, Burst::REP_INPUT));;

    addParam(createParam<ReSwitch2>(Vec(53.5, 291), module, Burst::GATE_MODE_PARAM));
    addParam(createParam<ReSnapKnobGreen>(Vec(45.5, 228), module, Burst::CV_MODE_PARAM));

    addOutput(createOutput<ReIOPort>(Vec(10.5,323), module, Burst::CV_OUTPUT));
    addOutput(createOutput<ReIOPort>(Vec(48,323), module, Burst::GATE_OUTPUT));
    addOutput(createOutput<ReIOPort>(Vec(85.5,323), module, Burst::EOC_OUTPUT));
  }
};

Model *modelBurst = createModel<Burst, BurstWidget>("reburst");
