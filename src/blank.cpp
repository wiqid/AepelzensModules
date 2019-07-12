#include "repelzen.hpp"

struct Blank : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		TRIG_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	Blank() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		bg = 0;
	}
	void process(const ProcessArgs &args) override;

	int bg;
	dsp::SchmittTrigger trig;
};

void Blank::process(const ProcessArgs &args) {
	if (inputs[TRIG_INPUT].isConnected()) {
		float schmittValue = trig.process(inputs[TRIG_INPUT].getVoltage());
		if (schmittValue) {
			bg++;
			if (bg > 7) { bg = 0; }
		}
	}
}

struct BlankWidget : ModuleWidget {
	SvgPanel* panel1;
	SvgPanel* panel2;
	SvgPanel* panel3;
	SvgPanel* panel4;
	SvgPanel* panel5;
	SvgPanel* panel6;
	SvgPanel* panel7;
	SvgPanel* panel8;

	BlankWidget(Blank *module) {
		setModule(module);
		box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/reface/repelzen_bg1.svg")));

		if (module) {
			panel1 = new SvgPanel();
			panel1->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/reface/repelzen_bg1.svg")));
			panel1->visible = false;
			addChild(panel1);

			panel2 = new SvgPanel();
			panel2->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/reface/repelzen_bg2.svg")));
			panel2->visible = false;
			addChild(panel2);

			panel3 = new SvgPanel();
			panel3->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/reface/repelzen_bg3.svg")));
			panel3->visible = false;
			addChild(panel3);

			panel4 = new SvgPanel();
			panel4->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/reface/repelzen_bg4.svg")));
			panel4->visible = false;
			addChild(panel4);

			panel5 = new SvgPanel();
			panel5->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/reface/repelzen_bg5.svg")));
			panel5->visible = false;
			addChild(panel5);

			panel6 = new SvgPanel();
			panel6->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/reface/repelzen_bg6.svg")));
			panel6->visible = false;
			addChild(panel6);

			panel7 = new SvgPanel();
			panel7->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/reface/repelzen_bg7.svg")));
			panel7->visible = false;
			addChild(panel7);

			panel8 = new SvgPanel();
			panel8->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/reface/repelzen_bg8.svg")));
			panel8->visible = false;
			addChild(panel8);
		}

		addInput(createInput<ReIOPort>(Vec(18.25, 325.25), module, Blank::TRIG_INPUT));

	}

	void step() override {
		if (module) {
			switch(((Blank*)module)->bg) {
				case 0:
				panel8->visible = false;
				panel1->visible = true;
				break;
				case 1:
				panel1->visible = false;
				panel2->visible = true;
				break;
				case 2:
				panel2->visible = false;
				panel3->visible = true;
				break;
				case 3:
				panel3->visible = false;
				panel4->visible = true;
				break;
				case 4:
				panel4->visible = false;
				panel5->visible = true;
				break;
				case 5:
				panel5->visible = false;
				panel6->visible = true;
				break;
				case 6:
				panel6->visible = false;
				panel7->visible = true;
				break;
				case 7:
				panel7->visible = false;
				panel8->visible = true;
				break;
			}
		}
		Widget::step();
	}
};

Model *modelBlank = createModel<Blank, BlankWidget>("reblank");
