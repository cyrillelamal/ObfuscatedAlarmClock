#include <reg52.h>

sfr DATA = 0xa0;  	// Display data byte
sbit RS = 0xb5;   // Command/Data checkbox
sbit RW = 0xb6;   // Read/Write checkbox
sbit E = 0xb7;   // Enable checkbox
sbit P3_2 = 0xb2;   // Button
sbit P3_3 = 0xb3;   // Speaker

unsigned char hour, min, sec;   // Current time
unsigned char alarmHour, alarmMinute;   // Alarm time
unsigned char S1, S2;   // Button pushed delay
unsigned char I;
unsigned char time;
unsigned char AS;
unsigned char alarmIsActivated;
unsigned int buttonIsPushedFor;

unsigned char SET[3] = {0x53, 0x45, 0x54};
unsigned char OFF[3] = {0x4f, 0x46, 0x46};
unsigned char OFFOFF[6] = {0x4f, 0x46, 0x46, 0x4f, 0x46, 0x46};
unsigned char ON[2] = {0x4f, 0x4e};
unsigned char free;

void timer2(void) interrupt 5 using 3 // Second timer interruption
{
	TF2 = 0;
	time++;
	buttonIsPushedFor++;
	RCAP2L = 0x53;
	RCAP2H = 0x34;
	TR2 = 1;
}

void delay(unsigned int DT) // DTx100 microseconds - 0 timer
{
	unsigned int I;
	TF0 = 0;
	TR0 = 0;
	for(I = 0; I < DT; I++)
	{
		if(time == 40)
		{
			time = 0;
			sec++;
			if(sec > 59)
			{
				min++;
				if(min > 59)
				{
					hour++;
					if(hour > 23)
						hour = 0;
				}
			}
		}
		TH0 = 0xff;
		TL0 = 0x2e;
		TR0 = 1;
		while(TF0 == 0);
		TF0 = 0;
	}
}

void sendToLCD(bit c0d1, unsigned char byteToSend)  //byteToSend: 0 - command, 1 - data
{
	RS = c0d1;
	RW = 0;
	E = 1;
	DATA = byteToSend;
	delay(1);
	E = 0;
	delay(1);
}

void timeToLCD(void)
{
	if(AS == 0)
	{
		unsigned char T[8];
		unsigned char in;
		T[0] = hour/10+0x30;
		T[1] = hour%10+0x30;
		T[2] = 0x3a;
		T[3] = min/10+0x30;
		T[4] = min%10+0x30;
		T[5] = 0x3a;
		T[6] = sec/10+0x30;
		T[7] = sec%10+0x30;
		sendToLCD(0, 0x80);
		for(in = 0; in < 8; in++)
		LCD_send(1, T[in]);
	}
	else
	{
		unsigned char T[8];
		unsigned char in;
		T[0] = alarmHour/10+0x30;
		T[1] = alarmHour%10+0x30;
		T[2] = 0x3a;
		T[3] = alarmMinute/10+0x30;
		T[4] = alarmMinute%10+0x30;
		T[5] = 0x3a;
		T[6] = sec/10+0x30;
		T[7] = sec%10+0x30;
		sendToLCD(0, 0x80);
		for(in = 0; in < 8; in++)
		LCD_send(1, T[in]);
	}
}

void prepareLCD(void)
{
	delay(200);
	sendToLCD(0, 0x30);
	sendToLCD(0, 0x30);
	sendToLCD(0, 0x37);
	sendToLCD(0, 0x0f);
	sendToLCD(0, 0x06);
	sendToLCD(0, 0x01);
	sendToLCD(0, 0x00);
	delay(50);
}

void addSecond(void)
{
	sec++;
	if(sec > 59)
	{
		sec = 0;
		min++;
		if(min > 59)
		{
			min = 0;
			hour++;
			if(hour > 23)
				hour = 0;
		}
	}
}

void SW1(void) interrupt 0 using 1  // Button interruption
{
	EX0 = 0;
	delay(50);
	if(P3_2 == 0)
	{
		free = 0;
		buttonIsPushedFor = 0;
		S1 = 0;
		while(free == 0)
		{
			if(time > 39)
			{
				time = 0;
				TR2 = 1;
				addSecond();
				timeToLCD();
			}
			if(buttonIsPushedFor > 99)
			{
				S1 = 2;
				free = 1;
			}
			else
			{
				if(P3_2 == 1)
				{
					free = 1;
					S1 = 1;
				}
			}
		}
	}
	EX0 = 1;
}

void setTime(void)
{
	AS = 0;
	sendToLCD(0, 0x01);
	delay(40);
	sendToLCD(0, 0x80);
	for(i = 0; i < 3; i++)
		sendToLCD(1, SET[i]);  // SET

	delay(30000);
	S1 = 0;
	sendToLCD(0, 0x01);
	delay(40);
	sendToLCD(0, 0x80);
	timeToLCD();
	while(S1 < 2)
	{
		if(time > 39)
		{
			time = 0;
			sec++;
			if(sec > 59)
				sec = 0;
			timeToLCD();
		}
		if(S1 == 1)
		{
			min++;
			if(min > 59)
				min = 0;
			timeToLCD();
			S1 = 0;
		}
	}
	S1 = 0;
	while(S1 < 2)
		{
		if(time > 39)
		{
			time = 0;
			sec++;
			if(sec > 59)
				sec = 0;
			timeToLCD();
		}
		if(S1 == 1)
		{
			hour++;
			if(hour > 23)
				hour = 0;
			timeToLCD();
			S1 = 0;
		}
	}
	S1 = 0;
	while(S1 < 2)
	{
		if(time > 39)
		{
			time = 0;
			sec++;
			if(sec > 59)
				sec = 0;
			timeToLCD();
		}
		if(S1 == 1)
		{
			sec = 0;
			timeToLCD();
			S1 = 0;
		}
	}
	S1 = 0;
}

