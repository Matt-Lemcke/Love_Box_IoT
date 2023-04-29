# Love-Box

An IoT microcontroller project designed to connect families and partners over long distances

---

## Final Product

The number display shows the number of days since a specific date, either an anniversary or the last day you've seen each other. The user can press the push button to send a notification to the other person's Phone via the IFTTT app. The person with the app can press a virtual button to blink the LED heart on the box or reset the device.

## Problems and Solutions
### 1. IFTTT app cannot directly ping the box
The box is able to send information to the IFTTT microservice I created using webhooks. However, the IFTTT app cannot send information back to the box through webhooks because it is connected to a secured private network. To work around this, I built and hosted a basic Flask web app on Repl.it using python that employed a custom REST api. This web app acts as a proxy server to allow the box and IFTTT app to communicate indirectly.

### 2. Button bouncing causing stack overflow
The original design for the software running on the microcontroller used hardware interrupts connected to the button to change the current state of the executing program. This way, the regular program can execute normally and only service the button requests when needed. However, the voltage bouncing caused by the hardware button was triggering multiple interrupts on press/release and the debouncing algorithms themselves were being interrupted. Instead of temporarily disabling the interrupts, I decided to move to a polling method strictly becuase of the better debouncing algorithms available with this method.

![](images/box.jpg)
![](images/boxOn.jpg)
![](images/app.png)
![](images/cicuit.jpg)
