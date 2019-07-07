#include "repelzen.hpp"
#include "aefilter.hpp"

#define NUM_CHANNELS 6

struct Mixer : Module {
    enum ParamIds {
	MASTER_GAIN_PARAM,
	MASTER_EQ_LOW_PARAM,
	MASTER_EQ_MID_PARAM,
	MASTER_EQ_HIGH_PARAM,
	GAIN_PARAM,
	MUTE_PARAM = GAIN_PARAM + NUM_CHANNELS,
	EQ_LOW_PARAM = MUTE_PARAM + NUM_CHANNELS,
	EQ_MID_PARAM = EQ_LOW_PARAM + NUM_CHANNELS,
	EQ_HIGH_PARAM = EQ_MID_PARAM + NUM_CHANNELS,
	PAN_PARAM = EQ_HIGH_PARAM + NUM_CHANNELS,
	AUX1_PARAM = PAN_PARAM + NUM_CHANNELS,
	AUX2_PARAM = AUX1_PARAM + NUM_CHANNELS,
	NUM_PARAMS = AUX2_PARAM + NUM_CHANNELS
    };
    enum InputIds {
	AUX1_L_INPUT,
	AUX1_R_INPUT,
	AUX2_L_INPUT,
	AUX2_R_INPUT,
	CH1_INPUT,
	CH1_GAIN_INPUT = CH1_INPUT + NUM_CHANNELS,
	CH1_PAN_INPUT = CH1_GAIN_INPUT + NUM_CHANNELS,
	NUM_INPUTS = CH1_PAN_INPUT + NUM_CHANNELS
    };
    enum OutputIds {
	L_OUTPUT,
	R_OUTPUT,
	AUX1_L_OUTPUT,
	AUX1_R_OUTPUT,
	AUX2_L_OUTPUT,
	AUX2_R_OUTPUT,
	NUM_OUTPUTS
    };
    enum LightIds {
	MUTE_LIGHT,
	METER_L_LIGHT = MUTE_LIGHT + NUM_CHANNELS,
	METER_R_LIGHT = METER_L_LIGHT + 6,
	NUM_LIGHTS = METER_R_LIGHT + 6
    };

    Mixer() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        for(int i=0;i<NUM_CHANNELS;i++) {
            configParam(Mixer::AUX1_PARAM + i, 0.0f, 1.0f, 0.0f, "send1 amount");
            configParam(Mixer::AUX2_PARAM + i, 0.0f, 1.0f, 0.0f, "send2 amount");
            configParam(Mixer::PAN_PARAM + i, -1.0f, 1.0f, 0.0f, "pan");
            configParam(Mixer::EQ_HIGH_PARAM + i, -15.0f, 15.0f, 0.0f, "eq high band gain", "dB");
            configParam(Mixer::EQ_MID_PARAM + i, -12.5f, 12.5f, 0.0f, "eq mid band gain", "dB");
            configParam(Mixer::EQ_LOW_PARAM + i, -20.0f, 20.0f, 0.0f, "eq low band gain", "dB");
            configParam(Mixer::MUTE_PARAM + i, 0.0f, 1.0f, 0.0f, "mute on/off");
            configParam(Mixer::GAIN_PARAM + i, -60.0f, 0.0f, -60.0f, "channel gain", "dB");
            channels[i].hp.setCutoff(35.0f, 0.8f, AeFilterType::AeHIGHPASS);
            channels[i].hs.setParams(12000.0f, 0.8f, -5.0f, AeEQType::AeHIGHSHELVE);
        }

        configParam(Mixer::MASTER_GAIN_PARAM, -60.0f, 0.0f, -30.0f, "master gain", "dB");
        configParam(Mixer::MASTER_EQ_HIGH_PARAM, -7.0f, 7.0f, 0.0f, "master eq high band gain", "dB");
        configParam(Mixer::MASTER_EQ_MID_PARAM, -7.0f, 7.0f, 0.0f, "master eq mid band gain", "dB");
        configParam(Mixer::MASTER_EQ_LOW_PARAM, -10.0f, 10.0f, 0.0f, "master eq low band gain", "dB");

