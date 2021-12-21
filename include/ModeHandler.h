#ifndef MODE_HANDLER_H
#define MODE_HANDLER_H

class ModeHandler {
public:
    virtual void switchTo();
    virtual void leaveOut();        // switchBackToCommandMode
	virtual void serialIncoming();
	virtual void loop();
};

#endif