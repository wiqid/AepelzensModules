#include <rack.hpp>

using namespace rack;
extern Plugin *pluginInstance;

/* the original modulo does not deal with negative numbers correctly
   For example -1%12 should be 11, but it is -1*/
inline int modN(int k, int n) {
    return ((k %= n) < 0) ? k+n : k;
}

/* modified version of ceil that works with negative values (example: -2.3 becomes -3) */
inline int ceilN(float x) {
    return (x < 0) ? (int)floor(x) : (int)ceil(x);
}

/* custom components for repelzen redesign */

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

// struct ReRedLight : app::GrayModuleLightWidget {
//     ReRedLight() {
//         addBaseColor(nvgRGB(0xff, 0x25, 0x4a));
//     }
// };

/* extern Model *modelQuadSeq;
extern Model *modelGateSeq;
extern Model *modelDice; */
extern Model *modelBurst;
extern Model *modelFolder;
/* extern Model *modelWalker;
extern Model *modelErwin; */
extern Model *modelWerner;
extern Model *modelMixer;
