#include "HandleButtons.h"

HandleButtons::HandleButtons(byte leftHandlePin, byte rightHandlePin)
  : _leftPin(leftHandlePin), _rightPin(rightHandlePin)
{
  _leftDebouncer.attach(_leftPin);
  _leftDebouncer.interval(10);
  _rightDebouncer.attach(_rightPin);
  _rightDebouncer.interval(10);
}

void HandleButtons::update() {
  if (!_enabled) return;

  _leftDebouncer.update();
  _rightDebouncer.update();

  bool leftPressed = !_leftDebouncer.read();
  bool rightPressed = !_rightDebouncer.read();

  // Handle both buttons hold event (only trigger once per hold)
  if (leftPressed && rightPressed) {
    if (!_bothHeld && buttons_hold_cb) {
      buttons_hold_cb(*this);
      _bothHeld = true;
    }
  }

  // Handle left button release event
  if (_prevLeftPressed && !leftPressed && left_click_cb && !_bothHeld) {
    left_click_cb(*this);
  }
  // Handle right button release event
  if (_prevRightPressed && !rightPressed && right_click_cb && !_bothHeld) {
    right_click_cb(*this);
  }

  _prevLeftPressed = leftPressed;
  _prevRightPressed = rightPressed;
  
  if (!leftPressed && !rightPressed) {
    _bothHeld = false;
  }
}

void HandleButtons::clearHandlers() {
  left_click_cb = nullptr;
  right_click_cb = nullptr;
  buttons_hold_cb = nullptr;
}

void HandleButtons::setLeftClickHandler(CallbackFunction f) {
  left_click_cb = f;
}

void HandleButtons::setRightClickHandler(CallbackFunction f) {
  right_click_cb = f;
}

void HandleButtons::setButtonsHoldHandler(CallbackFunction f) {
  buttons_hold_cb = f;
}

bool HandleButtons::arePressed() {
  _leftDebouncer.update();
  _rightDebouncer.update();
  return (!_leftDebouncer.read() && !_rightDebouncer.read());
}

bool HandleButtons::enabled() {
  return _enabled;
}

void HandleButtons::enable(bool e) {
  _enabled = e;
}