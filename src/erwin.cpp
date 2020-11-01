#include "repelzen.hpp"
#include "osdialog.h"
#include <math.h>

#define NUM_CHANNELS 4
#define NUM_SCALES 16

struct Erwin : Module {
    enum ParamIds {
        CHANNEL_TRANSPOSE_PARAM,
        NOTE_PARAM = CHANNEL_TRANSPOSE_PARAM + NUM_CHANNELS,
        SELECT_PARAM = NOTE_PARAM + 12,
        NUM_PARAMS
    };
    enum InputIds {
        TRANSPOSE_INPUT,
        SEMI_INPUT,
        IN_INPUT,
        SELECT_INPUT = IN_INPUT + 4,
        NUM_INPUTS
    };
    enum OutputIds {
        OUT_OUTPUT,
        NUM_OUTPUTS = OUT_OUTPUT + 4
    };
    enum LightIds {
        NOTE_LIGHT,
        NUM_LIGHTS = NOTE_LIGHT + 12
    };

    enum QModes {
        DOWN,
        UP,
        NEAREST
    };

    Erwin() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(Erwin::CHANNEL_TRANSPOSE_PARAM, -4, 4, 0, "octave");
        configParam(Erwin::CHANNEL_TRANSPOSE_PARAM + 1, -4, 4, 0, "octave");
        configParam(Erwin::CHANNEL_TRANSPOSE_PARAM + 2, -4, 4, 0, "octave");
        configParam(Erwin::CHANNEL_TRANSPOSE_PARAM + 3, -4, 4, 0, "octave");
        configParam(Erwin::SELECT_PARAM, 0, 15, 0, "scene", "", 0, 1, 1);
        for(int i=0; i<12; i++) {
            configParam(Erwin::NOTE_PARAM + i, 0.0, 1.0, 0.0, "enable note");
        }
        onReset();
    }

    void process(const ProcessArgs &args) override;
    json_t* dataToJson() override;
    void dataFromJson(json_t *rootJ) override;
    void onReset() override;

    int mode = 0;
    bool noteState[12 * NUM_SCALES] = {};
    int octave = 0;
    int transposeOctave = 0;
    int transposeSemi = 0;
    float freq = 0.0f;

    dsp::SchmittTrigger noteTriggers[12];
};

json_t* Erwin::dataToJson() {
    json_t *rootJ = json_object();

    // Note values
    json_t *gatesJ = json_array();
    for (int i = 0; i < 12 * NUM_SCALES; i++) {
        json_t *gateJ = json_boolean(noteState[i]);
        json_array_append_new(gatesJ, gateJ);
    }
    json_object_set_new(rootJ, "notes", gatesJ);

    // mode
    json_object_set_new(rootJ, "mode", json_integer(mode));
    return rootJ;
}

void Erwin::dataFromJson(json_t *rootJ) {
    // Note values
    json_t *gatesJ = json_object_get(rootJ, "notes");

    if(!gatesJ) {
        DEBUG("Erwin: Invalid Input file");
        return;
    }

    for (unsigned int i = 0; i < json_array_size(gatesJ); i++) {
        json_t *gateJ = json_array_get(gatesJ, i);
        noteState[i] = gateJ ? json_boolean_value(gateJ) : false;
    }
    json_t *modeJ = json_object_get(rootJ, "mode");
    if(modeJ) {
        mode = json_integer_value(modeJ);
    }
}

void Erwin::onReset() {
    for (int i = 0; i < 12 * NUM_SCALES; i++) noteState[i] = 0;
}