        maHp.setCutoff(35.0f, 0.8f, AeFilterType::AeHIGHPASS);
        maHs.setParams(12000.0f, 0.8f, -2.0f, AeEQType::AeHIGHSHELVE);

        meter.dBInterval = 10.0f;
    }

    struct mixerChannel {
        AeEqualizer eqLow;
        AeEqualizer eqMid;
        AeEqualizer eqHigh;

        AeFilter hp;
        AeEqualizer hs;

        //initialize out of range so it passes the check on initialisation
        float lastLowGain = -25.0f;
        float lastMidGain = -25.0f;
        float lastHighGain = -25.0f;
        bool mute = false;
    };

    mixerChannel channels[NUM_CHANNELS];
    dsp::VuMeter meter;  // DEPRECATED - TODO: REPLACE WITH VUMETER2
    dsp::SchmittTrigger muteTrigger[NUM_CHANNELS];

    //master EQ
    AeEqualizerStereo eqMaLow;
    AeEqualizerStereo eqMaMid;
    AeEqualizerStereo eqMaHigh;
    AeFilterStereo maHp;
    AeEqualizerStereo maHs;
    float lastMaLowGain = -25.0f;
    float lastMaMidGain = -25.0f;
    float lastMaHighGain = -25.0f;

    void process(const ProcessArgs &args) override;
};


