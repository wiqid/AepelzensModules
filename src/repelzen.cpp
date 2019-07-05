#include "repelzen.hpp"
#include <math.h>

Plugin *pluginInstance;

void init(rack::Plugin *p) {
	pluginInstance = p;

	// p->addModel(modelQuadSeq);
	// p->addModel(modelGateSeq);
	// p->addModel(modelDice);
	// p->addModel(modelWalker);
	p->addModel(modelBurst);
	p->addModel(modelFolder);
	p->addModel(modelErwin);
	p->addModel(modelWerner);
	p->addModel(modelMixer);
}
