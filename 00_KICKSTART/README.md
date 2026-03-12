# 00_KICKSTART

Drei schnelle Einstiegs-Sketches für Arduino.

## 1) Blink ohne delay + Serial
Pfad: `01_BlinkNoDelaySerial/01_BlinkNoDelaySerial.ino`
- Nutzt `millis()` statt `delay()`.
- Schaltet `LED_BUILTIN` alle 500 ms.
- Gibt jeden Zustandswechsel auf dem Serial Monitor aus.

Challenge:
- Reduziere das Blink-Intervall schrittweise und finde die kürzeste Zeit, bei der du das Blinken noch sicher wahrnehmen kannst.
- Notiere deinen Grenzwert (subjektiv) und vergleiche ihn mit anderen.

Advanced Challenge:
- Füge einen zweiten Timer hinzu, der unabhängig von der LED alle 1000 ms eine Statuszeile ausgibt.
- Ziel: Zwei parallele, nicht-blockierende Aufgaben im selben `loop()`.

## 2) Poti + Mini Servo (Signal an Pin 12)
Pfad: `02_PotiServoPin12/02_PotiServoPin12.ino`
- Liest Potentiometer an `A0`.
- Setzt Servowinkel 0..180 auf Basis des Poti-Werts.
- Serielle Ausgabe von Rohwert und Winkel.

Verdrahtung Poti:
- Außenpin 1 -> 5V
- Außenpin 2 -> GND
- Mittelpin -> A0

Hinweis: Beim Potentiometer **kein Pullup** verwenden.

Challenge:
- Begrenze die Servo-Auslenkung auf 0..90° statt 0..180°.
- Prüfe, ob eine `map()`-Funktion sinnvoll ist (ja: für saubere Skalierung von 0..1023 auf 0..90).
- Beobachte, ob der Servo an den Enden sauber stoppt.

Advanced Challenge:
- Baue eine kleine Deadzone ein (z. B. Winkel nur ändern, wenn Differenz >= 2°), damit der Servo weniger zittert.

## 3) LDR statt Poti
Pfad: `03_LDRServoPin12/03_LDRServoPin12.ino`
- Gleiche Servo-Logik wie Sketch 2, aber Sensor ist ein LDR.
- LDR als Spannungsteiler mit 10k Widerstand an `A0`.

Verdrahtung LDR:
- 5V -> LDR -> Messpunkt
- Messpunkt -> A0
- Messpunkt -> 10k -> GND

Wenn die Drehrichtung "falsch herum" ist, in `03_LDRServoPin12.ino` die `map(...)`-Richtung invertieren.

Challenge:
- Miss im Serial Monitor die LDR-Min/Max-Werte bei deiner realen Umgebung (hell/dunkel).
- Passe danach die Servo-Auslenkung an genau diesen Bereich an (statt fest 0..1023).

Wichtig:
- Beim LDR brauchst du einen Spannungsteiler.
- Ref Link: https://learn.sparkfun.com/tutorials/voltage-dividers/all

Advanced Challenge:
- Ergänze eine Auto-Kalibrierung für 5 Sekunden beim Start: min/max sammeln und danach dynamisch auf den Servowinkel mappen.
