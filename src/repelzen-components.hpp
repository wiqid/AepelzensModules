#pragma once
#include <rack.hpp>

using namespace rack;
extern Plugin *pluginInstance;

////////// custom components for repelzen redesign //////////

struct ReButtonL : app::SvgSwitch {
	ReButtonL() {
		momentary = true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/recomp/button_l.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/recomp/button_l_push.svg")));
	}
};

struct ReButtonM : app::SvgSwitch {
	ReButtonM() {
		momentary = true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/recomp/button_m.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/recomp/button_m_push.svg")));
	}
};

struct ReIOPort : app::SvgPort {
    ReIOPort() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/recomp/jack.svg")));
    }
};

/* knobs */
struct ReKnobLGrey : app::SvgKnob {
    ReKnobLGrey() {
        minAngle = -0.83 * M_PI;
        maxAngle = 0.83 * M_PI;
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/recomp/knob_l_grey.svg")));
    }
};

struct ReKnobMBlue : app::SvgKnob {
    ReKnobMBlue() {
        minAngle = -0.83 * M_PI;
        maxAngle = 0.83 * M_PI;
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/recomp/knob_m_blue.svg")));
    }
};

struct ReKnobMGreen : app::SvgKnob {
    ReKnobMGreen() {
        minAngle = -0.83 * M_PI;
        maxAngle = 0.83 * M_PI;
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/recomp/knob_m_green.svg")));
    }
};

struct ReKnobMGrey : app::SvgKnob {
    ReKnobMGrey() {
        minAngle = -0.83 * M_PI;
        maxAngle = 0.83 * M_PI;
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/recomp/knob_m_grey.svg")));
    }
};

struct ReKnobMRed : app::SvgKnob {
    ReKnobMRed() {
        minAngle = -0.83 * M_PI;
        maxAngle = 0.83 * M_PI;
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/recomp/knob_m_red.svg")));
    }
};

struct ReKnobMYellow : app::SvgKnob {
    ReKnobMYellow() {
        minAngle = -0.83 * M_PI;
        maxAngle = 0.83 * M_PI;
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/recomp/knob_m_yellow.svg")));
    }
};

struct ReKnobSBlue : app::SvgKnob {
    ReKnobSBlue() {
        minAngle = -0.83 * M_PI;
        maxAngle = 0.83 * M_PI;
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/recomp/knob_s_blue.svg")));
    }
};

struct ReKnobSGreen : app::SvgKnob {
    ReKnobSGreen() {
        minAngle = -0.83 * M_PI;
        maxAngle = 0.83 * M_PI;
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/recomp/knob_s_green.svg")));
    }
};

struct ReKnobSGrey : app::SvgKnob {
    ReKnobSGrey() {
        minAngle = -0.83 * M_PI;
        maxAngle = 0.83 * M_PI;
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/recomp/knob_s_grey.svg")));
    }
};

struct ReKnobSRed : app::SvgKnob {
    ReKnobSRed() {
        minAngle = -0.83 * M_PI;
        maxAngle = 0.83 * M_PI;
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/recomp/knob_s_red.svg")));
    }
};

struct ReKnobSYellow : app::SvgKnob {
    ReKnobSYellow() {
        minAngle = -0.83 * M_PI;
        maxAngle = 0.83 * M_PI;
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/recomp/knob_s_yellow.svg")));
    }
};

/* snap knobs */
struct ReSnapKnobLGrey : ReKnobLGrey {
    ReSnapKnobLGrey() {
        snap = true;
    }
};

struct ReSnapKnobBlue : ReKnobMBlue {
    ReSnapKnobBlue() {
        snap = true;
    }
};

struct ReSnapKnobGreen : ReKnobMGreen {
    ReSnapKnobGreen() {
        snap = true;
    }
};

struct ReSnapKnobSBlue : ReKnobSBlue {
    ReSnapKnobSBlue() {
        snap = true;
    }
};

struct ReSnapKnobSGreen : ReKnobSGreen {
    ReSnapKnobSGreen() {
        snap = true;
    }
};

struct ReSnapKnobSRed : ReKnobSRed {
    ReSnapKnobSRed() {
        snap = true;
    }
};

struct ReSnapKnobSYellow : ReKnobSYellow {
    ReSnapKnobSYellow() {
        snap = true;
    }
};

/* switches */
struct ReSwitch2 : app::SvgSwitch {
    ReSwitch2() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/recomp/switch2_1.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/recomp/switch2_2.svg")));
    }
};

struct ReSwitch3 : app::SvgSwitch {
    ReSwitch3() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/recomp/switch3_1.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/recomp/switch3_2.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/recomp/switch3_3.svg")));
    }
};

/* lights */
template <typename BASE>
struct ReLightM : BASE { // to go with ReButtonM as bezel
    ReLightM() {
        this->box.size = (Vec(17, 17));
    }
};

struct ReRedLight : app::ModuleLightWidget {
    ReRedLight() {
        bgColor = nvgRGB(0x5a, 0x5a, 0x5a);
        borderColor = nvgRGBA(0, 0, 0, 0x60);
        addBaseColor(nvgRGB(0xff, 0x25, 0x4a));
    }
};

struct ReBlueLight : app::ModuleLightWidget {
    ReBlueLight() {
        bgColor = nvgRGB(0x5a, 0x5a, 0x5a);
        borderColor = nvgRGBA(0, 0, 0, 0x60);
        addBaseColor(nvgRGB(0x25, 0xc0, 0xff));
    }
};
