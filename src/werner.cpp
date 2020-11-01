#include "repelzen.hpp"

#define NUM_CHANNELS 4
#define MAX_POLY 16

struct Werner : Module {
    enum ParamIds {
	TIME_PARAM,
	DELTA_PARAM,
	NUM_PARAMS
    };
    enum InputIds {
	CV_INPUT,
	NUM_INPUTS = NUM_CHANNELS
    };
    enum OutputIds {
	GATE_OUTPUT,
	NUM_OUTPUTS = NUM_CHANNELS
    };
    enum LightIds {
	NUM_LIGHTS
    };

    Werner() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(Werner::TIME_PARAM, 0.0, 1.0, 0.0, "pulse width");
        configParam(Werner::DELTA_PARAM, 0.0, 1.0, 0.0, "sensitivity");
    }

    void process(const ProcessArgs &args) override;

    dsp::PulseGenerator gatePulse[NUM_CHANNELS][MAX_POLY];
    float lastValue[NUM_CHANNELS][MAX_POLY];
    int res = 16;
    float minDelta = 0;
    int frame = 0;
};


void Werner::process(const ProcessArgs &args) {
    //max time is about 100ms at 44kHz
    res = (int)clamp(params[TIME_PARAM].getValue() * 4400.0f, 16.0f, 4400.0f);
    minDelta = params[DELTA_PARAM].getValue() * 2.0f;

    if(++frame > res) {
        for(int i=0; i<NUM_CHANNELS;i++) {
            int currentPolyphony = std::max(1, inputs[CV_INPUT + i].getChannels());
            outputs[GATE_OUTPUT + i].setChannels(currentPolyphony);
            for (int c = 0; c < currentPolyphony; c++) {
                float value = inputs[CV_INPUT + i].getPolyVoltage(c);

                if(abs(value - lastValue[i][c]) > minDelta) {
                    gatePulse[i][c].trigger(0.01);
                }

                lastValue[i][c] = value;
            }
        }
        frame = 0;
    }

    for(int i=0; i<NUM_CHANNELS;i++) {
        int currentPolyphony = std::max(1, inputs[CV_INPUT + i].getChannels());
        for (int c = 0; c < currentPolyphony; c++) {
	        outputs[GATE_OUTPUT + i].setVoltage(gatePulse[i][c].process(1.0 / args.sampleRate) ? 10.0 : 0.0, c);
        }
    }
}

struct WernerWidget : ModuleWidget {
    WernerWidget(Werner *module) {
    	setModule(module);
        box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/reface/retrig_bg.svg")));

        addParam(createParam<ReKnobLGrey>(Vec(9, 40), module, Werner::TIME_PARAM));
        addParam(createParam<ReKnobMGrey>(Vec(15.5, 106), module, Werner::DELTA_PARAM));

        for(int i=0;i<NUM_CHANNELS;i++) {
        	addInput(createInput<ReIOPort>(Vec(3.25, 198.75 + i*42), module, Werner::CV_INPUT + i));
        	addOutput(createOutput<ReIOPort>(Vec(33.25, 198.75 + i*42), module, Werner::GATE_OUTPUT + i));
        }
    }
};

Model *modelWerner = createModel<Werner, WernerWidget>("retrig");