void Erwin::process(const ProcessArgs &args) {

    // Scale selection
    int scaleOffset = clamp((int)(params[SELECT_PARAM].getValue()
        + inputs[SELECT_INPUT].getVoltage() * NUM_SCALES /10),0,15) * 12;
    bool* currentScale = noteState + scaleOffset;

    // limit to 1 octave
    transposeSemi = (int)round(inputs[SEMI_INPUT].getVoltage() * 1.2);

    for(unsigned int y=0; y<NUM_CHANNELS; y++) {
        // normalize to first channel
        if(!inputs[IN_INPUT + y].isConnected()) {
            inputs[IN_INPUT + y].setVoltage(inputs[IN_INPUT].getVoltage(), 0);
        }

        int currentPolyphony = std::max(1, inputs[IN_INPUT + y].getChannels());
        outputs[OUT_OUTPUT + y].setChannels(currentPolyphony);

        for (int c = 0; c < currentPolyphony; c++) {
            octave = trunc(inputs[IN_INPUT+y].getPolyVoltage(c));
            freq = inputs[IN_INPUT+y].getPolyVoltage(c) - octave;
            // limit to 4 octaves
            transposeOctave = clamp((int)round(inputs[TRANSPOSE_INPUT].getPolyVoltage(c) / 2.5)
                + (int)round(params[CHANNEL_TRANSPOSE_PARAM + y].getValue()),-4, 4);

            // index of the quantized note
            int index = 0;

            int semiUp = ceilN(freq * 12);
            int semiDown = (int)trunc(freq * 12);
            uint8_t stepsUp = 0;
            uint8_t stepsDown = 0;

            while(!currentScale[modN(semiUp + stepsUp,12)] && stepsUp < 12)
            stepsUp++;
            while(!currentScale[modN(semiDown - stepsDown, 12)] && stepsDown < 12)
            stepsDown++;

            // Reset for empty scales to avoid transposing by 1 octave
            stepsUp %= 12;
            stepsDown %= 12;

            switch(mode) {
            case QModes::UP:
                index = semiUp + stepsUp;
                break;
            case QModes::DOWN:
                index = semiDown - stepsDown;
                break;
            case QModes::NEAREST:
                if (stepsUp < stepsDown)
                index = semiUp + stepsUp;
                else
                index = semiDown - stepsDown;
                break;
            }

            if(transposeSemi)
            index += transposeSemi;

            outputs[OUT_OUTPUT + y].setVoltage(octave + index * 1/12.0 + transposeOctave, c);
        }

    }

    // Note buttons
    for (int i = 0; i < 12; i++) {
        if (noteTriggers[i].process(params[NOTE_PARAM + i].getValue())) {
            currentScale[i] = !currentScale[i];
        }
        lights[NOTE_LIGHT + i].value = (currentScale[i] >= 1.0) ? 0.7 : 0;
    }

}