void Mixer::process(const ProcessArgs &args) {

    float outL = 0.0f;
    float outR = 0.0f;
    float aux1L = 0.0f;
    float aux1R = 0.0f;
    float aux2L = 0.0f;
    float aux2R = 0.0f;

    float masterGain = params[MASTER_GAIN_PARAM].getValue();
    masterGain = pow(10, masterGain/20);

    float aux1LIn = inputs[AUX1_L_INPUT].getNormalVoltage(0.0f);
    float aux1RIn = inputs[AUX1_R_INPUT].getNormalVoltage(0.0f);
    float aux2LIn = inputs[AUX2_L_INPUT].getNormalVoltage(0.0f);
    float aux2RIn = inputs[AUX2_R_INPUT].getNormalVoltage(0.0f);

    for(int i=0;i<NUM_CHANNELS;i++) {
        // sum polyphonic cables, as per vcv voltage standards, #5
    	float in = inputs[CH1_INPUT + i].getVoltageSum();

    	if(muteTrigger[i].process(params[MUTE_PARAM + i].getValue())) {
    	    channels[i].mute = !channels[i].mute;
    	    lights[MUTE_LIGHT + i].value =  (channels[i].mute) ? 1.0f : 0.0f;
    	}

        if(!channels[i].mute) {
            float gain = pow(10, params[GAIN_PARAM + i].getValue()/20.0f);
            gain *= inputs[CH1_GAIN_INPUT + i].getNormalVoltage(10.0f) / 10.0f;

            float pan = clamp(params[PAN_PARAM + i].getValue() + inputs[CH1_PAN_INPUT + i].getVoltage() /5.0f, -1.0f, 1.0f);
            float lowGain = params[EQ_LOW_PARAM + i].getValue();
            float midGain = params[EQ_MID_PARAM + i].getValue();
            float highGain = params[EQ_HIGH_PARAM + i].getValue();

            //only calculate coefficients when neccessary
            if(lowGain != channels[i].lastLowGain) {
                channels[i].eqLow.setParams(125.0f, 0.45f, lowGain, AeEQType::AeLOWSHELVE);
                channels[i].lastLowGain = lowGain;
            }
            if(midGain != channels[i].lastMidGain) {
                channels[i].eqMid.setParams(1200.0f, 0.52f, midGain, AeEQType::AePEAKINGEQ);
                channels[i].lastMidGain = midGain;
            }
            if(highGain != channels[i].lastHighGain) {
                channels[i].eqHigh.setParams(1800.0f, 0.4f, highGain, AeEQType::AeHIGHSHELVE);
                channels[i].lastHighGain = highGain;
            }

            float out = channels[i].eqLow.process(in);
            out = channels[i].eqMid.process(out);
            out = channels[i].eqHigh.process(out);
            out = channels[i].hp.process(out);
            out = channels[i].hs.process(out);

            float leftGain = (pan < 0) ? gain : gain * (1 - pan);
            float rightGain = (pan > 0) ? gain : gain * (1 + pan);

            //outputs
            //out = tanh(out) * 5.0f;
            //out *= 5.0f;
            outL += out * leftGain;
            outR += out * rightGain;
            aux1L += out * leftGain * params[AUX1_PARAM + i].getValue();
            aux1R += out * rightGain * params[AUX1_PARAM + i].getValue();
            aux2L += out * leftGain * params[AUX2_PARAM + i].getValue();
            aux2R += out * rightGain * params[AUX2_PARAM + i].getValue();
        }
    }

    //master EQ
    if(lastMaLowGain != params[MASTER_EQ_LOW_PARAM].getValue()) {
        lastMaLowGain = params[MASTER_EQ_LOW_PARAM].getValue();
        eqMaLow.setParams(120.0f, 0.45f, lastMaLowGain, AeEQType::AeLOWSHELVE);
    }
    if(lastMaMidGain != params[MASTER_EQ_MID_PARAM].getValue()) {
        lastMaMidGain = params[MASTER_EQ_MID_PARAM].getValue();
        eqMaMid.setParams(1300.0f, 0.95f, lastMaMidGain, AeEQType::AePEAKINGEQ);
    }
    if(lastMaHighGain != params[MASTER_EQ_HIGH_PARAM].getValue()) {
        lastMaHighGain = params[MASTER_EQ_HIGH_PARAM].getValue();
        eqMaHigh.setParams(1700.0f, 0.45f, lastMaHighGain, AeEQType::AeHIGHSHELVE);
    }

    eqMaLow.process(&outL, &outR);
    eqMaMid.process(&outL, &outR);
    eqMaHigh.process(&outL, &outR);
    maHp.process(&outL,&outR);
    maHs.process(&outL,&outR);

    //outputs
    outL = (outL + aux1LIn + aux2LIn) * masterGain;
    outR = (outR + aux1RIn + aux2RIn) * masterGain;
    outputs[L_OUTPUT].setVoltage(outL);
    outputs[R_OUTPUT].setVoltage(outR);
    outputs[AUX1_L_OUTPUT].setVoltage(aux1L);
    outputs[AUX1_R_OUTPUT].setVoltage(aux1R);
    outputs[AUX2_L_OUTPUT].setVoltage(aux2L);
    outputs[AUX2_R_OUTPUT].setVoltage(aux2R);

    //meter
    for (int i = 0; i < 6; i++){
        // DEPRECATED - TODO: REPLACE WITH VUMETER2
    	meter.setValue(outL / 5.0f);
    	lights[METER_L_LIGHT + i].setBrightnessSmooth(meter.getBrightness(i));
    	meter.setValue(outR / 5.0f);
    	lights[METER_R_LIGHT + i].setBrightnessSmooth(meter.getBrightness(i));
    }
}

