#include "repelzen.hpp"

#define BUF_LEN 32
#define FOLDER_SAMPLE_RATE 192000

struct Folder : Module {
    enum ParamIds {
	GAIN_PARAM,
	GAIN_ATT_PARAM,
	SYM_PARAM,
	SYM_ATT_PARAM,
	STAGE_PARAM,
	NUM_PARAMS
    };
    enum InputIds {
	GATE_INPUT,
	GAIN_INPUT,
	SYM_INPUT,
	NUM_INPUTS
    };
    enum OutputIds {
	GATE_OUTPUT,
	NUM_OUTPUTS
    };
    enum LightIds {
	NUM_LIGHTS
    };

    void process(const ProcessArgs &args) override;

    Folder() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(Folder::STAGE_PARAM, 1, 3, 2, "folding stages", "", 0, 2, 0);
        configParam(Folder::GAIN_PARAM, 0.0, 14.0, 1.0, "folding amount");
        configParam(Folder::GAIN_ATT_PARAM, -1.0, 1.0, 0, "folding amount modulation");
        configParam(Folder::SYM_PARAM, -1.0, 1.0, 0.0, "symmetry");
        configParam(Folder::SYM_ATT_PARAM, -1.0, 1.0, 0.0, "symmetry modulation");

        convUp.setRates(APP->engine->getSampleRate(), FOLDER_SAMPLE_RATE);
        convDown.setRates(FOLDER_SAMPLE_RATE, APP->engine->getSampleRate());
    }

    void onSampleRateChange() override {
        convUp.setRates(APP->engine->getSampleRate(), FOLDER_SAMPLE_RATE);
        convDown.setRates(FOLDER_SAMPLE_RATE, APP->engine->getSampleRate());
    }

    json_t *dataToJson() override {
        json_t *rootJ = json_object();
        json_object_set_new(rootJ, "alternativeMode", json_boolean(alternativeMode));
        return rootJ;
    }

    void dataFromJson(json_t *rootJ) override {
        json_t *modeJ = json_object_get(rootJ, "alternativeMode");
        if(modeJ) {
            alternativeMode = json_boolean_value(modeJ);
        }
    }

    float in, out, gain, sym;
    float threshold = 1.0;

    bool alternativeMode = false;

    dsp::SampleRateConverter<1> convUp;
    dsp::SampleRateConverter<1> convDown;

    int frame = 0;
    //smallest supported sampleRate in Rack is 44100 (upsample ratio = 4.35)
    dsp::Frame<1> in_buffer[BUF_LEN] = {};
    dsp::Frame<1> out_buffer[5*BUF_LEN] = {};
    dsp::Frame<1> folded_buffer[BUF_LEN] = {};

    float fold(float in, float threshold) {
        float out;
        if (in>threshold || in<-threshold) {
            out = clamp(fabs(fabs(fmod(in - threshold, threshold*4)) - threshold*2) - threshold, -5.0f,5.0f);
        }
        else {
            out = in;
        }
        return out;
    }

    float fold3(float in, float t) {
        float out = 0;
        if(in>t) {
            out = t - (in - t);
        }
        else if (in < -t) {
            out = -t + (-t - in);
        }
        else {
            out = in;
        }
        return out;
    }

};

void Folder::process(const ProcessArgs &args) {
    int channels = inputs[GATE_INPUT].getChannels(); // enable polyphony
	for (int c = 0; c < channels; c++) {
        gain = clamp(params[GAIN_PARAM].getValue() + (inputs[GAIN_INPUT].getPolyVoltage(c) * params[GAIN_ATT_PARAM].getValue()), 0.0f,14.0f);
        sym = clamp(params[SYM_PARAM].getValue() + inputs[SYM_INPUT].getPolyVoltage(c)/5.0 * params[SYM_ATT_PARAM].getValue(), -1.0f, 1.0f);
        in = (inputs[GATE_INPUT].getVoltage(c)/5.0 + sym) * gain;

        if(++frame >= BUF_LEN) {
            //upsampling
            int outLen = 5 *BUF_LEN;
            int inLen = BUF_LEN;
            convUp.process(in_buffer, &inLen, out_buffer, &outLen);

            //fold
            for(int i=0;i<outLen;i++) {
                if(!alternativeMode) {
                    int stages = (int)(params[STAGE_PARAM].getValue())*2;
                    for (int y=0;y<stages;y++) {
                        out_buffer[i].samples[0] = fold3(out_buffer[i].samples[0], threshold);
                    }
                }
                else {
                    out_buffer[i].samples[0] = fold(out_buffer[i].samples[0], threshold);
                }
                out_buffer[i].samples[0] = tanh(out_buffer[i].samples[0]);
            }

            //downSampling
            int foldedLen = BUF_LEN;
            convDown.process(out_buffer, &outLen, folded_buffer, &foldedLen);
            frame = 0;
        }

        in_buffer[frame].samples[0] = in;
        outputs[GATE_OUTPUT].setVoltage(folded_buffer[frame].samples[0] * 5.0, c);
    }
    outputs[GATE_OUTPUT].setChannels(channels);
}

struct FolderWidget : ModuleWidget {
    FolderWidget(Folder *module){
        setModule(module);
        box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/reface/refold_bg.svg")));

        addParam(createParam<ReSwitch3>(Vec(15, 176), module, Folder::STAGE_PARAM));
        addParam(createParam<ReKnobLGrey>(Vec(9, 40), module, Folder::GAIN_PARAM));
        addParam(createParam<ReKnobSGrey>(Vec(5.5, 238.5), module, Folder::GAIN_ATT_PARAM));
        addParam(createParam<ReKnobMBlue>(Vec(15.5, 106), module, Folder::SYM_PARAM));
        addParam(createParam<ReKnobSBlue>(Vec(35.5, 238.5), module, Folder::SYM_ATT_PARAM));

        addInput(createInput<ReIOPort>(Vec(3.5, 325.5), module, Folder::GATE_INPUT));;
        addInput(createInput<ReIOPort>(Vec(3.5, 283), module, Folder::GAIN_INPUT));;
        addInput(createInput<ReIOPort>(Vec(33.5, 283), module, Folder::SYM_INPUT));;
        addOutput(createOutput<ReIOPort>(Vec(33.5, 325.5), module, Folder::GATE_OUTPUT));
    }

    struct FolderMenuItem : MenuItem {
        Folder *module;
        void onAction(const event::Action &e) override {
        	module->alternativeMode ^= true;
        }
    };

    void appendContextMenu(Menu *menu) override {
        Folder *module = dynamic_cast<Folder*>(this->module);
        assert(module);

        menu->addChild(new MenuSeparator());

        FolderMenuItem *altItem = createMenuItem<FolderMenuItem>(
            "Alternative folding algorithm", CHECKMARK(module->alternativeMode)
        );
        altItem->module = module;
        menu->addChild(altItem);
    }

};

Model *modelFolder = createModel<Folder, FolderWidget>("refold");