struct ErwinWidget : ModuleWidget {
    ErwinWidget(Erwin *module) {
        setModule(module);
        box.size = Vec(8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/reface/rewin_bg.svg")));

        /* "scenes" */
        addParam(createParam<ReSnapKnobLGrey>(Vec(39, 40), module, Erwin::SELECT_PARAM));
        addInput(createInput<ReIOPort>(Vec(10.75, 108.75), module, Erwin::SELECT_INPUT));

        /* transpose = octave; semi = root */
        addInput(createInput<ReIOPort>(Vec(85.75, 108.75), module, Erwin::TRANSPOSE_INPUT));
        addInput(createInput<ReIOPort>(Vec(48.25, 108.75), module, Erwin::SEMI_INPUT));

        for(int i=0;i<NUM_CHANNELS;i++) {
            addOutput(createOutput<ReIOPort>(Vec(92.75, 198.75 + i*42), module, Erwin::OUT_OUTPUT + i));
            addInput(createInput<ReIOPort>(Vec(62.75, 198.75 + i*42), module, Erwin::IN_INPUT + i));
        }

        addParam(createParam<ReSnapKnobSRed>(Vec(80, 181), module, Erwin::CHANNEL_TRANSPOSE_PARAM));
        addParam(createParam<ReSnapKnobSYellow>(Vec(80, 223), module, Erwin::CHANNEL_TRANSPOSE_PARAM + 1));
        addParam(createParam<ReSnapKnobSGreen>(Vec(80, 266), module, Erwin::CHANNEL_TRANSPOSE_PARAM + 2));
        addParam(createParam<ReSnapKnobSBlue>(Vec(80, 308), module, Erwin::CHANNEL_TRANSPOSE_PARAM + 3));

        /* note buttons */
        int white=0;
        int black = 0;
        for(int i=0; i<12; i++) {
            if (i == 1 || i == 3 || i == 6 || i == 8 || i == 10 ) {
                addParam(createParam<ReButtonM>(Vec(8, 312 - black*28), module, Erwin::NOTE_PARAM + i));
                addChild(createLight<ReLightM<ReBlueLight>>(Vec(10, 314 - black*28), module, Erwin::NOTE_LIGHT + i));
                black++;
            }
            else {
                if(i == 4)
                black++;
                addParam(createParam<ReButtonM>(Vec(33, 326 - white*28), module, Erwin::NOTE_PARAM + i));
                addChild(createLight<ReLightM<ReBlueLight>>(Vec(35, 328 - white*28), module, Erwin::NOTE_LIGHT + i));
                white++;
            }
        }
    }

    struct ErwinModeMenuItem : MenuItem {
        struct ErwinModeItem : MenuItem {
            Erwin *module;
            int mode_;
            void onAction(const event::Action &e) override {
                module->mode = mode_;
            }
            void step() override {
                rightText = (module->mode == mode_) ? "✔" : "";
                MenuItem::step();
            }
        };

        Erwin *module;
        Menu *createChildMenu() override {
            Menu *menu = new Menu;
            menu->addChild(construct<ErwinModeItem>(&MenuItem::text, "Up",
                &ErwinModeItem::module, module, &ErwinModeItem::mode_, Erwin::QModes::UP));
            menu->addChild(construct<ErwinModeItem>(&MenuItem::text, "Down",
                &ErwinModeItem::module, module, &ErwinModeItem::mode_, Erwin::QModes::DOWN));
            menu->addChild(construct<ErwinModeItem>(&MenuItem::text, "Nearest",
                &ErwinModeItem::module, module, &ErwinModeItem::mode_, Erwin::QModes::NEAREST));
            return menu;
        }
    };

    /* Export scales */
    struct ErwinSaveItem : MenuItem {
        Erwin *module;

        void onAction(const event::Action &e) override {
            json_t* rootJ = module->dataToJson();
            if(rootJ) {
                char* path = osdialog_file(OSDIALOG_SAVE, NULL, "rewin.json", NULL);
                if(path) {
                    if (json_dump_file(rootJ, path, 0))
                    DEBUG("Error: cannot export rewin scale file");
                }
                free(path);
            }
        }
    };

    /* Import scales */
    struct ErwinLoadItem : MenuItem {
        Erwin *module;
        json_error_t error;

        void onAction(const event::Action &e) override {
            char* path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, NULL);
            if(path) {
                json_t* rootJ = json_load_file(path, 0, &error);
                if(rootJ) {
                    // Check this here to not break compatibility with old (single scale) saves
                    json_t *gatesJ = json_object_get(rootJ, "notes");
                    if(!gatesJ || json_array_size(gatesJ) != 12 * NUM_SCALES) {
                        osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, "rewin: invalid input file");
                        return;
                    }
                    module->dataFromJson(rootJ);
                }
                else {
                    osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, "rewin: can't load file - see logfile for details");
                    DEBUG("Error: Can't import file %s", path);
                    DEBUG("Text: %s", error.text);
                    DEBUG("Source: %s", error.source);
                }
                free(path);
            }
        }
    };

    void appendContextMenu(Menu *menu) override {
        Erwin *module = dynamic_cast<Erwin*>(this->module);
        assert(module);

        menu->addChild(new MenuSeparator());

        ErwinModeMenuItem *modeMenuItem = construct<ErwinModeMenuItem>(&MenuItem::text, "Quantization mode", &ErwinModeMenuItem::module, module);
        modeMenuItem->rightText = RIGHT_ARROW;
        menu->addChild(modeMenuItem);

        ErwinLoadItem *ldItem = createMenuItem<ErwinLoadItem>("Load scales");
        ldItem->module = module;
        menu->addChild(ldItem);

        ErwinSaveItem *svItem = createMenuItem<ErwinSaveItem>("Save scales");
        svItem->module = module;
        menu->addChild(svItem);

    }
};

Model *modelErwin = createModel<Erwin, ErwinWidget>("rewin");