struct MixerWidget : ModuleWidget {
    MixerWidget(Mixer *module) {
        setModule(module);
        box.size = Vec(28 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/reface/rexmix_bg.svg")));

        for(int i=0;i<NUM_CHANNELS;i++) {
            addParam(createParam<ReKnobMGreen>(Vec(108 + i * 42, 64), module, Mixer::AUX1_PARAM + i));
            addParam(createParam<ReKnobMBlue>(Vec(108 + i * 42, 106), module, Mixer::AUX2_PARAM + i));
            addParam(createParam<ReKnobMYellow>(Vec(108 + i * 42, 280), module, Mixer::PAN_PARAM + i));
            addParam(createParam<ReKnobMGrey>(Vec(108 + i * 42, 151), module, Mixer::EQ_HIGH_PARAM + i));
            addParam(createParam<ReKnobMGrey>(Vec(108 + i * 42, 193), module, Mixer::EQ_MID_PARAM + i));
            addParam(createParam<ReKnobMGrey>(Vec(108 + i * 42, 235), module, Mixer::EQ_LOW_PARAM + i));

            addParam(createParam<ReButtonM>(Vec(112 + i * 42, 33), module, Mixer::MUTE_PARAM + i));

            addParam(createParam<ReKnobMRed>(Vec(108 + i * 42, 322), module, Mixer::GAIN_PARAM + i));

            addInput(createInput<ReIOPort>(Vec(4.3, 199.75 + i * 25), module, Mixer::CH1_INPUT + i));
            addInput(createInput<ReIOPort>(Vec(34.3, 199.75 + i * 25), module, Mixer::CH1_GAIN_INPUT + i));
            addInput(createInput<ReIOPort>(Vec(64.3, 199.75 + i * 25), module, Mixer::CH1_PAN_INPUT + i));
        }
        for(int i=0;i<3;i++) { // leds to the left
            addChild(createLight<SmallLight<ReRedLight>>(Vec(104 + i * 42, 30), module, Mixer::MUTE_LIGHT + i));
        }
        for(int i=3;i<6;i++) { // leds to the right
            addChild(createLight<SmallLight<ReRedLight>>(Vec(135 + i * 42, 30), module, Mixer::MUTE_LIGHT + i));
        }

        addParam(createParam<ReKnobLGrey>(Vec(361.5, 294), module, Mixer::MASTER_GAIN_PARAM));
        addParam(createParam<ReKnobMGrey>(Vec(368, 151), module, Mixer::MASTER_EQ_HIGH_PARAM));
        addParam(createParam<ReKnobMGrey>(Vec(368, 193), module, Mixer::MASTER_EQ_MID_PARAM));
        addParam(createParam<ReKnobMGrey>(Vec(368, 235), module, Mixer::MASTER_EQ_LOW_PARAM));
        // meter
        for(int i=0;i<6;i++) {
            addChild(createLight<SmallLight<ReRedLight>>(Vec(364, 33 + i * 9), module, Mixer::METER_L_LIGHT + i));
            addChild(createLight<SmallLight<ReRedLight>>(Vec(394, 33 + i * 9), module, Mixer::METER_R_LIGHT + i));
        }

        addOutput(createOutput<ReIOPort>(Vec(356, 108.75), module, Mixer::L_OUTPUT));
        addOutput(createOutput<ReIOPort>(Vec(386, 108.75), module, Mixer::R_OUTPUT));

        addOutput(createOutput<ReIOPort>(Vec(4.25, 66.75), module, Mixer::AUX1_L_OUTPUT));
        addOutput(createOutput<ReIOPort>(Vec(34.25, 66.75), module, Mixer::AUX1_R_OUTPUT));
        addInput(createInput<ReIOPort>(Vec(4.25, 31.75), module, Mixer::AUX1_L_INPUT));
        addInput(createInput<ReIOPort>(Vec(34.25, 31.75), module, Mixer::AUX1_R_INPUT));

        addOutput(createOutput<ReIOPort>(Vec(4.25, 108.75), module, Mixer::AUX2_L_OUTPUT));
        addOutput(createOutput<ReIOPort>(Vec(34.25, 108.75), module, Mixer::AUX2_R_OUTPUT));
        addInput(createInput<ReIOPort>(Vec(4.25, 143.75), module, Mixer::AUX2_L_INPUT));
        addInput(createInput<ReIOPort>(Vec(34.25, 143.75), module, Mixer::AUX2_R_INPUT));

    }
};

Model *modelMixer = createModel<Mixer, MixerWidget>("rexmix");
