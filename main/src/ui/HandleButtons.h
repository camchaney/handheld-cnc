#ifndef HandleButtons_h
#define HandleButtons_h

#include "Arduino.h"

#ifndef Bounce2_h
  #include <Bounce2.h>
#endif

#include <functional>

class HandleButtons {

  protected:
    typedef std::function<void(HandleButtons &btn)> CallbackFunction;

  public:
    HandleButtons(byte leftHandlePin, byte rightHandlePin);
    void update();
    void setLeftClickHandler(CallbackFunction f);
    void setRightClickHandler(CallbackFunction f);
    void setButtonsHoldHandler(CallbackFunction f);
    bool arePressed();
    bool enabled();

    /**
     * Set enabled to true of false
     * This will enable/disable all event callbacks.
     * When disabled the encoder positions will not be updated.
     */
    void enable(bool e=true);

  protected:
    CallbackFunction left_click_cb = NULL;
    CallbackFunction right_click_cb = NULL;
    CallbackFunction buttons_hold_cb = NULL;


  private:
    bool _enabled = true;
    byte _leftPin, _rightPin;
    Bounce _leftDebouncer, _rightDebouncer;
    bool _prevLeftPressed = false;
    bool _prevRightPressed = false;
    bool _bothHeld = false;
};
#endif