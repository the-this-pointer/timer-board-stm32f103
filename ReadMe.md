## Timer
###### Under heavy development (in my spare time :))

#### Description
A cheap multi-purpose timer that supports Monthly / Weekly / Daily schedules for toggling 1 output (4 for the future). I will upload the circuit schematics and PCB ASAP, you can reverse engineer it from the `STM32CubeMX` file for now.

I used the <a href="https://github.com/4ilo/ssd1306-stm32HAL" target="_blank">ssd1306-stm32HAL</a> library and <a href="https://github.com/the-this-pointer/glcd-font-calculator" target="_blank">glcd-font-calculator</a> for creating custom char icons for my SSD1306 LCD. (see `Icon11x18` in `Core/Src/fonts.c`)