void setAlarm(void)
{
	S1 = 0;
	sendToLCD(0, 0x01);
	delay(40);
	sendToLCD(0, 0x80);
	for(i = 0; i < 2; i++)
		sendToLCD(1, ON[i]);  // ON

	delay(30000);
	AS = 1;
	timeToLCD();
	while(S1 < 2)
	{
		if(time > 39)
		{
			time = 0;
			addSecond();
			timeToLCD();
		}
		if(S1 == 1)
		{
			alarmMinute++;
			if(alarmMinute > 59)
			alarmMinute = 0;
		timeToLCD();
		S1 = 0;
		}
	}
	S1 = 0;
	while(S1 < 2)
	{
		if(time > 39)
		{
			time = 0;
			addSecond();
			timeToLCD();
		}
		if(S1 == 1)
		{
			alarmHour++;
		if(alarmHour > 23)
			alarmHour = 0;
		timeToLCD();
		S1 = 0;
		}
	}
	S1 = 0;
	alarmIsActivated = 1;
	AS = 0;
}

void enableAlarm(void)
{
	sendToLCD(0, 0x01);
	delay(40);
	sendToLCD(0, 0x80);
	for(i = 0; i < 3; i++)
		sendToLCD(1, OFF[i]);

	delay(30000);
	sendToLCD(0, 0x01);
	delay(40);
	sendToLCD(0, 0x80);
	AS = 1;
	timeToLCD();
	S2 = 0;
	S1 = 0;
	while(S2 == 0)
	{
		if(time > 39)
		{
			time = 0;
			addSecond();
			timeToLCD();
		}
		if(S1 == 1)
			S2 = 1;
		if(S1 == 2)
			S2 = 2;
	}
	if(S2 == 2)
		setAlarm();
	S2 = 0;
	S1 = 0;
	AS = 0;
}

void checkAlarm(void)
	{
	if(alarmHour == hour)
		{
			if(alarmMinute == min)
				{
					TMOD =(TMOD & 0xcf);
					TMOD =(TMOD | 0x20);
					TF1 = 0;
					TR1 = 0;
					TH1 = 107;  // A note
					TR1 = 1;
					while(S1 == 0)
						{
						for(I = 0; I < 16; I++)
							{
							while(TF1 == 0);
							{
								if(time > 39)
								{
									time = 0;
									addSecond();
									timeToLCD();
								}
							}
							TF1 = 0;
						}
						P3_3 = !P3_3;
					}
					alarmIsActivated = 0;
					S1 = 0;
					TMOD =(TMOD & 0xcc);
					TMOD =(TMOD | 0x11);
				}
		}
}

void disableAlarm(void)
{
	sendToLCD(0, 0x01);
	delay(40);
	sendToLCD(0, 0x80);
	for(i = 0;  i < 2;  i++)
		sendToLCD(1, ON[i]);

	delay(30000);
	S2 = 0;
	S1 = 0;
	AS = 1;
	while(S2 == 0)
	{
		if(time > 39)
		{
			time = 0;
			addSecond();
			timeToLCD();
			checkAlarm();
		}
		if(S1 == 1)
			S2 = 1;
		if(S1 == 2)
			S2 = 2;
	}
	if(S2 == 2)
	{
		alarmIsActivated = 0;
		sendToLCD(0, 0x01);
		delay(40);
		sendToLCD(0, 0x80);
		for(i = 0; i < 6; i++)
			sendToLCD(1, OFFOFF[i]);

		delay(30000);
		sendToLCD(0, 0x01);
		delay(40);
		sendToLCD(0, 0x80);
	}
	AS = 0;
	S2 = 0;
	S1 = 0;
}


void main(void)
{
	alarmHour  =  0;
	alarmMinute  =  0;
	alarmIsActivated  =  0;

	S1  =  0;
	buttonIsPushedFor  =  0;

	// Set timers
	TMOD =(TMOD & 0xcc);
	TMOD =(TMOD | 0x11);

	TH1 = 0x00;
	TL1 = 0x00;

	TF1 = 0;
	TR1 = 0;
	TF0 = 0;
	TR0 = 0;

	T2CON =(T2CON & 0xfc);
	RCAP2L = 0x53;
	RCAP2H = 0x34;

	prepareLCD();

	P3_2 = 1;

	// Enable interruptions
	EA = 1;
	ET2 = 1;
	EX0 = 1;
	IT0 = 1;
	PT2 = 1;

	while(1)  // Main
	{
		EX0  =  1;
		while(time < 40)
		{
			TR2 = 1;
			if(S1 == 1)
			{
				if(alarmIsActivated == 1)
					disableAlarm();
				else
					enableAlarm();
			}
			if(S1 == 2)
				setTime();
		}
		time = 0;
		addSecond();
		timeToLCD();
		if(alarmIsActivated == 1)
			checkAlarm();
	}
}
