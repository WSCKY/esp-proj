# Example: GPIO

This test code shows how to configure gpio and how to use gpio interrupt.

## GPIO functions:

 * GPIO5: output
 * GPIO26: output
 * GPIO25:  input, pulled up, interrupt from rising edge and falling edge
 * GPIO35:  input, pulled up, interrupt from rising edge.

## Test:
 * Connect GPIO5 with GPIO25
 * Connect GPIO26 with GPIO35
 * Generate pulses on GPIO5/26, that triggers interrupt on GPIO25/35

## LyraT Board(V4.3) Configuration:
 * Connect I2S Connector(J21) Pin2 with Pin3
 * Connect I2S Connector(J21) Pin4 with Pin5
