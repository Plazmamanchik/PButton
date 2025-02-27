class PButton {
public:
    PButton(int8_t pin, bool type = false, bool dir = false) {
        if (pin != -1) {
            _PIN = static_cast<uint8_t>(pin);
            noPin = false;
            pinMode(_PIN, type ? INPUT : INPUT_PULLUP);
        } else {
            noPin = true;
        }
        inv_state = dir;
    }

    void setDebounce(uint16_t debounce) { _debounce = debounce; }
    void setTimeout(uint16_t timeout) { _timeout = timeout; }
    void setClickTimeout(uint16_t clickTimeout) { _click_timeout = clickTimeout; }
    void setStepTimeout(uint16_t stepTimeout) { _step_timeout = stepTimeout; }

    bool isPress() { return checkFlag(isPress_f); }
    bool isRelease() { return checkFlag(isRelease_f); }
    bool isClick() { return checkFlag(isOne_f); }
    bool isHolded() { return checkFlag(isHolded_f); }
    bool isHold() { return step_flag; }
    bool state() { return btn_state; }
    bool isSingle() { return checkCounter(1); }
    bool isDouble() { return checkCounter(2); }
    bool isTriple() { return checkCounter(3); }
    bool hasClicks() { return checkFlag(counter_flag); }
    uint8_t getClicks() { counter_reset = true; return last_counter; }
    uint8_t getHoldClicks() { return last_hold_counter; }
    bool isStep(uint8_t clicks) {
        if (btn_counter == clicks && step_flag && (millis() - btn_timer >= _step_timeout)) {
            btn_timer = millis();
            return true;
        }
        return false;
    }

    void resetStates() {
        isPress_f = isRelease_f = isOne_f = isHolded_f = step_flag = counter_flag = false;
        last_hold_counter = last_counter = 0;
    }

    void tick() {
        if (!noPin) btn_state = digitalRead(_PIN) ^ inv_state;
        uint32_t now = millis();

        if (btn_state && !btn_flag) {
            if (!btn_deb) {
                btn_deb = true;
                btn_timer = now;
            } else if (now - btn_timer >= _debounce) {
                btn_flag = isPress_f = oneClick_f = true;
            }
        } else {
            btn_deb = false;
        }

        if (!btn_state && btn_flag) {
            btn_flag = false;
            if (!hold_flag) btn_counter++;
            hold_flag = false;
            isRelease_f = true;
            btn_timer = now;
            if (step_flag) resetStep();
            if (oneClick_f) { oneClick_f = false; isOne_f = true; }
        }

        if (btn_flag && btn_state && (now - btn_timer >= _timeout) && !hold_flag) {
            hold_flag = true;
            last_hold_counter = btn_counter;
            isHolded_f = step_flag = true;
            oneClick_f = false;
            btn_timer = now;
        }

        if ((now - btn_timer >= _click_timeout) && btn_counter && !btn_state) {
            last_counter = btn_counter;
            btn_counter = 0;
            counter_flag = true;
        }

        if (counter_reset) resetCounter();
    }

private:
    uint8_t _PIN;
    uint16_t _debounce = 50, _timeout = 1000, _click_timeout = 400, _step_timeout = 500;
    uint32_t btn_timer = 0;
    uint8_t btn_counter = 0, last_counter = 0, last_hold_counter = 0;
    bool btn_state = false, btn_flag = false, hold_flag = false, step_flag = false;
    bool isPress_f = false, isRelease_f = false, isOne_f = false, isHolded_f = false;
    bool btn_deb = false, oneClick_f = false, counter_flag = false, counter_reset = false;
    bool noPin = true, inv_state = false;

    bool checkFlag(bool &flag) {
        if (flag) { flag = false; return true; }
        return false;
    }
    bool checkCounter(uint8_t count) {
        if (counter_flag && last_counter == count) { counter_reset = true; return true; }
        return false;
    }
    void resetCounter() { last_counter = 0; counter_flag = counter_reset = false; }
    void resetStep() { last_counter = 0; btn_counter = 0; step_flag = false; }
};
