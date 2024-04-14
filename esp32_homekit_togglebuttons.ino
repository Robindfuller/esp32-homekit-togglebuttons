#include <HomeSpan.h>

class MyPushButton : public Service::StatelessProgrammableSwitch {
  String name;
  Characteristic::ProgrammableSwitchEvent* switchEvent;  // Reference to the switch event characteristic

public:
  MyPushButton(const char* name)
    : Service::StatelessProgrammableSwitch(), name(name) {
    new Characteristic::Name(name);
    switchEvent = new Characteristic::ProgrammableSwitchEvent(0);  // Initialize with default event
  }


  void triggerSingleButtonPress() {
    Serial.println(name + " programmatically pressed");
    switchEvent->setVal(Characteristic::ProgrammableSwitchEvent::SINGLE_PRESS);  // Set to SINGLE_PRESS
  }

  void triggerDoubleButtonPress() {
    Serial.println(name + " programmatically pressed");
    switchEvent->setVal(Characteristic::ProgrammableSwitchEvent::DOUBLE_PRESS);  // Set to SINGLE_PRESS
  }
};

class ToggleSwitch : public Service::Switch {
  String name;
  SpanCharacteristic* power;  // Reference to the On Characteristic
  MyPushButton* button;

public:
  ToggleSwitch(const char* name)
    : Service::Switch(), name(name) {  // Use the Switch service
    new Characteristic::Name(name);
    power = new Characteristic::On();
    pinMode(2, OUTPUT);  // Ensure the pin is set to output mode
  }

  boolean update() override {
    boolean currentlyOn = power->getVal();
    boolean calledOn = power->getNewVal();
    boolean intendedOn = (currentlyOn != calledOn) ? calledOn : !currentlyOn;

    Serial.println("Switch command received:");
    Serial.print("currentlyOn: ");
    Serial.println(currentlyOn ? "true" : "false");
    Serial.print("calledOn: ");
    Serial.println(calledOn ? "true" : "false");
    Serial.print("IntendedOn: ");
    Serial.println(intendedOn ? "true" : "false");

    power->setVal(intendedOn);
    digitalWrite(2, intendedOn ? HIGH : LOW);

    if (intendedOn) {
      button->triggerSingleButtonPress();
    } else {
      button->triggerDoubleButtonPress();
    }

    return true;
  }

  void setButton(MyPushButton* button) {
    this->button = button;
  }
};


class ToggleButtonAccessory {
 

public:
  ToggleButtonAccessory(const char* name) {  // Use the Switch service
    new SpanAccessory();
    new Service::AccessoryInformation();
    new Characteristic::Name(name);
    new Characteristic::Manufacturer("BAR Technologies");
    new Characteristic::SerialNumber("300000001");
    new Characteristic::Model("BAR-VTS");
    new Characteristic::FirmwareRevision("1.0");
    new Characteristic::Identify();

    ToggleSwitch* toggleSwitch = new ToggleSwitch("Toggle Switch");  // Create a new toggle switch using the button instances
    MyPushButton* button = new MyPushButton("Push Button");
    toggleSwitch->setButton(button);
  }
};

void
setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);

  homeSpan.setWifiCallback([]() {
    homeSpan.setPairingCode("11122333");
    Serial.println("WiFi Connected. Pairing Code Set.");
  });
  homeSpan.begin(Category::Bridges, "BAR Hub");

  new SpanAccessory();
  new Service::AccessoryInformation();
  new Characteristic::Manufacturer("BAR Technologies");
  new Characteristic::SerialNumber("100000001");
  new Characteristic::Model("BAR-H1");
  new Characteristic::FirmwareRevision("1.0");
  new Characteristic::Identify();


  const int numberOfAccessories = 20;
  for (int i = 1; i <= numberOfAccessories; i++) {
    new ToggleButtonAccessory("Toggle Button 1");
  }

  new Service::HAPProtocolInformation();
  new Characteristic::Version("1.1.0");
}

void loop() {
  homeSpan.poll();  // Needed to process HomeKit-related tasks
}
