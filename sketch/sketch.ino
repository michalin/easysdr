
#include <Arduino.h>
#include <driver/ledc.h>
//#define TESTSIG 1
#define USE_SI5351 1

#define VERSION "EasySDR Firmware V1.0"

// Default output frequency
#define FREQ_LIMIT_LOWER 	48000 //Baud rate of capturing device
#if USE_SI5351
#define MAX_ESP_FREQ 		5000000
#define FREQ_LIMIT_UPPER 	30000000
#else
#define MAX_ESP_FREQ 20000000
#define FREQ_LIMIT_UPPER MAX_ESP_FREQ
#endif

#define I_CHANNEL ledc_channel_t::LEDC_CHANNEL_0
#define Q_CHANNEL ledc_channel_t::LEDC_CHANNEL_1
#define T_CHANNEL ledc_channel_t::LEDC_CHANNEL_2
#define OUTPIN_I 4
#define OUTPIN_Q 18
#define OUTPIN_T 26
#define PWMRES 2
#define PWMRES_T 1
#define DUTY_T 1
#define OFFSET_T 100000 
#define DUTY (1UL << (PWMRES - 1)) // Duty cycle 50%

#define SERIAL_BAUD 115200

#if USE_SI5351
#include <Wire.h>
#include <si5351.h>
Si5351 si5351;
#endif

uint32_t rx_frequency;

void setup();
void loop();
void set_rx_freq(uint32_t freq);

void setup()
{
	Serial.begin(SERIAL_BAUD);
	Serial.setTimeout(10);

#if USE_SI5351
	if (si5351.init(SI5351_CRYSTAL_LOAD_8PF, 27000000, 0))
		printf("Si5351 found\n");
	else
		printf("Error: Si5351 not found\n");

	//  Set output frequencies
	si5351.output_enable(SI5351_CLK2, 1);

	// Read the status register and return the chip revision ID.
	si5351.update_status();
#endif
	ledcSetup(I_CHANNEL, FREQ_LIMIT_LOWER, PWMRES);
	ledcWrite(Q_CHANNEL, DUTY);
	ledcWrite(I_CHANNEL, DUTY);
#if TESTSIG
	// Test signal generator on pin 26
	ledcSetup(T_CHANNEL, FREQ_LIMIT_LOWER + OFFSET_T, PWMRES_T);
	ledcAttachPin(OUTPIN_T, T_CHANNEL);
	ledcWrite(T_CHANNEL, DUTY_T);
#endif
}

void serialEvent()
{
	String command = Serial.readString();
	// printf("Received command: %s\n", command.c_str());
	if (command.startsWith("FREQ"))
	{
		String arg = command.substring(command.indexOf(',') + 1);
		int frq = arg.toInt();
		if (frq > FREQ_LIMIT_UPPER)
			frq = FREQ_LIMIT_UPPER;
		if (frq < FREQ_LIMIT_LOWER)
			frq = FREQ_LIMIT_LOWER;

		printf("FREQ,%d\n", frq);
		set_rx_freq(frq);
	}
	else if (command.startsWith("VER"))
	{
		printf("VER,%s\n",VERSION);
	}
}

void set_rx_freq(uint32_t freq)
{
#if USE_SI5351 // Si 5351 High-Z
	if (freq <= MAX_ESP_FREQ)
	{
		si5351.set_clock_disable(SI5351_CLK0, SI5351_CLK_DISABLE_HI_Z);
		si5351.set_clock_disable(SI5351_CLK1, SI5351_CLK_DISABLE_HI_Z);
		si5351.output_enable(SI5351_CLK0, 0);
		si5351.output_enable(SI5351_CLK1, 0);
	}
#else
	if (freq > MAX_ESP_FREQ)
		freq = MAX_ESP_FREQ;
#endif
	if (freq <= MAX_ESP_FREQ)
	{
		ledcAttachPin(OUTPIN_I, Q_CHANNEL);
		ledcAttachPin(OUTPIN_Q, I_CHANNEL);
		// Phase shift 90° (Duty cycle/2) to Channel 1
		ledc_set_duty_with_hpoint(LEDC_HIGH_SPEED_MODE, ledc_channel_t::LEDC_CHANNEL_1, DUTY, DUTY / 2);
		ledcChangeFrequency(I_CHANNEL, freq, DUTY);
		rx_frequency = freq; // (Hz)
		return;
	}

	rx_frequency = freq; // (Hz)
#if USE_SI5351
	pinMode(OUTPIN_I, INPUT);
	pinMode(OUTPIN_Q, INPUT);
	si5351.output_enable(SI5351_CLK0, 1);
	si5351.output_enable(SI5351_CLK1, 1);
	// This is for models that use the Si5351 to produce the I/Q on CLK0 and CLK1.
	uint32_t pll_freq;
	uint8_t mult;
	// mult must be less than 128 (7 bits) according to documentation
	// Determine mult so that pll frequency is >600 Mhz
	while ((pll_freq < 600000000) && (mult < 128))
	{
		pll_freq = freq * ++mult;
	}
	/*printf("nmult: %d\n", mult);
	printf("pll_freq: %d\n", pll_freq);*/

	// si5351.set_freq_manual(110000000 * 100ULL, pll_freq * 100ULL, SI5351_CLK2);
	si5351.set_freq_manual(freq * 100ULL, pll_freq * 100ULL, SI5351_CLK1);
	si5351.set_freq_manual(freq * 100ULL, pll_freq * 100ULL, SI5351_CLK0);
	// Now we can set CLK1 to have a 90 deg phase shift by entering
	// mult in the CLK1 phase register, since the ratio of the PLL to
	// the clock frequency is mult.
	si5351.set_phase(SI5351_CLK0, 0);
	si5351.set_phase(SI5351_CLK1, mult);
	// We need to reset the PLL before they will be in phase alignment
	si5351.pll_reset(SI5351_PLLA);
#endif
}

void loop()
{
}
