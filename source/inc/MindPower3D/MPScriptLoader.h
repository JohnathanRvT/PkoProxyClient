#ifndef MPScriptLoader_h
#define MPScriptLoader_h

#include "MPEffPrerequisites.h"

class MINDPOWER_API MPScriptLoader {
public:
	virtual ~ScriptLoader();

	virtual const MPStringVector& getScriptPatterns() const = 0;

	virtual void parseScript(MPDataStreamPtr& stream, const std::string& groupName) = 0;

	virtual float getLoadingOrder() const = 0;
};

#endif